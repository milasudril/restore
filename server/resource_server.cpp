//@	{"target":{"name":"resource_server.o"}}

#include "./resource_server.hpp"

std::string_view restore::resolve_resource(west::http::uri const& req_target)
{
	if(req_target == "/favicon.ico")
	{ return "ui/favicon.ico"; }

	if(req_target == "/")
	{ return "ui/mainpage.html"; }

	if(req_target.value().starts_with("/ui/"))
	{ return req_target.value().substr(1); }

	return std::string_view{};
}

std::pair<west::http::finalize_state_result,
	std::optional<restore::resource_server>>
restore::serve_resource(west::http::request_header const& header,
	std::reference_wrapper<resource_file const> res_file)
{
	auto const& req_method = header.request_line.method;
	auto const& req_target = header.request_line.request_target;
	auto const resource_name = resolve_resource(req_target);
	if(std::size(resource_name) == 0)
	{
		west::http::finalize_state_result validation_result;
		validation_result.http_status = west::http::status::not_found;
		validation_result.error_message = west::make_unique_cstr(req_target.value());

		return std::pair{std::move(validation_result), std::optional<resource_server>{}};
	}

	if(req_method != "GET")
	{
		west::http::finalize_state_result validation_result;
		validation_result.http_status = west::http::status::method_not_allowed;
		validation_result.error_message = west::make_unique_cstr(req_target.value());

		return std::pair{std::move(validation_result), std::optional<resource_server>{}};
	}

	try
	{
		auto [input_file, file_info] = res_file.get().get_resource(resource_name);
		west::http::finalize_state_result validation_result{};
		validation_result.http_status = west::http::status::ok;
		return std::pair{std::move(validation_result), resource_server{std::move(input_file), std::move(file_info)}};
	}
	catch(std::runtime_error const& err)
	{
		west::http::finalize_state_result validation_result;
		validation_result.http_status = west::http::status::not_found;
		validation_result.error_message = west::make_unique_cstr(err.what());
		return std::pair{std::move(validation_result), std::optional<resource_server>{}};
	}
}