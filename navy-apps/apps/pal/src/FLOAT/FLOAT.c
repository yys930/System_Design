#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  // assert(0);
  // return 0;
  return ((int64_t)a * (int64_t)b) >> 16;
}
FLOAT F_inv(FLOAT x) {
  // 使用牛顿迭代法计算倒数
  // 牛顿法：1 / x ≈ (2 - x * guess) * guess
  FLOAT guess = 1 << 16;  // 初始化猜测值
  for (int i = 0; i < 10; i++) {
    guess = (2 - F_mul_F(x, guess)) * guess;
  }
  return guess;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  // assert(0);
  // return 0;
  assert(b != 0);

  // 使用倒数法来实现除法
  FLOAT reciprocal_b = F_inv(b);  // 计算b的倒数
  return F_mul_F(a, reciprocal_b);  // 用乘法代替除法
  return ((int64_t)a << 16) / b;
}

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */

  // assert(0);
  // return 0;
  union {
    float f;
    uint32_t u;
  } v;
  v.f = a;

  uint32_t sign = v.u >> 31;
  int exp = ((v.u >> 23) & 0xFF) - 127;
  uint32_t frac = (v.u & 0x7FFFFF) | 0x800000; // 1.f, 24位

  int64_t val = (int64_t)frac;

  if (exp >= 7) {
    val = val << (exp - 7);
  } else {
    val = val >> (7 - exp);
  }

  val = val >> 8;  // Q7.24 -> Q16.16
  if (sign) val = -val;

  return (FLOAT)val;

}

FLOAT Fabs(FLOAT a) {
  // assert(0);
  // return 0;
  return a < 0 ? -a : a;
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
