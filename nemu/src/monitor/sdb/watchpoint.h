#ifndef WATCHPOINT_H
#define WATCHPOINT_H

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  struct watchpoint *pre;

  /* TODO: Add more members if necessary */
  char expr[100];
  int new_value;
  int old_value;

} WP;

extern WP wp_pool[NR_WP];
extern WP *head __attribute__((unused));
extern WP *free_ __attribute__((unused));

void init_wp_pool();
WP* new_wp();
void free_wp(WP *wp);
void sdb_watchpoint_display();
void delete_watchpoint(int no);
void create_watchpoint(char* args);

#endif