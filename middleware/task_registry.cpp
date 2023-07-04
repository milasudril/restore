//@	{"target":{"name":"task_registry.o"}}

#include "./task_registry.hpp"

#include <west/http_utils.hpp>
#include <jopp/serializer.hpp>

void restore::task_registry::create_task(std::string_view task_name, jopp::object const& params)
{
	if(std::ranges::any_of(task_name, [](char ch) {return ch == '/' || ch == '\\';}))
	{ throw std::runtime_error{"Invalid task name"}; }

	auto const ip = m_tasks.emplace(task_name, m_create_task(json::object_ref{params}));
	if(ip.second)
	{ throw std::runtime_error{"Task already exists"}; }

	std::string res_name{task_prefix};
	res_name.append(task_name)
		.append("/parameters.json");
	auto const params_json = to_string(params);
	m_storage_file.get().insert(std::as_bytes(std::span{params_json}), res_name);
}