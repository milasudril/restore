#ifndef RESTORE_LOGINSERVER_SERVER_HPP
#define RESTORE_LOGINSERVER_SERVER_HPP

#include "./http_request_result.hpp"

#include <west/http_message_header.hpp>
#include <jopp/parser.hpp>
#include <compare>

namespace restore
{
	class login_request_server
	{
	public:
		explicit login_request_server(std::string_view session_key):
			m_session_key{session_key},
			m_session_info{std::make_unique<jopp::container>()},
			m_session_info_parser{*m_session_info}
		{}

		constexpr std::strong_ordering operator<=>(null_server const&) const noexcept
		{ return std::strong_ordering::equal; }

		auto finalize_state(west::http::field_map& fields) const
		{
			fields.append("Content-Length", "0");
			fields.append("Set-Cookie", "session_key=foobar;SameSite=Strict;HttpOnly");
			puts("Finalize before read response");
			return west::http::finalize_state_result{};
		}

		auto read_response_content(std::span<char> buffer)
		{
			printf("Read response content %zu\n", std::size(buffer));
			return http_read_resp_result{
				0,
				http_req_processing_result{}
			};
		}

		auto process_request_content(std::span<char const> buffer)
		{
			auto const res = m_session_info_parser.parse(buffer);
			auto const bytes_written = static_cast<size_t>(res.ptr - std::begin(buffer));
			if(res.ec == jopp::parser_error_code::completed || res.ec == jopp::parser_error_code::more_data_needed)
			{
				return http_write_req_result{
					.bytes_written = bytes_written,
					.ec = http_req_processing_result{}
				};
			}

			return http_write_req_result{
				.bytes_written = bytes_written,
				.ec = http_req_processing_result{}
			};
		}

	private:
		std::string_view m_session_key;
		std::unique_ptr<jopp::container> m_session_info;
		jopp::parser m_session_info_parser;
	};
}

#endif