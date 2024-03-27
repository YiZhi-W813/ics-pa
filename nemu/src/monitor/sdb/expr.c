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
#include <memory/paddr.h>
#include <stdlib.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 1, TK_NUM, TK_ADD, TK_SUB, TK_MUL, TK_DIV, TK_LEFT, TK_RIGHT, 
  TK_HEX, TK_REG, TK_EQ, TK_NEQ, TK_LTOEQ, TK_OR, TK_AND, TK_NOT, TK_DEREF
  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
//basic
  {" +", TK_NOTYPE},    // spaces
  {"[0-9]+",TK_NUM},       // numbers
  {"\\+", TK_ADD},         // plus
  {"\\-", TK_SUB},         // sub
  {"\\*", TK_MUL},         // mul
  {"\\/", TK_DIV},         // divide
  {"\\(", TK_LEFT},         // bracket left
  {"\\)", TK_RIGHT},         // bracket right

//extend
  {"x[0-9A-Fa-f]+", TK_HEX}, // hex
  {"\\$[0-9a-z]+", TK_REG},   // regs
  {"\\=\\=", TK_EQ},          // equal
  {"\\!\\=", TK_NEQ},
  {"<\\=", TK_LTOEQ},
  {"\\|\\|", TK_OR},          
  {"\\&\\&", TK_AND},
  {"\\!", TK_NOT},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
int max(int a,int b){
  if(a >= b) return a;else return b;
}

void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0; //指示当前处理到哪
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        printf("substr_len = %d\n",substr_len);

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len; //加上子串长度

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;  //空格不做处理，即消除空格
          case TK_HEX:
            tokens[nr_token].type=rules[i].token_type;
				    strncpy(tokens[nr_token].str + 1,substr_start,substr_len);
            tokens[nr_token].str[0] = '0';
            printf("token str = %s\n",tokens[nr_token].str);
            nr_token++;
				    break;
          case TK_NUM:
            tokens[nr_token].type=rules[i].token_type;
				    strncpy(tokens[nr_token].str,substr_start,substr_len);
            nr_token++;
				    break;
          case TK_ADD:
            tokens[nr_token].type=rules[i].token_type;
            nr_token++;
				    break;
          case TK_SUB:
            tokens[nr_token].type=rules[i].token_type;
            nr_token++;
				    break;
          case TK_MUL:
            tokens[nr_token].type=rules[i].token_type;
            nr_token++;
				    break;
          case TK_DIV:
            tokens[nr_token].type=rules[i].token_type;
            nr_token++;
				    break;
          case TK_LEFT:
            tokens[nr_token].type=rules[i].token_type;
            nr_token++;
				    break;
          case TK_RIGHT:
            tokens[nr_token].type=rules[i].token_type;
            nr_token++;
				    break;
          case TK_REG:
            tokens[nr_token].type=rules[i].token_type;
				    strncpy(tokens[nr_token].str,substr_start,substr_len);
            nr_token++;
				    break;
          case TK_EQ:
            tokens[nr_token].type=rules[i].token_type;
            strcpy(tokens[nr_token].str, "==");
            nr_token++;
				    break;
          case TK_NEQ:
            tokens[nr_token].type=rules[i].token_type;
            strcpy(tokens[nr_token].str, "!=");
            nr_token++;
				    break;
          case TK_LTOEQ:
            tokens[nr_token].type=rules[i].token_type;
            strcpy(tokens[nr_token].str, "<=");
            nr_token++;
				    break;
          case TK_OR:
            tokens[nr_token].type=rules[i].token_type;
            strcpy(tokens[nr_token].str, "||");
            nr_token++;
				    break;
          case TK_AND:
            tokens[nr_token].type=rules[i].token_type;
            strcpy(tokens[nr_token].str, "&&");
            nr_token++;
				    break;
          case TK_NOT:
            tokens[nr_token].type=rules[i].token_type;
            nr_token++;
				    break;
          default:
            assert(0);
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
//  printf("Make tokens success!\n");
  return true;
}

bool check_parentheses(int p, int q)
{   
    if((tokens[p].type != TK_LEFT)  || (tokens[q].type != TK_RIGHT))
        return false; //第一和最后一个token是括号对

    int l = p , r = q;
    while(l < r)
    {
        if(tokens[l].type == TK_LEFT){
            if(tokens[r].type == TK_RIGHT)
            {
                l ++ , r --;
                continue;
            }

            else
                r --;
        }
        else if(tokens[l].type == TK_RIGHT)
            return false;
        else l ++;
    }
    return true;
}

