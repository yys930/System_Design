#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  uint32_t preval;
  char* expr;
} WP;

void print_watchpoint();
bool check_watchpoint();
bool free_wp(int NO);
bool new_wp(char* str);
#endif
