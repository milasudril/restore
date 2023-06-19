//@	{"dependencies_extra":[{"ref":"./json_loaders.o", "rel":"implementation"}]}

#ifndef RESTORE_JSON_LOADERS_HPP
#define RESTORE_JSON_LOADERS_HPP

#include "./resource_file.hpp"

#include <jopp/types.hpp>
#include <west/io_fd.hpp>

namespace restore::json
{
	jopp::container load(west::io::fd_ref fd);

	inline jopp::container load(char const* filename)
	{
		auto src = west::io::open(filename, O_RDONLY, 0);
		return load(src.get());
	}

	jopp::container load(Wad64::InputFile& src);

	inline jopp::container load(resource_file const& archive, char const* filename)
	{
		auto src = archive.get_resource(filename);
		return load(src);
	}


}

#endif