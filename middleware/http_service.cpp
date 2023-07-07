//@	{"target":{"name": "./http_service.o"}}

#include "./http_service.hpp"

#include <west/http_utils.hpp>

#include <ranges>

namespace
{
	enum class session_status{logged_in, logged_out};

	auto serve_mainpage(west::http::request_header const& header)
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
			restore::server_type{restore::redirect_server{"/ui/restricted/mainpage.html"}}
		};
	}

	auto serve_resource(west::http::request_header const& header,
		jopp::json_buffer_view content,
		enum session_status session_status)
	{
		if(session_status != session_status::logged_in)
		{
			return std::pair{west::http::finalize_state_result{
				.http_status = west::http::status::unauthorized,
				.error_message = west::make_unique_cstr(header.request_line.request_target.value()),
			}, restore::server_type{}};
		}

		if(header.request_line.method != "GET")
		{
			return std::pair{west::http::finalize_state_result{
				.http_status = west::http::status::method_not_allowed,
				.error_message = west::make_unique_cstr(header.request_line.request_target.value()),
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

	auto serve_task_parameters(west::http::request_header const& header,
		jopp::json_buffer_view content,
		enum session_status session_status)
	{ return serve_resource(header, content, session_status); }

	auto serve_parameter_types(west::http::request_header const& header,
		jopp::json_buffer_view content,
		enum session_status session_status)
	{ return serve_resource(header, content, session_status); }

	auto serve_task(west::http::request_header const& header,
		restore::task_registry& tasks,
		enum session_status session_status)
	{
		if(session_status != session_status::logged_in)
		{
			return std::pair{
				west::http::finalize_state_result{
					.http_status = west::http::status::unauthorized,
					.error_message = west::make_unique_cstr(header.request_line.request_target.value())
				},
				restore::server_type{}
			};
		}

		auto const uri = header.request_line.request_target.value().substr(strlen("/tasks/"));
		auto const task_name_end = std::ranges::find(uri, '/');
		auto task_name = west::http::decode_uri_component(std::string_view{std::begin(uri), task_name_end});
		auto const endpoint = std::string_view{task_name_end, std::end(uri)};

		if(endpoint.empty())
		{
			if(header.request_line.method == "DELETE")
			{
				if(!tasks.delete_task(task_name))
				{
					return std::pair{
						west::http::finalize_state_result{
							.http_status = west::http::status::not_found,
							.error_message = west::make_unique_cstr(header.request_line.request_target.value())
						},
						restore::server_type{}
					};
				}

				jopp::object ret{};
				ret.insert("result", "successful");
				return std::pair{
					west::http::finalize_state_result{},
					restore::server_type{restore::json_response_server{ret}}
				};
			}

			if(header.request_line.method == "POST")
			{
				return std::pair{
					west::http::finalize_state_result{},
					restore::server_type{restore::clone_task_server{tasks, std::move(task_name)}}
				};
			}

			return std::pair{west::http::finalize_state_result{
				.http_status = west::http::status::method_not_allowed,
				.error_message = west::make_unique_cstr(header.request_line.request_target.value()),
			}, restore::server_type{}};
		}

		if(endpoint == "/parameters")
		{
			if(header.request_line.method == "GET")
			{
				try
				{
					return std::pair{
						west::http::finalize_state_result{},
						restore::server_type{
							restore::resource_server{
								tasks.get_parameter_file(task_name),
								restore::resource_info{
									.name = std::string{},
									.mime_type = "application/json",
									.last_modified = std::nullopt
								}
							}
						}
					};
				}
				catch(std::exception const& err)
				{
					return std::pair{
						west::http::finalize_state_result{
							.http_status = west::http::status::internal_server_error,
							.error_message = west::make_unique_cstr(err.what())
						},
						restore::server_type{}
					};
				}
			}
			else
			{
				return std::pair{west::http::finalize_state_result{
					.http_status = west::http::status::method_not_allowed,
					.error_message = west::make_unique_cstr(header.request_line.request_target.value())
				}, restore::server_type{}};
			}
		}

		if(endpoint == "/running_status")
		{
			if(header.request_line.method == "GET")
			{
				auto const& tasklist = tasks.get_tasks();
				auto const i = tasklist.find(task_name);
				if(i == std::end(tasklist))
				{
					return std::pair{
						west::http::finalize_state_result{
							.http_status = west::http::status::not_found,
							.error_message = west::make_unique_cstr(header.request_line.request_target.value())
						},
						restore::server_type{}
					};
				}

				jopp::object ret{};
				ret.insert("value", to_string(i->second.running_status()));
				return std::pair{
					west::http::finalize_state_result{},
					restore::server_type{restore::json_response_server{std::move(ret)}}
				};
			}

			if(header.request_line.method == "PUT")
			{
				return std::pair{
					west::http::finalize_state_result{},
					restore::server_type{restore::set_task_running_status_server{tasks, std::move(task_name)}}
				};
			}

			return std::pair{
				west::http::finalize_state_result{
					.http_status = west::http::status::method_not_allowed,
					.error_message = west::make_unique_cstr(header.request_line.request_target.value())
				},
				restore::server_type{}
			};
		}

		return std::pair{
			west::http::finalize_state_result{
				.http_status = west::http::status::not_found,
				.error_message = west::make_unique_cstr(header.request_line.request_target.value())
			},
			restore::server_type{}
		};
	}

	auto serve_tasks(west::http::request_header const& header,
		restore::task_registry& tasks,
		enum session_status session_status)
	{
		if(session_status != session_status::logged_in)
		{
			return std::pair{
				west::http::finalize_state_result{
					.http_status = west::http::status::unauthorized,
					.error_message = west::make_unique_cstr(header.request_line.request_target.value())
				},
				restore::server_type{}
			};
		}

		if(header.request_line.method == "GET")
		{
			return std::pair{
				west::http::finalize_state_result{},
				restore::server_type{restore::json_response_server{get_entries_as_json(tasks)}}
			};
		}

		if(header.request_line.method == "POST")
		{
			return std::pair{
				west::http::finalize_state_result{},
				restore::server_type{restore::create_task_server{tasks}}
			};
		}

		return std::pair{west::http::finalize_state_result{
			.http_status = west::http::status::method_not_allowed,
			.error_message = west::make_unique_cstr(header.request_line.request_target.value()),
		}, restore::server_type{}};
	}

	auto serve_login_request(west::http::request_header const& header,
		std::string_view session_key,
		enum session_status session_status)
	{
		if(header.request_line.method == "GET")
		{
			jopp::object result{};
			if(session_status == session_status::logged_in)
			{ result.insert("session_status", "logged_in"); }
			else
			{ result.insert("session_status", "looged_out"); }

			return std::pair{
				west::http::finalize_state_result{},
				restore::server_type{restore::json_response_server{result}}
			};
		}

		if(header.request_line.method == "POST")
		{
			return std::pair{
				west::http::finalize_state_result{
					.http_status = west::http::status::ok,
					.error_message = nullptr,
				},
				restore::server_type{restore::login_server{session_key}}
			};
		}

		return std::pair{west::http::finalize_state_result{
			.http_status = west::http::status::method_not_allowed,
			.error_message = west::make_unique_cstr(header.request_line.request_target.value()),
		}, restore::server_type{}};
	}

	std::string_view resolve_resource(west::http::uri const& req_target)
	{
		if(req_target == "/favicon.ico")
		{ return "ui/public/favicon.ico"; }

		if(req_target.value().starts_with("/ui/public/") || req_target.value().starts_with("/ui/restricted/"))
		{ return req_target.value().substr(1); }

		return std::string_view{};
	}

	auto serve_resource(west::http::request_header const& header,
		std::reference_wrapper<restore::resource_file const> res_file,
		std::string_view resource_name,
		enum session_status session_status)
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
			if(!resource_name.starts_with("ui/public/") && session_status != session_status::logged_in)
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

	auto get_cookies(west::http::request_header const& header)
	{
		auto i = header.fields.find("Cookie");
		if(i == std::end(header.fields))
		{ return west::http::cookie_store{}; }

		west::http::cookie_store cookies;
		auto const res = west::http::parse_cookie_string(i->second, cookies);
		if(res.ec != west::http::cookie_string_parser_error_code::no_error)
		{ return west::http::cookie_store{}; }

		return cookies;
	}

	auto get_session_status(west::http::cookie_store const& cookies, std::string_view session_key)
	{
		auto const keyval = cookies.find("session_key");
		if(keyval == std::end(cookies))
		{ return session_status::logged_out; }

		return keyval->second == session_key? session_status::logged_in : session_status::logged_out;
	}
}

west::http::finalize_state_result restore::http_service::finalize_state(west::http::request_header const& header)
{
	auto const session_cookies = get_cookies(header);
	auto const session_status = get_session_status(session_cookies, m_mw_instance.get().session_key);
	auto const& req_target = header.request_line.request_target;

	if(req_target == "/")
	{
		auto [retval, server] = serve_mainpage(header);
		m_current_server = std::move(server);
		return retval;
	}

	if(req_target == "/login")
	{
		auto [retval, server] = serve_login_request(header, m_mw_instance.get().session_key, session_status);
		m_current_server = std::move(server);
		return retval;
	}

	if(req_target == "/task_parameters")
	{
		auto [retval, server] = serve_task_parameters(header,
			jopp::json_buffer_view{m_mw_instance.get().taskinfo.parameters},
			session_status);
		m_current_server = std::move(server);
		return retval;
	}

	if(req_target == "/parameter_types")
	{
		auto [retval, server] = serve_parameter_types(header,
			jopp::json_buffer_view{m_mw_instance.get().taskinfo.parameter_types},
			session_status);
		m_current_server = std::move(server);
		return retval;
	}

	if(req_target == "/tasks")
	{
		auto [retval, server] = serve_tasks(header, m_mw_instance.get().tasks, session_status);
		m_current_server = std::move(server);
		return retval;
	}

	if(req_target.value().starts_with("/tasks/"))
	{
		auto [retval, server] = serve_task(header, m_mw_instance.get().tasks, session_status);
		m_current_server = std::move(server);
		return retval;
	}

	if(auto res_name = resolve_resource(req_target); !res_name.empty())
	{
		auto [retval, server] = serve_resource(header, m_mw_instance.get().resource_file, res_name, session_status);
		m_current_server = std::move(server);
		return retval;
	}

	m_current_server = null_server{};
	return west::http::finalize_state_result{
		.http_status = west::http::status::not_found,
		.error_message = west::make_unique_cstr("No such API endpoint")
	};
}
