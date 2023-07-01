//@	{"target":{"name":"storage_file.o", "dependencies":[{"ref": "wad64", "origin":"pkg-config"}]}}

#include "./storage_file.hpp"

#include <west/http_utils.hpp>

jopp::object restore::generate_entity_list(storage_file const& file, std::string_view prefix)
{
	jopp::object ret;
	auto const& items = file.ls();
	for(auto const& item : items)
	{
		if(item.first.starts_with(prefix))
		{
			auto name = item.first.substr(std::size(prefix));
			auto const i = std::ranges::find(name, '/');

			auto ip = ret.insert(std::string{std::begin(name), i}, jopp::null{});
			if(ip.second)
			{
				jopp::object obj{};
				obj.insert("uri_name", west::http::encode_uri_component(ip.first->first));
				ip.first->second = jopp::value{std::move(obj)};
			}
		}
	}

	return ret;
}

namespace
{
	std::vector<std::string_view> collect_entries(std::reference_wrapper<restore::storage_file const> file,
		std::string_view prefix,
		size_t entry_count_estimate = 4)
	{
		auto const& items = file.get().ls();
		auto i = items.lower_bound(prefix);
		std::vector<std::string_view> ret;
		ret.reserve(entry_count_estimate);
		while(i != std::end(items) && i->first.starts_with(prefix))
		{
			ret.push_back(i->first);
			++i;
		}
		return ret;
	}
}

size_t restore::remove_entries(storage_file& file, std::string_view prefix)
{
	auto const to_be_removed = collect_entries(file, prefix);
	for(auto const& item : to_be_removed)
	{ file.remove(item); }
	return std::size(to_be_removed);
}

size_t restore::copy_entries(storage_file& file, std::string_view prefix, std::string_view new_prefix)
{
	auto const to_be_copyied = collect_entries(file, prefix);
	for(auto const& item : to_be_copyied)
	{
		auto const filename = item.substr(std::size(prefix));
		std::string dest{new_prefix};
		dest.append("/").append(filename);

		printf("%s -> %s\n", std::data(item), std::data(dest));
	}

	return 0;
}