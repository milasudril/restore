#ifndef RESTORE_MESSAGE_DECODER_HPP
#define RESTORE_MESSAGE_DECODER_HPP

#include "./http_request_result.hpp"
#include <jopp/parser.hpp>

namespace restore
{
	class message_decoder
	{
	public:
		message_decoder():m_container{std::make_unique<jopp::container>()},
			m_parser{*m_container},
			m_current_state{state::read_json}
		{}

		auto process_request_content(std::span<char const> buffer)
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

		auto const& get_json() const
		{ return m_container; }


	private:
		std::unique_ptr<jopp::container> m_container;
		jopp::parser m_parser;

		enum class state{
			read_json,
			wait_for_blobs,
			read_blob
		};

		state m_current_state;
	};
}

#endif