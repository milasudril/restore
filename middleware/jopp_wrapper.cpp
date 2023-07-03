//@	{"target":{"name":"jopp_wrapper.o"}}

#include "./jopp_wrapper.hpp"

#include <jopp/types.hpp>

template<>
auto
restore::json::object_ref::get_field_as<restore::json::object_ref>(std::string_view key) const
{ return object_ref{m_ref.get().get_field_as<jopp::object>(key)}; }

template<>
auto restore::json::object_ref::get_field_as<jopp::string>(std::string_view key) const
{ return m_ref.get().get_field_as<jopp::string>(key); }

template<>
auto restore::json::object_ref::get_field_as<jopp::number>(std::string_view key) const
{ return m_ref.get().get_field_as<jopp::number>(key); }

template<>
auto restore::json::object_ref::get_field_as<jopp::boolean>(std::string_view key) const
{ return m_ref.get().get_field_as<jopp::number>(key); }

template<>
auto restore::json::object_ref::get_field_as<jopp::null>(std::string_view key) const
{ return m_ref.get().get_field_as<jopp::null>(key); }

bool restore::json::object_ref::contains(std::string_view key) const
{ return m_ref.get().contains(key); }
