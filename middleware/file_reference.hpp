#ifndef RESTORE_FILE_REFERENCE_HPP
#define RESTORE_FILE_REFERENCE_HPP

namespace restore
{
	template<class T>
	concept random_access_file = requires(T a)
	{
		{
			read(a, std::declval<std::span<std::byte>>(), std::declval<int64_t>())
		}
		->std::same_as<std::size_t>;

		{
			write(a, std::declval<std::span<std::byte const>>(), std::declval<int64_t>())
		}
		->std::same_as<std::size_t>;

		{
			truncate(a, std::declval<int64_t>())
		}
		->std::same_as<void>;
	};

	namespace detail
	{
		template<random_access_file File>
		auto read(void const* handle, std::span<std::byte> buffer, int64_t offset)
		{
			return read(*static_cast<File const*>(handle), buffer, offset);
		}

		template<random_access_file File>
		auto write(void* handle, std::span<std::byte const> buffer, int64_t offset)
		{
			return write(*static_cast<File*>(handle), buffer, offset);
		}

		template<random_access_file File>
		size_t write_from_fd(void* handle, FdAdapter fd, int64_t offset)
		{
			return write(*static_cast<File*>(handle), fd, offset);
		}

		template<random_access_file File>
		void truncate(void* handle, int64_t new_size)
		{
			truncate(*static_cast<File*>(handle), new_size);
		}

		struct file_reference_vtable
		{
			size_t (*read)(void const*, std::span<std::byte>, int64_t offset);
			size_t (*write)(void*, std::span<std::byte const>, int64_t offset);
			void (*truncate)(void*, int64_t new_size);
		};

		template<random_access_file File>
		inline constexpr file_reference_vtable file_ref_vtable
		{
			read<File>,
			write<File>,
			truncate<File>
		};
	}

	class file_reference
	{
	public:
		template<random_access_file File>
		explicit file_reference(std::reference_wrapper<File> file)
		    : m_ref{&file.get()}
		    , m_vt{std::ref(detail::file_ref_vtable<File>)}
		{ }

		size_t read(std::span<std::byte> buffer, int64_t offset) const
		{ return m_vt.get().read(m_ref, buffer, offset); }

		size_t write(std::span<std::byte const> buffer, int64_t offset) const
		{ return m_vt.get().write(m_ref, buffer, offset); }

		void truncate(int64_t new_size) const
		{ m_vt.get().truncate(m_ref, new_size); }

		void* handle() const
		{ return m_ref; }

	private:
		void* m_ref;
		std::reference_wrapper<detail::file_reference_vtable const> m_vt;
	};
}

#endif