#include <iostream>
#include <http.h>

HTTP::HTTP()
{
	init_http11_parser();
}

bool HTTP::consumeBuffer(const char *data, size_t size)
{
	enum llhttp_errno err = llhttp_execute(&this->http11_parser, data, size);
	if (err == HPE_OK)
	{
		return true;
	}
	return false;
}

std::unique_ptr<HTTPRequest> HTTP::get_request(tcp::socket &socket, boost::asio::yield_context &yield)
{
	std::unique_ptr<HTTPRequest> request(new HTTPRequest());
	request_done = false;
	request_headers = &(request->headers);
	try {
		char data[8192];
		do {
			// Que pasa si el buffer solo contiene la linea parcial 
			std::size_t n = socket.async_read_some(boost::asio::buffer(data), yield);
			if(!consumeBuffer(data, n)) {
				return std::unique_ptr<HTTPRequest>{};
			}
			if (request_done) {
				return std::move(request);
			}
		} while(1);
	}
	catch (std::exception &e)
	{
		
	}
	return std::unique_ptr<HTTPRequest>{};
}

bool HTTP::is_delegated_to_user() {
	return false;
}

void HTTP::init_http11_parser()
{
	llhttp_settings_init(&this->http11_settings);
	this->http11_settings.on_message_complete = [](llhttp_t *parser) -> int {
		((HTTP *)parser->data)->on_http11_message_complete_cb(parser);
		return 0;
	};
	this->http11_settings.on_header_field = [](llhttp_t *parser, const char *at, size_t length) -> int {
		((HTTP *)parser->data)->on_http11_header_field_cb(parser, at, length);
		return 0;
	};
	this->http11_settings.on_header_value = [](llhttp_t *parser, const char *at, size_t length) -> int {
		((HTTP *)parser->data)->on_http11_header_value_cb(parser, at, length);
		return 0;
	};
	llhttp_init(&this->http11_parser, HTTP_BOTH, &this->http11_settings);
	this->http11_parser.data = this;
}

int HTTP::on_http11_message_complete_cb(llhttp_t *parser)
{
	this->request_done = true;
	return 0;
}

int HTTP::on_http11_header_field_cb(llhttp_t *parser, const char *at, size_t length)
{
	this->_header_field_name = std::string(at, length);
	return 0;
}

int HTTP::on_http11_header_value_cb(llhttp_t *parser, const char *at, size_t length)
{
	(*this->request_headers)[this->_header_field_name] = std::string(at, length);
	return 0;
}
