#ifndef RESTORE_HTTP_REQUEST_RESULT_HPP
#define RESTORE_HTTP_REQUEST_RESULT_HPP

#include <jopp/parser.hpp>

#include <memory>
#include <concepts>
#include <functional>
#include <bit>

namespace restore
{
	constexpr bool can_continue(jopp::parser_error_code val)
	{
		return val == jopp::parser_error_code::completed
			|| val == jopp::parser_error_code::more_data_needed;
	}

	constexpr bool is_error_indicator(jopp::parser_error_code val)
	{ return val != jopp::parser_error_code::completed; }

	constexpr bool can_continue(char const*)
	{ return false; }

	constexpr bool is_error_indicator(char const*)
	{ return true;}

	constexpr char const* to_string(char const* val)
	{ return val; }

	template<class T>
	concept status_code = (std::is_enum_v<T> || std::same_as<T, char const*>) && sizeof(T) <= sizeof(intptr_t) && requires(T x)
	{
		{can_continue(x)} -> std::same_as<bool>;
		{can_continue(x)} -> std::same_as<bool>;
		{to_string(x)} -> std::same_as<char const*>;
	};

	struct http_req_processing_result_vtable
	{
		bool (*can_continue)(intptr_t value);
		bool (*is_error_indicator)(intptr_t value);
		char const* (*to_string)(intptr_t value);
	};

	template<class T>
	inline auto to_intptr(T val)
	{ return static_cast<intptr_t>(val); }

	inline auto to_intptr(char const* val)
	{ return reinterpret_cast<intptr_t>(val); }

	template<class T>
	inline auto from_intptr(intptr_t val)
	{ return static_cast<T>(val); }

	template<>
	inline auto from_intptr<char const*>(intptr_t val)
	{ return reinterpret_cast<char const*>(val); }

	inline constexpr http_req_processing_result_vtable status_code_vtable_nop{
		.can_continue = [](intptr_t) {
			return true;
		},
		.is_error_indicator =[](intptr_t) {
			return false;
		},
		.to_string = [](intptr_t){
			return "";
		}
	};

	template<status_code T>
	inline constexpr http_req_processing_result_vtable status_code_vtable{
		.can_continue = [](intptr_t value) {
			return can_continue(from_intptr<T>(value));
		},
		.is_error_indicator =[](intptr_t value) {
			return is_error_indicator(from_intptr<T>(value));
		},
		.to_string = [](intptr_t value){
			return to_string(from_intptr<T>(value));
		}
	};

	class http_req_processing_result
	{
	public:
		http_req_processing_result():m_vtable{status_code_vtable_nop}{}

		template<status_code T>
		explicit http_req_processing_result(T value):
			m_vtable{status_code_vtable<T>},
			m_value{to_intptr(value)}
		{}

		bool can_continue() const
		{ return m_vtable.get().can_continue(m_value); }

		bool is_error_indicator() const
		{ return m_vtable.get().is_error_indicator(m_value); }

		char const* to_string() const
		{ return m_vtable.get().to_string(m_value); }

	private:
		std::reference_wrapper<http_req_processing_result_vtable const> m_vtable;
		intptr_t m_value;
	};

	inline bool can_continue(http_req_processing_result val)
	{ return val.can_continue(); }

	inline bool is_error_indicator(http_req_processing_result val)
	{ return val.is_error_indicator(); }

	inline char const* to_string(http_req_processing_result val)
	{ return val.to_string(); }

	struct http_write_req_result
	{
		size_t bytes_written;
		http_req_processing_result ec;
	};

	struct http_read_resp_result
	{
		size_t bytes_read;
		http_req_processing_result ec;
	};
}

#endif