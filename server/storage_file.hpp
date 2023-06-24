//@ {"dependencies":[{"ref": "wad64", "origin":"pkg-config"}]}

#ifndef RESTORE_STORAGE_FILE_HPP
#define RESTORE_STORAGE_FILE_HPP

#include "./utils.hpp"
#include "./json_loaders.hpp"

#include <jopp/types.hpp>
#include <wad64/fd_owner.hpp>
#include <wad64/readonly_archive.hpp>
#include <wad64/input_file.hpp>

#include <algorithm>

namespace restore
{
	class storage_file
	{
	public:
		// Because this class is self-referencial
		storage_file(storage_file const&) = delete;
		storage_file(storage_file&&) = delete;
		storage_file& operator=(storage_file const&) = delete;
		storage_file& operator=(storage_file&&) = delete;

		explicit storage_file(char const* filename):
			m_fd{filename, Wad64::IoMode::AllowRead().allowWrite(), Wad64::FileCreationMode::AllowCreation().allowOverwrite()},
			m_archive{std::ref(m_fd)}
		{}

		auto const& ls() const
		{ return m_archive.ls(); }

	private:
		Wad64::FdOwner m_fd;
		Wad64::Archive m_archive;
	};
}

#endif
