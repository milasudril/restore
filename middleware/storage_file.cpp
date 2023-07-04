//@	{"target":{"name":"storage_file.o", "dependencies":[{"ref": "wad64", "origin":"pkg-config"}]}}

#include "./storage_file.hpp"

std::vector<std::string_view> restore::collect_entries(std::reference_wrapper<storage_file const> file,
	std::string_view prefix,
	size_t entry_count_estimate)
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
