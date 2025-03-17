#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

void print_watchpoint() {
  if(head == NULL){
    printf("No watchpoint now!");
    return;
  }
  for(WP* tem = head; tem != NULL; tem=tem->next) {
    printf("%-4d %-20s\n", tem->NO, tem->expr);
  }
}

bool check_watchpoint() {
  bool flag = false;
  uint32_t newval;
  WP* tem = head;

  while (tem != NULL)
  {
    bool success = false;
    newval = expr(tem->expr,&success);
    if(!success)return false;
    if(newval != tem->preval) {
      printf("Hit watchpoint %d: %s\n", tem->NO, tem->expr);
      printf("Old_value: 0x%x\n", tem->preval);
      printf("New_value: 0x%x\n", newval);
      tem->preval = newval;
      flag = true;
    }
    tem = tem->next;
  }

  return flag;
}

bool free_wp(int NO) {
  WP* pre = NULL;
  WP* cur = head;
  while(cur!=NULL) {
    if(cur->NO == NO) {
      cur->expr[0] = '\0';
      cur->preval = 0;
      if(pre == NULL) {head = cur->next;}
      else {pre->next = cur->next;}
      cur->next = free_;
      free_ = cur;
      return true;
    }
    pre = cur;
    cur = cur->next;
  }
  return false;
}

bool new_wp(char* str) {
  bool success = false;
  uint32_t val = expr(str, &success);
  if(!success)return false;
  if(!free_) {
    printf("Error: No extra space for a new watchpoint.\n");
    return false;
  }
  WP* new_wp = free_;
  free_ = free_->next;
  strcpy(new_wp->expr, str);
  new_wp->preval = val;
  new_wp->next = head;
  head = new_wp;
  return true;
}
