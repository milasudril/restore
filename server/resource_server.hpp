//@	{"dependencies_extra":[{"ref":"./resource_server.o", "rel":"implementation"}]}

#ifndef RESTORE_RESOURCE_SERVER_HPP
#define RESTORE_RESOURCE_SERVER_HPP

#include "./resource_file.hpp"
#include "./http_request_result.hpp"

#include <west/http_message_header.hpp>
#include <west/http_request_handler.hpp>

namespace restore
{
	class resource_server
	{
	public:
		explicit resource_server(Wad64::InputFile&& input_file, resource_info&& res_info):
			m_input_file{std::move(input_file)},
			m_resource_info{std::move(res_info)}
		{}

		auto finalize_state(west::http::field_map& fields) const
		{
			fields.append("Content-Length", std::to_string(m_input_file.size()))
				.append("Content-Type", std::string{m_resource_info.mime_type})
				.append("Last-Modified", to_string(m_resource_info.last_modified));

			west::http::finalize_state_result validation_result{};
			validation_result.http_status = west::http::status::ok;
			return validation_result;
		}

		auto read_response_content(std::span<char> buffer)
		{
			auto const bytes_read = m_input_file.read(std::as_writable_bytes(std::span{std::data(buffer), std::size(buffer)}));

			return http_read_resp_result{
				bytes_read,
				http_req_processing_result{}
			};
		}

	private:
		Wad64::InputFile m_input_file;
		resource_info m_resource_info;
	};

	std::string_view resolve_resource(west::http::uri const& req_target);

	std::pair<west::http::finalize_state_result, std::optional<resource_server>>
serve_resource(west::http::request_header const& header,
	std::reference_wrapper<resource_file const> res_file);
}

#endif