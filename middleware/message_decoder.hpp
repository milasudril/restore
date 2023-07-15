//@	{"dependencies_extra":[{"ref":"./message_decoder.o", "rel":"implementation"}]}

#ifndef RESTORE_MESSAGE_DECODER_HPP
#define RESTORE_MESSAGE_DECODER_HPP

#include "./http_request_result.hpp"

#include <jopp/parser.hpp>
#include <west/io_fd.hpp>
#include <vector>

namespace restore
{
	enum class message_decoder_state{
		read_json,
		wait_for_blobs,
		read_blob
	};

	enum class message_decoder_error_code{
		root_is_not_an_object,
		unknown_data_present,
		blobs_is_not_an_object,
		message_truncated
	};

	inline constexpr bool can_continue(message_decoder_error_code)
	{ return false; }

	inline constexpr bool is_error_indicator(message_decoder_error_code)
	{ return true; }

	inline constexpr char const* to_string(message_decoder_error_code val)
	{
		switch(val)
		{
			case message_decoder_error_code::root_is_not_an_object:
				return "Root must be an object";

			case message_decoder_error_code::unknown_data_present:
				return "Unknown data present";

			case message_decoder_error_code::blobs_is_not_an_object:
				return "`blobs` field must be an object";

			case message_decoder_error_code::message_truncated:
				return "Message is truncated";

			default:
				__builtin_unreachable();
		}
	}

	std::pair<restore::http_write_req_result, restore::message_decoder_state>
	validate_bytes_to_read(size_t bytes_written, size_t bytes_left);

	struct blob_name_fd
	{
		std::string name;
		west::io::fd_owner fd;
	};

	struct offset_blob_name
	{
		size_t start_offset;
		std::string name;
	};

	struct blobinfo
	{
		std::vector<blob_name_fd> name_and_fd;
		std::vector<offset_blob_name> offset_and_name;
	};

	west::io::fd_ref find_fd(std::span<blob_name_fd const> blobs, std::string_view name);

	blobinfo collect_blob_descriptors(jopp::object const& blob_descriptor);

	std::pair<http_write_req_result, message_decoder_state>
	decode_json(jopp::parser& parser,
		blobinfo& blobs,
		std::span<char const> buffer, size_t bytes_to_read);

	class message_decoder
	{
	public:
		message_decoder():m_container{std::make_unique<jopp::container>()},
			m_parser{*m_container},
			m_current_state{message_decoder_state::read_json}
		{}

		http_write_req_result process_request_content(std::span<char const> buffer, size_t bytes_to_read);

		auto const& get_json() const
		{ return m_container; }


	private:
		std::unique_ptr<jopp::container> m_container;
		blobinfo m_blobs;
		size_t m_bytes_read;
		offset_blob_name const* m_current_blob;
		west::io::fd_ref m_current_fd;
		jopp::parser m_parser;
		message_decoder_state m_current_state;
	};
}

#endif