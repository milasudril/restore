//@	{"target":{"name":"restore.o"}}

#include "./config.hpp"
#include "./server_socket_config.hpp"

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

	return 0;
}