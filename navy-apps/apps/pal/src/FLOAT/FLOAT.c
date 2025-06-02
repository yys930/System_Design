#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b)
{
  // assert(0);
  // return 0;
  return ((int64_t)a * (int64_t)b) >> 16;
}

FLOAT F_div_F(FLOAT a, FLOAT b)
{
  // assert(0);
  // return 0;
  assert(b != 0);

  // 先取绝对值做无符号除法
  FLOAT abs_num = Fabs(a);
  FLOAT abs_den = Fabs(b);
  FLOAT result = abs_num / abs_den;
  FLOAT remainder = abs_num % abs_den;

  // 模拟小数位右移，对结果进行16次“乘2”的迭代逼近
  for (int i = 0; i < 16; ++i) {
    remainder <<= 1;
    result <<= 1;
    if (remainder >= abs_den) {
      remainder -= abs_den;
      result += 1;
    }
  }

  // 如果原始两个数符号相反，结果取反
  if ((a ^ b) < 0) {
    result = -result;
  }

  return result;
}

struct float_
  {
    uint32_t frac : 23;
    uint32_t exp : 8;
    uint32_t sign : 1;
  };

FLOAT f2F(float a)
{
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */
  
  struct float_ *f = (struct float_ *)&a;

  // 屏蔽 NaN/Inf
  assert(f->exp != 0xFF);

  uint32_t frac = f->frac;
  int e = 0;

  if (f->exp == 0) {
    // 非规格化数
    e = 1 - 127;
  } else {
    // 规格化数，加上隐藏位
    frac |= (1 << 23);
    e = f->exp - 127;
  }

  uint32_t fixed = 0;
  int shift = e - 7;

  if (shift >= 0 && shift < 32) {
    fixed = frac << shift;
  } else if (shift < 0 && shift > -32) {
    fixed = frac >> -shift;
  } else {
    // 超出定点数表示范围
    assert(0);
  }

  return f->sign ? -fixed : fixed;
}

FLOAT Fabs(FLOAT a)
{
  return (a > 0) ? a : -a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
