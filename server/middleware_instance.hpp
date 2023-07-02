//@	{"dependencies_extra":[{"ref": "./middleware_instance.o", "rel":"implementation"}]}

#ifndef RESTORE_MIDDLEWARE_INSTANCE_HPP
#define RESTORE_MIDDLEWARE_INSTANCE_HPP

#include "./resource_file.hpp"
#include "./storage_file.hpp"
#include "./task_metadata.hpp"

#include <jopp/types.hpp>
#include <string>

namespace restore
{
	struct middleware_instance
	{
		class resource_file resource_file;
		class storage_file storage_file;
		std::string session_key;
		struct task_metadata task_metadata;
	};

	std::string generate_session_key(size_t length_in_bytes);

	std::string get_session_key(jopp::object const& key_cfg);

	middleware_instance create_middleware_instance(jopp::object const& cfg);
}

#endif