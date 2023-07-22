#ifndef RESTORE_TASK_METADATA_HPP
#define RESTORE_TASK_METADATA_HPP

#include "./task.hpp"

#include <jopp/serializer.hpp>

namespace restore
{
	struct task_metadata
	{
			jopp::json_buffer compound_types;
			jopp::json_buffer state_descriptor;
			jopp::json_buffer parameters;
			task_factory factory;
	};
}

#endif