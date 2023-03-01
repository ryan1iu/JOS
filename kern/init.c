/* See COPYRIGHT for copyright information. */

#include <inc/assert.h>
#include <inc/stdio.h>
#include <inc/string.h>

#include <kern/console.h>
#include <kern/env.h>
#include <kern/kclock.h>
#include <kern/monitor.h>
#include <kern/pmap.h>
#include <kern/trap.h>

void i386_init(void) {
  extern char edata[], end[]; // ？如何通过这两个变量确定BSS段的位置
  // BSS段中可能存在多个未初始化的全局或静态变量，为了使edata和end分别在BSS段的首尾
  // 需要利用链接器来为其指定地址

  // Before doing anything else, complete the ELF loading process.
  // Clear the uninitialized global data (BSS) section of our program.
  // This ensures that all static/global variables start out zero.
  memset(edata, 0, end - edata);

  // Initialize the console.
  cons_init();

  cprintf("6828 decimal is %o octal!\n", 6828);

  // Lab 2 memory management initialization functions
  mem_init();

  // Lab 3 user environment initialization functions
  env_init();

  trap_init();

#if defined(TEST)
  // Don't touch -- used by grading script!
  ENV_CREATE(TEST, ENV_TYPE_USER);
#else
  // Touch all you want.
  ENV_CREATE(user_hello, ENV_TYPE_USER);
#endif // TEST*

  // We only have one user environment for now, so just run it.
  env_run(&envs[0]);
  // Drop into the kernel monitor.
  while (1)
    monitor(NULL);
}

/*
 * Variable panicstr contains argument to first call to panic; used as flag
 * to indicate that the kernel has already called panic.
 */
const char *panicstr;

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then enters the kernel monitor.
 */
void _panic(const char *file, int line, const char *fmt, ...) {
  va_list ap;

  if (panicstr)
    goto dead;
  panicstr = fmt;

  // Be extra sure that the machine is in as reasonable state
  asm volatile("cli; cld");

  va_start(ap, fmt);
  cprintf("kernel panic at %s:%d: ", file, line);
  vcprintf(fmt, ap);
  cprintf("\n");
  va_end(ap);

dead:
  /* break into the kernel monitor */
  while (1)
    monitor(NULL);
}

/* like panic, but don't */
void _warn(const char *file, int line, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  cprintf("kernel warning at %s:%d: ", file, line);
  vcprintf(fmt, ap);
  cprintf("\n");
  va_end(ap);
}
