//@	{"target":{"name":"restore.o"}}

#include "./config.hpp"
#include "./server_socket.hpp"
#include "./resource_file.hpp"
#include "./http_service.hpp"

#include <west/service_registry.hpp>
#include <west/http_server.hpp>

jopp::object get_parameter_types()
{
	// Set dummy data for testing purpose
	jopp::object ret{};
	{
		jopp::object composite{};
		{
			jopp::object planet{};
			{
				jopp::object fields{};
				{
					jopp::object name{};
					{
						name.insert("display", "inline");
						{
							jopp::object type{};
							type.insert("category", "atom");
							type.insert("name", "string");
							type.insert("input_size", "short");
							name.insert("type", std::move(type));
						}
					}
					fields.insert("name", std::move(name));
				}
				
				{
					jopp::object description{};
					{
						description.insert("display", "block");
						{
							jopp::object type{};
							type.insert("category", "atom");
							type.insert("name", "string");
							type.insert("input_size", "long");
							description.insert("type", std::move(type));
						}
					}
					fields.insert("description", std::move(description));
				}
				
				{
					jopp::object mass{};
					{
						mass.insert("display", "inline");
						{
							jopp::object type{};
							type.insert("category", "atom");
							type.insert("name", "number");
							type.insert("input_size", "short");
							mass.insert("type", std::move(type));
						}
					}
					fields.insert("mass", std::move(mass));
				}
				
				{
					jopp::object radius{};
					{
						radius.insert("display", "inline");
						{
							jopp::object type{};
							type.insert("category", "atom");
							type.insert("name", "number");
							type.insert("input_size", "short");
							radius.insert("type", std::move(type));
						}
					}
					fields.insert("radius", std::move(radius));
				}
				
				{
					jopp::object planet_type{};
					{
						planet_type.insert("display", "inline");
						{
							jopp::object type{};
							type.insert("category", "enum");
							type.insert("name", "planet_type");
							planet_type.insert("type", std::move(type));
						}
					}
					fields.insert("type", std::move(planet_type));
				}
				
				{
					jopp::object has_water{};
					{
						has_water.insert("display", "inline");
						{
							jopp::object type{};
							type.insert("category", "atom");
							type.insert("name", "boolean");
							has_water.insert("type", std::move(type));
						}
					}
					fields.insert("has_water", std::move(has_water));
				}
				
				planet.insert("fields", std::move(fields));
			}
			composite.insert("planet", std::move(planet));
		}
		ret.insert("composite", std::move(composite));
	}
	
	{
		jopp::object enumlist{};
		{
			jopp::object planet_type{};
			{
				jopp::object allowed_values{};
				{
					jopp::object rocky{};
					rocky.insert("display_name", "Rocky");
					allowed_values.insert("rocky", std::move(rocky));
				}
				
				{
					jopp::object gas_giant{};
					gas_giant.insert("display_name", "Gas giant");
					allowed_values.insert("gas_giant", std::move(gas_giant));
				}
				planet_type.insert("allowed_values", std::move(allowed_values));
			}
			enumlist.insert("planet_type", std::move(planet_type));
		}
		ret.insert("enum", std::move(enumlist));
	}
	
	return ret;
}

jopp::object get_task_parameters()
{
#if 0
	"{\
	\"description\": {\
		\"display\": \"block\",\
		\"type\": {\
			\"category\":\"atom\",\
			\"name\": \"string\",\
			\"input_size\": \"long\"\
		}\
	},\
	\"name\": {\
		\"display\": \"inline\",\
		\"type\": {\
			\"category\":\"atom\",\
			\"name\": \"string\",\
			\"input_size\": \"short\"\
		}\
	},\
	\"length\": {\
		\"display\": \"inline\",\
		\"type\": {\
			\"category\":\"atom\",\
			\"name\": \"number\"\
		}\
	},\
	\"home_world\": {\
		\"display\": \"block\",\
		\"type\": {\
			\"category\":\"composite\",\
			\"name\": \"planet\"\
		}\
	}\
}"
#endif
	jopp::object ret{};
	{
		jopp::object description{};
		description.insert("display", "block");
		{
			jopp::object type{};
			type.insert("category", "atom");
			type.insert("name", "string");
			type.insert("input_size", "long");
			description.insert("type", std::move(type));
		}
		ret.insert("description", std::move(description));
	}

	{
		jopp::object name{};
		name.insert("display", "inline");
		{
			jopp::object type{};
			type.insert("category", "atom");
			type.insert("name", "string");
			type.insert("input_size", "short");
			name.insert("type", std::move(type));
		}
		ret.insert("name", std::move(name));
	}
	
	{
		jopp::object length{};
		length.insert("display", "inline");
		{
			jopp::object type{};
			type.insert("category", "atom");
			type.insert("name", "number");
			type.insert("input_size", "short");
			length.insert("type", std::move(type));
		}
		ret.insert("length", std::move(length));
	}
	
	{
		jopp::object home_world{};
		home_world.insert("display", "block");
		{
			jopp::object type{};
			type.insert("category", "composite");
			type.insert("name", "planet");
			home_world.insert("type", std::move(type));
		}
		ret.insert("home_world", std::move(home_world));
	}
	
	return ret;
}

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
	
	jopp::json_buffer param_types{jopp::container{get_parameter_types()}};
	jopp::json_buffer task_params{jopp::container{get_task_parameters()}};

	west::service_registry services{};
	enroll_http_service<restore::http_service>(services,
		std::move(http_socket),
		std::cref(resources),
		std::cref(param_types),
		std::cref(task_params))
		.process_events();


	return 0;
}
