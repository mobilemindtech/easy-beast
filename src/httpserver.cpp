//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
// Portions Copyright (c) 2021 anticrisis <https://github.com/anticrisis>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP server, synchronous
//
// Changes by anticrisis marked with 'anticrisis'
//
//------------------------------------------------------------------------------


#include "../include/httpserver.h"
#include "../include/extra/bench.h"


// anticrisis: add namespace
namespace httpserver
{

// anticrisis: add thread_count
//std::atomic<int> thread_count;

//------------------------------------------------------------------------------
class http_session : public std::enable_shared_from_this<http_session>{

public:

    http_session(tcp::socket&& socket,
                 std::unique_ptr<http_handler> handler_ptr)
        :stream_(std::move(socket)),
        //deadline_timer_(socket),
        http_handler_(std::move(handler_ptr))
    {
    }

    tcp::socket& socket(){
        return this->stream_.socket();
    }

    void run(){
        net::dispatch(
            stream_.get_executor(),
            beast::bind_front_handler(
                &http_session::do_read,
                this->shared_from_this()));
    }


    // Handles an HTTP server connection
    void do_read(){

        req_ = {};
        stream_.expires_after(std::chrono::seconds(5));

        //std::cout << "new connection" << std::endl;
        bench_stop("do_read");

        http::async_read(
            stream_,
            buffer_,
            req_,
            beast::bind_front_handler(
                &http_session::on_read,
                shared_from_this()));
    }

private:

    void on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if(ec == http::error::end_of_stream)
            return do_close();

        if(ec)
            return fail(ec, "read");

        bench_stop("on_read");
        // Send the response
        handle_request(std::move(req_));
    }

    void abort(){
        stream_.socket().cancel();
        stream_.close();
        //stream_.socket().close();
    }

    // Returns a bad request response
    http::message_generator bad_request(beast::string_view why) {
        http::response<http::string_body> res{ http::status::bad_request,
                                              req_.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain");
        res.keep_alive(req_.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    }

    void send_body(int status,
                   tl::optional<std::unordered_map<std::string, std::string>>&& headers,
                   std::string&&            body,
                   std::string&&            content_type) {
        http::response<http::string_body> res{ static_cast<http::status>(status),
                                              req_.version() };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, content_type.empty() ? "text/plain" : content_type);
        res.content_length(body.size());
        if (headers)
            for (auto& kv: *headers)
                res.base().set(kv.first, std::move(kv.second));
        res.body() = std::move(body);
        res.keep_alive(req_.keep_alive());
        res.prepare_payload();
        send_response(std::move(res));
    }


    void create_string_response(response_t* response) {

        http::response<http::string_body> res{ static_cast<http::status>(response->status_code),
                                              req_.version() };

        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, response->content_type);

        headers_t* headers = response->headers;

        if(headers != NULL){
            int size = headers->size;
            header_t* hs = headers->headers;
            for(int i = 0; i < size; i++){
                std::string name {hs->name};
                std::string value {hs->value};
                res.base().set(name, std::move(value));
                hs++;
            }                       
        }

        body_t* body = response->body;
        if(body != NULL) {
            res.content_length(body->size);
            if(body->body != NULL){
                auto sbody = std::string { body->body };
                res.body() = std::move(sbody);
            }
        }


        res.keep_alive(req_.keep_alive());
        res.prepare_payload();
        send_response(std::move(res));
    }

    void create_empty_response(response_t* response) {

        http::response<http::empty_body> res{ static_cast<http::status>(response->status_code),
                                              req_.version() };

        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, response->content_type);

        headers_t* headers = response->headers;

        if(headers != NULL){
            int size = headers->size;
            header_t* hs = headers->headers;
            for(int i = 0; i < size; i++){
                std::string name {hs->name};
                std::string value {hs->value};
                res.base().set(name, std::move(value));
                hs++;
            }
        }


