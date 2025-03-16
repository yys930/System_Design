#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
enum {
  TK_NOTYPE = 256,
  
  /* TODO: Add more token types */
  TK_DNUM,
  TK_HNUM,
  TK_REG,

  TK_OR,
  TK_AND,
  TK_EQ,
  TK_NEQ,

  TK_NOT,
  TK_DEREF,
  TK_NEG
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\(", '('},         // (
  {"\\)", ')'},         // )

  {"0x[0-9a-fA-F]+", TK_HNUM},
  {"[0-9]+", TK_DNUM},
  {"\\$(e?[a-d][x]|e?[sb][p]|e?[sd][i]|[a-d][hl]|eip)", TK_REG},

  {"&&", TK_AND},
  {"\\|\\|", TK_OR},

  {"\\+", '+'},         // plus
  {"\\-", '-'},
  {"\\*", '*'},
  {"/", '/'},

  {"==", TK_EQ},         // equal
  {"!=", TK_NEQ},

  {"!", TK_NOT}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
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

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if(nr_token >= 32 || substr_len >= 32) {
          printf("ERROR!!!");
          return false;
        }

        switch (rules[i].token_type) {
          case TK_DNUM:
          case TK_HNUM:
          case TK_REG: {
            tokens[nr_token].type = rules[i].token_type;
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          }
          case '(':
          case ')':
          case '+':
          case '-':
          case '*':
          case '/':
          case TK_OR:
          case TK_AND:
          case TK_EQ:
          case TK_NEQ:
          case TK_NOT: {
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = '\0';
            nr_token++;
            break;
          }

          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static int operator_priority(int op_type) {
  switch (op_type) {
    case TK_OR: return 1;
    case TK_AND: return 2;
    case TK_EQ:
    case TK_NEQ: return 3;
    case '+':
    case '-': return 4;
    case '*':
    case '/': return 5;
    case TK_NOT:
    case TK_DEREF:
    case TK_NEG: return 6;
    default: return 0;
  }
}

static bool check_parentheses(int a, int b) {
  int top = 0;
  for(int i = a; i < b; i++) {
    if(tokens[i].type == '(') {
      top++;
    } else if(tokens[i].type == ')') {
      if(top == 0) return false;
      top--;
    }
  }
  if(top == 0) {
    return true;
  }
  return false;
}

static int dominant_operator(int a, int b) {
	int type, cur_dominant = -1, cur_priority = 99;
	for (int i = a; i < b; ++i) {
		type = tokens[i].type;
		if (type == '(')
		{
			int top = 1;
			for (++i; i < nr_token; ++i)
			{
				if (tokens[i].type == '(')
				{
					top++;
				}
				else if (tokens[i].type == ')')
				{
					top--;
					if (top == 0)
					{
						break;
					}
				}
			}
		}
		else if (type >= TK_OR && type <= TK_NEG) {
			int pri = operator_priority(type);
			if(pri <= cur_priority) {
				cur_dominant = i;
				cur_priority = pri;
			}
		}
	}	
	return cur_dominant;
}

static int find_reg_index(char *reg, const char *regs[8]) {
	for (int i = 0; i < 8; ++i) 
		if (strcmp(reg, regs[i]) == 0)
			return i;
	return -1;
}


static uint32_t regname_to_val(char *name) {
	int index;
	if (strcmp(name, "eip") == 0)
		return cpu.eip;
	else if (name[0] == 'e') {
		index = find_reg_index(name, regsl);
		Assert(index >= 0, "Error: %s doesn't exist!", name);
		return reg_l(index);
	}
	else {
		index = find_reg_index(name, regsw);
		if (index >= 0) 
			return reg_w(index);

		index = find_reg_index(name, regsb);
		Assert(index >= 0, "Error: %s doesn't exist!", name);
		return reg_b(index);
	}	
}

static bool check_parenthes(int a, int b) {
  bool flag1 = (tokens[a].type == '(');
	bool flag2 = false;
	int top = 1;
    for (++a; a < nr_token; ++a) {
        if (tokens[a].type == '(') {
            top++;
        } else if (tokens[a].type == ')') {
            top--;
            if (top == 0) {
                break;
            }
        }
    }
	flag2 = (a == b);
	return (flag1 && flag2);
}

static int eval(int a, int b, bool *success) {
	if (!(*success))
		return -1;

	if (a > b) {
		*success = false;
		printf("Error: Bad expression.");
		return -1;	
	}	


	else if (a == b) {
		int type = tokens[b].type;
		char *str = tokens[b].str;

		if (type == TK_HNUM) {
			int val;
			if (!str || (sscanf(str, "%x", &val) != 1)) {
				*success = false;
				printf("Error: Fail to read hexadecimal number!");
				return -1;
			}
			return val;
		}
		else if (type == TK_DNUM) {
      return atoi(str);
    }
		else if (type == TK_REG) 
			return (int)regname_to_val(str + 1);	
		
		Assert(0, "Neither int nor hex int nor reg!");
	}	


	else if (check_parenthes(a, b) == true){
		return eval(a + 1, b - 1, success);
	}

	
	else {
		int pos = dominant_operator(a, b);
		if (pos < 0) {
			*success  = false;
			printf("Error: Fail to find dominant operator.");
			return -1;
		}

		int op = tokens[pos].type;
	
		if (((op >= TK_OR) && (op <= TK_NEQ)) 
		|| op == '+' || op == '-' || op == '*' || op == '/') {
			int lval = eval(a, pos - 1, success),
					rval = eval(pos + 1, b, success);
			if (!(*success)) 
				return -1;
		
			switch (op) {
				case TK_OR:  return lval || rval;
				case TK_AND: return lval && rval;
				case TK_EQ:  return lval == rval;
				case TK_NEQ: return lval != rval;				 
				case '+': return lval +  rval;
				case '-': return lval -  rval;
				case '*': return lval *  rval;
				case '/':
					if (rval == 0) {
						*success = false;
						printf("Error: Divisor is zero.");
						return -1;
					} 
					return lval / rval;
				default:
					assert(0);
			}
		}

		else if ((op >= TK_NOT) && (op <= TK_NEG)) {
			int rval = eval(pos + 1, b, success);
			if (!(*success)) 
				return -1;
			
			vaddr_t addr;
			uint32_t mem_val;

			switch (op) {
				case TK_NOT:  return !rval;
				case TK_NEG:  return -rval;
				case TK_DEREF:
					addr = rval;
					mem_val = vaddr_read(addr, 4);
					return (int)mem_val;					
				default:
					assert(0);
			}
		}
		else
			Assert(0, "Neither binary operator nor unary operator!");
	}
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  if (!check_parentheses(0, nr_token)) {
		*success = false;
		printf("Syntex Error: Parentheses are not matched!");
		return 0;
	} 

	for (int i = 0; i < nr_token; ++i) {
		if (tokens[i].type == '*' 
				&& (i == 0 || (tokens[i - 1].type>=TK_OR && tokens[i - 1].type <= TK_NEG)
					|| tokens[i - 1].type == '('))
			tokens[i].type = TK_DEREF; 
		else if (tokens[i].type == '-'
				&& (i == 0 || (tokens[i - 1].type>=TK_OR && tokens[i - 1].type <= TK_NEG)
				 	|| tokens[i - 1].type == '('))
			tokens[i].type = TK_NEG;
	}
	
	*success = true;
	return eval(0, nr_token - 1, success);

}
