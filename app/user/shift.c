#include <stdint.h>

void shift_c(uint8_t symbol){
  asm(
    "li      a5,0\n\t"
    "li      a2,540672\n\t"
    "addi    a2,a2,4\n\t"
    "li      a7,256\n\t"
    "li      a1,540672\n\t"
    "li      a3,1024\n\t"
    "li      a6,8\n\t"
    "j       .L4\n\t"
    ".L2:\n\t"
    "sw      a7,0(a2)\n\t"
    ".L3:\n\t"
    "sw      a3,0(a1)\n\t"
    "sw      a3,0(a2)\n\t"
    "addi    a5,a5,1\n\t"
    "beq     a5,a6,.L6\n\t"
    ".L4:\n\t"
    "sra     a4,a0,a5\n\t"
    "andi    a4,a4,1\n\t"
    "beq     a4,zero,.L2\n\t"
    "sw      a7,0(a1)\n\t"
    "j       .L3\n\t"
    ".L6:\n\t"
    "ret\n\t" );
}