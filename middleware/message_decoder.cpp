//@	{"target":{"name": "./message_decoder.o"}}

#include "./message_decoder.hpp"
#include "./utils.hpp"

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

restore::blobinfo restore::collect_blob_descriptors(jopp::object const& blobs, char const* tempdir)
{
	blobinfo ret;
	ret.name_and_fd = name_to_fd_map(blobs, tempdir);
	ret.offset_and_name.reserve(std::size(blobs));

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
		ret.offset_and_name.push_back(restore::offset_blob_name{
			static_cast<size_t>(start_offset),
			i.first
		});
	}

	std::ranges::sort(ret.offset_and_name, [](auto const& a, auto const& b) {
		return a.start_offset < b.start_offset;
	});

	return ret;
}

std::pair<restore::http_write_req_result, restore::message_decoder_state>
restore::decode_json(jopp::parser& parser,
	blobinfo& blobs,
	char const* tempdir,
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

			blobs = collect_blob_descriptors(*blobs_obj, tempdir);
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

west::io::fd_ref restore::find_fd(std::span<blob_name_fd const> blobs, std::string_view name)
{
	auto const i = binary_find(std::begin(blobs),
		std::end(blobs),
		name,
		west::overload{
			[](std::string_view a, blob_name_fd const& b){
				return a < b.name;
			},
			[](blob_name_fd const& a, std::string_view b){
				return a.name < b;
			}
		}
	);

	assert(i != std::end(blobs));

	return i->fd.get();
}

void restore::write_full(west::io::fd_ref fd, std::span<char const> buffer)
{
	auto ptr       = std::data(buffer);
	auto const end = std::data(buffer) + std::size(buffer);
	while(ptr != end)
	{
		auto const bytes_left = end - ptr;
		auto const n_written  = ::write(fd, ptr, bytes_left);
		if(n_written == -1) [[unlikely]]
		{
			auto const err = errno;
			if(err == EAGAIN || err == EWOULDBLOCK) [[likely]]
			{ continue; }
			throw west::system_error{"I/O error", err};
		}

		if(n_written == 0) { return; }

		ptr += n_written;
	}
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
				auto [ret, new_state] = decode_json(m_parser, m_blobs, m_tempdir, buffer, bytes_to_read);
				m_current_state = new_state;
				if(std::size(m_blobs.offset_and_name) != 0)
				{
					m_current_blob = std::data(m_blobs.offset_and_name);
				}
				m_bytes_read = 0;
				return ret;
			}

			case message_decoder_state::wait_for_blobs:
			{
				auto const bytes_left = static_cast<size_t>(m_current_blob->start_offset - m_bytes_read);
				auto const bytes_to_skip = std::min(bytes_left, std::size(buffer));
				if(bytes_left == 0)
				{
					auto const i = m_blobs.name_and_fd.find(m_current_blob->name);
					assert(i != std::end(m_blobs.name_and_fd));
					m_current_fd = i->fd.get();

					m_current_state = message_decoder_state::read_blob;
					printf("(wait for blob) Reading blob %s\n", m_current_blob->name.c_str());
					++m_current_blob;

					// If we return 0, west will think there is a blocking socket, and we may not be called
					// again. Therefore, we call ourself to process the data in the correct state.
					return process_request_content(buffer, bytes_to_read);
				}

				m_bytes_read += bytes_to_skip;
				return http_write_req_result{
					.bytes_written = bytes_to_skip,
					.ec = http_req_processing_result{}
				};
			}

			case message_decoder_state::read_blob:
				if(m_current_blob != std::data(m_blobs.offset_and_name) + std::size(m_blobs.offset_and_name))
				{
					auto const bytes_left = static_cast<size_t>(m_current_blob->start_offset - m_bytes_read);
					auto const bytes_to_write = std::min(bytes_left, std::size(buffer));
					if(bytes_to_write == 0)
					{
						printf("(read blob) Reading blob %s\n", m_current_blob->name.c_str());
						::lseek(m_current_fd, 0, SEEK_SET);
						auto const i = m_blobs.name_and_fd.find(m_current_blob->name);
						assert(i != std::end(m_blobs.name_and_fd));
						m_current_fd = i->fd.get();
						++m_current_blob;

						// If we return 0, west will think there is a blocking socket, and we may not be called
						// again. Therefore, we call ourself to process the data in the correct state.
						return process_request_content(buffer, bytes_to_read);
					}

					write_full(m_current_fd, std::span{std::data(buffer), bytes_to_write});
					m_bytes_read += bytes_to_write;

					return http_write_req_result{
						.bytes_written = bytes_to_write,
						.ec = http_req_processing_result{}
					};
				}

				write_full(m_current_fd, buffer);
				m_bytes_read += std::size(buffer);

				return http_write_req_result{
					.bytes_written = std::size(buffer),
					.ec = http_req_processing_result{}
				};
			default:
				__builtin_unreachable();
		}
	}
	catch(west::system_error const&)
	{ throw; }
	catch(std::runtime_error const& err)
	{
		errbuff = west::make_unique_cstr(err.what());
		return http_write_req_result{
			.bytes_written = 0,
			.ec = http_req_processing_result{errbuff.get()}
		};
	}
}