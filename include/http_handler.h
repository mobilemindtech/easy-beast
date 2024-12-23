#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include <cstdlib>
#include <iostream>
#include <functional>
#include <unordered_map>
#include <string>

#include "http_handler.h"
#include "../public/EasyBeastInterop.h"
#include "extra/optional.h"
#include "extra/string_view.h"

namespace httpserver {



class http_handler{


public:

    template<typename resp_t>
    using callback_t = void(resp_t);

    http_handler() {}
    http_handler(http_handler_callback_t,
                 http_handler_async_callback_t);

    ~http_handler() {}

    response_t*
    dispatch(request_t *);

    void
    dispatch_async(request_t *, std::function<callback_t<response_t*>>);

    std::function<callback_t<response_t*>> callback_response();

    bool
    use_async();

    void
    use_async(bool b);

private:
    http_handler_callback_t http_handler_callback_;
    http_handler_async_callback_t http_handler_async_callback_;
    std::function<callback_t<response_t*>> callback_response_;
    bool use_async_;
};

}

#endif // HTTP_HANDLER_SERVER_H
