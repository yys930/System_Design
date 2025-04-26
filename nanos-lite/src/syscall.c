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
  Log();
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
int fs_open(const char *pathname, int flags, int mode);
static inline _RegSet* sys_open(_RegSet *r) {
  const char *path = (const char *)SYSCALL_ARG2(r);
  int flags = (int)SYSCALL_ARG3(r);
  int mode = (int)SYSCALL_ARG4(r);
  SYSCALL_ARG1(r) = fs_open(path, flags, mode);
  return NULL;
}

ssize_t fs_read(int fd, void *buf, size_t len);
static inline _RegSet* sys_read(_RegSet *r) {
  int fd = (int)SYSCALL_ARG2(r);
  void* buf = (void*)SYSCALL_ARG3(r);
  size_t count = (size_t)SYSCALL_ARG4(r);
  SYSCALL_ARG1(r) = fs_read(fd, buf, count);
  return NULL;
}

int fs_close(int fd);
static inline _RegSet* sys_close(_RegSet *r) {
  int fd = (int)SYSCALL_ARG2(r);
  SYSCALL_ARG1(r) = fs_close(fd);
  return NULL;
}

off_t fs_lseek(int fd, off_t offset, int whence);
static inline _RegSet* sys_lseek(_RegSet *r) {
  int fd = (int)SYSCALL_ARG2(r);
  off_t offset = (off_t)SYSCALL_ARG3(r);
  int whence = (int)SYSCALL_ARG4(r);
  SYSCALL_ARG1(r) = fs_lseek(fd, offset, whence);
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
    case SYS_open: sys_open(r);
                   break;
    case SYS_read: sys_read(r);
                   break; 
    case SYS_close: sys_close(r);
                   break;  
    case SYS_lseek: sys_lseek(r);
                   break;                                     
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
