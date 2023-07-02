#ifndef RESTORE_MIDDLEWARE_INSTANCE_HPP
#define RESTORE_MIDDLEWARE_INSTANCE_HPP

#include "./resource_file.hpp"
#include "./storage_file.hpp"

#include <jopp/serializer.hpp>
#include <string>

namespace restore
{
	struct middleware_instance
	{
		class resource_file resource_file;
		class storage_file storage_file;
		std::string session_key;
		jopp::json_buffer param_types;
		jopp::json_buffer task_params;
	};
}

#endif