//@	{"target":{"name":"jopp_wrapper.test"}}

#include "extproj/testfwk/testfwk.hpp"

#include "./jopp_wrapper.hpp"

#include <jopp/types.hpp>

TESTCASE(restore_jopp_wrapper_object_ref_get_field_as)
{
	jopp::object testobj;
	testobj.insert("a number", 1.25);

	jopp::object inner_obj;
	inner_obj.insert("a_string", "Hello, World");
	testobj.insert("an object", std::move(inner_obj));

	restore::json::object_ref ref{testobj};

	auto val1 = ref.get_field_as<double>("a number");
	EXPECT_EQ(val1, 1.25);

	auto val2 = ref.get_field_as<restore::json::object_ref>("an object");
	auto str = val2.get_field_as<std::string>("a_string");
	EXPECT_EQ(str, "Hello, World");
}