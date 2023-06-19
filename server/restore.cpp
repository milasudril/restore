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
	auto const& socket_cfg = cfg.get_field_as<jopp::object>("socket_config");
	auto http_socket = restore::create_server_socket(socket_cfg);

	printf("Listening on port %u\n", http_socket.port());
	fflush(stdout);

	return 0;
}