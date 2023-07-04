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

	std::string get_init_file_name(std::string_view task_name)
	{
		std::string ret{task_prefix};
		ret.append(task_name).append("/initial_state.dat");
		return ret;
	}

	std::string get_state_file_name(std::string_view task_name)
	{
		std::string ret{task_prefix};
		ret.append(task_name).append("/current_state.dat");
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

bool restore::task_registry::create_task(std::string_view task_name, jopp::object const& params)
{
	validate_task_name(task_name);
	auto const params_json = to_string(params);
	auto const param_file_name = get_param_file_name(task_name);
	m_storage_file.get().insert(std::as_bytes(std::span{params_json}), param_file_name);

	auto const ip = m_tasks.emplace(task_name, m_create_task());
	assert(ip.second);

	ip.first->second.set_parameters(json::object_ref{params});
	ip.first->second.set_state(-1);  // TODO: It should be possible to upload initial state via rest

	return true;
}

bool restore::task_registry::delete_task(std::string_view task_name)
{
	// TODO: remove type-cast in c++23
	if(m_tasks.erase(std::string{task_name}) == 0)
	{ return false; }

	m_storage_file.get().remove(get_param_file_name(task_name));
	m_storage_file.get().remove(get_state_file_name(task_name));
	m_storage_file.get().remove(get_init_file_name(task_name));

	return true;
}

[[nodiscard]] bool restore::task_registry::clone_task(std::string_view src_name, std::string_view target_name)
{
	validate_task_name(target_name);

	auto const new_params = get_param_file_name(target_name);

	insert(m_storage_file.get().archive(),
		Wad64::FileCreationMode::AllowCreation(),
		m_storage_file.get().archive(),
		get_param_file_name(src_name),
		get_param_file_name(target_name));

	insert(m_storage_file.get().archive(),
		Wad64::FileCreationMode::AllowCreation(),
		m_storage_file.get().archive(),
		get_state_file_name(src_name),
		get_state_file_name(target_name));

	insert(m_storage_file.get().archive(),
		Wad64::FileCreationMode::AllowCreation(),
		m_storage_file.get().archive(),
		get_init_file_name(src_name),
		get_init_file_name(target_name));

	auto const src_item = m_tasks.find(src_name);
	if(src_item == std::end(m_tasks))
	{ return false; }

	m_tasks.emplace(target_name, src_item->second.task());

	return true;
}
