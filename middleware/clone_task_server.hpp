#ifndef RESTORE_CLONE_TASK_SERVER_HPP
#define RESTORE_CLONE_TASK_SERVER_HPP

#include "./http_request_result.hpp"
#include "./task_registry.hpp"
#include "./message_decoder.hpp"

#include <west/http_message_header.hpp>
#include <compare>

namespace restore
{
	class clone_task_server
	{
	public:
		explicit clone_task_server(task_registry& tasks, std::string&& source):
			m_source{std::move(source)},
			m_msg_decoder{tasks.get_tempdir()},
			m_resp_ptr{nullptr},
			m_bytes_to_read{0},
			m_tasks{tasks}
		{ }

		constexpr std::strong_ordering operator<=>(null_server const&) const noexcept
		{ return std::strong_ordering::equal; }

		auto finalize_state(west::http::field_map& resposne_fields)
		{
			try
			{
				auto obj = m_msg_decoder.get_json()->get_if<jopp::object>();
				if(obj == nullptr)
				{ throw std::runtime_error{"Expected request to be an object"}; }

				auto const& fields = obj->get_field_as<jopp::object>("fields");
				auto const& new_name = fields.get_field_as<jopp::string>("new_name");
				if(!m_tasks.get().clone_task(m_source, new_name))
				{
					return west::http::finalize_state_result{
						.http_status = west::http::status::not_found,
						.error_message = west::make_unique_cstr("Source task does not exist")
					};
				}

				jopp::object resp_obj{};
				resp_obj.insert("result", "successful");
				m_response = to_string(resp_obj);
				m_resp_ptr = std::data(m_response);
				m_bytes_to_read = std::size(m_response);
				resposne_fields.append("Content-Length", std::to_string(m_bytes_to_read));

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
		std::string m_source;

		message_decoder m_msg_decoder;

		std::string m_response;
		char const* m_resp_ptr;
		size_t m_bytes_to_read;

		std::reference_wrapper<task_registry> m_tasks;
	};
}

#endif