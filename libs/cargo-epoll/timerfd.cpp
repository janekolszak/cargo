/*
*  Copyright (c) 2016 Jan Olszak, All Rights Reserved
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
 * @author  Jan Olszak (janekolszak@gmail.com)
 * @brief   timerfd wrapper
 */

#include "config.hpp"

#include "cargo-utils/fd-utils.hpp"
#include "cargo-logger/logger.hpp"
#include "timerfd.hpp"
#include <functional>

namespace cargo {
namespace epoll {

TimerFD::~TimerFD()
{
	mEventPoll.removeFD(mFD);
	utils::close(mFD);
}

int TimerFD::getFD() const
{
	return mFD;
}

void TimerFD::setHandler(const Callback&& callback)
{
	Lock lock(mMutex);
	mCallback = callback;
}

void TimerFD::handleInternal()
{
	unsigned long long wakeupsMissed;
	utils::read(mFD,  &wakeupsMissed, sizeof(wakeupsMissed));
	if (wakeupsMissed == 0) {
		LOGE("Missed wakeup counter shouldn't be 0");
		return;
	}

	{
		Lock lock(mMutex);
		for (; wakeupsMissed != 0; --wakeupsMissed)	{
			mCallback();
		}
	}
}

} // namespace epoll
} // namespace cargo
