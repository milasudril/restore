//@	{"target":{"name": "./message_decoder.o"}}

#include "./message_decoder.hpp"

restore::http_write_req_result restore::message_decoder::process_request_content(std::span<char const> buffer, size_t)
{
	switch(m_current_state)
	{
		case state::read_json:
		{
			auto const res = m_parser.parse(buffer);
			auto const bytes_written = static_cast<size_t>(res.ptr - std::begin(buffer));

			switch(res.ec)
			{
				case jopp::parser_error_code::completed:
					m_current_state = state::wait_for_blobs;
					return http_write_req_result{
						.bytes_written = bytes_written,
						.ec = http_req_processing_result{res.ec}
					};

				case jopp::parser_error_code::more_data_needed:
					return http_write_req_result{
						.bytes_written = bytes_written,
						.ec = http_req_processing_result{res.ec}
					};

				default:
					return http_write_req_result{
					.bytes_written = bytes_written,
					.ec = http_req_processing_result{res.ec}
				};
			}
		}

		case state::wait_for_blobs:
			abort();
			break;
		case state::read_blob:
			abort();
			break;
		default:
			__builtin_unreachable();
	}
}