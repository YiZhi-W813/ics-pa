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

WP wp_pool[NR_WP] = {};
WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].pre = (i == 0 ? NULL : &wp_pool[i - 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
  WP* temp_wp;
  WP* last_free_wp;
  WP* last_used_wp;
  if(free_ == NULL){
    printf("No free watchpoint!");
    assert(0);
  }else{
    temp_wp = free_;
    while (temp_wp -> next != NULL)
    {
      temp_wp = temp_wp -> next;
    }
    last_free_wp = temp_wp;//找到freewp的最后一个节点
  }

  if(head == NULL){
    (last_free_wp -> pre) -> next = NULL;
    last_free_wp -> pre = NULL;
    last_free_wp -> next = NULL;
    head = last_free_wp;
    return last_free_wp;
  }else{
    temp_wp = head;
    while (temp_wp -> next != NULL)
    {
      temp_wp = temp_wp -> next;
    }
    last_used_wp = temp_wp;//找到usedwp的最后一个节点
    if(last_free_wp -> pre == NULL){
      last_free_wp -> next = NULL;
      last_free_wp -> pre = last_used_wp;
      last_used_wp -> next = last_free_wp;
      free_ = NULL;
    }else{
      (last_free_wp -> pre) -> next = NULL;
      last_used_wp -> next = last_free_wp;
      last_free_wp -> pre = last_used_wp;
      last_free_wp -> next = NULL;
    }
    return last_free_wp;
  }
}

void free_wp(WP *wp){
  WP* temp_wp;
  WP* last_free_wp;

  if(free_ == NULL){  //如果free里什么也没有，wp就自己过去,
    free_ = wp;

    if(wp -> pre == NULL){
      (wp -> next) -> pre = NULL;
    }else if (wp -> next == NULL){
      (wp -> pre) -> next = NULL;  
    }else{
      (wp -> pre) -> next = wp -> next;
      (wp -> next) -> pre = wp -> pre;
    }
    
    wp -> pre = NULL;
    wp -> next = NULL;
  }else{
    temp_wp = free_;
    while (temp_wp -> next != NULL)
    {
      temp_wp = temp_wp -> next;
    }
    last_free_wp = temp_wp;//找到freewp的最后一个节点

    if((wp -> pre == NULL) && (wp -> next == NULL)){
      head = NULL;
    }else if(wp -> pre == NULL){
      (wp -> next) -> pre = NULL;
    }else if (wp -> next == NULL){
      (wp -> pre) -> next = NULL;  
    }

      wp -> pre = last_free_wp;
      wp -> next = NULL;
  }
}


void sdb_watchpoint_display(){
  WP *p = head;
  if(head == NULL)
    printf("We dont have watchpoint!\n");
  for(int i = 0; i < NR_WP; i ++ ){
	  printf("Watchpoint.No: %d, expr = \"%s\", old_value = %d, new_value = %d\n", 
		p -> NO, p -> expr, p -> old_value, p -> new_value);
    if(p -> next == NULL)
      break;
    else
      p = p -> next;
  }
}

void delete_watchpoint(int no){
    for(int i = 0 ; i < NR_WP ; i ++)
	    if(wp_pool[i].NO == no){
	      free_wp(&wp_pool[i]);
	      return ;
	    }
}

void create_watchpoint(char* args){
    WP* p =  new_wp();
    strcpy(p -> expr, args);
    bool success = false;
    int tmp = expr(p -> expr,&success);
    printf("The value of this expr is %d",tmp);
    if(success) p -> old_value = tmp;
    else printf("Expression evalution failed!\n");
    printf("Create watchpoint No.%d success.\n", p -> NO);
}