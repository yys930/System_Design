#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"
#include <stdlib.h>

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  uint32_t preval;
  char expr[64];
} WP;

void print_watchpoint();
bool check_watchpoint();
bool free_wp(int NO);
bool new_wp(char* str);
#endif
