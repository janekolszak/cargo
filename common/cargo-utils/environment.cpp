/*
 *  Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Contact: Michal Witanowski <m.witanowski@samsung.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License
 */

/**
 * @file
 * @author  Michal Witanowski (m.witanowski@samsung.com)
 * @brief   Implementaion of environment setup routines that require root privileges
 */

#include "config.hpp"

#include "cargo-utils/environment.hpp"
#include "cargo-utils/execute.hpp"
#include "cargo-utils/exception.hpp"
#include "cargo-utils/make-clean.hpp"
#include "cargo-utils/fd-utils.hpp"
#include "cargo-logger/logger.hpp"

#include <grp.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>
#include <iomanip>
#include <cassert>
#include <features.h>
#include <linux/capability.h>
#include <sys/prctl.h>
#include <sys/syscall.h>


#if !__GLIBC_PREREQ(2, 14)

#ifdef __NR_setns
static inline int setns(int fd, int nstype)
{
    // setns system call are available since v2.6.39-6479-g7b21fdd
    return syscall(__NR_setns, fd, nstype);
}
#else
#error "setns syscall isn't available"
#endif

#endif

#ifdef __NR_capset
static inline int capset(cap_user_header_t header, const cap_user_data_t data)
{
    return syscall(__NR_capset, header, data);
}
#else
#error "capset syscall isn't available"
#endif

using namespace utils;

namespace {

#define CAP_SET_INHERITABLE (1 << 0)
#define CAP_SET_PERMITTED (1 << 1)
#define CAP_SET_EFFECTIVE (1 << 2)

// number of __user_cap_data_struct elements needed
#define CAP_DATA_ELEMENT_COUNT 2

typedef unsigned int CapSet;

const std::map<int, std::string> NAMESPACES = {
    {CLONE_NEWIPC, "ipc"},
    {CLONE_NEWNET, "net"},
    {CLONE_NEWNS, "mnt"},
    {CLONE_NEWPID, "pid"},
    {CLONE_NEWUSER, "user"},
    {CLONE_NEWUTS, "uts"}};

inline bool isValidCap(unsigned int cap)
{
    return cap <= CAP_LAST_CAP;
}

// hasCap assumes that "set" variable will refer to only one set of capabilities
inline bool hasCap(unsigned int cap, const cap_user_data_t data, CapSet set)
{
    // calculate which half of data we need to update
    int dataInd = 0;
    if (cap > 31) {
        dataInd = cap >> 5;
        cap %= 32;
    }

    switch (set) {
    case CAP_SET_INHERITABLE:
        return CAP_TO_MASK(cap) & data[dataInd].inheritable ? true : false;
    case CAP_SET_PERMITTED:
        return CAP_TO_MASK(cap) & data[dataInd].permitted ? true : false;
    case CAP_SET_EFFECTIVE:
        return CAP_TO_MASK(cap) & data[dataInd].effective ? true : false;
    default:
        return false;
    };
}

// these inlines work in-place and update provided "data" array
// in these inlines, "set" can refer to mulitple sets of capabilities
inline void addCap(unsigned int cap, cap_user_data_t data, CapSet set)
{
    // calculate which half of data we need to update
    int dataInd = 0;
    if (cap > 31) {
        dataInd = cap >> 5;
        cap %= 32;
    }

    if ((set & CAP_SET_INHERITABLE) == CAP_SET_INHERITABLE) {
        data[dataInd].inheritable |= CAP_TO_MASK(cap);
    }
    if ((set & CAP_SET_PERMITTED) == CAP_SET_PERMITTED) {
        data[dataInd].permitted |= CAP_TO_MASK(cap);
    }
    if ((set & CAP_SET_EFFECTIVE) == CAP_SET_EFFECTIVE) {
        data[dataInd].effective |= CAP_TO_MASK(cap);
    }
}

inline void removeCap(unsigned int cap, cap_user_data_t data, CapSet set)
{
    // calculate which half of data we need to update
    int dataInd = 0;
    if (cap > 31) {
        dataInd = cap >> 5;
        cap %= 32;
    }

    if ((set & CAP_SET_INHERITABLE) == CAP_SET_INHERITABLE) {
        data[dataInd].inheritable &= ~(CAP_TO_MASK(cap));
    }
    if ((set & CAP_SET_PERMITTED) == CAP_SET_PERMITTED) {
        data[dataInd].permitted &= ~(CAP_TO_MASK(cap));
    }
    if ((set & CAP_SET_EFFECTIVE) == CAP_SET_EFFECTIVE) {
        data[dataInd].effective &= ~(CAP_TO_MASK(cap));
    }
}

} // namespace

