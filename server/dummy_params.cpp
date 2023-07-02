//@	{"target":{"name":"dummy_params.o"}}

#include "./dummy_params.hpp"

jopp::object restore::get_parameter_types()
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

jopp::object restore::get_task_parameters()
{
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