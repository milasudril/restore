#ifndef RESTORE_LOGIN_SERVER_HPP
#define RESTORE_LOGIN_SERVER_HPP


#include "./message_decoder.hpp"
#include "./http_request_result.hpp"

#include <west/http_message_header.hpp>
#include <compare>

namespace restore
{
	class login_server
	{
	public:
		explicit login_server(std::string_view session_key):
			m_session_key{session_key},
			m_msg_decoder{"/tmp"},
			m_resp_ptr{nullptr},
			m_bytes_to_read{0}
		{ }

		constexpr std::strong_ordering operator<=>(null_server const&) const noexcept
		{ return std::strong_ordering::equal; }

		auto finalize_state(west::http::field_map& header_fields)
		{
			try
			{
				auto obj = m_msg_decoder.get_json()->get_if<jopp::object>();
				if(obj == nullptr)
				{ throw std::runtime_error{"Expected login request to be an object"}; }

				auto const& fields = obj->get_field_as<jopp::object>("fields");
				auto const& session_key = fields.get_field_as<jopp::string>("session_key");
				if(session_key != m_session_key)
				{
					jopp::object resp_obj{};
					resp_obj.insert("result", "failed");
					resp_obj.insert("cause", "Invalid session key");

					m_response = to_string(resp_obj);
					m_resp_ptr = std::data(m_response);
					m_bytes_to_read = std::size(m_response);

					header_fields.append("Content-Length", std::to_string(m_bytes_to_read));

					return west::http::finalize_state_result{};
				}

				jopp::object resp_obj{};
				resp_obj.insert("result", "successful");

				m_response = to_string(resp_obj);
				m_resp_ptr = std::data(m_response);
				m_bytes_to_read = std::size(m_response);

				std::string cookie_value{"session_key="};
				cookie_value.append(m_session_key)
					.append(";SameSite=Strict;HttpOnly;Path=/");

				header_fields.append("Content-Length", std::to_string(m_bytes_to_read))
					.append("Set-Cookie", std::move(cookie_value));

				return west::http::finalize_state_result{};
			}
			catch(std::runtime_error const& error)
			{
				return west::http::finalize_state_result{
					.http_status = west::http::status::bad_request,
					.error_message = west::make_unique_cstr(error.what())
				};
			}
		}

		auto read_response_content(std::span<char> buffer)
		{
			auto const bytes_to_read = std::min(std::size(buffer), m_bytes_to_read);
			std::copy_n(m_resp_ptr, bytes_to_read, std::data(buffer));
			m_resp_ptr += bytes_to_read;
			m_bytes_to_read -= bytes_to_read;

			return http_read_resp_result{
				bytes_to_read,
				http_req_processing_result{}
			};
		}

		auto process_request_content(std::span<char const> buffer, size_t bytes_to_read)
		{ return m_msg_decoder.process_request_content(buffer, bytes_to_read); }

	private:
		std::string_view m_session_key;
		message_decoder m_msg_decoder;

		std::string m_response;
		char const* m_resp_ptr;
		size_t m_bytes_to_read;
	};
}

#endif