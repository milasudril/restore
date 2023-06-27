//@	{"dependencies_extra":[{"ref":"./http_service.o", "rel":"implementation"}]}

#ifndef RESTORE_HTTP_SERVICE_HPP
#define RESTORE_HTTP_SERVICE_HPP

#include "./json_loaders.hpp"
#include "./resource_file.hpp"
#include "./storage_file.hpp"
#include "./resource_server.hpp"
#include "./null_server.hpp"
#include "./json_response_server.hpp"
#include "./cached_json_response_server.hpp"
#include "./login_request_server.hpp"

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
	class http_service
	{
		using server = std::variant<null_server, resource_server, json_response_server, cached_json_response_server, login_request_server>;
	public:
		explicit http_service(std::reference_wrapper<resource_file const> res_file,
			std::reference_wrapper<storage_file> storage,
			std::string_view session_key,
			jopp::json_buffer_view param_types,
			jopp::json_buffer_view task_params):
			m_res_file{res_file},
			m_storage_file{storage},
			m_session_key{session_key},
			m_param_types{param_types},
			m_task_params{task_params}
		{ }

		west::http::finalize_state_result finalize_state(west::http::request_header const& header);

		auto process_request_content(std::span<char const> buffer)
		{
			return http_write_req_result{
				.bytes_written = std::size(buffer),
				.ec = http_req_processing_result{}
			};
		}

		auto finalize_state(west::http::field_map& fields)
		{
			return std::visit([&fields](auto& server) {
				return server.finalize_state(fields);
			}, m_current_server);
		}

		void finalize_state(west::http::field_map& fields, west::http::finalize_state_result const& res)
		{
			m_current_server = json_response_server{jopp::container{jopp::to_json(res)}};
			std::visit([&fields](auto& server) {
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
		std::reference_wrapper<storage_file> m_storage_file;
		std::string_view m_session_key;
 		jopp::json_buffer_view m_param_types;
 		jopp::json_buffer_view m_task_params;

		server m_current_server;
	};
}

#endif