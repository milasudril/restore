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

	return [&res_file = m_res_file](west::http::request_header const& header, server& current_server) {
		auto const& req_method = header.request_line.method;
		auto const& req_target = header.request_line.request_target;
		auto const resource_name = resolve_resource(req_target);
		if(std::size(resource_name) == 0)
		{
			west::http::finalize_state_result validation_result;
			validation_result.http_status = west::http::status::not_found;
			validation_result.error_message = west::make_unique_cstr(req_target.value());

			return validation_result;
		}

		if(req_method != "GET")
		{
			west::http::finalize_state_result validation_result;
			validation_result.http_status = west::http::status::method_not_allowed;
			validation_result.error_message = west::make_unique_cstr(req_target.value());

			return validation_result;
		}

		try
		{
			auto [input_file, file_info] = res_file.get().get_resource(resource_name);
			current_server = resource_server{std::move(input_file), std::move(file_info)};
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
	}(header, m_current_server);
}