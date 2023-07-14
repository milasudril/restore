#ifndef RESTORE_CACHED_JSON_RESPONSE_SERVER_HPP
#define RESTORE_CACHED_JSON_RESPONSE_SERVER_HPP

#include "./http_request_result.hpp"

#include <jopp/serializer.hpp>
#include <west/http_message_header.hpp>
#include <west/http_request_handler.hpp>

namespace restore
{
	class cached_json_response_server
	{
	public:
		explicit cached_json_response_server(jopp::json_buffer_view response):
			m_response_ptr{std::data(response)},
			m_bytes_to_write{std::size(response)}
		{ }

		auto finalize_state(west::http::field_map& fields) const
		{
			fields.append("Content-Length", std::to_string(m_bytes_to_write))
				.append("Content-Type", "application/json");

			west::http::finalize_state_result validation_result{};
			validation_result.http_status = west::http::status::ok;
			return validation_result;
		}

		auto read_response_content(std::span<char> buffer)
		{
			auto const bytes_to_write = std::min(std::size(buffer), m_bytes_to_write);
			std::copy_n(m_response_ptr, bytes_to_write, std::data(buffer));
			m_response_ptr += bytes_to_write;
			m_bytes_to_write -= bytes_to_write;
			return http_read_resp_result{
				bytes_to_write,
				http_req_processing_result{}
			};
		}

		auto process_request_content(std::span<char const> buffer, size_t) const
		{
			return http_write_req_result{
				.bytes_written = std::size(buffer),
				.ec = http_req_processing_result{}
			};
		}

	private:
		char const* m_response_ptr;
		size_t m_bytes_to_write;
	};
}

#endif
