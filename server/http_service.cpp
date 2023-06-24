//@	{"target":{"name": "./http_service.o"}}

#include "./http_service.hpp"

namespace
{
	auto serve_resource(west::http::request_header const& header, jopp::json_buffer_view content)
	{
		if(header.request_line.method == "GET")
		{
			return std::pair{
				west::http::finalize_state_result {
					.http_status = west::http::status::ok,
					.error_message = nullptr,
				},
				std::optional{restore::cached_json_response_server{content}}
			};
		}
		return std::pair{west::http::finalize_state_result{
			.http_status = west::http::status::method_not_allowed,
			.error_message = west::make_unique_cstr("Endpoint only supports method GET"),
		}, std::optional<restore::cached_json_response_server>{}};
	}
	
	auto serve_task_parameters(west::http::request_header const& header, jopp::json_buffer_view content)
	{ return serve_resource(header, content); }
	
	auto serve_parameter_types(west::http::request_header const& header, jopp::json_buffer_view content)
	{ return serve_resource(header, content); }
}

west::http::finalize_state_result restore::http_service::finalize_state(west::http::request_header const& header)
{
	auto const& req_method = header.request_line.method;
	auto const& req_target = header.request_line.request_target;

	printf("%s %s\n", req_method.value().data(),
		req_target.value().data());

	if(req_target == "/task_parameters")
	{
		auto [retval, server] = serve_task_parameters(header, m_task_params);
		if(server.has_value())
		{ m_current_server = std::move(*server); }
		
		return retval;
	}

	if(req_target == "/parameter_types")
	{
		auto [retval, server] = serve_parameter_types(header, m_param_types);
		if(server.has_value())
		{ m_current_server = std::move(*server); }
		
		return retval;
	}

	if(req_target == "/tasks")
	{
		puts("Get tasks");
		return west::http::finalize_state_result{
			.http_status = west::http::status::not_implemented,
			.error_message = west::make_unique_cstr("Under construction")
		};
	}

	if(req_target.value().starts_with("/tasks/"))
	{
		puts("Manipulate task");
		return west::http::finalize_state_result{
			.http_status = west::http::status::not_implemented,
			.error_message = west::make_unique_cstr("Under construction")
		};
	}

	{
		auto [retval, server] = serve_resource(header, m_res_file);
		if(server.has_value())
		{ m_current_server = std::move(*server); }

		return retval;
	}
}
