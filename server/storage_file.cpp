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

size_t restore::remove_entries(storage_file& file, std::string_view prefix)
{
	auto const& items = file.ls();
	fprintf(stderr, "Removing %s\n", std::data(prefix));

	auto i = items.lower_bound(prefix);
	std::vector<std::string_view> to_be_removed;
	to_be_removed.reserve(4);
	while(i != std::end(items) && i->first.starts_with(prefix))
	{
		to_be_removed.push_back(i->first);
		++i;
	}

	for(auto const& item : to_be_removed)
	{ file.remove(item); }

	return std::size(to_be_removed);
}