#ifndef RESTORE_TASK_RUNNER_HPP
#define RESTORE_TASK_RUNNER_HPP

#include "./task.hpp"

#include <atomic>
#include <mutex>
#include <thread>

namespace restore
{
	enum class task_running_status{suspended, running, completed};

	inline constexpr char const* to_string(task_running_status val)
	{
		switch(val)
		{
			case task_running_status::suspended:
				return "suspended";
			case task_running_status::running:
				return "running";
			case task_running_status::completed:
				return "completed";
			default:
				__builtin_unreachable();
		}
	}

	inline constexpr task_running_status to_task_running_status(std::string_view value)
	{
		if(value == "suspended")
		{ return task_running_status::suspended; }
		if(value == "running")
		{ return task_running_status::running; }
		if(value == "completed")
		{ return task_running_status::completed; }

		throw std::runtime_error{std::string{"Invalid task running state" } + std::string{value}};
	}

	template<task Task>
	class task_runner
	{
	public:
		using running_state = task_running_status;

		explicit task_runner(Task const& task):
			m_should_stop{false},
			m_running_status{running_state::suspended},
			m_task{task}
		{}

		explicit task_runner(Task&& task):
			m_should_stop{false},
			m_running_status{running_state::suspended},
			m_task{std::move(task)}
		{}

		void suspend()
		{
			if(m_runner.joinable())
			{
				m_should_stop = true;
				m_runner.join();
			}

			auto const current_status = m_running_status.load();
			m_running_status = current_status == running_state::running?
				running_state::suspended : current_status;
		}

		void resume()
		{
			if(m_running_status != running_state::completed)
			{
				suspend();
				m_should_stop = false;
				m_runner = std::thread{[this](){ do_run(); }};
				m_running_status = running_state::running;
			}
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

		void set_running_status(running_state new_value)
		{
			switch(new_value)
			{
				case running_state::suspended:
					suspend();
					return;

				case running_state::running:
					resume();
					return;

				case running_state::completed:
					throw std::runtime_error{"It is not possible to set running status to completed"};
			}
		}

		~task_runner()
		{ suspend(); }

		Task task() const
		{
			std::lock_guard lock{m_task_mtx};
			return m_task;
		}

	private:
		std::atomic<bool> m_should_stop;
		std::atomic<running_state> m_running_status;
		Task m_task;
		mutable std::mutex m_task_mtx;
		std::thread m_runner;

		void do_run()
		{
			while(!m_should_stop.load())
			{
				std::lock_guard lock{m_task_mtx};
				if(m_task.step() == task_step_result::task_is_completed)
				{
					m_running_status = running_state::completed;
					return;
				}
			}
		}
	};
}

#endif