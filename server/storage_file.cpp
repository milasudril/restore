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