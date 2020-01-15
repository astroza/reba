#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>
#include <iostream>
#include <memory>

#include <worker_group.h>
#include <unistd.h>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <string>

#ifndef BOOST_BEAST_EXAMPLE_FIELDS_ALLOC_HPP
#define BOOST_BEAST_EXAMPLE_FIELDS_ALLOC_HPP

#include <boost/throw_exception.hpp>
#include <cstdlib>
#include <memory>
#include <stdexcept>

#include <router.h>
namespace detail {

struct static_pool
{
    std::size_t size_;
    std::size_t refs_ = 1;
    std::size_t count_ = 0;
    char* p_;

    char*
    end()
    {
        return reinterpret_cast<char*>(this + 1) + size_;
    }

    explicit
    static_pool(std::size_t size)
        : size_(size)
        , p_(reinterpret_cast<char*>(this + 1))
    {
    }

public:
    static
    static_pool&
    construct(std::size_t size)
    {
        auto p = new char[sizeof(static_pool) + size];
        return *(::new(p) static_pool{size});
    }

    static_pool&
    share()
    {
        ++refs_;
        return *this;
    }

    void
    destroy()
    {
        if(refs_--)
            return;
        this->~static_pool();
        delete[] reinterpret_cast<char*>(this);
    }

    void*
    alloc(std::size_t n)
    {
        auto last = p_ + n;
        if(last >= end())
            BOOST_THROW_EXCEPTION(std::bad_alloc{});
        ++count_;
        auto p = p_;
        p_ = last;
        return p;
    }

    void
    dealloc()
    {
        if(--count_)
            return;
        p_ = reinterpret_cast<char*>(this + 1);
    }
};

} // detail

/** A non-thread-safe allocator optimized for @ref basic_fields.

    This allocator obtains memory from a pre-allocated memory block
    of a given size. It does nothing in deallocate until all
    previously allocated blocks are deallocated, upon which it
    resets the internal memory block for re-use.

    To use this allocator declare an instance persistent to the
    connection or session, and construct with the block size.
    A good rule of thumb is 20% more than the maximum allowed
    header size. For example if the application only allows up
    to an 8,000 byte header, the block size could be 9,600.

    Then, for every instance of `message` construct the header
    with a copy of the previously declared allocator instance.
*/
template<class T>
struct fields_alloc
{
    detail::static_pool* pool_;

public:
    using value_type = T;
    using is_always_equal = std::false_type;
    using pointer = T*;
    using reference = T&;
    using const_pointer = T const*;
    using const_reference = T const&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template<class U>
    struct rebind
    {
        using other = fields_alloc<U>;
    };

#if defined(_GLIBCXX_USE_CXX11_ABI) && (_GLIBCXX_USE_CXX11_ABI == 0)
    // Workaround for g++
    // basic_string assumes that allocators are default-constructible
    // See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56437
    fields_alloc() = default;
#endif

    explicit
    fields_alloc(std::size_t size)
        : pool_(&detail::static_pool::construct(size))
    {
    }

    fields_alloc(fields_alloc const& other)
        : pool_(&other.pool_->share())
    {
    }

    template<class U>
    fields_alloc(fields_alloc<U> const& other)
        : pool_(&other.pool_->share())
    {
    }

    ~fields_alloc()
    {
        pool_->destroy();
    }

    value_type*
    allocate(size_type n)
    {
        return static_cast<value_type*>(
            pool_->alloc(n * sizeof(T)));
    }

    void
    deallocate(value_type*, size_type)
    {
        pool_->dealloc();
    }

    template<class U>
    friend
    bool
    operator==(
        fields_alloc const& lhs,
        fields_alloc<U> const& rhs)
    {
        return &lhs.pool_ == &rhs.pool_;
    }

    template<class U>
    friend
    bool
    operator!=(
        fields_alloc const& lhs,
        fields_alloc<U> const& rhs)
    {
        return ! (lhs == rhs);
    }
};

