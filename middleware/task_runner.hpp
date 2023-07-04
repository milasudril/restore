#ifndef RESTORE_TASK_RUNNER_HPP
#define RESTORE_TASK_RUNNER_HPP

#include "./task.hpp"

#include <atomic>
#include <mutex>
#include <thread>

namespace restore
{
	template<task Task>
	class task_runner
	{
	public:
		enum class running_state{suspended, running, completed};

		explicit task_runner(Task&& task):
			m_should_stop{false},
			m_running_status{running_state::suspended},
			m_task{std::move(task)}
		{}

		void suspend()
		{
			if(m_runner.joinable())
			{
				m_running_status = running_state::suspended;
				m_should_stop = true;
				m_runner.join();
			}
		}

		void resume()
		{
			suspend();
			m_runner = std::thread{[this](){ do_run(); }};
		}

		double get_progress() const
		{
			std::lock_guard lock{m_task_mtx};
			return m_task.get_progress();
		}

		void dump_state(int output_fd) const
		{
			std::lock_guard lock{m_task_mtx};
			m_task.dump_state(output_fd);
		}

		void suspend_and_dump_state(int output_fd)
		{
			suspend();
			dump_state(output_fd);
		}

		void set_state(int output_fd)
		{
			std::lock_guard lock{m_task_mtx};
			m_task.set_state(output_fd);
		}

		void set_parameters(json::object_ref obj)
		{
			std::lock_guard lock{m_task_mtx};
			m_task.set_parameters(obj);
		}

		void reset()
		{
			std::lock_guard lock{m_task_mtx};
			m_task.reset();
			m_running_status = m_runner.joinable()? running_state::running : running_state::suspended;
		}

		running_state running_status() const
		{ return m_running_status; }

		~task_runner()
		{ suspend(); }

	private:
		std::atomic<bool> m_should_stop;
		std::atomic<running_state> m_running_status;
		Task m_task;
		mutable std::mutex m_task_mtx;
		std::thread m_runner;

		void do_run()
		{
			m_running_status = running_state::running;
			while(!m_should_stop.load())
			{
				std::lock_guard lock{m_task_mtx};
				if(m_task->step() == task_step_result::task_is_completed)
				{
					m_running_status = running_state::completed;
					return;
				}
			}
		}
	};
}

#endif