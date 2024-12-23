
#include <stdlib.h>
#include "../public/EasyBeastInterop.h"
#include "../include/httpserver.h"

#ifdef __cplusplus
extern "C" {
#endif



headers_t* headers_new(int size){
    headers_t* headers = (headers_t*) malloc(sizeof(headers_t));
    headers->headers = (header_t*) malloc(sizeof(header_t) * size);
    headers->size = size;
    return headers;
}


body_t* body_new(const char* content, const char* raw, int size) {
    body_t* body = (body_t*) malloc(sizeof(body_t));
    body->body = content;
    body->body_raw = raw;
    body->size = size;
    return body;
}

request_t* request_new(const char* verb, const char* target){
    request_t* req = (request_t*) malloc(sizeof(request_t));
    req->headers = NULL;
    req->body = NULL;
    req->verb = verb;
    req->target = target;
    req->content_type = NULL;
    return req;
}

response_t* response_new(int status_code){
    response_t* resp = (response_t *) malloc(sizeof(response_t));
    resp->body = NULL;
    resp->headers = NULL;
    resp->status_code = status_code;
    resp->content_type = NULL;
    return resp;
}

void headers_free(headers_t* headers){
    if(headers != NULL){
        free(headers->headers);
        free(headers);
    }
}

void request_free(request_t* req){
    if(req->body != NULL){
        free(req->body);
    }
    headers_free(req->headers);
    free(req);
}

void response_free(response_t* resp){
    if(resp->body != NULL){
        free(resp->body);
    }
    headers_free(resp->headers);
    free(resp);
}

int run(
    char* hostname,
    unsigned short port,
    unsigned short max_thread_count,
    beast_handler_t* handler){

    return httpserver::run(hostname, port, max_thread_count, handler);
}

//typedef void (*http_get_async_callback_t) (request* req, response_callback_t resp);
int run_sync(
    char* hostname,
    unsigned short port,
    unsigned short max_thread_count,
    thread_starter_t thread_starter,
    http_handler_callback_t callback){

    beast_handler_t* handler = (beast_handler_t *) malloc(sizeof(beast_handler_t));
    handler->sync = callback;
    handler->async = NULL;
    handler->thread_starter = thread_starter;
    return run(hostname, port, max_thread_count, handler);
}

int run_async(
    char* hostname,
    unsigned short port,
    unsigned short max_thread_count,
    thread_starter_t thread_starter,
    http_handler_async_callback_t callback){

    beast_handler_t* handler = (beast_handler_t *) malloc(sizeof(beast_handler_t));
    handler->async = callback;
    handler->sync = NULL;
    handler->thread_starter = thread_starter;
    return run(hostname, port, max_thread_count, handler);
}
#ifdef __cplusplus
}
#endif
