/*
 *  Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Contact: Pawel Broda <p.broda@partner.samsung.com>
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
 * @author  Pawel Broda (p.broda@partner.samsung.com)
 * @brief   Logging backend
 */

#ifndef LOGGER_BACKEND_HPP
#define LOGGER_BACKEND_HPP

#include "cargo-logger/level.hpp"

#include <string>

namespace cargo {
namespace logger {

/**
 * @brief Abstract class for logger backends
 * @ingroup libcargo-logger
 */
class LogBackend {
public:
    virtual void log(LogLevel logLevel,
                     const std::string& file,
                     const unsigned int& line,
                     const std::string& func,
                     const std::string& message) = 0;
    virtual void relog(LogLevel /*logLevel*/,
                     const std::string& /*file*/,
                     const unsigned int& /*line*/,
                     const std::string& /*func*/,
                     const std::istream& /*stream*/) {}

    virtual ~LogBackend() {}
};

} // namespace logger
} // namespace cargo

#endif // LOGGER_BACKEND_HPP