namespace utils {


bool setSuppGroups(const std::vector<std::string>& groups)
{
    std::vector<gid_t> gids;

    for (const std::string& group : groups) {
        // get GID from name
        struct group* grp = ::getgrnam(group.c_str());
        if (grp == NULL) {
            LOGE("getgrnam failed to find group '" << group << "'");
            return false;
        }

        LOGD("'" << group << "' group ID: " << grp->gr_gid);
        gids.push_back(grp->gr_gid);
    }

    if (::setgroups(gids.size(), gids.data()) != 0) {
        LOGE("setgroups() failed: " << getSystemErrorMessage());
        return false;
    }

    return true;
}

bool dropRoot(uid_t uid, gid_t gid, const std::vector<unsigned int>& caps)
{
    ::__user_cap_header_struct header;
    ::__user_cap_data_struct data[CAP_DATA_ELEMENT_COUNT];

    // initial setup - equivalent to capng_clear
    header.version = _LINUX_CAPABILITY_VERSION_3;
    header.pid = ::getpid();
    memset(data, 0, CAP_DATA_ELEMENT_COUNT*sizeof(__user_cap_data_struct));

    // update cap sets - equivalent to capng_update
    for (const auto cap : caps) {
        if (!isValidCap(cap)) {
            LOGE("Capability " << cap << " is invalid.");
            return false;
        }

        addCap(cap, data, CAP_SET_INHERITABLE | CAP_SET_PERMITTED | CAP_SET_EFFECTIVE);
    }

    // perform some checks and cap updates
    bool updatedSetUid, updatedSetGid;
    // check if we are capable of switching our UID
    if (hasCap(CAP_SETUID, data, CAP_SET_EFFECTIVE)) {
        // we want to keep CAP_SETUID after change
        updatedSetUid = false;
    } else {
        // we don't have CAP_SETUID and switch is needed - add SETUID to effective and permitted set
        updatedSetUid = true;
        addCap(CAP_SETUID, data, CAP_SET_PERMITTED | CAP_SET_EFFECTIVE);
    }

    // do the same routine for CAP_SETGID
    if (hasCap(CAP_SETGID, data, CAP_SET_EFFECTIVE)) {
        updatedSetGid = false;
    } else {
        updatedSetGid = true;
        addCap(CAP_SETGID, data, CAP_SET_PERMITTED | CAP_SET_EFFECTIVE);
    }

    // we need CAP_SETPCAP as well to clear bounding caps
    if (!hasCap(CAP_SETPCAP, data, CAP_SET_EFFECTIVE)) {
        addCap(CAP_SETPCAP, data, CAP_SET_PERMITTED | CAP_SET_EFFECTIVE);
    }

    // now we can work - first, use prctl to tell system we want to keep our caps when changing UID
    if (::prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0)) {
        LOGE("prctl failed while trying to enable keepcaps: " << strerror(errno));
        return false;
    }

    LOGD("Setting temporary caps to process -" << std::hex << std::setfill('0')
         << " inh:" << std::setw(8) << data[1].inheritable << std::setw(8) << data[0].inheritable
         << " prm:" << std::setw(8) << data[1].permitted << std::setw(8) << data[0].permitted
         << " eff:" << std::setw(8) << data[1].effective << std::setw(8) << data[0].effective);