#endif

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class HTTPWorker
{
public:
    HTTPWorker(HTTPWorker const&) = delete;
    HTTPWorker& operator=(HTTPWorker const&) = delete;

    HTTPWorker(tcp::acceptor& acceptor, const std::string& doc_root) :
        acceptor_(acceptor),
        doc_root_(doc_root)
    {
    }

    void start()
    {
        accept();
        check_deadline();
    }

private:
    using alloc_t = fields_alloc<char>;
    //using request_body_t = http::basic_dynamic_body<beast::flat_static_buffer<1024 * 1024>>;
    using request_body_t = http::string_body;

    // The acceptor used to listen for incoming connections.
    tcp::acceptor& acceptor_;

    // The path to the root of the document directory.
    std::string doc_root_;

    // The socket for the currently connected client.
    tcp::socket socket_{acceptor_.get_executor()};

    // The buffer for performing reads
    beast::flat_static_buffer<8192> buffer_;

    // The allocator used for the fields in the request and reply.
    alloc_t alloc_{8192};

    // The parser for reading the requests
    boost::optional<http::request_parser<request_body_t, alloc_t>> parser_;

    // The timer putting a time limit on requests.
    net::basic_waitable_timer<std::chrono::steady_clock> request_deadline_{
        acceptor_.get_executor(), (std::chrono::steady_clock::time_point::max)()};

    // The string-based response message.
    boost::optional<http::response<http::string_body, http::basic_fields<alloc_t>>> string_response_;

    // The string-based response serializer.
    boost::optional<http::response_serializer<http::string_body, http::basic_fields<alloc_t>>> string_serializer_;

    // The file-based response message.
    boost::optional<http::response<http::file_body, http::basic_fields<alloc_t>>> file_response_;

    // The file-based response serializer.
    boost::optional<http::response_serializer<http::file_body, http::basic_fields<alloc_t>>> file_serializer_;

    void accept()
    {
        // Clean up any previous connection.
        beast::error_code ec;
        socket_.close(ec);
        buffer_.consume(buffer_.size());

        acceptor_.async_accept(
            socket_,
            [this](beast::error_code ec)
            {
                if (ec)
                {
                    accept();
                }
                else
                {
                    // Request must be fully processed within 60 seconds.
                    request_deadline_.expires_after(
                        std::chrono::seconds(60));

                    read_request();
                }
            });
    }

    void read_request()
    {
        // On each read the parser needs to be destroyed and
        // recreated. We store it in a boost::optional to
        // achieve that.
        //
        // Arguments passed to the parser constructor are
        // forwarded to the message object. A single argument
        // is forwarded to the body constructor.
        //
        // We construct the dynamic body with a 1MB limit
        // to prevent vulnerability to buffer attacks.
        //
        parser_.emplace(
            std::piecewise_construct,
            std::make_tuple(),
            std::make_tuple(alloc_));

        http::async_read(
            socket_,
            buffer_,
            *parser_,
            [this](beast::error_code ec, std::size_t)
            {
                if (ec)
                    accept();
                else
                    process_request(parser_->get());
            });
    }

    void process_request(http::request<request_body_t, http::basic_fields<alloc_t>> const& req)
    {
        auto host = std::string(req[http::field::host]);
        auto worker_group_bind = lake::global_router_.route_by_host(host);
        if(worker_group_bind) {
            auto worker_group = static_cast<lake::WorkerGroup *>(worker_group_bind->get_native_object());
            worker_group->enqueue_request();
        }
        switch (req.method())
        {
        case http::verb::get:
            send_file(req.target());
            break;

        default:
            // We return responses indicating an error if
            // we do not recognize the request method.
            send_bad_response(
                http::status::bad_request,
                "Invalid request-method '" + std::string(req.method_string()) + "'\r\n");
            break;
        }
    }

    void send_bad_response(
        http::status status,
        std::string const& error)
    {
        string_response_.emplace(
            std::piecewise_construct,
            std::make_tuple(),
            std::make_tuple(alloc_));

        string_response_->result(status);
        string_response_->keep_alive(false);
        string_response_->set(http::field::server, "Lake");
        string_response_->set(http::field::content_type, "text/plain");
        string_response_->body() = error;
        string_response_->prepare_payload();

        string_serializer_.emplace(*string_response_);

        http::async_write(
            socket_,
            *string_serializer_,
            [this](beast::error_code ec, std::size_t)
            {
                socket_.shutdown(tcp::socket::shutdown_send, ec);
                string_serializer_.reset();
                string_response_.reset();
                accept();
            });
    }

    void send_file(beast::string_view target)
    {
        // Request path must be absolute and not contain "..".
        if (target.empty() || target[0] != '/' || target.find("..") != std::string::npos)
        {
            send_bad_response(
                http::status::not_found,
                "File not found\r\n");
            return;
        }

        std::string full_path = doc_root_;
        full_path.append(
            target.data(),
            target.size());

        http::file_body::value_type file;
        beast::error_code ec;
        file.open(
            full_path.c_str(),
            beast::file_mode::read,
            ec);
        if(ec)
        {
            send_bad_response(
                http::status::not_found,
                "File not found\r\n");
            return;
        }

        file_response_.emplace(
            std::piecewise_construct,
            std::make_tuple(),
            std::make_tuple(alloc_));

        file_response_->result(http::status::ok);
        file_response_->keep_alive(false);
        file_response_->set(http::field::server, "Lake");
        file_response_->body() = std::move(file);
        file_response_->prepare_payload();

        file_serializer_.emplace(*file_response_);

        http::async_write(
            socket_,
            *file_serializer_,
            [this](beast::error_code ec, std::size_t)
            {
                socket_.shutdown(tcp::socket::shutdown_send, ec);
                file_serializer_.reset();
                file_response_.reset();
                accept();
            });
    }

    void check_deadline()
    {
        // The deadline may have moved, so check it has really passed.
        if (request_deadline_.expiry() <= std::chrono::steady_clock::now())
        {
            // Close socket to cancel any outstanding operation.
            beast::error_code ec;
            socket_.close();

            // Sleep indefinitely until we're given a new deadline.
            request_deadline_.expires_at(
                std::chrono::steady_clock::time_point::max());
        }

        request_deadline_.async_wait(
            [this](beast::error_code)
            {
                check_deadline();
            });
    }
};

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: lake <governator script file> <port>\n";
		return 1;
	}
	lake::engine::init();
	std::ifstream gov_script_file { argv[1] };
	std::string gov_script { std::istreambuf_iterator<char>(gov_script_file), std::istreambuf_iterator<char>() };
	lake::WorkerGroup *gov_worker_group = new lake::WorkerGroup(gov_script, true);

    try
    {
        auto const address = net::ip::make_address("0.0.0.0");
        unsigned short port = static_cast<unsigned short>(std::atoi(argv[2]));
        int num_workers = 4;

        net::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {address, port}};

        std::list<HTTPWorker> workers;
        for (int i = 0; i < num_workers; ++i)
        {
            workers.emplace_back(acceptor, "test");
            workers.back().start();
        }

        ioc.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
