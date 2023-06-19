#ifndef RESTORE_HTTP_SERVICE_HPP
#define RESTORE_HTTP_SERVICE_HPP

#include <west/http_message_header.hpp>
#include <west/http_request_handler.hpp>

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
			validation_result.http_status = west::http::status::ok;
			validation_result.error_message = nullptr;

			return validation_result;
		}

		auto process_request_content(std::span<char const> buffer)
		{
			return http_write_req_result{
				.bytes_written = std::size(buffer),
				.ec = http_req_processing_result{}
			};
		}

		auto finalize_state(west::http::field_map&)
		{
#if 0
			// This function is called after the request body has been processed. It is time
			// to prepare for the answer. You should append Content-Length and Content-Type to
			// `fields`

			fields.append("Content-Length", std::to_string(std::size( /* content-size */ )))
				.append("Content-Type", /*content-type*/ );
#endif

			west::http::finalize_state_result validation_result;
			validation_result.http_status = west::http::status::ok;
			validation_result.error_message = nullptr;
			return validation_result;
		}

		void finalize_state(west::http::field_map&, west::http::finalize_state_result&&)
		{
#if 0
			// This function is called in case of an error during the processing of the request. `res`
			// contains information about the error

			fields.append("Content-Length", std::to_string(std::size( /* content-size */ )))
				.append("Content-Type", /* content-type */);
#endif
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
	};
}

#endif