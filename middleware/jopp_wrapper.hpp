//@	{"dependencies_extra":[{"ref":"./jopp_wrapper.o", "rel":"implementation"}]}

#ifndef RESTORE_JOPP_WRAPPER_HPP
#define RESTORE_JOPP_WRAPPER_HPP

#include <functional>
#include <string>
#include <memory>

namespace jopp  // Sorry for opening external namespace
{
	class object;
};

namespace restore::json
{
	class object_ref
	{
	public:
		explicit object_ref(std::reference_wrapper<jopp::object const> ref):m_ref{ref}{}

		template<class T>
		requires std::is_same_v<T, std::string>
		std::string const& get_field_as(std::string_view key) const;

		template<class T>
		T get_field_as(std::string_view key) const;

		bool contains(std::string_view key) const;

	private:
		std::reference_wrapper<jopp::object const> m_ref;
	};
}

#endif