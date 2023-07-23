//@	{"dependencies_extra":[{"ref":"./task_registry.o", "rel":"implementation"}]}

#ifndef RESTORE_TASK_REGISTRY_HPP
#define RESTORE_TASK_REGISTRY_HPP

#include "./storage_file.hpp"
#include "./task_runner.hpp"
#include "./name_to_fd_map.hpp"

namespace restore
{
	class task_registry
	{
	public:
		explicit task_registry(char const* storage_file_name, task_factory create_task);

		void create_task(std::string_view task_name,
			jopp::object const& params,
			jopp::object const& initial_state,
			name_to_fd_map const& blobs);

		[[nodiscard]] bool delete_task(std::string_view task_name);

		[[nodiscard]] bool clone_task(std::string_view src_name, std::string_view target_name);

		[[nodiscard]] auto const& get_tasks() const
		{ return m_tasks; }

		Wad64::InputFile get_parameter_file(std::string_view task_name) const;

		std::optional<task_running_status> set_running_status(std::string_view task_name, task_running_status new_status);

		char const* get_tempdir() const
		{ return m_sf_dir.c_str(); }

	private:
		task_factory m_create_task;
		std::map<std::string, task_runner<type_erased_task>, std::less<>> m_tasks;
		storage_file m_storage_file;
		std::filesystem::path m_sf_dir;
	};

	jopp::object get_entries_as_json(task_registry const& registry);

	//	jopp::object const& ge
};

#endif