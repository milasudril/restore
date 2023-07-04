//@	{"dependencies_extra":[{"ref":"./task_registry.o"}]}

#ifndef RESTORE_TASK_REGISTRY_HPP
#define RESTORE_TASK_REGISTRY_HPP

#include "./storage_file.hpp"
#include "./task_runner.hpp"

namespace restore
{
	class task_registry
	{
	public:
		explicit task_registry(task_factory create_task, storage_file& storage_file);

		[[nodiscard]] bool create_task(std::string_view task_name, jopp::object const& params);

		[[nodiscard]] bool delete_task(std::string_view task_name);

		[[nodiscard]] bool clone_task(std::string_view src_name, std::string_view target_name);

	private:
		task_factory m_create_task;
		std::map<std::string, task_runner<type_erased_task>, std::less<>> m_tasks;
		std::reference_wrapper<storage_file> m_storage_file;
	};
};

#endif