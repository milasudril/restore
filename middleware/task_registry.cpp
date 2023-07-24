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

	std::string get_task_dir(std::string_view task_name)
	{ return std::string{task_prefix}.append(task_name).append("/"); }

	std::string get_param_dir(std::string_view task_name)
	{ return get_task_dir(task_name).append("/parameters/"); }

	std::string get_init_state_dir(std::string_view task_name)
	{ return get_task_dir(task_name).append("/initial_state/"); }
}

restore::task_registry::task_registry(char const* storage_file_name, task_factory create_task):
	m_create_task{create_task},
	m_storage_file{storage_file{storage_file_name}},
	m_sf_dir{absolute(std::filesystem::path{storage_file_name}).parent_path()}
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

#if 0
		// FIXME:
		auto const params = json::load_object(m_storage_file.get_file(get_param_file_name(task_name)));
		ip.first->second.set_parameters(json::object_ref{params});
		ip.first->second.set_state(-1);  // TODO: Load state from file
#endif
	}
}

void restore::task_registry::create_task(std::string_view task_name,
	jopp::object const& params,
	jopp::object const& initial_state,
	name_to_fd_map const& blobs)
{
	validate_task_name(task_name);
	rewind_all(blobs);
	std::ranges::for_each(blobs, [&storage_file = m_storage_file, task_name](auto const& item) {
		if(item.json_path.empty())
		{ throw std::runtime_error{"Data may not be stored at root level"}; }

		auto const& prefix = item.json_path.front();
		if(!(prefix == "parameters" || prefix == "initial_state"))
		{ throw std::runtime_error{std::string{"Bad path prefix `"}.append(prefix).append("`")}; }

		auto const path = std::string{task_prefix}
			.append(task_name).append("/")
			.append(prefix).append("/")
			.append("blobs/")
			.append(item.name);

		storage_file.insert(item.fd.get(), path);
	});

	{
		auto const params_json = to_string(params);
		auto const path = get_param_dir(task_name).append("fields.json");
		m_storage_file.insert(std::as_bytes(std::span{params_json}), path);
	}

	{
		auto const params_json = to_string(initial_state);
		auto const path = get_init_state_dir(task_name).append("fields.json");
		m_storage_file.insert(std::as_bytes(std::span{params_json}), path);
	}

	auto const ip = m_tasks.emplace(task_name, m_create_task());
	assert(ip.second);

	rewind_all(blobs);
	// TODO: pass blobs to state and parameters
	ip.first->second.set_state(-1);
	ip.first->second.set_parameters(json::object_ref{params});
}

bool restore::task_registry::delete_task(std::string_view task_name)
{
	validate_task_name(task_name);

	// TODO: remove type-cast in c++23
	if(m_tasks.erase(std::string{task_name}) == 0)
	{ return false; }

	auto const files = collect_entries(m_storage_file, get_task_dir(task_name), 16);
	for(auto const& file : files)
	{ m_storage_file.remove(file); }

	return true;
}

[[nodiscard]] bool restore::task_registry::clone_task(std::string_view, std::string_view target_name)
{
	validate_task_name(target_name);
#if 0
	// FIXME:
	auto const new_params = get_param_file_name(target_name);

	insert(m_storage_file.archive(),
		Wad64::FileCreationMode::AllowCreation(),
		m_storage_file.archive(),
		get_param_file_name(src_name),
		get_param_file_name(target_name));

	insert(m_storage_file.archive(),
		Wad64::FileCreationMode::AllowCreation(),
		m_storage_file.archive(),
		get_state_file_name(src_name),
		get_state_file_name(target_name));

	insert(m_storage_file.archive(),
		Wad64::FileCreationMode::AllowCreation(),
		m_storage_file.archive(),
		get_init_file_name(src_name),
		get_init_file_name(target_name));

	auto const src_item = m_tasks.find(src_name);
	if(src_item == std::end(m_tasks))
	{ return false; }

	m_tasks.emplace(target_name, src_item->second.task());
#endif
	return false;
}

Wad64::InputFile restore::task_registry::get_parameter_file(std::string_view) const
{
	throw std::runtime_error{"Blah"};
//	return m_storage_file.get_file(get_param_file_name(task_name));

}

jopp::object restore::get_entries_as_json(task_registry const& registry)
{
	jopp::object ret;
	auto const& tasks = registry.get_tasks();
	for(auto const& item : tasks)
	{
		jopp::object obj{};
		obj.insert("uri_name", west::http::encode_uri_component(item.first));
		auto const progress = item.second.progress();
		obj.insert("running_status", to_string(progress.running_status));
		obj.insert("progress", progress.value);
		ret.insert(std::string{item.first}, std::move(obj));
	}

	return ret;
}

std::optional<restore::task_running_status> restore::task_registry::set_running_status(
	std::string_view task_name,
	task_running_status new_status)
{
	auto const i = m_tasks.find(task_name);
	if(i == std::end(m_tasks))
	{ return std::nullopt; }
	i->second.set_running_status(new_status);

	return i->second.running_status();
}
