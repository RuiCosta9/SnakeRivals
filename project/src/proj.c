/**
 * @file proj.c
 * @brief LCF entry point for the Snake Rivals project.
 */

#include <lcom/lcf.h>
#include <lcom/proj.h>

#include "app/app.h"

static const char *const TRACE_PATH = "/home/lcom/labs/proj_trace.txt";
static const char *const OUTPUT_PATH = "/home/lcom/labs/proj_output.txt";

int main(int argc, char *argv[]) {
  lcf_set_language("EN-US");

#ifdef LCOM_DEBUG
  lcf_trace_calls(TRACE_PATH);
  lcf_log_output(OUTPUT_PATH);
#else
  (void) TRACE_PATH;
  (void) OUTPUT_PATH;
#endif

  int ret = lcf_start(argc, argv);

  if (ret == 0)
    lcf_cleanup();

  return ret;
}

int(proj_main_loop)(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  int ret = app_init();

  if (ret != 0)
    return ret;

  ret = app_run();

  int cleanup_ret = app_cleanup();

  if (ret != 0)
    return ret;

  return cleanup_ret;
}
