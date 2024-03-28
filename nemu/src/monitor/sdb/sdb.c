/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <memory/paddr.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <cpu/cpu.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args) {
  int n = 0;
  if(args == NULL)n = 1;else sscanf(args, "%d", &n);
  printf("execute %d steps.\n", n);
  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args) {
  if(strcmp(args, "r") == 0){
    isa_reg_display();
  }else if(strcmp(args, "w") == 0){
    sdb_watchpoint_display();
  }else printf("invalid args.\n");
  return 0;
}

static int cmd_x(char *args) {
    char* arg1;
    char* arg2;
    int len = 0;
    paddr_t baseaddr = 0;

    arg1 = strtok(args, " ");
    arg2 = strtok(NULL, " ");

    sscanf(arg1, "%d", &len);
    sscanf(arg2, "%x", &baseaddr);

    for(int i = 0 ; i < len ; i ++)
    {
        printf("%x\n",paddr_read(baseaddr,4));
        baseaddr = baseaddr + 4;
    }
  return 0;
}

static int cmd_p(char *args) {
  int result = 0;
  bool success = false;
  result = expr(args, &success);
  printf("The result of the expression is %d.\n", result);
  return 0;
}

static int cmd_w(char* args){
    create_watchpoint(args);
    return 0;
}

static int cmd_d (char *args){
    if(args == NULL)
        printf("No args.\n");
    else{
        delete_watchpoint(atoi(args));
    }
    return 0;
}


static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single-step execution n setps", cmd_si },
  { "info", "Use info r to print the status of registers or use info w to print monitoring point information", cmd_info },
  { "x", "Scan memory", cmd_x },
  { "p", "Evolution expression", cmd_p },
  { "w", "Create a watchpoint for u.", cmd_w },
  { "d", "Delete a watchpoint u choose", cmd_d },
  /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
