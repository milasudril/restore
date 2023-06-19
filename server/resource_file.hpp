//@ {"dependencies":[{"ref": "wad64", "origin":"pkg-config"}]}

#ifndef RESTORE_RESOURCE_FILE_HPP
#define RESTORE_RESOURCE_FILE_HPP

#include <wad64/fd_owner.hpp>
#include <wad64/readonly_archive.hpp>

namespace restore
{
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
			m_archive{std::ref(m_fd)}
		{}

	private:
		Wad64::FdOwner m_fd;
		Wad64::ReadonlyArchive m_archive;
	};
}

#endif