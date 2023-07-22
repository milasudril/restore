#ifndef RESTORE_MESSAGE_ENCODER_HPP
#define RESTORE_MESSAGE_ENCODER_HPP

#include <jopp/types.hpp>

namespace restore
{
	inline jopp::object wrap_in_message(jopp::object&& obj)
	{
		jopp::object ret;
		ret.insert("fields", std::move(obj));
		return ret;
	}
}

#endif