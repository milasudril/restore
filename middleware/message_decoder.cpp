//@	{"target":{"name": "./message_decoder.o"}}

#include "./message_decoder.hpp"

#include <west/utils.hpp>

#include <algorithm>

std::pair<restore::http_write_req_result, restore::message_decoder_state>
restore::validate_bytes_to_read(size_t bytes_written, size_t bytes_left)
{
	if(bytes_left == 0) [[likely]]
	{
		return std::pair{
			http_write_req_result{
				.bytes_written = bytes_written,
				.ec = http_req_processing_result{jopp::parser_error_code::completed}
			},
			message_decoder_state::wait_for_blobs
		};
	}

	return std::pair{
		http_write_req_result{
			.bytes_written = bytes_written,
			.ec = http_req_processing_result{message_decoder_error_code::unknown_data_present}
		},
		message_decoder_state::wait_for_blobs
	};
}

restore::blobinfo restore::collect_blob_descriptors(jopp::object const& blobs)
{
	auto const n_objs = std::size(blobs);
	blobinfo ret;
	ret.name_and_fd.reserve(n_objs);
	ret.offset_and_name.reserve(n_objs);

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

		auto const start_offset = val->get_field_as<jopp::number>("start_offset");
		ret.name_and_fd.push_back(restore::blob_name_fd{i.first, west::io::fd_owner{}});
		ret.offset_and_name.push_back(restore::offset_blob_name{
			static_cast<size_t>(start_offset),
			i.first
		});
	}

	// NOTE: name_and_fd is already sorted by name, since blobs is an std::map

	std::ranges::sort(ret.offset_and_name, [](auto const& a, auto const& b) {
		return a.start_offset < b.start_offset;
	});

	return ret;
}

std::pair<restore::http_write_req_result, restore::message_decoder_state>
restore::decode_json(jopp::parser& parser,
	blobinfo& blobs,
	std::span<char const> buffer,
	size_t bytes_to_read)
{
	auto const res = parser.parse(buffer);
	auto const bytes_written = static_cast<size_t>(res.ptr - std::begin(buffer));

	switch(res.ec)
	{
		case jopp::parser_error_code::completed:
		{
			auto const root = parser.root().get_if<jopp::object>();
			if(root == nullptr)
			{
				return std::pair{
					http_write_req_result{
						.bytes_written = bytes_written,
						.ec = http_req_processing_result{message_decoder_error_code::root_is_not_an_object}
					},
					message_decoder_state::wait_for_blobs
				};
			}

			auto const bytes_left = bytes_to_read - bytes_written;
			auto const blobs_iter = root->find("blobs");
			if(blobs_iter == std::end(*root))
			{ return validate_bytes_to_read(bytes_written, bytes_left); }

			auto const blobs_obj = blobs_iter->second.get_if<jopp::object>();
			if(blobs_obj == nullptr)
			{
				return std::pair{
					http_write_req_result{
						.bytes_written = bytes_written,
						.ec = http_req_processing_result{message_decoder_error_code::blobs_is_not_an_object}
					},
					message_decoder_state::wait_for_blobs
				};
			}

			if(std::size(*blobs_obj) == 0)
			{ return validate_bytes_to_read(bytes_written, bytes_left); }

			blobs = collect_blob_descriptors(*blobs_obj);
			auto const min_size = blobs.offset_and_name.back().start_offset;
			if(min_size > bytes_left)
			{
				return std::pair{
					http_write_req_result{
						.bytes_written = bytes_written,
						.ec = http_req_processing_result{message_decoder_error_code::message_truncated}
					},
					message_decoder_state::wait_for_blobs
				};
			}

			return std::pair{
				http_write_req_result{
					.bytes_written = bytes_written,
					.ec = http_req_processing_result{res.ec}
				},
				message_decoder_state::wait_for_blobs
			};
		}

		case jopp::parser_error_code::more_data_needed:
			return std::pair{
				http_write_req_result{
					.bytes_written = bytes_written,
					.ec = http_req_processing_result{res.ec}
				},
				message_decoder_state::read_json
			};

		default:
			return std::pair{
				http_write_req_result{
					.bytes_written = bytes_written,
					.ec = http_req_processing_result{res.ec}
				},
				message_decoder_state::read_json
			};
	}
}

namespace
{
	thread_local std::unique_ptr<char const[]> errbuff;
}

restore::http_write_req_result
restore::message_decoder::process_request_content(std::span<char const> buffer, size_t bytes_to_read)
{
	try
	{
		switch(m_current_state)
		{
			case message_decoder_state::read_json:
			{
				auto [ret, new_state] = decode_json(m_parser, m_blobs, buffer, bytes_to_read);
				m_current_state = new_state;
				if(std::size(m_blobs.offset_and_name) != 0)
				{ m_next_start_offset = m_blobs.offset_and_name.front().start_offset; }
				m_bytes_read = 0;
				return ret;
			}

			case message_decoder_state::wait_for_blobs:
			{
				printf("Wait for blobs\n");
				printf("%zu %zu\n", m_next_start_offset, m_bytes_read);
				auto const bytes_left = static_cast<size_t>(m_next_start_offset - m_bytes_read);
				auto const bytes_to_skip = std::min(bytes_left, std::size(buffer));
				if(bytes_left == 0)
				{ m_current_state = message_decoder_state::read_blob;}
				m_bytes_read += bytes_to_skip;
				return http_write_req_result{
					.bytes_written = bytes_to_skip,
					.ec = http_req_processing_result{}
				};
			}

			case message_decoder_state::read_blob:
				printf("Read blob\n");
				abort();
				break;
			default:
				__builtin_unreachable();
		}
	}
	catch(const std::runtime_error& err)
	{
		errbuff = west::make_unique_cstr(err.what());
		return http_write_req_result{
			.bytes_written = 0,
			.ec = http_req_processing_result{errbuff.get()}
		};
	}
}