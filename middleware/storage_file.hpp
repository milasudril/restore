//@ {
//@  "dependencies":[{"ref": "wad64", "origin":"pkg-config"}],
//@  "dependencies_extra":[{"ref": "./storage_file.o", "rel":"implementation"}]
//@ }

#ifndef RESTORE_STORAGE_FILE_HPP
#define RESTORE_STORAGE_FILE_HPP

#include "./utils.hpp"
#include "./json_loaders.hpp"

#include <jopp/types.hpp>
#include <wad64/fd_owner.hpp>
#include <wad64/archive.hpp>
#include <wad64/wad64.hpp>

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

		void insert(std::span<std::byte const> data, std::string_view name)
		{	Wad64::insert(m_archive, Wad64::FileCreationMode::AllowCreation(), data, name); }

		void insert(int fd, std::string_view name)
		{ Wad64::insert(m_archive, Wad64::FileCreationMode::AllowCreation(), fd, name); }

		void remove(std::string_view item)
		{ m_archive.remove(item); }

		auto get_file(std::string_view name) const
		{ return Wad64::InputFile{m_archive, name}; }

		auto& archive() { return m_archive; }

	private:
		Wad64::FdOwner m_fd;
		Wad64::Archive m_archive;
	};

	std::vector<std::string_view> collect_entries(std::reference_wrapper<storage_file const> file,
		std::string_view prefix,
		size_t entry_count_estimate);
}

#endif
