//@ {"dependencies":[{"ref": "wad64", "origin":"pkg-config"}]}

#ifndef RESTORE_RESOURCE_FILE_HPP
#define RESTORE_RESOURCE_FILE_HPP

#include "./utils.hpp"
#include "./json_loaders.hpp"

#include <jopp/types.hpp>
#include <wad64/fd_owner.hpp>
#include <wad64/readonly_archive.hpp>
#include <wad64/input_file.hpp>

#include <algorithm>

namespace restore
{
	struct resource_info
	{
		jopp::string name;
		jopp::string mime_type;
		std::chrono::system_clock::time_point last_modified;
	};

	inline resource_info create_resource_info(std::string_view name, jopp::object const& obj)
	{
		resource_info ret;
		ret.name = name;
		ret.mime_type = obj.get_field_as<jopp::string>("mime_type");

		auto const last_modified = obj.get_field_as<jopp::number>("last_modified");
		ret.last_modified = std::chrono::system_clock::time_point{
			std::chrono::seconds{static_cast<int64_t>(last_modified)}
		};

		return ret;
	}

	class resource_metadata
	{
	public:
		static constexpr auto pred = jopp::overload{
			[](resource_info const& a, resource_info const& b){
				return a.name < b.name;
			},
			[](resource_info const& a, std::string_view b) {
				return a.name < b;
			},
			[](std::string_view a, resource_info const& b) {
				return a < b.name;
			}
		};

		explicit resource_metadata(jopp::object const& obj)
		{
			std::ranges::transform(obj, std::back_inserter(m_resources), [](auto const& item) {
				auto obj = item.second.template get_if<jopp::object>();
				if(obj == nullptr)
				{ throw std::runtime_error{"Failed to load resource info: object expected"}; }
				return create_resource_info(item.first, *obj);
			});

			std::ranges::sort(m_resources, pred);
		}

		auto const& get_resource_info(std::string_view filename) const
		{
			auto i = binary_find(std::begin(m_resources), std::end(m_resources), filename, pred);
			if(i == std::end(m_resources))
			{ throw std::runtime_error{"The requested resource has no associated metadata"}; }

			return *i;
		}

		auto begin() const { return std::begin(m_resources); }

		auto end() const { return std::begin(m_resources); }

	private:
		std::vector<resource_info> m_resources;
	};

	class resource_file
	{
	public:
		// Because this class is self-referencial
		resource_file(resource_file const&) = delete;
		resource_file(resource_file&&) = delete;
		resource_file& operator=(resource_file const&) = delete;
		resource_file& operator=(resource_file&&) = delete;

		explicit resource_file(char const* filename):
			m_fd{filename, Wad64::IoMode::AllowRead(), Wad64::FileCreationMode::DontCare()},
			m_archive{std::ref(m_fd)},
			m_res_metadata{json::load_object(Wad64::InputFile{m_archive, "file_metadata.json"})}
		{}

		auto get_resource(std::string_view filename) const
		{
			return std::pair{
				Wad64::InputFile{m_archive, filename},
				m_res_metadata.get_resource_info(filename)
			};
		}

		auto const& ls() const
		{ return m_archive.ls(); }

	private:
		Wad64::FdOwner m_fd;
		Wad64::ReadonlyArchive m_archive;
		resource_metadata m_res_metadata;
	};
}

#endif