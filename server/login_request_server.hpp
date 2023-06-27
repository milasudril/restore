#ifndef RESTORE_LOGINSERVER_SERVER_HPP
#define RESTORE_LOGINSERVER_SERVER_HPP

#include "./http_request_result.hpp"

#include <west/http_message_header.hpp>
#include <compare>

namespace restore
{
	struct login_request_server
	{
		constexpr std::strong_ordering operator<=>(null_server const&) const noexcept
		{ return std::strong_ordering::equal; }

		auto finalize_state(west::http::field_map&) const {
			return west::http::finalize_state_result{};
		}

		auto read_response_content(std::span<char>)
		{
			return http_read_resp_result{
				0,
				http_req_processing_result{}
			};
		}

		std::string_view session_key;
	};
}

#endif