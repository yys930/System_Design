#include "cpu/exec.h"

make_EHelper(test) {
 //TODO();
  rtl_and(&t0, &id_dest->val, &id_src->val);
  
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template2(test);
}

make_EHelper(and) {
  //TODO();
  rtl_and(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);

  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template2(and);
}

make_EHelper(xor) {
  //TODO();
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);

  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t0, id_dest->width);
  
  print_asm_template2(xor);
}

make_EHelper(or) {
  //TODO();
  rtl_or(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);
  
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template2(or);
}

make_EHelper(sar) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_sext(&t0, &id_dest->val, id_dest->width);
  rtl_sar(&t0, &t0, &id_src->val);
  operand_write(id_dest, &t0);
  rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template2(sar);
}

make_EHelper(shl) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shl(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);
  rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shr(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);
  rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  //TODO();
  rtl_mv(&t0, &id_dest->val);
  rtl_not(&t0);
  operand_write(id_dest, &t0);

  print_asm_template1(not);
}

make_EHelper(rol) {
  int i;
  rtl_mv(&t1, &id_dest->val);
  for (i = 0; i < id_src->val; ++i) {
    rtl_msb(&t0, &t1, id_dest->width);
    rtl_shli(&t1, &t1, 1);
    rtl_or(&t1, &t1, &t0);
  }
  operand_write(id_dest, &t1);
  rtl_set_CF(&t0);
  rtl_msb(&t1, &t1, id_dest->width);
  rtl_xor(&t0, &t1, &t0);
  rtl_set_OF(&t0);
  
  print_asm_template2(rol);
}

make_EHelper(shld)
{
  rtlreg_t result;
  rtl_shx(&id_dest->val, &id_src->val, &id_src2->val, id_src2->width, true, &result);
  operand_write(id_dest, &result);
  rtl_update_ZFSF(&result, id_dest->width);
  print_asm_template3(shld);
}

make_EHelper(shrd)
{
  rtlreg_t result;
  rtl_shx(&id_dest->val, &id_src->val, &id_src2->val, id_src2->width, false, &result);
  operand_write(id_dest, &result);
  rtl_update_ZFSF(&result, id_dest->width);
  print_asm_template3(shrd);
}
 
