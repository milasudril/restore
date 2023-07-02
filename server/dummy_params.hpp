//@	{"dependencies_extra": [{"ref":"./dummy_params.o", "rel":"implementation"}]}

#ifndef RESTORE_SERVER_DUMMYPARAMS_HPP
#define RESTORE_SERVER_DUMMYPARAMS_HPP

#include <jopp/types.hpp>

namespace restore
{
	jopp::object get_parameter_types();
	jopp::object get_task_parameters();
}

#endif