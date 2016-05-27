/*
 *  Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Contact: Pawel Kubik (p.kubik@samsung.com)
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
 * @author  Pawel Kubik (p.kubik@samsung.com)
 * @brief   Unit tests of fd utils
 */

#include "config.hpp"

#include "ut.hpp"

#include "cargo-utils/fd-utils.hpp"

#include "cargo-logger/logger.hpp"


using namespace utils;


BOOST_AUTO_TEST_SUITE(FDUtilsSuite)

BOOST_AUTO_TEST_CASE(GetSetMaxFDNumber)
{
    unsigned oldLimit = utils::getMaxFDNumber();
    unsigned newLimit = 50;

    utils::setMaxFDNumber(newLimit);
    BOOST_CHECK_EQUAL(newLimit, utils::getMaxFDNumber());

    utils::setMaxFDNumber(oldLimit);
    BOOST_CHECK_EQUAL(oldLimit, utils::getMaxFDNumber());
}

BOOST_AUTO_TEST_SUITE_END()
