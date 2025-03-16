#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
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

static int cmd_help(char *args);

static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);


static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  
  /* TODO: Add more commands */
  { "si", "Execute N instructions step by step", cmd_si},
  { "info", "Print informations", cmd_info},
  { "x", "Scan memory", cmd_x},
  { "p", "Print the value of an expreesion", cmd_p},
  { "w", "Set a watchpoint", cmd_w},
  { "d", "Delete a watch point", cmd_d}

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

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

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  int steps = (arg != NULL) ? atoi(arg) : 1;
  cpu_exec(steps);
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");

  if (arg == NULL || strlen(arg) != 1)
  {
    printf("Error: Missing argument in cmd_info\n");
    return 0;
  }

  if (arg[0] == 'r')
  {
    printf("\n=== General Purpose Registers (32-bit) ===\n");
    for (int i = 0; i < 8; i++)
    {
      printf("%-8s : 0x%08x\n", regsl[i], reg_l(i));
    }

    printf("\nEIP (Instruction Pointer)\n");
    printf("%-8s : 0x%08x\n", "eip", cpu.eip);
    printf("\n=== General Purpose Registers (16-bit) ===\n");
    for (int i = 0; i < 8; i++) {
      printf("%-8s : 0x%04x\n", regsw[i], reg_w(i));
    }
    printf("\n=== General Purpose Registers (8-bit) ===\n");
    for (int i = 0; i < 8; i++) {
      printf("%-8s : 0x%02x\n", regsb[i], reg_b(i));
    }
    printf("\n");
  }
  else if (arg[0] == 'w')
  {
    printf("\n=== Watchpoints Information ===\n");
    print_watchpoint();
  }
  else
  {
    printf("Error: Invalid argum");
  }
  return 0;
}

static int cmd_x(char *args) {
  char *arg1 = strtok(NULL, " ");
  if (arg1 == NULL) {
      printf("Error: Missing parameter N. Please specify the number of consecutive memory reads.\n");
      return 0;
  }

  int num_reads = atoi(arg1);
  if (num_reads <= 0) {
      printf("Error: Invalid value for N. It must be a positive integer.\n");
      return 0;
  }

  char *arg2 = strtok(NULL, " ");
  if (arg2 == NULL) {
      printf("Error: Missing parameter EXPR. Please provide a valid memory address in hexadecimal format.\n");
      return 0;
  }
  
  bool finish = false;
  uint32_t addr_begin = strtoul(arg2, NULL, 16);
  if(!finish) {
    return 0;
  }
  printf("\nMemory Dump (Starting at 0x%08x):\n", addr_begin);
  for (int i = 0; i < num_reads; i++) {
      printf("0x%08x: 0x%02x\n", addr_begin, vaddr_read(addr_begin, 1));
      addr_begin += 1;
  }
  printf("\n");

  return 0;
}

static int cmd_p(char *args) {
  if(args == NULL) {
    printf("Error: Expression is required.");
    return 0;
  }

  bool finish = false;
  uint32_t value = expr(args, &finish);

  if(finish) {
    printf("Expression: %s\n", args);
    printf("Result: %u (unsigned) = %d (signed) = 0x%X (hex)\n", value, (int)value, value);
  }
  else {
    printf("Error: Failed to parse the expression.");
  }
  return 0;
}

static int cmd_w(char *args) {
  return 0;
}
static int cmd_d(char *args) {
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
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

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
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
