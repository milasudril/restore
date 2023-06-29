#ifndef RESTORE_HTTP_REQUEST_RESULT_HPP
#define RESTORE_HTTP_REQUEST_RESULT_HPP

#include <jopp/parser.hpp>

namespace restore
{
	struct http_req_processing_result
	{
		std::variant<jopp::parser_error_code> value;
	};

	constexpr bool can_continue(http_req_processing_result val)
	{
		return std::visit(jopp::overload{
			[](jopp::parser_error_code val) {
				return val == jopp::parser_error_code::completed || val == jopp::parser_error_code::more_data_needed;
			}
		}, val.value);
	}

	constexpr bool is_error_indicator(http_req_processing_result val)
	{
		return std::visit(jopp::overload{
			[](jopp::parser_error_code val) {
				return val != jopp::parser_error_code::completed;
			}
		}, val.value);
	}


	constexpr char const* to_string(http_req_processing_result val)
	{
		return std::visit(jopp::overload{
			[](jopp::parser_error_code val) {
				return to_string(val);
			}
		}, val.value);
	}

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