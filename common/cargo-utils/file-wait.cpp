/*
 *  Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Contact: Piotr Bartosiewicz <p.bartosiewi@partner.samsung.com>
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
 * @author  Piotr Bartosiewicz (p.bartosiewi@partner.samsung.com)
 * @brief   Wait for file utility function
 */

#include "config.hpp"

#include "cargo-utils/file-wait.hpp"
#include "cargo-utils/exception.hpp"
#include "cargo-utils/inotify.hpp"
#include "cargo-utils/paths.hpp"
#include "cargo-utils/fs.hpp"

#include "cargo-ipc/epoll/event-poll.hpp"

#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>
#include <chrono>
#include <thread>
#include "cargo-logger/logger.hpp"

namespace utils {

void waitForFile(const std::string& file, const unsigned int timeoutMs)
{
    std::string dir = dirName(file);
    assertExists(dir, S_IFDIR);

    cargo::ipc::epoll::EventPoll poll;
    Inotify inotify(poll);

    std::string filename = file.substr(dir.size() + 1);
    bool isWaiting = true;
    inotify.setHandler(dir, IN_CREATE | IN_ISDIR, [&isWaiting, &filename](const std::string& name, uint32_t) {
        if (name == filename) {
            isWaiting = false;

            // There's a race between inotify event and filesystem update.
            ::sync();
        }
    });

    auto deadline = std::chrono::steady_clock::now() +
                    std::chrono::milliseconds(timeoutMs);

    while (isWaiting && std::chrono::steady_clock::now() < deadline ) {
        auto duration = deadline - std::chrono::steady_clock::now();
        poll.dispatchIteration(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    }

    if (isWaiting) {
        THROW_EXCEPTION(UtilsException, "Timeout waiting for file '" << file << "'");
    }
}

} // namespace utils
