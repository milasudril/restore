//@	{"target":{"name":"jopp_wrapper.o"}}

#include "./jopp_wrapper.hpp"

#include <jopp/types.hpp>

template<>
restore::json::object_ref
restore::json::object_ref::get_field_as<restore::json::object_ref>(std::string_view key) const
{ return object_ref{m_ref.get().get_field_as<jopp::object>(key)}; }

template<>
std::string const& restore::json::object_ref::get_field_as<std::string>(std::string_view key) const
{ return m_ref.get().get_field_as<std::string>(key); }

template<>
double restore::json::object_ref::get_field_as<double>(std::string_view key) const
{ return m_ref.get().get_field_as<double>(key); }

template<>
bool restore::json::object_ref::get_field_as<bool>(std::string_view key) const
{ return m_ref.get().get_field_as<bool>(key); }

bool restore::json::object_ref::contains(std::string_view key) const
{ return m_ref.get().contains(key); }
