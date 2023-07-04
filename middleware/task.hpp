#ifndef RESTORE_TASK_HPP
#define RESTORE_TASK_HPP

#include "./jopp_wrapper.hpp"
#include <memory>

namespace restore
{
	enum class task_step_result{task_is_completed, keep_going};

	template<class T>
	concept task = requires(T x, json::object_ref params, int fd)
	{
		{x.step()} -> std::same_as<task_step_result>;
		{x.get_progress()} -> std::same_as<double>;
		{x.set_parameters(params)} -> std::same_as<void>;
		{x.dump_state(fd)} -> std::same_as<void>;
		{x.set_state(fd)} -> std::same_as<void>;
		{x.reset()} -> std::same_as<void>;
	};

	class abstract_task
	{
	public:
		virtual ~abstract_task() = default;
		virtual task_step_result step() = 0;
		virtual double get_progress() const = 0;
		virtual void dump_state(int output_fd) const = 0;
		virtual void set_parameters(json::object_ref params) = 0;
		virtual void set_state(int input_fd) = 0;
		virtual void reset() = 0;
		virtual std::unique_ptr<abstract_task> clone() const = 0;
	};

	template<task Task>
	struct task_adaptor : public abstract_task
	{
		explicit task_adaptor(Task&& task): m_task{std::move(task)}{}

		task_step_result step() override
		{ return m_task.step(); }

		double get_progress() const override
		{ return m_task.get_progress(); }

		void dump_state(int output_fd) const override
		{ m_task.dump_state(output_fd); }

		void set_state(int input_fd) override
		{ m_task.set_state(input_fd); }

		void reset() override
		{ m_task.reset(); }

		std::unique_ptr<abstract_task> clone() const override
		{ return std::make_unique<task_adaptor<Task>>(*this); }

		void set_parameters(json::object_ref params) override
		{ m_task.set_parameters(params); }

	private:
		Task m_task;
	};

	class type_erased_task
	{
	public:
		template<task Task>
		explicit type_erased_task(Task&& task):
			m_task{std::make_unique<task_adaptor<Task>>(std::forward<Task>(task))}
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

		void dump_state(int output_fd) const
		{ m_task->dump_state(output_fd); }

		void set_state(int input_fd)
		{ m_task->set_state(input_fd); }

		void reset()
		{ m_task->reset(); }

		[[nodiscard]] task_step_result step()
		{ return m_task->step(); }

		void set_parameters(json::object_ref params)
		{ m_task->set_parameters(params); }

	private:
		std::unique_ptr<abstract_task> m_task;
	};

	template<task Task>
	type_erased_task create_task()
	{ return type_erased_task{Task{}}; }

	using task_factory = type_erased_task (*)();
}
#endif