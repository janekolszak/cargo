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
 * @brief   Create directory in constructor, delete it in destructor
 */

#include "config.hpp"

#include "cargo-utils/scoped-dir.hpp"
#include "cargo-utils/fs.hpp"
#include "cargo-utils/exception.hpp"
#include "cargo-logger/logger.hpp"

namespace utils {

ScopedDir::ScopedDir()
{
}

ScopedDir::ScopedDir(const std::string& path)
{
    create(path);
}

ScopedDir::~ScopedDir()
{
    remove();
}

void ScopedDir::create(const std::string& path)
{
    remove();
    if (!path.empty()) {
        mPath = path;
        remove();
        utils::createDirs(path);
    }
}

void ScopedDir::remove()
{
    if (mPath.empty()) {
        return ;
    }
    try {
        utils::removeDir(mPath);
    } catch (const UtilsException&) {
        LOGE("ScopedDir: can't remove " + mPath);
    }
}

} // namespace utils
