#ifndef RESTORE_CONFIG_HPP
#define RESTORE_CONFIG_HPP

#include <jopp/parser.hpp>
#include <jopp/serializer.hpp>

namespace restore::config
{
	jopp::object load(char const* filename);
}

#endif