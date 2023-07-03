#ifndef RESTORE_TASK_HPP
#define RESTORE_TASK_HPP

#include "./jopp_wrapper.hpp"
#include <memory>

namespace restore
{
	class abstract_task
	{
	public:
		virtual ~abstract_task() = default;
		virtual double get_progress() const = 0;
		virtual void set_parameters(json::object_ref params) = 0;
		virtual void dump_state(int output_fd) const = 0;
		virtual void set_state(int input_fd) = 0;
		virtual void reset() = 0;
		virtual std::unique_ptr<abstract_task> clone() const = 0;
	};

	class type_erased_task
	{
	public:
		template<class Task>
		explicit type_erased_task(Task&& task):
			m_task{std::make_unique<Task>(std::forward<Task>(task))}
		{}

		type_erased_task(type_erased_task const& other):m_task{other.m_task->clone()}{}

		type_erased_task(type_erased_task&&) = default;

		type_erased_task& operator=(type_erased_task&&) = default;

		~type_erased_task() = default;

		type_erased_task& operator=(type_erased_task const& other)
		{
			m_task = other.m_task->clone();
			return *this;
		}

		double get_progress() const
		{ return m_task->get_progress(); }

		void set_parameters(json::object_ref params)
		{ m_task->set_parameters(params); }

		void dump_state(int output_fd) const
		{ m_task->dump_state(output_fd); }

		void set_state(int input_fd)
		{ m_task->set_state(input_fd); }

		void reset()
		{ m_task->reset(); }

	private:
		std::unique_ptr<abstract_task> m_task;
	};

	template<class Task>
	std::unique_ptr<type_erased_task> create_task()
	{ return std::make_unique<Task>(); }

	using task_factory = std::unique_ptr<type_erased_task> (*)();
}
#endif