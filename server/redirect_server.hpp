#ifndef RESTORE_REDIRECTs_SERVER_HPP

#include "./http_request_result.hpp"

#include <west/http_message_header.hpp>
#include <compare>

namespace restore
{
	class redirect_server
	{
	public:
		explicit redirect_server(std::string_view uri,
			west::http::status redirect_status = west::http::status::moved_permanently):
			m_uri{uri},
			m_redirect_status{redirect_status}
		{}

		constexpr std::strong_ordering operator<=>(null_server const&) const noexcept
		{ return std::strong_ordering::equal; }

		auto finalize_state(west::http::field_map& fields) const
		{
			fields.append("Content-Length", "0")
				.append("Location", std::string{m_uri});
			return west::http::finalize_state_result{
				.http_status = m_redirect_status,
				.error_message = nullptr
			};
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
			return http_write_req_result{
				.bytes_written = std::size(buffer),
				.ec = http_req_processing_result{}
			};
		}

	private:
		std::string_view m_uri;
		west::http::status m_redirect_status;
	};
}

#endif