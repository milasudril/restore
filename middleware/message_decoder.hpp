//@	{"dependencies_extra":[{"ref":"./message_decoder.o", "rel":"implementation"}]}

#ifndef RESTORE_MESSAGE_DECODER_HPP
#define RESTORE_MESSAGE_DECODER_HPP

#include "./http_request_result.hpp"
#include <jopp/parser.hpp>

namespace restore
{
	class message_decoder
	{
	public:
		message_decoder():m_container{std::make_unique<jopp::container>()},
			m_parser{*m_container},
			m_current_state{state::read_json}
		{}

		http_write_req_result process_request_content(std::span<char const> buffer, size_t);

		auto const& get_json() const
		{ return m_container; }


	private:
		std::unique_ptr<jopp::container> m_container;
		jopp::parser m_parser;

		enum class state{
			read_json,
			wait_for_blobs,
			read_blob
		};

		state m_current_state;
	};
}

#endif