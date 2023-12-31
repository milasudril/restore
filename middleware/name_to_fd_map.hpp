//@	{"dependencies_extra":[{"ref":"./name_to_fd_map.o", "rel":"implementation"}]}

#ifndef RESTORE_NAME_TO_FD_MAP_HPP
#define RESTORE_NAME_TO_FD_MAP_HPP

#include <west/io_fd.hpp>
#include <jopp/types.hpp>

#include <string>
#include <algorithm>

namespace restore
{
	struct name_and_fd
	{
		std::string name;
		west::io::fd_owner fd;
		std::vector<std::string> json_path;
	};

	class name_to_fd_map
	{
	public:
		using iterator = std::vector<name_and_fd>::iterator;
		using const_iterator = std::vector<name_and_fd>::const_iterator;

		name_to_fd_map() = default;

		explicit name_to_fd_map(jopp::object const& blobs, char const* tempdir);

		const_iterator find(std::string_view name) const;

		auto begin() const
		{ return std::begin(m_data); }

		auto end() const
		{ return std::end(m_data); }

		auto size() const
		{ return std::size(m_data); }

		auto data() const
		{ return std::data(m_data); }

	private:
		std::vector<name_and_fd> m_data;
	};

	inline void rewind_all(name_to_fd_map const& fds)
	{
		std::ranges::for_each(fds, [](auto const& item) {
			::lseek(item.fd.get(), 0, SEEK_SET);
		});
	}
}

#endif