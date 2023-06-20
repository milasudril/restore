#ifndef RESTORE_HTTP_SERVICE_HPP
#define RESTORE_HTTP_SERVICE_HPP

#include "./json_loaders.hpp"
#include "./resource_file.hpp"

#include <jopp/serializer.hpp>
#include <west/http_message_header.hpp>
#include <west/http_request_handler.hpp>

template<>
struct jopp::object_converter<west::http::status>
{
	static auto to_json(west::http::status status)
	{
		return static_cast<jopp::number>(static_cast<std::underlying_type_t<west::http::status>>(status));
	}
};

template<>
struct jopp::object_converter<west::http::finalize_state_result>
{
	static auto to_json(west::http::finalize_state_result const& obj)
	{
		jopp::object ret;
		ret.insert("http_status", jopp::to_json(obj.http_status));
		if(obj.error_message != nullptr)
		{ ret.insert("error_message", obj.error_message.get()); }

		return ret;
	}
};

namespace restore
{
	enum class http_req_processing_result{};

	constexpr bool can_continue(http_req_processing_result)
	{ return true; }

	constexpr bool is_error_indicator(http_req_processing_result)
	{ return false; }


	constexpr char const* to_string(http_req_processing_result)
	{ return "No error"; }

	struct http_write_req_result
	{
		size_t bytes_written;
		http_req_processing_result ec;
	};

	struct http_read_resp_result
	{
		size_t bytes_read;
		http_req_processing_result ec;
	};

	inline std::string_view resolve_resource(west::http::uri const& req_target)
	{
		if(req_target == "/favicon.ico")
		{ return "ui/favicon.ico"; }

		if(req_target == "/")
		{ return "ui/mainpage.html"; }

		if(req_target.value().starts_with("/ui/"))
		{ return req_target.value().substr(1); }

		return std::string_view{};
	}

	class resource_server
	{
	public:
		explicit resource_server(resource_info&& res_info, Wad64::InputFile&& input_file):
			m_resource_info{std::move(res_info)},
			m_input_file{std::move(input_file)}
		{}

		auto finalize_state(west::http::field_map& fields) const
		{
			fields.append("Content-Length", std::to_string(m_input_file.size()))
				.append("Content-Type", std::string{m_resource_info.mime_type})
				.append("Last-Modified", to_string(m_resource_info.last_modified));

			west::http::finalize_state_result validation_result{};
			validation_result.http_status = west::http::status::ok;
			return validation_result;
		}

		auto read_response_content(std::span<char> buffer)
		{
			auto const bytes_read = m_input_file.read(std::as_writable_bytes(std::span{std::data(buffer), std::size(buffer)}));

			return http_read_resp_result{
				bytes_read,
				http_req_processing_result{}
			};
		}

	private:
		resource_info m_resource_info;
		Wad64::InputFile m_input_file;
	};

	class json_error_response_server
	{
	public:
		json_error_response_server() = default;

		explicit json_error_response_server(west::http::finalize_state_result const& res):
			m_err_msg{to_string(jopp::container{jopp::to_json(res)})},
			m_response_ptr{std::data(m_err_msg)},
			m_bytes_to_write{std::size(m_err_msg)}
		{ }

		void finalize_state(west::http::field_map& fields) const
		{
			fields.append("Content-Length", std::to_string(m_bytes_to_write))
				.append("Content-Type", "application/json");
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

	private:
		std::string m_err_msg;
		char const* m_response_ptr{nullptr};
		size_t m_bytes_to_write{0};
	};

	class http_service
	{
	public:
		explicit http_service(std::reference_wrapper<resource_file const> res_file):
			m_res_file{res_file}
		{ }

		auto finalize_state(west::http::request_header const& header)
		{
			m_served_resource.reset();
			printf("%s %s\n",
				header.request_line.method.value().data(),
				header.request_line.request_target.value().data());

			if(header.request_line.method == "GET")
			{
				auto const resource_name = resolve_resource(header.request_line.request_target);
				if(std::size(resource_name) == 0)
				{
					west::http::finalize_state_result validation_result;
					validation_result.http_status = west::http::status::not_found;
					validation_result.error_message = west::make_unique_cstr("Resource not found");

					return validation_result;
				}

				auto [res_info, input_file] = m_res_file.get().get_resource(resource_name);
				m_served_resource = resource_server{std::move(res_info), std::move(input_file)};
				west::http::finalize_state_result validation_result{};
				validation_result.http_status = west::http::status::ok;
				return validation_result;
			}

			west::http::finalize_state_result validation_result;
			validation_result.http_status = west::http::status::not_found;
			validation_result.error_message = west::make_unique_cstr("Resource not found");

			return validation_result;
		}

		auto process_request_content(std::span<char const> buffer)
		{
			return http_write_req_result{
				.bytes_written = std::size(buffer),
				.ec = http_req_processing_result{}
			};
		}

		auto finalize_state(west::http::field_map& fields)
		{
			if(m_served_resource.has_value())
			{
				return m_served_resource->finalize_state(fields);
			}

			puts("Finalize state ok");

			fields.append("Content-Length", std::to_string(0))
				.append("Content-Type", "text/plain");

			west::http::finalize_state_result validation_result;
			validation_result.http_status = west::http::status::ok;
			validation_result.error_message = nullptr;
			return validation_result;
		}

		void finalize_state(west::http::field_map& fields, west::http::finalize_state_result const& res)
		{
			m_served_resource.reset();

			m_json_error_resposne = json_error_response_server{res};
			m_json_error_resposne.finalize_state(fields);
		}

		auto read_response_content(std::span<char> buffer)
		{
			if(m_served_resource.has_value())
			{ return m_served_resource->read_response_content(buffer); }
			else
			{ return m_json_error_resposne.read_response_content(buffer); }
		}
	private:
		std::reference_wrapper<resource_file const> m_res_file;

		std::optional<resource_server> m_served_resource;
		json_error_response_server m_json_error_resposne;
	};
}

#endif