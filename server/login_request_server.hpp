#ifndef RESTORE_LOGINSERVER_SERVER_HPP
#define RESTORE_LOGINSERVER_SERVER_HPP

#include "./http_request_result.hpp"

#include <west/http_message_header.hpp>
#include <compare>

namespace restore
{
	class login_request_server
	{
	public:
		explicit login_request_server(std::string_view session_key):
			m_session_key{session_key},
			m_session_info{std::make_unique<jopp::container>()},
			m_session_info_parser{*m_session_info},
			m_resp_ptr{nullptr},
			m_bytes_to_read{0}
		{ }

		constexpr std::strong_ordering operator<=>(null_server const&) const noexcept
		{ return std::strong_ordering::equal; }

		auto finalize_state(west::http::field_map& fields)
		{
			try
			{
				auto obj = m_session_info->template get_if<jopp::object>();
				if(obj == nullptr)
				{ throw std::runtime_error{"Expected login request to be an object"}; }

				auto const& session_key = obj->get_field_as<jopp::string>("session_key");
				if(session_key != m_session_key)
				{
					jopp::object resp_obj{};
					resp_obj.insert("result", "failed");
					resp_obj.insert("cause", "Invalid session key");

					m_response = to_string(jopp::container{std::move(resp_obj)});
					m_resp_ptr = std::data(m_response);
					m_bytes_to_read = std::size(m_response);

					fields.append("Content-Length", std::to_string(m_bytes_to_read));

					return west::http::finalize_state_result{};
				}

				jopp::object resp_obj{};
				resp_obj.insert("result", "successful");

				m_response = to_string(jopp::container{std::move(resp_obj)});
				m_resp_ptr = std::data(m_response);
				m_bytes_to_read = std::size(m_response);

				std::string cookie_value{"session_key="};
				cookie_value.append(m_session_key)
					.append(";SameSite=Strict;HttpOnly;Path=/");

				fields.append("Content-Length", std::to_string(m_bytes_to_read))
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

		auto process_request_content(std::span<char const> buffer)
		{
			auto const res = m_session_info_parser.parse(buffer);
			auto const bytes_written = static_cast<size_t>(res.ptr - std::begin(buffer));

			if(res.ec == jopp::parser_error_code::completed || res.ec == jopp::parser_error_code::more_data_needed)
			{
				return http_write_req_result{
					.bytes_written = bytes_written,
					.ec = http_req_processing_result{res.ec}
				};
			}

			return http_write_req_result{
				.bytes_written = bytes_written,
				.ec = http_req_processing_result{res.ec}
			};
		}

	private:
		std::string_view m_session_key;
		std::unique_ptr<jopp::container> m_session_info;
		jopp::parser m_session_info_parser;

		std::string m_response;
		char const* m_resp_ptr;
		size_t m_bytes_to_read;
	};
}

#endif