        res.keep_alive(req_.keep_alive());
        send_response(std::move(res));
    }

    void create_buffer_response(response_t* response) {
        http::response<http::buffer_body> res{ static_cast<http::status>(response->status_code),
                                              req_.version() };

        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, response->content_type);
        res.content_length(response->body->size);

        headers_t* headers = response->headers;

        if(headers != NULL){
            int size = headers->size;
            header_t* hs = headers->headers;
            for(int i = 0; i < size; i++){
                res.base().set(hs->name, hs->value);
                hs++;
            }
        }

        //http::buffer_body buffer(response->body->body_raw, response->body->size);
        //boost::asio::buffer buffer();

        res.body().data = (char *)response->body->body_raw;
        res.body().size = response->body->size;

        res.keep_alive(req_.keep_alive());
        res.prepare_payload();
        send_response(std::move(res));
    }

    void send_response_t() {

        bool has_body = this->resp_t_->body != NULL;
        bool body_bytes =  has_body && this->resp_t_->body->body_raw != NULL;
        bool body_str = has_body && this->resp_t_->body->body != NULL;

        if(body_bytes)
            create_buffer_response(this->resp_t_);
        else if(body_str)
            create_string_response(this->resp_t_);
        else
            create_empty_response(this->resp_t_);

	//request_free(request);
	//response_free(response);
    }

    //template <bool isRequest, class Body, class Fields>
    void send_response(http::message_generator&& msg)
    {

        //std::cout << "send_response" << std::endl;

        bool keep_alive = msg.keep_alive();

        beast::async_write(
            stream_,
            std::move(msg),
            beast::bind_front_handler(
                &http_session::on_write, shared_from_this(), keep_alive));
    }

    void on_write(
        bool keep_alive,
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");


        response_free(this->resp_t_);
        request_free(this->req_t_);
	
        if(!keep_alive)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }
    }

    void do_close()
    {
        // Send a TCP shutdown
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
        //deadline_timer_.cancel();
        // At this point the connection is closed gracefully
    }

    std::string get_verb(http::verb verb){
        switch(verb) {
        case http::verb::get:
            return "GET";
            break;
        case http::verb::post:
            return "POST";
            break;
        case http::verb::put:
            return "PUT";
            break;
        case http::verb::delete_:
            return "DELETE";
            break;
        case http::verb::head:
            return "HEAD";
            break;
        case http::verb::options:
            return "OPTIONS";
            break;
        case http::verb::patch:
            return "PATCH";
            break;
        default:
            return "";
        }
    }


    // This function produces an HTTP response for the given
    // request. The type of the response object depends on the
    // contents of the request, so the interface requires the
    // caller to pass a generic lambda for receiving the response.
    // anticrisis: remove support for doc_root and static files; add support for
    // http_handler
    template <class Body, class Allocator>
    void handle_request(http::request<Body, http::basic_fields<Allocator>>&& req)
    {

        bench_stop("http parse done");

        std::string verb = get_verb(req.method());

        if(verb.empty()){
            return send_response(bad_request("Unknown HTTP-method"));
        }

        std::string target = std::string { req.target().data(), req.target().size() };
        std::string body_str = std::string { req.body().data(), req.body().size() };
        int body_size = body_str.size();

        //auto body_data = req.body().data();
        //const auto buffer_bytes = buffer_.cdata();


        //const char* body_raw = static_cast<const char*>(buffer_bytes.data());

        this->req_t_ = request_new(verb.c_str(), target.c_str());

        std::vector<std::pair<std::string, std::string>> hs;
        for (auto const& kv: req.base()){

            auto name = std::string{ kv.name_string().data(), kv.name_string().size() };
            auto value = std::string{ kv.value().data(), kv.value().size() };

            if(name == "Content-Type" || name == "content-type") {
                this->req_t_->content_type = value.c_str();
            }

            hs.push_back({name, value});
        }

        int hsize = hs.size();
        if(hsize > 0){
            this->req_t_->headers = headers_new(hsize);
            auto pt = this->req_t_->headers->headers;
            //header_t* headers = (header_t*) malloc(sizeof(header_t)*hsize);

            for(int i = 0; i < hsize; i++){
                pt->name = hs[i].first.c_str();
                pt->value = hs[i].second.c_str();
                pt++;
            }

            //this->req_t_->headers->headers = headers;
        }

        if(body_size > 0){
            body_t* body = body_new(body_str.c_str(), NULL, body_size);
            this->req_t_->body = body;
        }

        bench_stop("dispach app start");

        if(http_handler_->use_async()) {
	  return http_handler_->dispatch_async(this->req_t_, [this](response_t* resp){
                bench_stop("dispach app async done");
		this->resp_t_ = resp;
                send_response_t();
            });
        } else {
            this->resp_t_ = http_handler_->dispatch(this->req_t_);
            bench_stop("dispach app sync done");
            return send_response_t();
        }
    }

    //------------------------------------------------------------------------------

    // Report a failure
    void fail(beast::error_code ec, char const* what)
    {
        // anticrisis: ignore these common errors
        if (ec == net::error::operation_aborted || ec == beast::error::timeout
            || ec == net::error::connection_reset)
            return;

        std::cerr << what << ": " << ec.message() << "\n";
    }


