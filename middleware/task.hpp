#ifndef RESTORE_TASK_HPP
#define RESTORE_TASK_HPP

#include <jopp/types.hpp>
#include <atomic>
#include <mutex>

namespace restore
{
	class task_wrapper
	{
	public:
		task_wrapper():m_should_stop{false}
		{}

		void stop()
		{ m_should_stop = true;}

		double get_progress() const
		{
			std::lock_guard lock{m_task_mtx};
			return m_task->progress();
		}

		void set_state(state_source src)
		{
			std::lock_guard lock{m_task_mtx};
			m_tasl->set_state(src);
		}

		void dump_state(state_sink sink)
		{
			std::lock_guard lock{m_task_mtx};
			m_task->dump_state(sink);
		}

		~task_wrapper()
		{ stop(); }

	private:
		std::atomic<bool> m_should_stop;
		std::unique_ptr<task> m_task;
		mutable std::mutex m_task_mtx;

		void do_run()
		{
			while(!m_should_stop.load())
			{
				std::lock_guard lock{m_task_mtx};
				m_task->run();
			}
		}
	};
}

#endif