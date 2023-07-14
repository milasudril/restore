#ifndef RESTORE_JSON_RESPONSE_SERVER_HPP
#define RESTORE_JSON_RESPONSE_SERVER_HPP

#include "./http_request_result.hpp"

#include <jopp/serializer.hpp>
#include <west/http_message_header.hpp>
#include <west/http_request_handler.hpp>

namespace restore
{
	class json_response_server
	{
	public:
		explicit json_response_server(jopp::object const& response):
			m_response{to_string(response)}
		{ }

		auto finalize_state(west::http::field_map& fields)
		{
			// Initialize after ctor to avoid having to deal with SBO-related stuff in std::string
			m_response_ptr = std::data(m_response);
			m_bytes_to_write = std::size(m_response);

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
		std::string m_response;
		char const* m_response_ptr{nullptr};
		size_t m_bytes_to_write{0};
	};
}

#endif