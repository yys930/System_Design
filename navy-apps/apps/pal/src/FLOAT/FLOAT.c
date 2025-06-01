#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

// FLOAT F_mul_F(FLOAT a, FLOAT b) {
//   //assert(0);
//   return 0;
// }
FLOAT F_mul_F(FLOAT a, FLOAT b) {
  int sign = (a ^ b) >> 31;  // 计算符号位

  // 将a和b的符号位清零
  a = a & 0x7FFFFFFF;
  b = b & 0x7FFFFFFF;

  // 执行乘法运算
  int product_hi = (a >> 16) * (b >> 16);  // 高16位的乘积
  int product_lo = (a & 0xFFFF) * (b & 0xFFFF);  // 低16位的乘积

  // 处理溢出
  int carry = product_lo >> 16;  // 检查低16位乘积是否产生进位
  product_hi += carry;  // 加上进位

  // 舍入处理
  int rounding = 0x8000;  // 用于舍入的值
  int result = (product_hi << 16) + ((product_lo + rounding) >> 16);

  // 恢复符号位
  result = (result ^ sign) - sign;

  return result;
}
// FLOAT F_div_F(FLOAT a, FLOAT b) {
//   //assert(0);
//   return 0;
// }
FLOAT F_div_F(FLOAT a, FLOAT b) {
  FLOAT result = Fabs(a) / Fabs(b);
  FLOAT m = Fabs(a);
  FLOAT n = Fabs(b);
  m = m % n;
    //竖式除法
  for (int i = 0; i < 16; i++) {
    m <<= 1;
    result <<= 1;
    if (m >= n) {
      m -= n;
      result++;
    }
  }
  //恢复符号位
  if (((a ^ b) & 0x80000000) == 0x80000000) {
    result = -result;
  }
  return result;
}
// FLOAT f2F(float a) {
//   /* You should figure out how to convert `a' into FLOAT without
//    * introducing x87 floating point instructions. Else you can
//    * not run this code in NEMU before implementing x87 floating
//    * point instructions, which is contrary to our expectation.
//    *
//    * Hint: The bit representation of `a' is already on the
//    * stack. How do you retrieve it to another variable without
//    * performing arithmetic operations on it directly?
//    */

//   //assert(0);
//   return 0;

// }

FLOAT f2F(float a) {
  union float_ {
    struct {
      uint32_t m : 23;
      uint32_t e : 8;
      uint32_t signal : 1;
    };
    uint32_t value;
  };
  union float_ f;
  f.value = *((uint32_t*)(void*)&a);

  int e = f.e - 127;

  FLOAT result;
  if (e <= 7) {
    result = (f.m | (1 << 23)) >> 7 - e;
  }
  else {
    result = (f.m | (1 << 23)) << (e - 7);
  }
  return f.signal == 0 ? result : (result|(1<<31));
}
FLOAT Fabs(FLOAT a) {
  //assert(0);
  //return 0;
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
