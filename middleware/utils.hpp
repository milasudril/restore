//@	{"dependencies_extra":[{"ref":"./utils.o", "rel":"implementation"}]}

#ifndef RESTORE_UTILS_HPP
#define RESTORE_UTILS_HPP

#include <algorithm>
#include <chrono>
#include <string>
#include <mutex>
#include <functional>
#include <optional>
#include <condition_variable>

namespace restore
{
	template<class ForwardIt, class T, class Compare>
	ForwardIt binary_find(ForwardIt first, ForwardIt last, const T& value, Compare cmp)
	{
		ForwardIt i = std::lower_bound(first, last, value, cmp);
		if (i != last && !cmp(value, *i))
		{ return i; }
		else
		{ return last; }
	}

	std::string to_string(std::chrono::system_clock::time_point t);

	template<class Callable, class ... Args>
	decltype(auto) call(std::mutex& m, Callable&& f, Args&& ... args)
	{
		std::lock_guard lock{m};
		return std::invoke(std::forward<Callable>(f), std::forward<Args>(args)...);
	}

	class sync_message_bus
	{
	public:
		void operator()()
		{
			std::lock_guard lock{m_mtx};
			if(m_closure != nullptr) [[unlikely]]
			{
				m_callback(m_closure, m_result);
				m_closure = nullptr;
				m_cv.notify_one();
			}
		}

		template<class Callable>
		auto process(Callable&& f, bool invoke_now)
		{
			if(invoke_now)
			{ return std::forward<Callable>(f)(); }

			using type = std::invoke_result_t<Callable>;
			std::optional<type> retval;
			{
				std::unique_lock lock{m_mtx};
				m_closure = &f;
				m_callback = [](void* closure, void* result) {
					auto& f = *static_cast<Callable*>(closure);
					auto& retval = *static_cast<std::optional<type>*>(result);
					retval = f();
				};
				m_result = &retval;
				m_cv.wait(lock, [&retval](){
					return retval.has_value();
				});
			}

			return std::move(*retval);
		}

	private:
		std::mutex m_mtx;
		void* m_closure{nullptr};
		void (*m_callback)(void* value, void* result){nullptr};
		void* m_result{nullptr};
		std::condition_variable m_cv;
	};
}

#endif