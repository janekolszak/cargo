/*
*  Copyright (c) 2016 Jan Olszak, All Rights Reserved
*
*  Contact: Jan Olszak <janekolszak@gmail.com>
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

#ifndef CARGO_EPOLL_TIMERFD_HPP
#define CARGO_EPOLL_TIMERFD_HPP

#include "cargo-epoll/event-poll.hpp"
#include "cargo-utils/exception.hpp"

#include <sys/timerfd.h>

#include <functional>
#include <mutex>
#include <memory>

using namespace utils;

namespace cargo {
namespace epoll {

// TODO: Implement time_point constructor
// TODO: Implement getKind()
// TODO: Implement a time getter and setter form

class TimerFD {
public:
	typedef std::function<void(void)> Callback;

	// template<typename Clock, typename Duration>
	// TimerFD(const std::chrono::time_point<Clock, Duration>& timePoint, cargo::EventPoll& eventPoll);

	template<typename Rep, typename Period>
	TimerFD(const std::chrono::duration<Rep, Period>& duration, cargo::EventPoll& eventPoll);

	virtual ~TimerFD();

	TimerFD(const TimerFD&) = delete;
	TimerFD& operator=(const TimerFD&) = delete;

	/**
	 * Add a callback for a specified deadline.
	 *
	 * @param callback handler callback
	 */
	void setHandler(const Callback&& callback);

	/**
	 * @return timer file descriptor
	 */
	int getFD() const;

private:

	typedef std::unique_lock<std::mutex> Lock;

	int mFD;
	std::mutex mMutex;
	cargo::EventPoll& mEventPoll;
	Callback mCallback;

	void handleInternal();
};


// template<typename Clock, typename Duration>
// TimerFD::TimerFD(const std::chrono::time_point<Clock, Duration>& timePoint, cargo::EventPoll& eventPoll)
// 	: mEventPoll(eventPoll)
// {
// 	// Create the timerfd
// 	mFD = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
// 	if (mFD == -1) {
// 		THROW_EXCEPTION(utils::UtilsException, "Error in timerfd_create()", errno);
// 	}


// }

template<typename Rep, typename Period>
TimerFD::TimerFD(const std::chrono::duration<Rep, Period>& duration, cargo::EventPoll& eventPoll)
	: mEventPoll(eventPoll)
{
	// Create the timerfd
	mFD  = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (mFD == -1) {
		THROW_EXCEPTION(UtilsException, "Error in timerfd_create()", errno);
	}

	auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	auto ns  = std::chrono::duration_cast<std::chrono::nanoseconds>(duration % std::chrono::seconds(1));

	// Setup periodic wakeup
	struct itimerspec itval;
	itval.it_interval.tv_sec  = static_cast<time_t>(sec);
	itval.it_interval.tv_nsec = static_cast<long>(ns.count());
	itval.it_value.tv_sec  = itval.it_interval.tv_sec;
	itval.it_value.tv_nsec = itval.it_interval.tv_nsec;

	if (-1 == ::timerfd_settime(mFD, 0, &itval, NULL)) {
		THROW_EXCEPTION(UtilsException, "Error in timerfd_settime()", errno);
	}

	mEventPoll.addFD(mFD, EPOLLIN, std::bind(&TimerFD::handleInternal, this));
}


} // namespace epoll
} // namespace cargo

#endif // CARGO_EPOLL_TIMERFD_HPP
