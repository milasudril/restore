//@	{"dependencies_extra":[{"ref": "./middleware_instance.o", "rel":"implementation"}]}

#ifndef RESTORE_MIDDLEWARE_INSTANCE_HPP
#define RESTORE_MIDDLEWARE_INSTANCE_HPP

#include "./resource_file.hpp"
#include "./storage_file.hpp"
#include "./task_metadata.hpp"
#include "./task_registry.hpp"

#include <jopp/types.hpp>
#include <string>

namespace restore
{
	struct middleware_instance
	{
		class resource_file resource_file;
		std::string session_key;
		task_metadata taskinfo;
		task_registry tasks;
	};

	std::string generate_session_key(size_t length_in_bytes);

	std::string get_session_key(jopp::object const& key_cfg);

	middleware_instance create_middleware_instance(jopp::object const& cfg,
		task_metadata const& taskinfo);
}

#endif