    // set our modified caps before UID/GID change
    if (::capset(&header, data)) {
        LOGE("capset failed: " << strerror(errno));
        return false;
    }

    // CAP_SETPCAP is available, drop bounding caps
    for (int i = 0; i <= CAP_LAST_CAP; ++i) {
        if (::prctl(PR_CAPBSET_DROP, i, 0, 0, 0)) {
            LOGE("prctl failed while dropping bounding caps: " << strerror(errno));
            return false;
        }
    }

    // set up GID and UID
    if (::setresgid(gid, gid, gid)) {
        LOGE("setresgid failed: " << strerror(errno));
        return false;
    }
    if (::setresuid(uid, uid, uid)) {
        LOGE("setresuid failed: " << strerror(errno));
        return false;
    }

    // we are after switch now - disable PR_SET_KEEPCAPS
    if (::prctl(PR_SET_KEEPCAPS, 0, 0, 0, 0)) {
        LOGE("prctl failed while trying to disable keepcaps: " << strerror(errno));
        return false;
    }

    // disable rendundant caps
    if (updatedSetUid) {
        removeCap(CAP_SETUID, data, CAP_SET_PERMITTED | CAP_SET_EFFECTIVE);
    }
    if (updatedSetGid) {
        removeCap(CAP_SETGID, data, CAP_SET_PERMITTED | CAP_SET_EFFECTIVE);
    }
    removeCap(CAP_SETPCAP, data, CAP_SET_PERMITTED | CAP_SET_EFFECTIVE);

    LOGD("Setting final caps to process -" << std::hex << std::setfill('0')
         << " inh:" << std::setw(8) << data[1].inheritable << std::setw(8) << data[0].inheritable
         << " prm:" << std::setw(8) << data[1].permitted << std::setw(8) << data[0].permitted
         << " eff:" << std::setw(8) << data[1].effective << std::setw(8) << data[0].effective);

    // finally, apply correct caps
    if (::capset(&header, data)) {
        LOGE("capset failed: " << strerror(errno));
        return false;
    }

    return true;
}

bool launchAsRoot(const std::vector<std::string>& argv)
{
    return executeAndWait(0, argv);
}

bool joinToNs(int nsPid, int ns)
{
    auto ins = NAMESPACES.find(ns);
    if (ins == NAMESPACES.end()) {
        LOGE("Namespace isn't supported: " << ns);
        return false;
    }
    std::string nsPath = "/proc/" + std::to_string(nsPid) + "/ns/" + ins->second;
    int nsFd = ::open(nsPath.c_str(), O_RDONLY);
    if (nsFd == -1) {
        LOGE("Can't open namesace: " + getSystemErrorMessage());
        return false;
    }
    int ret = setns(nsFd, ins->first);
    if (ret != 0) {
        LOGE("Can't set namesace: " + getSystemErrorMessage());
        close(nsFd);
        return false;
    }
    close(nsFd);
    return true;
}

// FIXME remove (not used in lxcpp)
int passNamespacedFd(int nsPid, int ns, const std::function<int()>& fdFactory)
{
    int fds[2];
    int ret = socketpair(PF_LOCAL, SOCK_RAW, 0, fds);
    if (ret == -1) {
        LOGE("Can't create socket pair: " << getSystemErrorMessage());
        return -1;
    }
    bool success = executeAndWait([&, fds, nsPid, ns]() {
        close(fds[0]);

        int fd = -1;
        if (joinToNs(nsPid, ns)) {
            fd = fdFactory();
        }
        if (fd == -1) {
            close(fds[1]);
            _exit(EXIT_FAILURE);
        }
        LOGT("FD pass, send: " << fd);
        fdSend(fds[1], fd);
        close(fds[1]);
        close(fd);
    });

    close(fds[1]);
    int fd = -1;
    if (success) {
        fd = fdRecv(fds[0]);
    }
    close(fds[0]);
    LOGT("FD pass, rcv: " << fd);
    return fd;
}

} // namespace utils
