#ifndef RESTORE_TASK_RUNNER_HPP
#define RESTORE_TASK_RUNNER_HPP

#include "./task.hpp"

#include <atomic>
#include <mutex>
#include <thread>

namespace restore
{
	template<class Task>
	class task_runner
	{
	public:
		explicit task_runner(Task&& task):
			m_should_stop{false},
			m_task{std::move(task)}
		{}

		void suspend()
		{
			if(m_runner.joinable())
			{
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

		void set_parameters(json::object_ref params)
		{
			std::lock_guard lock{m_task_mtx};
			m_task.set_parameters(params);
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

		void reset()
		{
			std::lock_guard lock{m_task_mtx};
			m_task.reset();
		}

		~task_wrapper()
		{ suspend(); }

	private:
		std::atomic<bool> m_should_stop;
		Task m_task;
		mutable std::mutex m_task_mtx;
		std::thread m_runner;

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