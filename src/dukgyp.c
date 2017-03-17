#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "duktape.h"

#include "src/dukgyp-js.h"


static void dukgyp_fatal_handler(void* udata, const char* msg) {
  fprintf(stderr, "Fatal dukgyp error: %s\n", msg);
  abort();
}


static duk_ret_t dukgyp_native_log(duk_context* ctx) {
  fprintf(stdout, "%s\n", duk_to_string(ctx, 0));
  return 0;
}


static duk_ret_t dukgyp_native_error(duk_context* ctx) {
  fprintf(stderr, "%s\n", duk_to_string(ctx, 0));
  return 0;
}


static duk_ret_t dukgyp_native_exit(duk_context* ctx) {
  exit(duk_to_int(ctx, 0));
  return 0;
}


static duk_ret_t dukgyp_native_cwd(duk_context* ctx) {
  char* cwd;

  cwd = getwd(NULL);
  if (cwd == NULL)
    abort();
  duk_push_string(ctx, cwd);
  free(cwd);
  return 1;
}


static void dukgyp_bindings_general(duk_context* ctx) {
  duk_push_c_function(ctx, dukgyp_native_log, 1);
  duk_put_prop_string(ctx, -2, "log");

  duk_push_c_function(ctx, dukgyp_native_error, 1);
  duk_put_prop_string(ctx, -2, "error");

  duk_push_c_function(ctx, dukgyp_native_exit, 1);
  duk_put_prop_string(ctx, -2, "exit");

  duk_push_c_function(ctx, dukgyp_native_cwd, 0);
  duk_put_prop_string(ctx, -2, "cwd");

  /* TODO(indutny): replace this with define */
  duk_push_string(ctx, "darwin");
  duk_put_prop_string(ctx, -2, "platform");
}


/* NOTE: just a stub */
static duk_ret_t dukgyp_native_fs_readdir(duk_context* ctx) {
  duk_push_array(ctx);
  return 1;
}


static void dukgyp_bindings_fs(duk_context* ctx) {
  duk_push_object(ctx);

  duk_push_c_function(ctx, dukgyp_native_fs_readdir, 0);
  duk_put_prop_string(ctx, -2, "readdirSync");

  duk_put_prop_string(ctx, -2, "fs");
}


static void dukgyp_bindings_child_process(duk_context* ctx) {
  duk_push_object(ctx);

  duk_put_prop_string(ctx, -2, "childProcess");
}


static void dukgyp_bindings_env(duk_context* ctx) {
  duk_push_object(ctx);
  duk_put_prop_string(ctx, -2, "env");
}


static void dukgyp_bindings_argv(duk_context* ctx, int argc, char** argv) {
  int i;

  duk_push_array(ctx);

  duk_push_string(ctx, "duktape");
  duk_put_prop_index(ctx, -2, 0);

  for (i = 0; i < argc; i++) {
    duk_push_string(ctx, argv[i]);
    duk_put_prop_index(ctx, -2, i + 1);
  }

  duk_put_prop_string(ctx, -2, "argv");
}


int main(int argc, char** argv) {
  duk_context* ctx;

  ctx = duk_create_heap(NULL, NULL, NULL, NULL, dukgyp_fatal_handler);
  if (ctx == NULL)
    abort();

  duk_push_global_object(ctx);

  /* `bindings` */
  duk_push_object(ctx);
  dukgyp_bindings_general(ctx);
  dukgyp_bindings_fs(ctx);
  dukgyp_bindings_child_process(ctx);
  dukgyp_bindings_env(ctx);
  dukgyp_bindings_argv(ctx, argc, argv);

  duk_put_prop_string(ctx, -2, "bindings");

  duk_pop(ctx);

  duk_eval_string(ctx, dukgyp_js);

  duk_destroy_heap(ctx);
  return 0;
}
