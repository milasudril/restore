#ifndef RESTORE_HTTP_REQUEST_RESULT_HPP
#define RESTORE_HTTP_REQUEST_RESULT_HPP

#include <jopp/parser.hpp>

#include <memory>
#include <concepts>
#include <functional>

namespace restore
{
	constexpr bool can_continue(jopp::parser_error_code val)
	{
		return val == jopp::parser_error_code::completed
			|| val == jopp::parser_error_code::more_data_needed;
	}

	constexpr bool is_error_indicator(jopp::parser_error_code val)
	{ return val != jopp::parser_error_code::completed; }

	template<class T>
	concept status_code = std::is_enum_v<T> && sizeof(T) <= sizeof(size_t) && requires(T x)
	{
		{can_continue(x)} -> std::same_as<bool>;
		{can_continue(x)} -> std::same_as<bool>;
		{to_string(x)} -> std::same_as<char const*>;
	};

	struct http_req_processing_result_vtable
	{
		bool (*can_continue)(size_t value);
		bool (*is_error_indicator)(size_t value);
		char const* (*to_string)(size_t value);
	};

	inline constexpr http_req_processing_result_vtable status_code_vtable_nop{
		.can_continue = [](size_t) {
			return true;
		},
		.is_error_indicator =[](size_t) {
			return false;
		},
		.to_string = [](size_t){
			return "";
		}
	};

	template<status_code T>
	inline constexpr http_req_processing_result_vtable status_code_vtable{
		.can_continue = [](size_t value) {
			return can_continue(static_cast<T>(value));
		},
		.is_error_indicator =[](size_t value) {
			return is_error_indicator(static_cast<T>(value));
		},
		.to_string = [](size_t value){
			return to_string(static_cast<T>(value));
		}
	};

	class http_req_processing_result
	{
	public:
		http_req_processing_result():m_vtable{status_code_vtable_nop}{}

		template<status_code T>
		explicit http_req_processing_result(T value):
			m_vtable{status_code_vtable<T>},
			m_value{static_cast<size_t>(value)}
		{}

		bool can_continue() const
		{ return m_vtable.get().can_continue(m_value); }

		bool is_error_indicator() const
		{ return m_vtable.get().is_error_indicator(m_value); }

		char const* to_string() const
		{ return m_vtable.get().to_string(m_value); }

	private:
		std::reference_wrapper<http_req_processing_result_vtable const> m_vtable;
		size_t m_value;
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