uint32_t eval(int p, int q) {
    printf("compute p:%d q:%d\n",p,q);
    if (p > q) {
        printf("Bad expression because p=%d and q=%d.\n",p,q);
//        printf("p=%d,q=%d.\n",p,q);
        assert(0);
    }
    else if (p == q) {
        /* Single token.
         * For now this token should be a number.
         * Return the value of the number.
         */
        return atoi(tokens[p].str);
    }
    else if (check_parentheses(p, q) == true) {
        /* The expression is surrounded by a matched pair of parentheses.
         * If that is the case, just throw away the parentheses.
         */
        return eval(p + 1, q - 1);
    }

    else {
        int op = 0; // op = the position of 主运算符 in the token expression;
        int prio_min = 0;
        for(int i = p ; i <= q ; i ++)
        {
            if(tokens[i].type == TK_LEFT)
            {
                while(tokens[i].type != TK_RIGHT)
                    i ++;
            }
            if(tokens[i].type == TK_OR){ //优先级12
                if(prio_min <= 12){
                  op = max(op, i);  //同优先级靠后的是主操作符
                  prio_min = 12;
                }
            }
            if(tokens[i].type == TK_AND){ //优先级11
                if(prio_min <= 11){
                  op = max(op, i);  //同优先级靠后的是主操作符
                  prio_min = 11;
                }
            }
            if((tokens[i].type == TK_EQ) || (tokens[i].type == TK_NEQ)){ //优先级7
                if(prio_min <= 7){
                  op = max(op, i);  //同优先级靠后的是主操作符
                  prio_min = 7;
                }
            }
            if(tokens[i].type == TK_LTOEQ){ //优先级6
                if(prio_min <= 6){
                  op = max(op, i);
                  prio_min = 6;
                }
            }
            if((tokens[i].type == TK_ADD) || (tokens[i].type == TK_SUB)){ //优先级4
                if(prio_min <= 4){
                  op = max(op, i);
                  prio_min = 4;
                }
            }
            if((tokens[i].type == TK_MUL) || (tokens[i].type == TK_DIV)){ //优先级3
                if(prio_min <= 3){
                  op = max(op, i);
                  prio_min = 3;
                }
            }
        }

        int  op_type = tokens[op].type;

        uint32_t  val1 = eval(p, op - 1);
        uint32_t  val2 = eval(op + 1, q);

        switch (op_type) {
            case TK_ADD:
                return val1 + val2;
            case TK_SUB:
                return val1 - val2;
            case TK_MUL:
                return val1 * val2;
            case TK_DIV:
                if(val2 == 0){
                    printf("division can't zero;\n");
                    assert(0);
                }
                return val1 / val2;
            case TK_EQ:
                return val1 == val2;
            case TK_NEQ:
                return val1 != val2;
            case TK_LTOEQ:
                return val1 <= val2;
            case TK_OR:
                return val1 || val2;
            case TK_AND:
                return val1 && val2;
            default:
                printf("no match op.\n");
                assert(0);
        }
    }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  int tokens_len = 0;
  for(int i = 0 ; i < 32 ; i ++){
	  if(tokens[i].type == 0){
      break;
    }
    else{
	    tokens_len ++;
    }
  }

  for(int i = 0 ; i < tokens_len ; i ++){
	  printf("before tokens[%d].type = %d\n",i,tokens[i].type);
  }
//  printf("compute tk len success!\n");
  /* TODO: Insert codes to evaluate the expression. */
   for(int i = 0; i < tokens_len; i ++){    //初始化regs
      if(tokens[i].type == TK_REG){
          bool success = false;
          int tmp = isa_reg_str2val(tokens[i].str, &success);    

          if(success) 
            sprintf(tokens[i].str, "%d", tmp);
          else{
            printf("reg2val failed!\n");
            assert(0);
          }
      }
    }

//  printf("initial reg tk success!\n");

    for(int i = 0; i < tokens_len; i ++){   //初始化hex
      if(tokens[i].type == TK_HEX){
          int tmp = strtol(tokens[i].str, NULL, 0);
          sprintf(tokens[i].str, "%d", tmp);
        }
    }

//  printf("initial hex tk success!\n");

    for(int i = 0; i < tokens_len; i ++){ //初始化负数，负号在字符串的最前面或是负号的前面不是数字而后面是数字，则为负数
	    if(tokens[i].type == TK_SUB && ((i > 0 && tokens[i-1].type != TK_NUM && tokens[i+1].type == TK_NUM) || (i == 0))){
        tokens[i].type = TK_NOTYPE; //判断出负号后把负号后一个token的全部内容向后移一位，最前面加个负号
        for(int j = 31; j > 0; j --){
          tokens[i+1].str[j] = tokens[i+1].str[j-1];
        }
        tokens[i+1].str[0] = '-';

        for(int j = i + 1; j < tokens_len; j ++){
          tokens[j - 1] = tokens[j];
        }
        tokens_len --;
	    }
    }

//  printf("initial neg num tk success!\n");
  
    for(int i = 0 ; i < tokens_len ; i ++){ //初始化非运算
	    if(tokens[i].type == TK_NOT){
        tokens[i].type = TK_NOTYPE;
        int tmp = atoi(tokens[i+1].str);
        if(tmp == 0){
          memset(tokens[i+1].str, 0 ,sizeof(tokens[i+1].str));
          tokens[i+1].str[0] = '1';
        }
        else{
          memset(tokens[i+1].str, 0 , sizeof(tokens[i+1].str));
        }
        for(int j = i + 1; j < tokens_len; j ++){
          tokens[j - 1] = tokens[j];
        }
        tokens_len --;
	    } 
    }

//  printf("initial ! tk success!\n");

    for(int i = 0 ; i < tokens_len ; i ++)  //初始化指针解引用
    {
      if((tokens[i].type == TK_MUL && i > 0 && tokens[i-1].type != TK_NUM && tokens[i-1].type != TK_HEX && tokens[i-1].type != TK_REG && tokens[i+1].type == TK_NUM )
        ||(tokens[i].type == TK_MUL && i > 0 && tokens[i-1].type != TK_NUM && tokens[i-1].type != TK_HEX && tokens[i-1].type != TK_REG && tokens[i+1].type == TK_HEX )
        ||(tokens[i].type == TK_MUL && i == 0)){
        printf("tokens[%d].type = %d .str=%s\n",i+1,tokens[i+1].type,tokens[i+1].str);
        tokens[i].type = TK_NOTYPE;
        int addr = atoi(tokens[i+1].str);
        int value = paddr_read(addr, 4);
        sprintf(tokens[i+1].str, "%d", value);	    
        for(int j = i + 1; j < tokens_len ; j ++){
          tokens[j - 1] = tokens[j];
        }
        tokens_len --;
      }
    }


//    printf("initial * tk success!\n");
//    printf("After inital tk len = %d\n",tokens_len);
  *success = true;
  return eval(0, tokens_len - 1);
}