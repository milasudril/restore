#ifndef RESTORE_HTTP_SERVICE_HPP
#define RESTORE_HTTP_SERVICE_HPP

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

	template<class ResourceFile>
	class http_service
	{
	public:
		explicit http_service(std::reference_wrapper<ResourceFile const> res_file):
			m_res_file{res_file}
		{}

		auto finalize_state(west::http::request_header const& header)
		{
			printf("%s %s\n",
				header.request_line.method.value().data(),
				header.request_line.request_target.value().data());

			west::http::finalize_state_result validation_result;
			validation_result.http_status = west::http::status::not_found;

			std::string errmsg{"Unknown resource `"};
			errmsg.append(header.request_line.request_target.value())
				.append("`");
			validation_result.error_message = west::make_unique_cstr(errmsg.c_str());

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
			puts("Finalize state failed");
			auto const errmsg = jopp::to_json(res);
			fields.append("Content-Length", std::to_string(0))
				.append("Content-Type", "text/plain");
		}

		auto read_response_content(std::span<char> buffer)
		{
			return http_read_resp_result{
				std::size(buffer),
				http_req_processing_result{}
			};
		}
	private:
		std::reference_wrapper<ResourceFile const> m_res_file;
		std::string m_err_msg;
	};
}

#endif