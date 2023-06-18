//@	{"target":{"name":"main.o"}}

#include <west/io_inet_server_socket.hpp>
#include <west/service_registry.hpp>
#include <west/http_request_handler.hpp>
#include <west/http_session_factory.hpp>
#include <west/http_server.hpp>

namespace
{
	// Some stuff needed to convey processing results back to the framework

	enum class request_handler_error_code{no_error};

	struct read_result
	{
		size_t bytes_read;
		request_handler_error_code ec;
	};

	constexpr bool can_continue(request_handler_error_code ec)
	{
		switch(ec)
		{
			case request_handler_error_code::no_error:
				return true;
			default:
				__builtin_unreachable();
		}
	}

	constexpr bool is_error_indicator(request_handler_error_code ec)
	{
		switch(ec)
		{
			case request_handler_error_code::no_error:
				return false;
			default:
				__builtin_unreachable();
		}
	}

	constexpr char const* to_string(request_handler_error_code ec)
	{
		switch(ec)
		{
			case request_handler_error_code::no_error:
				return "No error";
			default:
				__builtin_unreachable();
		}
	}

	struct request_handler_write_result
	{
		size_t bytes_written;
		request_handler_error_code ec;
	};

	struct request_handler_read_result
	{
		size_t bytes_read;
		request_handler_error_code ec;
	};


	// The actual request handler. This class has all entry points for the application

	class your_request_handler
	{
	public:
		your_request_handler(/* request-handler-args */)
		{
			// Called per connection
		}

		auto finalize_state(west::http::request_header const&)
		{
			// This function is called after the request header has been read. If you
			// had any state left over from a previous request like temporary buffers,
			// you may need to clear it in this function.
			//
			// `header` contains the request header sent by the client.
			// It contains the method, request target and all fields. If you
			// want to serve files, you should map header.request_target
			// to the corresponding file.

			// Return the result of this state
			west::http::finalize_state_result validation_result;
			validation_result.http_status = west::http::status::ok;
			validation_result.error_message = nullptr;

			return validation_result;
		}

		auto process_request_content(std::span<char const> buffer)
		{
			// This function may be called multiple times
			//
			// `buffer` always contains the lastly read portion of the request body.
			//
			return request_handler_write_result{
				.bytes_written = std::size(buffer),
				.ec = request_handler_error_code::no_error
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

		auto read_response_content(std::span<char>)
		{
			// This function will be called multiple times
			// Copy appropriate content to `buffer`

			return request_handler_read_result{
				0,
				request_handler_error_code::no_error
			};
		}

	private:
	// Session specific data
	};
}

int main()
{
	// Create a server socket listening for connections
	// on localhost. Choose first free port in range [49152, 65535].
	west::io::inet_server_socket http{
		west::io::inet_address{"127.0.0.1"},
		std::ranges::iota_view{49152, 65536},
		128
	};

	printf("Listening on port %u\n", http.port());
	fflush(stdout);

	// Register the service and start processing events
	west::service_registry services{};
	enroll_http_service<your_request_handler>(services, std::move(http))
		.process_events();
}