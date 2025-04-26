#include "common.h"
#include "syscall.h"

static inline _RegSet* sys_none(_RegSet *r) {
  SYSCALL_ARG1(r) = 1;
  return NULL;
}

static inline _RegSet* sys_exit(_RegSet *r) {
  _halt(SYSCALL_ARG2(r));
  return NULL;
}

static inline _RegSet* sys_write(_RegSet *r) {
  int fd = (int)SYSCALL_ARG2(r);
  const void* buf = (const void*)SYSCALL_ARG3(r);
  size_t count = (size_t)SYSCALL_ARG4(r);

  if(fd == 1 || fd == 2) {
    int i;
    for(i = 0;i < count; i++){
      _putc(((char *)buf)[i]);
    }
    SYSCALL_ARG1(r) = i;
  }
  return NULL;
}

static inline _RegSet* sys_brk(_RegSet *r) {
  _heap.end = (void *)SYSCALL_ARG2(r);
  SYSCALL_ARG1(r) = 0;
  return NULL;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);

  switch (a[0]) {
    case SYS_none: sys_none(r);
                   break;
    case SYS_exit: sys_exit(r);
                   break;
    case SYS_write: sys_write(r);
                   break;
    case SYS_brk: sys_brk(r);
                   break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
