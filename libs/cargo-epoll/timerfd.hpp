// /*
// *  Copyright (c) 2016 Jan Olszak, All Rights Reserved
// *
// *  Contact: Jan Olszak <janekolszak@gmail.com>
// *
// *  Licensed under the Apache License, Version 2.0 (the "License");
// *  you may not use this file except in compliance with the License.
// *  You may obtain a copy of the License at
// *
// *      http://www.apache.org/licenses/LICENSE-2.0
// *
// *  Unless required by applicable law or agreed to in writing, software
// *  distributed under the License is distributed on an "AS IS" BASIS,
// *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// *  See the License for the specific language governing permissions and
// *  limitations under the License
// */

// /**
//  * @file
//  * @author  Jan Olszak (janekolszak@gmail.com)
//  * @brief   timerfd wrapper
//  */

// #ifndef CARGO_EPOLL_TIMERFD_HPP
// #define CARGO_EPOLL_TIMERFD_HPP

// #include "cargo-epoll/event-poll.hpp"

// #include <sys/timerfd.h>

// #include <functional>
// #include <mutex>
// #include <memory>

// namespace cargo {

// class TimerFD {
// public:
// 	typedef std::function<void(void)> Callback;

// 	TimerFD(std::chrono::time_point& timePoint, cargo::EventPoll& eventPoll);
// 	virtual ~TimerFD();

// 	TimerFD(const TimerFD&) = delete;
// 	TimerFD& operator=(const TimerFD&) = delete;

// 	/**
// 	 * Add a callback for a specified deadline.
// 	 * Blocks the async signal handler if it's not already blocked.
// 	 *
// 	 * @param sigNum number of the signal
// 	 * @param callback handler callback
// 	 */
// 	void setHandler(const Callback&& callback);

// 	/**
// 	 * @return timer file descriptor
// 	 */
// 	int getFD() const;

// private:
// 	typedef std::unique_lock<std::mutex> Lock;

// 	int mFD;
// 	std::mutex mMutex;
// 	cargo::EventPoll& mEventPoll;
// 	Callback mCallbacks;

// 	void handleInternal();
// };

// } // namespace utils

// #endif // CARGO_EPOLL_TIMERFD_HPP
