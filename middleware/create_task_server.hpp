#ifndef RESTORE_CREATE_TASK_SERVER_HPP
#define RESTORE_CREATE_TASK_SERVER_HPP

#include "./http_request_result.hpp"
#include "./storage_file.hpp"

#include <west/http_message_header.hpp>
#include <compare>

namespace restore
{
	class create_task_server
	{
	public:
		explicit create_task_server(storage_file& storage_file):
			m_request_body{std::make_unique<jopp::container>()},
			m_request_body_parser{*m_request_body},
			m_resp_ptr{nullptr},
			m_bytes_to_read{0},
			m_storage_file{storage_file}
		{ }

		constexpr std::strong_ordering operator<=>(null_server const&) const noexcept
		{ return std::strong_ordering::equal; }

		auto finalize_state(west::http::field_map& fields)
		{
			try
			{
				auto obj = m_request_body->get_if<jopp::object>();
				if(obj == nullptr)
				{ throw std::runtime_error{"Expected request to be an object"}; }

				auto const& task_name = obj->get_field_as<jopp::string>("name");
				auto const& params = obj->get_field_as<jopp::object>("parameters");

				// TODO: Pass params to client application for validation. Since the client application may
				//       only know about one task, it is for validation only.

				if(std::ranges::any_of(task_name, [](char ch) {return ch == '/' || ch == '\\';}))
				{ throw std::runtime_error{"Invalid task name"}; }

				std::string res_name{"shared/tasks/"};
				res_name.append(task_name)
					.append("/parameters.json");
				auto const params_json = jopp::to_string(params);
				m_storage_file.get().insert(std::as_bytes(std::span{params_json}), res_name);

				jopp::object resp_obj{};
				resp_obj.insert("result", "successful");

				m_response = to_string(resp_obj);
				m_resp_ptr = std::data(m_response);
				m_bytes_to_read = std::size(m_response);
				fields.append("Content-Length", std::to_string(m_bytes_to_read));

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
			auto const res = m_request_body_parser.parse(buffer);
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
		std::unique_ptr<jopp::container> m_request_body;
		jopp::parser m_request_body_parser;

		std::string m_response;
		char const* m_resp_ptr;
		size_t m_bytes_to_read;

		std::reference_wrapper<storage_file> m_storage_file;
	};
}

#endif