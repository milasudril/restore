//@	{"target":{"name": "./message_decoder.o"}}

#include "./message_decoder.hpp"

std::pair<restore::http_write_req_result, restore::message_decoder_state>
restore::decode_json(jopp::parser& parser, std::span<char const> buffer, size_t bytes_to_read)
{
	auto const res = parser.parse(buffer);
	auto const bytes_written = static_cast<size_t>(res.ptr - std::begin(buffer));

	switch(res.ec)
	{
		case jopp::parser_error_code::completed: {
			auto const root = parser.root().get_if<jopp::object>();
			if(root == nullptr)
			{
				return std::pair{
					http_write_req_result{
						.bytes_written = bytes_written,
						.ec = http_req_processing_result{message_decoder_error_code::root_is_not_an_object}
					},
					message_decoder_state::wait_for_blobs
				};
			}

			auto const bytes_left = bytes_to_read - bytes_written;
			auto const blobs_iter = root->find("blobs");
			if(blobs_iter == std::end(*root))
			{
				if(bytes_left == 0) [[likely]]
				{
					return std::pair{
						http_write_req_result{
							.bytes_written = bytes_written,
							.ec = http_req_processing_result{res.ec}
						},
						message_decoder_state::wait_for_blobs
					};
				}

				return std::pair{
					http_write_req_result{
						.bytes_written = bytes_written,
						.ec = http_req_processing_result{message_decoder_error_code::unknown_data_present}
					},
					message_decoder_state::wait_for_blobs
				};
			}

			auto const blobs = blobs_iter->second.get_if<jopp::object>();
			if(blobs == nullptr)
			{
				return std::pair{
					http_write_req_result{
						.bytes_written = bytes_written,
						.ec = http_req_processing_result{message_decoder_error_code::blobs_is_not_an_object}
					},
					message_decoder_state::wait_for_blobs
				};
			}

			return std::pair{
				http_write_req_result{
					.bytes_written = bytes_written,
					.ec = http_req_processing_result{res.ec}
				},
				message_decoder_state::wait_for_blobs
			};
		}

		case jopp::parser_error_code::more_data_needed:
			return std::pair{
				http_write_req_result{
					.bytes_written = bytes_written,
					.ec = http_req_processing_result{res.ec}
				},
				message_decoder_state::read_json
			};

		default:
			return std::pair{
				http_write_req_result{
					.bytes_written = bytes_written,
					.ec = http_req_processing_result{res.ec}
				},
				message_decoder_state::read_json
			};
	}
}

restore::http_write_req_result
restore::message_decoder::process_request_content(std::span<char const> buffer, size_t bytes_to_read)
{
	switch(m_current_state)
	{
		case message_decoder_state::read_json: {
			auto [ret, new_state] = decode_json(m_parser, buffer, bytes_to_read);
			m_current_state = new_state;
			return ret;
		}

		case message_decoder_state::wait_for_blobs:
			abort();
			break;
		case message_decoder_state::read_blob:
			abort();
			break;
		default:
			__builtin_unreachable();
	}
}