//@	{"target":{"name": "./http_service.o"}}

#include "./http_service.hpp"

west::http::finalize_state_result restore::http_service::finalize_state(west::http::request_header const& header)
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

	auto res = serve_resource(header, m_res_file);
	if(res.second.has_value())
	{ m_current_server = std::move(*res.second); }

	return std::move(res.first);
}