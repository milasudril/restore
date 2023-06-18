#ifndef RESTORE_CONFIG_HPP
#define RESTORE_CONFIG_HPP

#include <jopp/types.hpp>

namespace restore::config
{
	jopp::object load(char const* filename);
}

#endif