private:
  beast::tcp_stream stream_;
  //boost::asio::deadline_timer deadline_timer_;
  std::unique_ptr<http_handler> http_handler_;
  http::request<http::string_body> req_;
  beast::flat_buffer buffer_;
  request_t* req_t_;
  response_t* resp_t_;
};

class http_server : public std::enable_shared_from_this<http_server>  {

public:

    http_server(net::io_context& io,
                std::shared_ptr<beast_handler_t> handler,
                tcp::endpoint endpoint)
        :io_(io),
        acceptor_(net::make_strand(io)),
        http_handler_(handler)
    {

        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
        if(ec)
        {
            fail(ec, "listen");
            return;
        }

    }


    void run(){
        //std::cout << "http_server::run" << std::endl;
        net::dispatch(
            acceptor_.get_executor(),
            beast::bind_front_handler(
                &http_server::do_accept,
                this->shared_from_this()));
    }


private:

    void
    fail(beast::error_code ec, char const* what)
    {
        std::cerr << what << ": " << ec.message() << "\n";
    }

    void do_accept(){

        //std::cout << "http_server::do_accept" << std::endl;
        acceptor_.async_accept(
            net::make_strand(io_),
            beast::bind_front_handler(
                &http_server::on_accept,
                shared_from_this()));
    }

    void on_accept(const boost::system::error_code& ec, tcp::socket socket){

        //std::cout << "http_server::on_accept" << std::endl;
        if(!ec){

            bench_start();

            std::unique_ptr<http_handler> handler(new http_handler(http_handler_->sync, http_handler_->async));

            // Create the http session and run it
            std::make_shared<http_session>(
                std::move(socket),
                std::move(handler))->run();

        }
        do_accept();
    }


    net::io_context& io_;
    tcp::acceptor acceptor_;
    std::shared_ptr<beast_handler_t> http_handler_;
};


void thread_init(void* v){
    net::io_context *io = (net::io_context*) v;
    io->run();
}

// anticrisis: change main to run; remove doc_root
int run(char* address_,
        unsigned short   port,
        unsigned short   max_thread_count,
        beast_handler_t* handler){


    try
    {
        //thread_count = 0;

        auto const address = net::ip::make_address(address_);

        // The io_context is required for all I/O
        net::io_context io{max_thread_count};

        net::signal_set signals(io, SIGINT, SIGTERM);
        signals.async_wait(
            [&](beast::error_code const&, int)
            {
                // Stop the `io_context`. This will cause `run()`
                // to return immediately, eventually destroying the
                // `io_context` and all of the sockets in it.
                std::cout << "Server stopping.." << std::endl;
                io.stop();
            });

        std::shared_ptr<beast_handler_t> handler_ptr(handler);
        std::make_shared<http_server>(
            io, handler_ptr, tcp::endpoint{address, port})->run();

        std::cout << "http server at http://" << address_ << ":" << port << std::endl;

#ifdef USE_STD_THREAD
	std::vector<std::thread> thread_pool;
	thread_pool.reserve(max_thread_count - 1);
	for(auto i = max_thread_count - 1; i > 0; --i)
	  thread_pool.emplace_back([&io]
	  {
	    io.run();
	  });
	  
	//io.run();
	
	for (auto& th : thread_pool)
	  th.join();   
#else
	handler->thread_starter(thread_init, max_thread_count, &io);
	//io.run();
#endif

        

    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

} // namespace httpserver


