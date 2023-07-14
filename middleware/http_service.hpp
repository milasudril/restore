//@	{"dependencies_extra":[{"ref":"./http_service.o", "rel":"implementation"}]}

#ifndef RESTORE_HTTP_SERVICE_HPP
#define RESTORE_HTTP_SERVICE_HPP

#include "./middleware_instance.hpp"

#include "./resource_server.hpp"
#include "./null_server.hpp"
#include "./json_response_server.hpp"
#include "./cached_json_response_server.hpp"
#include "./login_server.hpp"
#include "./redirect_server.hpp"
#include "./create_task_server.hpp"
#include "./clone_task_server.hpp"
#include "./set_task_running_status_server.hpp"

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
	using server_type = std::variant<null_server,
		resource_server,
		json_response_server,
		cached_json_response_server,
		login_server,
		redirect_server,
		create_task_server,
		clone_task_server,
		set_task_running_status_server>;

	class http_service
	{
	public:
		explicit http_service(middleware_instance& mw_instance):m_mw_instance{mw_instance}
		{ }

		west::http::finalize_state_result finalize_state(west::http::request_header const& header);

		auto process_request_content(std::span<char const> buffer, size_t bytes_to_read)
		{
			return std::visit([buffer, bytes_to_read](auto& server){
				return server.process_request_content(buffer, bytes_to_read);
			}, m_current_server);
		}

		auto finalize_state(west::http::field_map& fields)
		{
			return std::visit([&fields](auto& server) {
				return server.finalize_state(fields);
			}, m_current_server);
		}

		void finalize_state(west::http::field_map& fields, west::http::finalize_state_result const& res)
		{
			m_current_server = json_response_server{jopp::to_json(res)};
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
		std::reference_wrapper<middleware_instance> m_mw_instance;

		server_type m_current_server;
	};
}

#endif