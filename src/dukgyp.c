#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "duktape.h"

#include "src/dukgyp-js.h"

static size_t kDukgypReadBlock = 65536;


typedef struct dukgyp_exec_opt_s dukgyp_exec_opt_t;

struct dukgyp_exec_opt_s {
  const char* cwd;
  int inherit_stdio;
};


static char* dukgyp_read_fd(int fd, size_t suggested_size, size_t* out_size) {
  char* res;
  size_t off;
  size_t size;
  int err;

  off = 0;
  size = suggested_size;
  if (size == 0)
    size = kDukgypReadBlock;
  res = malloc(size);
  if (res == NULL)
    return NULL;

  for (;;) {
    do
      err = read(fd, res + off, size - off);
    while (err == -1 && errno == EINTR);

    if (err < 0)
      goto fail_read;

    off += err;
    if (off == size) {
      char* tmp;

      size += kDukgypReadBlock;
      tmp = realloc(res, size);
      if (tmp == NULL)
        goto fail_read;

      res = tmp;
    }

    /* EOF */
    if (err == 0)
      break;
  }

  *out_size = off;
  return res;

fail_read:
  free(res);
  return NULL;
}


static int dukgyp_close_fd(int fd) {
  int err;

  do
    err = close(fd);
  while (err == -1 && errno == EINTR);

  return err;
}


static char* dukgyp_exec_cmd(duk_context* ctx, const char* cmd,
                             dukgyp_exec_opt_t* options,
                             size_t* out_len) {
  int err;
  pid_t pid;
  int pair[2];
  char* buf;

  /* TODO(indutny): CLOEXEC */
  if (!options->inherit_stdio) {
    err = socketpair(AF_UNIX, SOCK_STREAM, 0, pair);
    if (err != 0)
      duk_fatal(ctx, "socketpair() failure");
  }

  pid = fork();
  if (pid == -1)
    duk_fatal(ctx, "fork() failure");

  /* Child process */
  if (pid == 0) {
    const char* argv[] = { "/bin/sh", "-c", cmd, NULL };

    /* Replace stdout */
    if (!options->inherit_stdio) {
      int fd;

      /* Black hole stdin/stderr */
      do
        fd = open("/dev/null", O_RDWR);
      while (fd == -1 && errno == EINTR);

      do
        err = dup2(fd, 0);
      while (err == -1 && errno == EINTR);
      do
        err = dup2(fd, 2);
      while (err == -1 && errno == EINTR);

      /* Pipe instead of stdout */
      do
        err = dup2(pair[1], 1);
      while (err == -1 && errno == EINTR);

      dukgyp_close_fd(pair[1]);
      dukgyp_close_fd(fd);
    }

    if (err == -1)
      abort();

    err = execvp(argv[0], (char**) argv);
    if (err != 0)
      abort();

    /* Not reachable */
    abort();
    return NULL;
  }

  if (options->inherit_stdio)
    return NULL;

  dukgyp_close_fd(pair[1]);
  buf = dukgyp_read_fd(pair[0], 0, out_len);
  dukgyp_close_fd(pair[0]);

  return buf;
}


static void dukgyp_fatal_handler(void* udata, const char* msg) {
  fprintf(stderr, "Fatal dukgyp error: %s\n", msg);
  exit(-1);
}


static duk_ret_t dukgyp_native_log(duk_context* ctx) {
  fprintf(stdout, "%s", duk_to_string(ctx, 0));
  return 0;
}


static duk_ret_t dukgyp_native_error(duk_context* ctx) {
  fprintf(stderr, "%s", duk_to_string(ctx, 0));
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
  char* storage;
  size_t len;

  arg = duk_to_string(ctx, 0);

  do
    fd = open(arg, O_RDONLY);
  while (fd == -1 && errno == EINTR);

  if (fd == -1 && errno == ENOENT)
    duk_fatal(ctx, "fs.readFile() error: no file");
  else if (fd == -1)
    duk_fatal(ctx, "fs.readFile() error: other failure");

  do
    err = fstat(fd, &st);
  while (err == -1 && errno == EINTR);

  if (err != 0)
    duk_fatal(ctx, "fs.readFile() error: fstat error");

  buf = dukgyp_read_fd(fd, st.st_size + 1, &len);

  dukgyp_close_fd(fd);

  if (buf == NULL)
    duk_fatal(ctx, "fs.readFile() error: can't read");

  /* TODO(indutny): avoid copying */
  storage = duk_push_fixed_buffer(ctx, len);
  memcpy(storage, buf, len);
  free(buf);

  duk_push_buffer_object(ctx, -1, 0, len, DUK_BUFOBJ_NODEJS_BUFFER);
  duk_remove(ctx, -2);

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


static duk_ret_t dukgyp_native_cp_exec(duk_context* ctx) {
  const char* cmd;
  dukgyp_exec_opt_t opts;
  char* out;
  size_t len;

  cmd = duk_to_string(ctx, 0);
  if (duk_is_object(ctx, 1)) {
    const char* stdio;

    duk_get_prop_string(ctx, 1, "cwd");
    opts.cwd = duk_to_string(ctx, -1);
    duk_pop(ctx);

    duk_get_prop_string(ctx, 1, "stdio");
    stdio = duk_to_string(ctx, -1);
    duk_pop(ctx);

    opts.inherit_stdio = strcmp(stdio, "inherit") == 0;
  } else {
    opts.cwd = NULL;
    opts.inherit_stdio = 0;
  }

  out = dukgyp_exec_cmd(ctx, cmd, &opts, &len);
  if (out == NULL) {
    if (!opts.inherit_stdio)
      duk_fatal(ctx, "dukgyp_exec_cmd() failure");

    duk_push_null(ctx);
    return 1;
  }

  duk_push_string(ctx, out);
  free(out);
  return 1;
}


static void dukgyp_bindings_child_process(duk_context* ctx) {
  duk_push_object(ctx);

  duk_push_c_function(ctx, dukgyp_native_cp_exec, 2);
  duk_put_prop_string(ctx, -2, "execSync");

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

  signal(SIGPIPE, SIG_IGN);

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
