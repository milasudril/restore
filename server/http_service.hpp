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

	struct null_server
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
	};

	class resource_server
	{
	public:
		explicit resource_server(Wad64::InputFile&& input_file, resource_info&& res_info):
			m_input_file{std::move(input_file)},
			m_resource_info{std::move(res_info)}
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
		Wad64::InputFile m_input_file;
		resource_info m_resource_info;
	};

	class json_response_server
	{
	public:
		explicit json_response_server(jopp::container const& response):
			m_response{to_string(response)},
			m_response_ptr{std::data(m_response)},
			m_bytes_to_write{std::size(m_response)}
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

	private:
		std::string m_response;
		char const* m_response_ptr{nullptr};
		size_t m_bytes_to_write{0};
	};

	class http_service
	{
		using server = std::variant<null_server, resource_server, json_response_server>;
	public:
		explicit http_service(std::reference_wrapper<resource_file const> res_file):
			m_res_file{res_file}
		{ }

		auto finalize_state(west::http::request_header const& header)
		{
			auto const& req_method = header.request_line.method;
			auto const& req_target = header.request_line.request_target;

			printf("%s %s\n", req_method.value().data(),
				req_target.value().data());


			if(req_target == "/task_parameters")
			{
				puts("Get task_parameters");
			}

			if(req_target == "/parameter_types")
			{
				puts("Get parameter_types");
			}

			if(req_target == "/tasks")
			{
				puts("Get tasks");
			}

			if(req_target.value().starts_with("/tasks/"))
			{
				puts("Manipulate task");
			}


			auto const resource_name = resolve_resource(req_target);
			if(std::size(resource_name) == 0)
			{
				west::http::finalize_state_result validation_result;
				validation_result.http_status = west::http::status::not_found;
				validation_result.error_message = west::make_unique_cstr(header.request_line.request_target.value());

				return validation_result;
			}

			if(req_method != "GET")
			{
				west::http::finalize_state_result validation_result;
				validation_result.http_status = west::http::status::method_not_allowed;
				validation_result.error_message = west::make_unique_cstr(header.request_line.request_target.value());

				return validation_result;
			}

			try
			{
				auto [input_file, file_info] = m_res_file.get().get_resource(resource_name);
				m_current_server = resource_server{std::move(input_file), std::move(file_info)};
				west::http::finalize_state_result validation_result{};
				validation_result.http_status = west::http::status::ok;
				return validation_result;
			}
			catch(std::runtime_error const& err)
			{
				west::http::finalize_state_result validation_result;
				validation_result.http_status = west::http::status::not_found;
				validation_result.error_message = west::make_unique_cstr(err.what());
				return validation_result;
			}
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
			return std::visit([&fields](auto const& server) {
				return server.finalize_state(fields);
			}, m_current_server);
		}

		void finalize_state(west::http::field_map& fields, west::http::finalize_state_result const& res)
		{
			m_current_server = json_response_server{jopp::container{jopp::to_json(res)}};
			std::visit([&fields](auto const& server) {
				return server.finalize_state(fields);
			}, m_current_server);
		}

		auto read_response_content(std::span<char> buffer)
		{
			return std::visit([buffer](auto& server){
				return server.read_response_content(buffer);
			}, m_current_server);
		}

	private:
		std::reference_wrapper<resource_file const> m_res_file;

		server m_current_server;
	};
}

#endif