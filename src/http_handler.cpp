

#include "../include/http_handler.h"

namespace httpserver{


static void
async_response_callback_wrap(request_t* req, response_t* resp)
{
    auto handler = static_cast<http_handler *>(req->handler_);
    auto callback = handler->callback_response();
    callback(resp);
    //response_free(resp);
}

http_handler::http_handler(
    http_handler_callback_t http_handler_callback,
    http_handler_async_callback_t http_handler_async_callback)
    :http_handler_callback_(http_handler_callback),
     http_handler_async_callback_(http_handler_async_callback)
{
    use_async_ = http_handler_async_callback_ != NULL;
}


/**
 * @brief http_handler::use_async Use async handler
 * @return
 */
bool
http_handler::use_async()
{
    return use_async_;
}

/**
 * @brief http_handler::use_async Use async handler
 * @param b
 */
void
http_handler::use_async(bool b)
{
    use_async_ = b;
}

/**
 * @brief http_handler::dispatch Dispatch interop handler
 * @param req
 * @return
 */
response_t* http_handler::dispatch(request_t* req)
{
    return (*http_handler_callback_)(req);
}

/**
 * @brief http_handler::dispatch_async Dispatch interop async handler
 * @param req
 * @param callback
 */
void http_handler::dispatch_async(request_t* req, std::function<callback_t<response_t*>> callback)
{
    callback_response_ = callback;
    req->handler_ = this;
    (*http_handler_async_callback_)(req, &async_response_callback_wrap);
}

/**
 * @brief http_handler::callback_response Async handler callback response
 * @return
 */
std::function<http_handler::callback_t<response_t*>>
http_handler::callback_response()
{
    return callback_response_;
}

}
