//@	{"target":{"name":"name_to_fd_map.o"}}

#include "./name_to_fd_map.hpp"
#include "./utils.hpp"

#include <west/utils.hpp>

restore::name_to_fd_map::name_to_fd_map(jopp::object const& blobs, char const* tempdir)
{
	m_data.reserve(std::size(blobs));
	for(auto const& i : blobs)
	{
		auto const val = i.second.get_if<jopp::object>();
		if(val == nullptr)
		{
			std::string errmesg{"Blob descriptor `"};
			errmesg.append(i.first)
				.append("` is not an object");

			throw std::runtime_error{errmesg};
		}

		std::vector<std::string> json_path;
		for(auto const& item : val->get_field_as<jopp::array>("json_path"))
		{
			auto str = item.get_if<jopp::string>();
			if(str == nullptr)
			{ throw std::runtime_error{"Expected json path element to be a string"}; }

			json_path.push_back(*str);
		}

		m_data.push_back(restore::name_and_fd{
			.name = i.first,
			.fd = west::io::open(tempdir, O_TMPFILE|O_RDWR, S_IRUSR|S_IWUSR),
			.json_path = std::move(json_path)
		});
	}

	// NOTE: Since jopp sorts elements by key, we do not need to sort m_data
}

restore::name_to_fd_map::const_iterator
restore::name_to_fd_map::find(std::string_view name) const
{
	auto const i = binary_find(std::begin(m_data),
		std::end(m_data),
		name,
		west::overload{
			[](std::string_view a, name_and_fd const& b){
				return a < b.name;
			},
			[](name_and_fd const& a, std::string_view b){
				return a.name < b;
			}
		}
	);
	return i;
}