
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../public/EasyBeastInterop.h"

//#define USE_STD_THREAD 1

typedef struct {
  thread_init_t cb;
  void* any;
  int id;
} thread_args;

response_t* callback_sync(request_t* req) {
  response_t* resp = response_new(200);
  resp->content_type = "application/json";
  int size = 6;
  resp->body = body_new("{\"x\": 1}", NULL, size);
  return resp;
}


void *thread_start(void* any) {
  thread_args* args = (thread_args*) any;
  printf("new thread id %d started\n", args->id);
  args->cb(args->any);
  printf("thread %d done\n", args->id);
  pthread_exit(NULL);
}

void thread_starter(thread_init_t thread_init, int thread_count, void* any) {
  pthread_t threads[thread_count];
  void* retval;
  thread_args args[thread_count];
  int rc;
  printf("IO init threads %d\n", thread_count);
  for(int i = 0; i < thread_count; i++) {
    args[i].cb = thread_init;
    args[i].any = any;
    args[i].id = i;
    rc = pthread_create(&threads[i], NULL, thread_start, (void*)&args[i]);
    if(rc){
      printf("can't create thread: %d\n", rc);
      exit(-1);
      return;
    }
  }

  for(int i = 0; i < thread_count; i++) {
    rc = pthread_join(threads[i], &retval);
    if(rc){
      printf("can't join thread: %d\n", rc);
      exit(-1);
      return;
      printf("thread %d exits successfully\n", args[i].id);
    }
  }
  pthread_exit(NULL);
}

int main(int argc, char** argv) {
  run_sync("0.0.0.0", 8080, 4, &thread_starter, &callback_sync);
  return 0;
}
