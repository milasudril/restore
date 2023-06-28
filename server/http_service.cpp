//@	{"target":{"name": "./http_service.o"}}

#include "./http_service.hpp"

#include <west/http_utils.hpp>

namespace
{
	auto serve_mainpage(west::http::request_header const& header)
	{
		if(header.request_line.method != "GET")
		{
			return std::pair{west::http::finalize_state_result{
				.http_status = west::http::status::method_not_allowed,
				.error_message = west::make_unique_cstr("Endpoint only supports method GET"),
			}, restore::server_type{}};
		}

		// TODO: If not logged in, redirect to login page
		return std::pair{
			west::http::finalize_state_result {
				.http_status = west::http::status::ok,
				.error_message = nullptr,
			},
			restore::server_type{restore::redirect_server{"/ui/restricted/mainpage.html"}}
		};
	}

	auto serve_resource(west::http::request_header const& header, jopp::json_buffer_view content)
	{
		if(header.request_line.method != "GET")
		{
			return std::pair{west::http::finalize_state_result{
				.http_status = west::http::status::method_not_allowed,
				.error_message = west::make_unique_cstr("Endpoint only supports method GET"),
			}, restore::server_type{}};
		}

		return std::pair{
			west::http::finalize_state_result {
				.http_status = west::http::status::ok,
				.error_message = nullptr,
			},
			restore::server_type{restore::cached_json_response_server{content}}
		};
	}

	auto serve_task_parameters(west::http::request_header const& header, jopp::json_buffer_view content)
	{ return serve_resource(header, content); }

	auto serve_parameter_types(west::http::request_header const& header, jopp::json_buffer_view content)
	{ return serve_resource(header, content); }

	auto serve_login_request(west::http::request_header const& header, std::string_view session_key)
	{
		if(header.request_line.method != "POST")
		{
			return std::pair{
				west::http::finalize_state_result{
					.http_status = west::http::status::method_not_allowed,
					.error_message = west::make_unique_cstr("Endpoint only supports method POST"),
				},
				std::optional<restore::login_request_server>{}
			};
		}

		{
			auto const i = header.fields.find("content-type");
			if(i == std::end(header.fields))
			{
				return std::pair{
					west::http::finalize_state_result{
						.http_status = west::http::status::bad_request,
						.error_message = west::make_unique_cstr("Content-Type is missing"),
					},
					std::optional<restore::login_request_server>{}
				};
			}

			if(i->second != "application/json;charset=UTF-8")
			{
				return std::pair{
					west::http::finalize_state_result{
						.http_status = west::http::status::bad_request,
						.error_message = west::make_unique_cstr("Bad content-type"),
					},
					std::optional<restore::login_request_server>{}
				};
			}
		}

		{
			auto const i = header.fields.find("accept");
			if(i == std::end(header.fields))
			{
				return std::pair{
					west::http::finalize_state_result{
						.http_status = west::http::status::bad_request,
						.error_message = west::make_unique_cstr("Accept is missing"),
					},
					std::optional<restore::login_request_server>{}
				};
			}

			if(i->second != "application/json")
			{
				return std::pair{
					west::http::finalize_state_result{
						.http_status = west::http::status::bad_request,
						.error_message = west::make_unique_cstr("This endpont will only return a response in JSON format"),
					},
					std::optional<restore::login_request_server>{}
				};
			}
		}

		return std::pair{
			west::http::finalize_state_result{
				.http_status = west::http::status::ok,
				.error_message = nullptr,
			},
			std::optional{restore::login_request_server{session_key}}
		};
	}

	std::string_view resolve_resource(west::http::uri const& req_target)
	{
		if(req_target == "/favicon.ico")
		{ return "ui/public/favicon.ico"; }

		if(req_target.value().starts_with("/ui/"))
		{ return req_target.value().substr(1); }

		return std::string_view{};
	}

	auto login_is_valid(west::http::request_header const& header,
		std::string_view session_key)
	{
		auto i = header.fields.find("Cookie");
		if(i == std::end(header.fields))
		{ return false; }

		west::http::cookie_store cookies;
		auto const res = west::http::parse_cookie_string(i->second, cookies);
		if(res.ec != west::http::cookie_string_parser_error_code::no_error)
		{ return false; }

		auto const keyval = cookies.find("session_key");
		if(keyval == std::end(cookies))
		{ return false; }

		return keyval->second == session_key;
	}

	auto serve_resource(west::http::request_header const& header,
		std::reference_wrapper<restore::resource_file const> res_file,
		std::string_view resource_name,
		std::string_view session_key)
	{
		if(header.request_line.method != "GET")
		{
			return std::pair {
				west::http::finalize_state_result{
					.http_status = west::http::status::method_not_allowed,
					.error_message = west::make_unique_cstr(header.request_line.request_target.value())
				},
				restore::server_type{}
			};
		}

		try
		{
			if(!resource_name.starts_with("ui/public") && !login_is_valid(header, session_key))
			{
				return std::pair{
					west::http::finalize_state_result{},
					restore::server_type{
						restore::redirect_server{
							"/ui/public/login.html",
							header.request_line.request_target.value(),
							west::http::status::see_other
						}
					}
				};
			}
			auto [file, file_info] = res_file.get().get_resource(resource_name);

			return std::pair{
				west::http::finalize_state_result{},
				restore::server_type{
					restore::resource_server{
						std::move(file),
						std::move(file_info)
					}
				}
			};
		}
		catch(std::runtime_error const& err)
		{
			return std::pair{
				west::http::finalize_state_result{
					.http_status = west::http::status::not_found,
					.error_message = west::make_unique_cstr(err.what())
				},
				restore::server_type{}
			};
		}
	}
}

west::http::finalize_state_result restore::http_service::finalize_state(west::http::request_header const& header)
{
	auto const& req_target = header.request_line.request_target;

	if(req_target == "/")
	{
		auto [retval, server] = serve_mainpage(header);
		m_current_server = std::move(server);
		return retval;
	}

	if(req_target == "/login")
	{
		auto [retval, server] = serve_login_request(header, m_session_key);
		if(server.has_value())
		{ m_current_server = std::move(*server); }

		return retval;
	}

	if(req_target == "/task_parameters")
	{
		auto [retval, server] = serve_task_parameters(header, m_task_params);
		m_current_server = std::move(server);
		return retval;
	}

	if(req_target == "/parameter_types")
	{
		auto [retval, server] = serve_parameter_types(header, m_param_types);
		m_current_server = std::move(server);
		return retval;
	}

	if(req_target == "/tasks")
	{
		puts("Get tasks");
		m_current_server = null_server{};
		return west::http::finalize_state_result{
			.http_status = west::http::status::not_implemented,
			.error_message = west::make_unique_cstr("Under construction")
		};
	}

	if(req_target.value().starts_with("/tasks/"))
	{
		puts("Manipulate task");
		m_current_server = null_server{};
		return west::http::finalize_state_result{
			.http_status = west::http::status::not_implemented,
			.error_message = west::make_unique_cstr("Under construction")
		};
	}

	if(auto res_name = resolve_resource(req_target); !res_name.empty())
	{
		auto [retval, server] = serve_resource(header, m_res_file, res_name, m_session_key);
		m_current_server = std::move(server);
		return retval;
	}

	m_current_server = null_server{};
	return west::http::finalize_state_result{
		.http_status = west::http::status::not_found,
		.error_message = west::make_unique_cstr("No such API endpoint")
	};
}
