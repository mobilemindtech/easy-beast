#ifndef EASYBEASTINTEROP_H
#define EASYBEASTINTEROP_H


#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        const char* name;
        const char* value;
    } header_t;

    typedef struct {
        const char* body;
        const char* body_raw;
        long unsigned int size;
    } body_t;

    typedef struct {
        header_t* headers;
        int size;
    } headers_t;

    typedef struct  {
        const char* verb;
        const char* target;
        const char* content_type;
        body_t* body;
        headers_t* headers;
        void *handler_;
    } request_t;


    typedef struct  {
        int status_code;
        char* content_type;
        body_t* body;
        headers_t* headers;
    } response_t;

    typedef void (*response_callback_t)(request_t* req, response_t* resp);

    typedef response_t* (*http_handler_callback_t) (request_t* req);
    typedef void (*http_handler_async_callback_t) (request_t* req, response_callback_t cb);

    /**
     * Call io->run() into new created thread
     */
    typedef void (*thread_init_t)(void*);

    /**
     * Callback to init independent thread and use asio IO. thread_init_t should be called into
     * thread new created thread. The callback of thread_init_t is managed by server implementation, and only
     * call io->run() into created thread.
     */
    typedef void (*thread_starter_t)(thread_init_t, int thread_count, void*);

    typedef struct {
        http_handler_callback_t sync;
        http_handler_async_callback_t async;
        thread_starter_t thread_starter;
    } beast_handler_t;

    // initializers


    headers_t* headers_new(int size);

    body_t* body_new(const char* content, const char* raw, int size) ;

    request_t* request_new(const char* verb, const char* target);

    response_t* response_new(int status_code);

    void headers_free(headers_t* headers);

    void request_free(request_t* req);

    void response_free(response_t* resp);

    int run(
        char* hostname,
        unsigned short port,
        unsigned short max_thread_count,
        beast_handler_t* handler);

    //typedef void (*http_get_async_callback_t) (request* req, response_callback_t resp);
    int run_sync(
        char* hostname,
        unsigned short port,
        unsigned short max_thread_count,
        thread_starter_t thread_starter,
        http_handler_callback_t callback);

    int run_async(
        char* hostname,
        unsigned short port,
        unsigned short max_thread_count,
        thread_starter_t thread_starter,
        http_handler_async_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif // EASYBEASTINTEROP_H
