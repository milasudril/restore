#ifndef RESTORE_HTTP_REQUEST_RESULT_HPP
#define RESTORE_HTTP_REQUEST_RESULT_HPP

namespace restore
{
	enum class http_req_processing_result{};

	constexpr bool can_continue(http_req_processing_result)
	{ return true; }

	constexpr bool is_error_indicator(http_req_processing_result)
	{ return false; }


	constexpr char const* to_string(http_req_processing_result)
	{ return "No error"; }

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