//@	{"target":{"name":"restore.o"}}

#include "./config.hpp"
#include "./server_socket.hpp"
#include "./resource_file.hpp"
#include "./http_service.hpp"

#include <west/service_registry.hpp>
#include <west/http_server.hpp>

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "Config file not specified\n");
		return -1;
	}

	auto const cfg = restore::config::load(argv[1]);
	auto const& http_cfg = cfg.get_field_as<jopp::object>("http_server");
	auto const& http_server_socket_cfg = http_cfg.get_field_as<jopp::object>("socket");
	auto http_socket = restore::create_server_socket(http_server_socket_cfg);

	printf("Listening on port %u\n", http_socket.port());
	fflush(stdout);

	auto const& website_cfg = cfg.get_field_as<jopp::object>("website");
	restore::resource_file resources{website_cfg.get_field_as<jopp::string>("resource_file").c_str()};

	west::service_registry services{};
	using http_service = restore::http_service<restore::resource_file>;
	enroll_http_service<http_service>(services, std::move(http_socket), std::cref(resources))
		.process_events();


	return 0;
}