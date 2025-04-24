#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  uint32_t t0 = cpu.cs;
  rtl_push(&cpu.eflags);
  rtl_push(&t0);
  rtl_push(&ret_addr);

  uint32_t base = cpu.idtr.base;
  uint32_t low = vaddr_read(base+NO*8, 4) & 0x0000ffff;
  uint32_t high = vaddr_read(base+NO*8+4, 4) & 0xffff0000;
  uint32_t jmp_addr = low | high;

  decoding.jmp_eip = jmp_addr;
  decoding.is_jmp = 1;
}

void dev_raise_intr() {
}
