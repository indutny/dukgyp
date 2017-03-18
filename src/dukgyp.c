#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "duktape.h"

#include "src/dukgyp-js.h"


static void dukgyp_fatal_handler(void* udata, const char* msg) {
  fprintf(stderr, "Fatal dukgyp error: %s\n", msg);
  exit(-1);
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
    duk_fatal(ctx, "cwd() failure");
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


static duk_ret_t dukgyp_native_fs_realpath(duk_context* ctx) {
  char* p;

  p = realpath(duk_to_string(ctx, 0), NULL);
  duk_push_string(ctx, p);
  free(p);
  return 1;
}


static duk_ret_t dukgyp_native_fs_exists(duk_context* ctx) {
  struct stat st;
  int err;
  const char* arg;

  arg = duk_to_string(ctx, 0);

  do
    err = lstat(arg, &st);
  while (err == -1 && errno == EINTR);

  if (err == -1 && errno != ENOENT)
    duk_fatal(ctx, "fs.exists failure");

  duk_push_boolean(ctx, err == 0);
  return 1;
}


static duk_ret_t dukgyp_native_fs_read_file(duk_context* ctx) {
  struct stat st;
  int fd;
  int err;
  const char* arg;
  char* buf;

  arg = duk_to_string(ctx, 0);

  do
    fd = open(arg, O_RDONLY);
  while (fd == -1 && errno == EINTR);

  if (fd == -1)
    duk_fatal(ctx, "fs.readFile() error: no file");

  do
    err = fstat(fd, &st);
  while (err == -1 && errno == EINTR);

  if (err != 0)
    duk_fatal(ctx, "fs.readFile() error: fstat error");

  /* TODO(indutny): file can change its size between these calls */
  buf = duk_push_fixed_buffer(ctx, st.st_size);

  /* TODO(indutny): read it */
  err = read(fd, buf, st.st_size);
  if (err < 0)
    duk_fatal(ctx, "fs.readFile() error: can't read");

  duk_push_buffer_object(ctx, -1, 0, err, DUK_BUFOBJ_NODEJS_BUFFER);
  duk_remove(ctx, -2);

  close(fd);
  return 1;
}


static duk_ret_t dukgyp_native_fs_write_file(duk_context* ctx) {
  const char* arg;

  arg = duk_to_string(ctx, 0);

  fprintf(stderr, "write: %s\n", arg);

  return 0;
}


static duk_ret_t dukgyp_native_fs_mkdirp(duk_context* ctx) {
  const char* arg;

  arg = duk_to_string(ctx, 0);

  fprintf(stderr, "mkdir: %s\n", arg);

  return 0;
}


static void dukgyp_bindings_fs(duk_context* ctx) {
  duk_push_object(ctx);

  duk_push_c_function(ctx, dukgyp_native_fs_readdir, 0);
  duk_put_prop_string(ctx, -2, "readdirSync");

  duk_push_c_function(ctx, dukgyp_native_fs_realpath, 1);
  duk_put_prop_string(ctx, -2, "realpathSync");

  duk_push_c_function(ctx, dukgyp_native_fs_exists, 1);
  duk_put_prop_string(ctx, -2, "existsSync");

  duk_push_c_function(ctx, dukgyp_native_fs_read_file, 1);
  duk_put_prop_string(ctx, -2, "readFileSync");

  duk_push_c_function(ctx, dukgyp_native_fs_write_file, 2);
  duk_put_prop_string(ctx, -2, "writeFileSync");

  duk_push_c_function(ctx, dukgyp_native_fs_mkdirp, 1);
  duk_put_prop_string(ctx, -2, "mkdirpSync");

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

  /* argv[0] is usually `node` */
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

  duk_eval_string_noresult(ctx, dukgyp_js);

  duk_destroy_heap(ctx);
  return 0;
}