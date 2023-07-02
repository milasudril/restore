//@	{"target":{"name":"restore.o"}}

#include "./server_socket.hpp"
#include "./resource_file.hpp"
#include "./http_service.hpp"
#include "./storage_file.hpp"
#include "./dummy_params.hpp"

#include <west/service_registry.hpp>
#include <west/http_server.hpp>
#include <west/io_signal_fd.hpp>

#include <thread>

struct signal_handler
{
	void fd_is_ready(auto event_monitor, west::io::fd_ref) const
	{ event_monitor.clear(); }

	void fd_is_idle(auto, west::io::fd_ref) const
	{}
};

std::string generate_session_key()
{
	auto random = west::io::open("/dev/urandom", 0);
	std::array<char, 32> buffer{};
	if(::read(random.get(), std::data(buffer), std::size(buffer)) != 32)
	{ throw std::runtime_error{"Failed to generate a session key"}; }

	auto to_hex_digit = [](auto nibble) {
		return static_cast<char>(nibble < 10 ? nibble + '0' : (nibble - 10 ) + 'A');
	};

	std::string ret{};

	for(size_t k = 0; k != std::size(buffer); ++k)
	{
		auto const msb = (0xf0 & buffer[k]) >> 4;
		auto const lsb = 0x0f & buffer[k];

		ret.push_back(to_hex_digit(msb));
		ret.push_back(to_hex_digit(lsb));
	}

	return ret;
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
	restore::resource_file resources{cfg.get_field_as<jopp::string>("resource_file").c_str()};
	restore::storage_file storage{cfg.get_field_as<jopp::string>("storage_file").c_str()};

	jopp::json_buffer param_types{restore::get_parameter_types()};
	jopp::json_buffer task_params{restore::get_task_parameters()};

	auto const session_key = generate_session_key();

	west::service_registry services{};
	enroll_http_service<restore::http_service>(services,
		std::move(http_socket),
		std::cref(resources),
		std::ref(storage),
		std::string_view{session_key},
		std::cref(param_types),
		std::cref(task_params))
		.enroll(west::io::signal_fd{west::io::make_sigmask(SIGINT, SIGTERM)}, signal_handler{});

	// TODO: replace localhost with something more generig (ip address or host name)
	printf("The Restore application is available from http://localhost:%u. You may manage tasks by "
		"opening this URL in your favorite browser. Using CTRL+LMB works in most graphical TTY:s. Your "
		"key for this session is\n%s\n", http_socket.port(), session_key.c_str());
	services.process_events();

	return 0;
}
