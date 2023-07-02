#ifndef RESTORE_MIDDLEWARE_INSTANCE_HPP
#define RESTORE_MIDDLEWARE_INSTANCE_HPP

#include "./resource_file.hpp"
#include "./storage_file.hpp"
#include "./task_metadata.hpp"

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
}

#endif