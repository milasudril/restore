#ifndef RESTORE_TASK_METADATA_HPP
#define RESTORE_TASK_METADATA_HPP

#include <jopp/serializer.hpp>

namespace restore
{
	struct task_metadata
	{
			jopp::json_buffer parameter_types;
			jopp::json_buffer parameters;
	};
}

#endif