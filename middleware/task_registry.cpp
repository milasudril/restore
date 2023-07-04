//@	{"target":{"name":"task_registry.o"}}

#include "./task_registry.hpp"

#include <west/http_utils.hpp>
#include <jopp/serializer.hpp>

namespace
{
	constexpr std::string_view task_prefix{"shared/tasks/"};

	void validate_task_name(std::string_view task_name)
	{
		if(std::ranges::any_of(task_name, [](char ch) {return ch == '/' || ch == '\\';}))
		{ throw std::runtime_error{"Invalid task name"}; }
	}

	std::string get_param_file_name(std::string_view task_name)
	{
		std::string ret{task_prefix};
		ret.append(task_name).append("/parameters.json");
		return ret;
	}
}

restore::task_registry::task_registry(task_factory create_task, storage_file& storage_file):
	m_create_task{create_task},
	m_storage_file{storage_file}
{
	auto const entries = collect_entries(m_storage_file, task_prefix, 16);
	for(auto const item : entries)
	{
		auto const path = item.substr(std::size(task_prefix));
		auto const separator_index = std::ranges::find(path, '/');
		std::string_view task_name{std::begin(path), separator_index};

		auto ip = m_tasks.emplace(task_name, m_create_task());
		if(!ip.second)
		{ continue; }

		auto const params = json::load_object(storage_file.get_file(get_param_file_name(task_name)));
		ip.first->second.set_parameters(json::object_ref{params});
		ip.first->second.set_state(-1);  // TODO: Load state from file
	}
}

void restore::task_registry::create_task(std::string_view task_name, jopp::object const& params)
{
	validate_task_name(task_name);

	auto const ip = m_tasks.emplace(task_name, m_create_task());
	if(ip.second)
	{ throw std::runtime_error{"Task already exists"}; }

	ip.first->second.set_parameters(json::object_ref{params});
	ip.first->second.set_state(-1);  // TODO: It should be possible to upload initial state via rest

	auto const params_json = to_string(params);
	m_storage_file.get().insert(std::as_bytes(std::span{params_json}), get_param_file_name(task_name));
}