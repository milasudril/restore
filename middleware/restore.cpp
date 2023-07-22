//@	{"target":{"name":"restore.o"}}

#include "./middleware_instance.hpp"
#include "./server_socket.hpp"
#include "./http_service.hpp"
#include "./dummy_params.hpp"
#include "./message_encoder.hpp"

#include <west/service_registry.hpp>
#include <west/http_server.hpp>
#include <west/io_signal_fd.hpp>

struct signal_handler
{
	void fd_is_ready(auto event_monitor, west::io::fd_ref) const
	{ event_monitor.clear(); }

	void fd_is_idle(auto, west::io::fd_ref) const
	{}
};

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "Config file not specified\n");
		return -1;
	}

	auto const cfg = restore::json::load_object(argv[1]);
	auto const& http_server_socket_cfg = cfg.get_field_as<jopp::object>("http_socket");
	auto http_socket = restore::create_server_socket(http_server_socket_cfg);

	auto const& mw_config = cfg.get_field_as<jopp::object>("middleware_instance");
	jopp::object task_params_response;
	auto mw_instance = restore::create_middleware_instance(mw_config,
		restore::task_metadata{
			.compound_types = jopp::json_buffer{restore::wrap_in_message(restore::get_parameter_types())},
			.parameters = jopp::json_buffer{restore::wrap_in_message(restore::get_task_parameters())},
			.factory = restore::create_task<restore::dummy_task>
		});

	west::service_registry services{};
	enroll_http_service<restore::http_service>(services, std::move(http_socket), std::ref(mw_instance))
		.enroll(west::io::signal_fd{west::io::make_sigmask(SIGINT, SIGTERM)}, signal_handler{});

	// TODO: replace localhost with something more generig (ip address or host name)
	printf("The Restore application is available from http://localhost:%u. You may manage tasks by "
		"opening this URL in your favorite browser. Using CTRL+LMB works in most graphical TTY:s. Your "
		"key for this session is\n%s\n", http_socket.port(), mw_instance.session_key.c_str());
	services.process_events();

	return 0;
}
