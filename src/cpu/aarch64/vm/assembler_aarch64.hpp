/*
 * Copyright (c) 1997, 2011, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef CPU_AARCH64_VM_ASSEMBLER_AARCH64_HPP
#define CPU_AARCH64_VM_ASSEMBLER_AARCH64_HPP

#include "register_aarch64.hpp"

#define assert_cond(ARG1) assert(ARG1, #ARG1)

namespace asm_util {
  uint32_t encode_immediate_v2(int is32, uint64_t imm);
};

using namespace asm_util;


class Assembler_aarch64;

class Instruction {
  unsigned insn;
  unsigned bits;
  Assembler_aarch64 *assem;

public:

  Instruction(class Assembler_aarch64 *as) {
    bits = 0;
    insn = 0;
    assem = as;
  }

  ~Instruction();

  unsigned &get_insn() { return insn; }
  unsigned &get_bits() { return bits; }

  static inline int32_t extend(unsigned val, int hi = 31, int lo = 0) {
    union {
      unsigned u;
      int n;
    };

    u = val << (31 - hi);
    n = n >> (31 - hi + lo);
    return n;
  }

  void f(unsigned val, int msb, int lsb) {
    int nbits = msb - lsb + 1;
    assert_cond(val < (1U << nbits));
    assert_cond(msb >= lsb);
    unsigned mask = (1U << nbits) - 1;
    val <<= lsb;
    mask <<= lsb;
    insn |= val;
    assert_cond((bits & mask) == 0);
    bits |= mask;
  }

  void f(unsigned val, int bit) {
    f(val, bit, bit);
  }

  void sf(long val, int msb, int lsb) {
    int nbits = msb - lsb + 1;
    long chk = val >> (nbits - 1);
    assert_cond (chk == -1 || chk == 0);
    unsigned uval = val;
    unsigned mask = (1U << nbits) - 1;
    uval &= mask;
    f(uval, lsb + nbits - 1, lsb);
  }

  void rf(Register r, int lsb) {
    f(r->encoding_nocheck(), lsb + 4, lsb);
  }

  void rf(FloatRegister r, int lsb) {
    f(r->encoding_nocheck(), lsb + 4, lsb);
  }

  unsigned get(int msb = 31, int lsb = 0) {
    int nbits = msb - lsb + 1;
    unsigned mask = ((1U << nbits) - 1) << lsb;
    assert_cond(bits & mask == mask);
    return (insn & mask) >> lsb;
  }

  void fixed(unsigned value, unsigned mask) {
    assert_cond ((mask & bits) == 0);
    bits |= mask;
    insn |= value;
  }
};

#define starti Instruction do_not_use(this); set_current(&do_not_use)

class Pre {
  int _offset;
  Register _r;
public:
  Pre(Register reg, int o) : _r(reg), _offset(o) { }
  int offset() { return _offset; }
  Register reg() { return _r; }
};

class Post {
  int _offset;
  Register _r;
public:
  Post(Register reg, int o) : _r(reg), _offset(o) { }
  int offset() { return _offset; }
  Register reg() { return _r; }
};

// Address_aarch64ing modes
class Address_aarch64 VALUE_OBJ_CLASS_SPEC {
 public:
  enum mode { base_plus_offset, pre, post, pcrel,
	      base_plus_offset_reg, base_plus_offset_reg_extended };
 private:
  Register _base;
  Register _index;
  int _offset;
  enum mode _mode;
  address _adr;
  int _scale;

 public:
  Address_aarch64(Register r)
    : _mode(base_plus_offset), _base(r), _offset(0) { }
  Address_aarch64(Register r, int o)
    : _mode(base_plus_offset), _base(r), _offset(o) { }
  Address_aarch64(Register r, Register r1, int scale = 0)
    : _mode(base_plus_offset_reg), _base(r), _index(r1), _scale(scale) { }
  Address_aarch64(Pre p)
    : _mode(pre), _base(p.reg()), _offset(p.offset()) { }
  Address_aarch64(Post p)
    : _mode(post), _base(p.reg()), _offset(p.offset()) { }

  void encode(Instruction *i) {
    i->f(0b111, 29, 27);
    i->rf(_base, 5);

    switch(_mode) {
    case base_plus_offset:
      {
	unsigned size = i->get(31, 30);
	unsigned mask = (1 << size) - 1;
	if (_offset < 0 || _offset & mask)
	  {
	    i->f(0b00, 25, 24);
	    i->f(0, 21), i->f(0b00, 11, 10);
	    i->sf(_offset, 20, 12);
	  } else {
	    i->f(0b01, 25, 24);
	    _offset >>= size;
	    i->f(_offset, 21, 10);
	  }
      }
      break;

    case base_plus_offset_reg:
      assert_cond(_scale == 0);
      i->f(0b00, 25, 24);
      i->f(1, 21);
      i->rf(_index, 16);
      i->f(0b011, 15, 13); // Offset is always an X register
      i->f(0, 12); // Shift is 0
      i->f(0b10, 11, 10);
      break;

    case pre:
      i->f(0b00, 25, 24);
      i->f(0, 21), i->f(0b11, 11, 10);
      i->f(_offset, 20, 12);
      break;

    case post:
      i->f(0b00, 25, 24);
      i->f(0, 21), i->f(0b01, 11, 10);
      i->f(_offset, 20, 12);
      break;

    default:
      assert_cond(false);
    }
  }
};

namespace ext
{
  enum operation { uxtb, uxth, uxtw, uxtx, sxtb, sxth, sxtw, sxtx };
};

class Assembler_aarch64 : public AbstractAssembler {
public:
  Address_aarch64 pre(Register base, int offset) {
    return Address_aarch64(Pre(base, offset));
  }

  Address_aarch64 post (Register base, int offset) {
    return Address_aarch64(Post(base, offset));
  }

  Instruction* current;
public:
  void set_current(Instruction* i) { current = i; }

  void f(unsigned val, int msb, int lsb) {
    current->f(val, msb, lsb);
  }
  void f(unsigned val, int msb) {
    current->f(val, msb, msb);
  }
  void sf(long val, int msb, int lsb) {
    current->sf(val, msb, lsb);
  }
  void rf(Register reg, int lsb) {
    current->rf(reg, lsb);
  }
  void rf(FloatRegister reg, int lsb) {
    current->rf(reg, lsb);
  }
  void fixed(unsigned value, unsigned mask) {
    current->fixed(value, mask);
  }

  void emit() {
    emit_long(current->get_insn());
    assert_cond(current->get_bits() == 0xffffffff);
    current = NULL;
  }

  // PC-rel. addressing
#define INSN(NAME, op, shift)						\
  void NAME(Register Rd, address adr) {					\
    long offset = adr - pc();						\
    offset >>= shift;							\
    int offset_lo = offset & 3;						\
    offset >>= 2;							\
    starti;								\
    f(0, 31), f(offset_lo, 30, 29), f(0b10000, 28, 24), sf(offset, 23, 5); \
    rf(Rd, 0);								\
  }

  INSN(adr, 0, 0);
  INSN(adrp, 1, 12);

#undef INSN
  // Add/subtract (immediate)
#define INSN(NAME, decode)						\
  void NAME(Register Rd, Register Rn, unsigned imm, unsigned shift = 0) { \
    starti;								\
    f(decode, 31, 29), f(0b10001, 28, 24), f(shift, 23, 22), f(imm, 21, 10); \
    rf(Rd, 0), rf(Rn, 5);						\
  }

  INSN(addwi,  0b000);
  INSN(addswi, 0b001);
  INSN(subwi,  0b010);
  INSN(subswi, 0b011);
  INSN(addi,   0b100);
  INSN(addsi,  0b101);
  INSN(subi,   0b110);
  INSN(subsi,  0b111);

#undef INSN

 // Logical (immediate)
#define INSN(NAME, decode, is32)				\
  void NAME(Register Rd, Register Rn, uint64_t imm) {		\
    starti;							\
    uint32_t val = encode_immediate_v2(is32, imm);		\
    f(decode, 31, 29), f(0b100100, 28, 23), f(val, 22, 10);	\
    rf(Rd, 0), rf(Rn, 5);					\
  }

  INSN(andwi, 0b000, true);
  INSN(orrwi, 0b001, true);
  INSN(eorwi, 0b000, true);
  INSN(andswi, 0b011, true);
  INSN(andi,  0b100, false);
  INSN(orri,  0b101, false);
  INSN(eori,  0b100, false);
  INSN(andsi, 0b111, false);

#undef INSN

  // Move wide (immediate)
#define INSN(NAME, opcode)						\
  void NAME(Register Rd, unsigned imm, unsigned shift = 0) {		\
    starti;								\
    f(opcode, 31, 29), f(0b100101, 28, 23), f(shift, 22, 21), f(imm, 20, 5); \
    rf(Rd, 0);								\
  }

  INSN(movnw, 0b000);
  INSN(movzw, 0b010);
  INSN(movkw, 0b011);
  INSN(movn, 0b100);
  INSN(movz, 0b110);
  INSN(movk, 0b111);

#undef INSN

  // Bitfield
#define INSN(NAME, opcode)						\
  void NAME(Register Rd, Register Rn, unsigned immr, unsigned imms) {	\
    starti;								\
    f(opcode, 31, 22), f(immr, 21, 16), f(imms, 15, 10);		\
    rf(Rn, 5), rf(Rd, 0);						\
  }

  INSN(sbfmw, 0b0000);
  INSN(bfmw,  0b0010);
  INSN(ubfmw, 0b0100);
  INSN(sbfm,  0b1000);
  INSN(bfm,   0b1010);
  INSN(ubfm,  0b1100);

#undef INSN

  // Extract
#define INSN(NAME, opcode)						\
  void NAME(Register Rd, Register Rn, Register Rm, unsigned imms) {	\
    starti;								\
    f(opcode, 31, 21), f(imms, 15, 10);					\
    rf(Rm, 16), rf(Rn, 5), rf(Rd, 0);					\
  }

  INSN(extrw, 0b00010011100);
  INSN(extr,  0b10010011110);

#undef INSN

  // Unconditional branch (immediate)
#define INSN(NAME, opcode)					\
  void NAME(address dest) {					\
    starti;							\
    long offset = (dest - pc()) >> 2;				\
    f(opcode, 31), f(0b00101, 30, 26), sf(offset, 25, 0);	\
  }

  INSN(b, 0);
  INSN(bl, 1);

#undef INSN

  // Compare & branch (immediate)
#define INSN(NAME, opcode)				\
  void NAME(Register Rt, address dest) {		\
    long offset = (dest - pc()) >> 2;			\
    starti;						\
    f(opcode, 31, 24), sf(offset, 23, 5), rf(Rt, 0);	\
  }

  INSN(cbzw,  0b00110100);
  INSN(cbnzw, 0b00110101);
  INSN(cbz,   0b10110100);
  INSN(cbnz,  0b10110101);

#undef INSN

  // Test & branch (immediate)
#define INSN(NAME, opcode)						\
  void NAME(Register Rt, int bitpos, address dest) {			\
    long offset = (dest - pc()) >> 2;					\
    int b5 = bitpos >> 5;						\
    bitpos &= 0x1f;							\
    starti;								\
    f(b5, 31), f(opcode, 30, 24), f(bitpos, 23, 19), sf(offset, 18, 5);	\
    rf(Rt, 0);								\
  }

  INSN(tbz,  0b0110110);
  INSN(tbnz, 0b0110111);

#undef INSN

  // Conditional branch (immediate)
  void cond_branch(int cond, address dest) {
    long offset = (dest - pc()) >> 2;
    starti;
    f(0b0101010, 31, 25), f(0, 24), sf(offset, 23, 5), f(0, 4), f(cond, 3, 0);
  }

  enum condition_code
    {EQ, NE, HS, CS=HS, LO, CC=LO, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE, AL, NV};

#define INSN(NAME, cond)			\
  void NAME(address dest) {			\
    cond_branch(cond, dest);			\
  }

  INSN(beq, EQ);
  INSN(bne, NE);
  INSN(bhs, HS);
  INSN(bcs, CS);
  INSN(blo, LO);
  INSN(bcc, CC);
  INSN(bmi, MI);
  INSN(bpl, PL);
  INSN(bvs, VS);
  INSN(bvc, VC);
  INSN(bhi, HI);
  INSN(bls, LS);
  INSN(bge, GE);
  INSN(blt, LT);
  INSN(bgt, GT);
  INSN(ble, LE);
  INSN(bal, AL);
  INSN(bnv, NV);

#undef INSN

  // Exception generation
  void generate_exception(int opc, int op2, int LL, unsigned imm) {
    starti;
    f(0b11010100, 31, 24);
    f(opc, 23, 21), f(imm, 20, 5), f(op2, 4, 2), f(LL, 1, 0);
  }

#define INSN(NAME, opc, op2, LL)		\
  void NAME(unsigned imm) {			\
    generate_exception(opc, op2, LL, imm);	\
  }

  INSN(svc, 0b000, 0, 0b01);
  INSN(hvc, 0b000, 0, 0b10);
  INSN(smc, 0b000, 0, 0b11);
  INSN(brk, 0b001, 0, 0b00);
  INSN(hlt, 0b010, 0, 0b00);
  INSN(dpcs1, 0b101, 0, 0b01);
  INSN(dpcs2, 0b101, 0, 0b10);
  INSN(dpcs3, 0b101, 0, 0b11);

#undef INSN

  // System
  void system(int op0, int op1, int CRn, int CRm_op2, Register rt)
  {
    starti;
    f(0b11010101000, 31, 21);
    f(op0, 20, 19);
    f(op1, 18, 16);
    f(CRn, 15, 12);
    f(CRm_op2, 11, 5);
    rf(rt, 0);
  }

  void hint(int imm) {
    system(0b00, 0b011, 0b0010, imm, (Register)0b11111);
  }

  void nop() {
    hint(0);
  }

  // Unconditional branch (register)
  void branch_reg(Register R, int opc) {
    starti;
    f(0b1101011, 31, 25);
    f(opc, 24, 21);
    f(0b11111000000, 20, 10);
    rf(R, 5);
    f(0b00000, 4, 0);
  }

#define INSN(NAME, opc)				\
  void NAME(Register R) {			\
    branch_reg(R, opc);				\
  }

  INSN(br, 0b0000);
  INSN(blr, 0b0001);
  INSN(ret, 0b0010);

#undef INSN

#define INSN(NAME, opc)				\
  void NAME() {			\
    branch_reg((Register)0b11111, opc);		\
  }

  INSN(eret, 0b0100);
  INSN(drps, 0b0101);

#undef INSN



  // Load/store exclusive
  enum operand_size { byte, halfword, word, xword };

  void load_store_exclusive(Register Rs, Register Rt1, Register Rt2,
    Register Rn, enum operand_size sz, int op, int o0) {
    starti;
    f(sz, 31, 30), f(0b001000, 29, 24), f(op, 23, 21);
    rf(Rs, 16), f(o0, 15), rf(Rt2, 10), rf(Rn, 5), rf(Rt1, 0);
  }

#define INSN4(NAME, sz, op, o0) /* Four registers */			\
  void NAME(Register Rs, Register Rt1, Register Rt2, Register Rn) {	\
    load_store_exclusive(Rs, Rt1, Rt2, Rn, sz, op, o0);			\
  }

#define INSN3(NAME, sz, op, o0) /* Three registers */			\
  void NAME(Register Rs, Register Rt, Register Rn) {			\
    load_store_exclusive(Rs, Rt, (Register)0b11111, Rn, sz, op, o0);	\
  }

#define INSN2(NAME, sz, op, o0) /* Two registers */			\
  void NAME(Register Rt, Register Rn) {					\
    load_store_exclusive((Register)0b11111, Rt, (Register)0b11111,	\
			 Rn, sz, op, o0);				\
  }

#define INSN_FOO(NAME, sz, op, o0) /* Three registers, encoded differently */ \
  void NAME(Register Rt1, Register Rt2, Register Rn) {			\
    load_store_exclusive((Register)0b11111, Rt1, Rt2, Rn, sz, op, o0);	\
  }

  // bytes
  INSN3(stxrb, byte, 0b000, 0);
  INSN3(stlxrb, byte, 0b000, 1);
  INSN2(ldxrb, byte, 0b010, 0);
  INSN2(ldaxrb, byte, 0b010, 1);
  INSN2(stlrb, byte, 0b100, 1);
  INSN2(ldarb, byte, 0b110, 1);

  // halfwords
  INSN3(stxrh, halfword, 0b000, 0);
  INSN3(stlxrh, halfword, 0b000, 1);
  INSN2(ldxrh, halfword, 0b010, 0);
  INSN2(ldaxrh, halfword, 0b010, 1);
  INSN2(stlrh, halfword, 0b100, 1);
  INSN2(ldarh, halfword, 0b110, 1);

  // words
  INSN3(stxrw, word, 0b000, 0);
  INSN3(stlxrw, word, 0b000, 1);
  INSN4(stxpw, word, 0b001, 0);
  INSN4(stlxpw, word, 0b001, 1);
  INSN2(ldxrw, word, 0b010, 0);
  INSN2(ldaxrw, word, 0b010, 1);
  INSN_FOO(ldxpw, word, 0b011, 0);
  INSN_FOO(ldaxpw, word, 0b011, 1);
  INSN2(stlrw, word, 0b100, 1);
  INSN2(ldarw, word, 0b110, 1);

  // xwords
  INSN3(stxr, xword, 0b000, 0);
  INSN3(stlxr, xword, 0b000, 1);
  INSN4(stxp, xword, 0b001, 0);
  INSN4(stlxp, xword, 0b001, 1);
  INSN2(ldxr, xword, 0b010, 0);
  INSN2(ldaxr, xword, 0b010, 1);
  INSN_FOO(ldxp, xword, 0b011, 0);
  INSN_FOO(ldaxp, xword, 0b011, 1);
  INSN2(stlr, xword, 0b100, 1);
  INSN2(ldar, xword, 0b110, 1);

#undef INSN2
#undef INSN3
#undef INSN4
#undef INSN_FOO

  // Load register (literal)
#define INSN(NAME, opc, V)						\
  void NAME(Register Rt, address dest) {				\
    long offset = (dest - pc()) >> 2;					\
    starti;								\
    f(opc, 31, 30), f(0b011, 29, 27), f(V, 26), f(0b00, 25, 24),	\
      sf(offset, 23, 5);						\
    rf(Rt, 0);								\
  }

  INSN(ldrw, 0b00, 0);
  INSN(ldr, 0b01, 0);
  INSN(ldrsw, 0b10, 0);

#undef INSN

#define INSN(NAME, opc, V)						\
  void NAME(FloatRegister Rt, address dest) {				\
    long offset = (dest - pc()) >> 2;					\
    starti;								\
    f(opc, 31, 30), f(0b011, 29, 27), f(V, 26), f(0b00, 25, 24),	\
      sf(offset, 23, 5);						\
    rf((Register)Rt, 0);						\
  }

  INSN(ldrs, 0b00, 1);
  INSN(ldrd, 0b01, 1);

#undef INSN

#define INSN(NAME, opc, V)						\
  void NAME(int prfop, address dest) {					\
    long offset = (dest - pc()) >> 2;					\
    starti;								\
    f(opc, 31, 30), f(0b011, 29, 27), f(V, 26), f(0b00, 25, 24),	\
      sf(offset, 23, 5);						\
    f(prfop, 4, 0);							\
  }

  INSN(prfm, 0b11, 0);

#undef INSN

  // Load/store
  void ld_st1(int opc, int p1, int V, int p2, int L,
	      Register Rt1, Register Rt2, Register Rn, int imm) {
    starti;
    f(opc, 31, 30), f(p1, 29, 27), f(V, 26), f(p2, 25, 23), f(L, 22);
    sf(imm, 21, 15);
    rf(Rt2, 10), rf(Rn, 5), rf(Rt1, 0);
  }

  int scale_ld_st(int size, int offset) {
    int imm;
    switch(size) {
      case 0b01:
      case 0b00:
	imm = offset >> 2;
	assert_cond(imm << 2 == offset);
	break;
      case 0b10:
	imm = offset >> 3;
	assert_cond(imm << 3 == offset);
	break;
      default:
	assert_cond(false);
      }
    return imm;
  }

  // Load/store register pair (offset)
#define INSN(NAME, size, p1, V, p2, L)					\
  void NAME(Register Rt1, Register Rt2, Register Rn, int offset) {	\
    ld_st1(size, p1, V, p2, L, Rt1, Rt2, Rn, scale_ld_st(size, offset)); \
  }

  INSN(stpw, 0b00, 0b101, 0, 0b0010, 0);
  INSN(ldpw, 0b00, 0b101, 0, 0b0010, 1);
  INSN(ldpsw, 0b01, 0b101, 0, 0b0010, 1);
  INSN(stp, 0b10, 0b101, 0, 0b0010, 0);
  INSN(ldp, 0b10, 0b101, 0, 0b0010, 1);

  // Load/store no-allocate pair (offset)
  INSN(stnpw, 0b00, 0b101, 0, 0b000, 0);
  INSN(ldnpw, 0b00, 0b101, 0, 0b000, 1);
  INSN(stnp, 0b10, 0b101, 0, 0b000, 0);
  INSN(ldnp, 0b10, 0b101, 0, 0b000, 1);

#undef INSN

  // Load/store register (all modes)
  void ld_st2(Register Rt, Address_aarch64 adr, int size, int op, int V = 0) {
    starti;
    f(size, 31, 30);
    f(op, 23, 22); // str
    f(V, 26); // general reg?
    rf(Rt, 0);
    adr.encode(current);
  }

#define INSN(NAME, size, op)				\
  void NAME(Register Rt, Address_aarch64 adr) {		\
    ld_st2(Rt, adr, size, op);				\
  }							\

  INSN(str, 0b11, 0b00);
  INSN(strw, 0b10, 0b00);
  INSN(strb, 0b00, 0b00);
  INSN(strh, 0b01, 0b00);

  INSN(ldr, 0b11, 0b01);
  INSN(ldrw, 0b10, 0b01);
  INSN(ldrb, 0b00, 0b01);
  INSN(ldrh, 0b01, 0b01);

  INSN(ldrsb, 0b00, 0b11);
  INSN(ldrsh, 0b01, 0b11);
  INSN(ldrshw, 0b01, 0b10);
  INSN(ldrsw, 0b10, 0b10);

  INSN(prfm, 0b11, 0b10); // FIXME: PRFM should not be used with
			  // writeback modes, but the assembler
			  // doesn't enfore that.

#undef INSN

#define INSN(NAME, size, op)				\
  void NAME(FloatRegister Rt, Address_aarch64 adr) {	\
    ld_st2((Register)Rt, adr, size, op, 1);		\
  }

  INSN(strd, 0b11, 0b00);
  INSN(strs, 0b10, 0b00);
  INSN(ldrd, 0b11, 0b01);
  INSN(ldrs, 0b10, 0b01);

#undef INSN

  enum shift_kind { lsl, lsr, asr, ror };

  void op_shifted_reg(unsigned decode,
		      Register Rd, Register Rn, Register Rm,
		      enum shift_kind kind, unsigned shift,
		      unsigned size, unsigned op) {
    f(size, 31);
    f(op, 30, 29);
    f(decode, 28, 24);
    rf(Rm, 16), rf(Rn, 5), rf(Rd, 0);
    f(shift, 15, 10);
    f(kind, 23, 22);
  }

  // Logical (shifted regsiter)
#define INSN(NAME, size, op, N)					\
  void NAME(Register Rd, Register Rn, Register Rm,		\
	    enum shift_kind kind = lsl, unsigned shift = 0) {	\
    starti;							\
    f(N, 21);							\
    op_shifted_reg(0b01010, Rd, Rn, Rm, kind, shift, size, op);	\
  }

  INSN(andr, 1, 0b00, 0);
  INSN(orr, 1, 0b01, 0);
  INSN(eor, 1, 0b10, 0);
  INSN(ands, 1, 0b10, 0);
  INSN(andw, 0, 0b00, 0);
  INSN(orrw, 0, 0b01, 0);
  INSN(eorw, 0, 0b10, 0);
  INSN(andsw, 0, 0b10, 0);

  INSN(bic, 1, 0b00, 1);
  INSN(orn, 1, 0b01, 1);
  INSN(eon, 1, 0b10, 1);
  INSN(bics, 1, 0b10, 1);
  INSN(bicw, 0, 0b00, 1);
  INSN(ornw, 0, 0b01, 1);
  INSN(eonw, 0, 0b10, 1);
  INSN(bicsw, 0, 0b10, 1);

#undef INSN

  // Add/subtract (shifted regsiter)
#define INSN(NAME, size, op)					\
  void NAME(Register Rd, Register Rn, Register Rm,		\
	    enum shift_kind kind = lsl, unsigned shift = 0) {	\
    starti;							\
    f(0, 21);							\
    assert_cond(kind != ror);					\
    op_shifted_reg(0b01011, Rd, Rn, Rm, kind, shift, size, op);	\
  }

  INSN(add, 1, 0b000);
  INSN(adds, 1, 0b001);
  INSN(sub, 1, 0b10);
  INSN(subs, 1, 0b11);
  INSN(addw, 0, 0b000);
  INSN(addsw, 0, 0b001);
  INSN(subw, 0, 0b10);
  INSN(subsw, 0, 0b11);

#undef INSN

  // Add/subtract (extended register)
#define INSN(NAME, op)							\
  void NAME(Register Rd, Register Rn, Register Rm,			\
           ext::operation option, int amount) {				\
    add_sub_extended_reg(op, 0b01011, Rd, Rn, Rm, 0b00, option, amount); \
  }

  void add_sub_extended_reg(unsigned op, unsigned decode,
    Register Rd, Register Rn, Register Rm,
    unsigned opt, ext::operation option, unsigned imm) {
    starti;
    f(op, 31, 29), f(decode, 28, 24), f(opt, 23, 22), f(1, 21);
    f(option, 15, 13), f(imm, 12, 10);
    rf(Rm, 16), rf(Rn, 5), rf(Rd, 0);
  }

  INSN(addw, 0b000);
  INSN(addsw, 0b001);
  INSN(subw, 0b010);
  INSN(subsw, 0b011);
  INSN(add, 0b100);
  INSN(adds, 0b101);
  INSN(sub, 0b110);
  INSN(subs, 0b111);

#undef INSN

  // Add/subtract (with carry)
  void add_sub_carry(unsigned op, Register Rd, Register Rn, Register Rm) {
    starti;
    f(op, 31, 29);
    f(0b11010000, 28, 21);
    f(0b000000, 15, 10);
    rf(Rm, 16), rf(Rn, 5), rf(Rd, 0);
  }

  #define INSN(NAME, op)				\
    void NAME(Register Rd, Register Rn, Register Rm) {	\
      add_sub_carry(op, Rd, Rn, Rm);			\
    }

  INSN(adcw, 0b000);
  INSN(adcsw, 0b001);
  INSN(sbcw, 0b010);
  INSN(sbcsw, 0b011);
  INSN(adc, 0b100);
  INSN(adcs, 0b101);
  INSN(sbc,0b110);
  INSN(sbcs, 0b111);

#undef INSN

  // Conditional compare (both kinds)
  void conditional_compare(unsigned op, int o2, int o3,
                           Register Rn, unsigned imm5, unsigned nzcv,
                           unsigned cond) {
    f(op, 31, 29);
    f(0b11010010, 28, 21);
    f(cond, 15, 12);
    f(o2, 10);
    f(o3, 4);
    f(nzcv, 3, 0);
    f(imm5, 20, 16), rf(Rn, 5);
  }

#define INSN(NAME, op)							\
  void NAME(Register Rn, Register Rm, int imm, condition_code cond) {	\
    starti;								\
    f(0, 11);								\
    conditional_compare(op, 0, 0, Rn, (uintptr_t)Rm, imm, cond);	\
  }									\
									\
  void NAME(Register Rn, unsigned imm5, int imm, condition_code cond) {	\
    starti;								\
    f(1, 11);								\
    conditional_compare(op, 0, 0, Rn, imm5, imm, cond);			\
  }

  INSN(ccmnw, 0b001);
  INSN(ccmpw, 0b011);
  INSN(ccmn, 0b101);
  INSN(ccmp, 0b111);

#undef INSN

  // Conditional select
  void conditional_select(unsigned op, unsigned op2,
			  Register Rd, Register Rn, Register Rm,
			  unsigned cond) {
    starti;
    f(op, 31, 29);
    f(0b11010100, 28, 21);
    f(cond, 15, 12);
    f(0, 11, 10);
    rf(Rm, 16), rf(Rn, 5), rf(Rd, 0);
  }

#define INSN(NAME, op, op2)						\
  void NAME(Register Rd, Register Rn, Register Rm, condition_code cond) { \
    conditional_select(op, op2, Rd, Rn, Rm, cond);			\
  }

  INSN(cselw, 0b000, 0b00);
  INSN(csincw, 0b000, 0b01);
  INSN(csinvw, 0b010, 0b00);
  INSN(csnegw, 0b010, 0b01);
  INSN(csel, 0b100, 0b00);
  INSN(csinc, 0b000, 0b01);
  INSN(csinv, 0b110, 0b00);
  INSN(csneg, 0b110, 0b01);

#undef INSN

  // Data processing
  void data_processing(unsigned op29, unsigned opcode,
		       Register Rd, Register Rn) {
    f(op29, 31, 29), f(0b11010110, 28, 21);
    f(opcode, 15, 10);
    rf(Rn, 5), rf(Rd, 0);
  }

  // (1 source)
#define INSN(NAME, op29, opcode2, opcode)	\
  void NAME(Register Rd, Register Rn) {		\
    starti;					\
    f(opcode2, 20, 16);				\
    data_processing(op29, opcode, Rd, Rn);	\
  }

  INSN(rbitw,  0b010, 0b00000, 0b00000);
  INSN(rev16w, 0b010, 0b00000, 0b00001);
  INSN(revw,   0b010, 0b00000, 0b00010);
  INSN(clzw,   0b010, 0b00000, 0b00100);
  INSN(clsw,   0b010, 0b00000, 0b00101);
 
  INSN(rbit,   0b110, 0b00000, 0b00000);
  INSN(rev16,  0b110, 0b00000, 0b00001);
  INSN(rev32,  0b110, 0b00000, 0b00010);
  INSN(rev,    0b110, 0b00000, 0b00011);
  INSN(clz,    0b110, 0b00000, 0b00100);
  INSN(cls,    0b110, 0b00000, 0b00101);

#undef INSN

  // (2 sources)
#define INSN(NAME, op29, opcode)			\
  void NAME(Register Rd, Register Rn, Register Rm) {	\
    starti;						\
    rf(Rm, 16);						\
    data_processing(op29, opcode, Rd, Rn);		\
  }

  INSN(udivw, 0b000, 0b000010);
  INSN(sdivw, 0b000, 0b000011);
  INSN(lslvw, 0b000, 0b001000);
  INSN(lsrvw, 0b000, 0b001001);
  INSN(asrvw, 0b000, 0b001010);
  INSN(rorvw, 0b000, 0b001011);

  INSN(udiv, 0b100, 0b000010);
  INSN(sdiv, 0b100, 0b000011);
  INSN(lslv, 0b100, 0b001000);
  INSN(lsrv, 0b100, 0b001001);
  INSN(asrv, 0b100, 0b001010);
  INSN(rorv, 0b100, 0b001011);

#undef INSN
 
  // (3 sources)
  void data_processing(unsigned op54, unsigned op31, unsigned o0,
		       Register Rd, Register Rn, Register Rm,
		       Register Ra) {
    starti;
    f(op54, 31, 29), f(0b11011, 28, 24);
    f(op31, 23, 21), f(o0, 15);
    rf(Rm, 16), rf(Ra, 10), rf(Rn, 5), rf(Rd, 0);
  }

#define INSN(NAME, op54, op31, o0)					\
  void NAME(Register Rd, Register Rn, Register Rm, Register Ra) {	\
    data_processing(op54, op31, o0, Rd, Rn, Rm, Ra);			\
  }

  INSN(maddw, 0b000, 0b000, 0);
  INSN(msubw, 0b000, 0b000, 1);
  INSN(madd, 0b100, 0b000, 0);
  INSN(msub, 0b100, 0b000, 1);
  INSN(smaddl, 0b100, 0b001, 0);
  INSN(smsubl, 0b100, 0b001, 1);
  INSN(umaddl, 0b100, 0b101, 0);
  INSN(umsubl, 0b100, 0b101, 1);

#undef INSN

#define INSN(NAME, op54, op31, o0)			\
  void NAME(Register Rd, Register Rn, Register Rm) {	\
    data_processing(op54, op31, o0, Rd, Rn, Rm, (Register)31);	\
  }

  INSN(smulh, 0b100, 0b010, 0);
  INSN(umulh, 0b100, 0b110, 0);

#undef INSN

  // Floating-point data-processing (1 source)
  void data_processing(unsigned op31, unsigned type, unsigned opcode,
		       FloatRegister Vd, FloatRegister Vn) {
    starti;
    f(op31, 31, 29);
    f(0b11110, 28, 24);
    f(type, 23, 22), f(1, 21), f(opcode, 20, 15), f(0b10000, 14, 10);
    rf(Vn, 5), rf(Vd, 0);
  }

#define INSN(NAME, op31, type, opcode)			\
  void NAME(FloatRegister Vd, FloatRegister Vn) {	\
    data_processing(op31, type, opcode, Vd, Vn);	\
  }

  INSN(fmovs, 0b000, 0b00, 0b000000);
  INSN(fabss, 0b000, 0b00, 0b000001);
  INSN(fnegs, 0b000, 0b00, 0b000010);
  INSN(fsqrts, 0b000, 0b00, 0b000011);
  INSN(fcvts, 0b000, 0b00, 0b000101);

  INSN(fmovd, 0b000, 0b01, 0b000000);
  INSN(fabsd, 0b000, 0b01, 0b000001);
  INSN(fnegd, 0b000, 0b01, 0b000010);
  INSN(fsqrtd, 0b000, 0b01, 0b000011);
  INSN(fcvtd, 0b000, 0b01, 0b000100);

#undef INSN

  // Floating-point data-processing (2 source)
  void data_processing(unsigned op31, unsigned type, unsigned opcode,
		       FloatRegister Vd, FloatRegister Vn, FloatRegister Vm) {
    starti;
    f(op31, 31, 29);
    f(0b11110, 28, 24);
    f(type, 23, 22), f(1, 21), f(opcode, 15, 12), f(0b10, 11, 10);
    rf(Vm, 16), rf(Vn, 5), rf(Vd, 0);
  }

#define INSN(NAME, op31, type, opcode)			\
  void NAME(FloatRegister Vd, FloatRegister Vn, FloatRegister Vm) {	\
    data_processing(op31, type, opcode, Vd, Vn, Vm);	\
  }

  INSN(fmuls, 0b000, 0b00, 0b0000);
  INSN(fdivs, 0b000, 0b00, 0b0001);
  INSN(fadds, 0b000, 0b00, 0b0010);
  INSN(fsubs, 0b000, 0b00, 0b0011);
  INSN(fnmuls, 0b000, 0b00, 0b1000);

  INSN(fmuld, 0b000, 0b01, 0b0000);
  INSN(fdivd, 0b000, 0b01, 0b0001);
  INSN(faddd, 0b000, 0b01, 0b0010);
  INSN(fsubd, 0b000, 0b01, 0b0011);
  INSN(fnmuld, 0b000, 0b01, 0b1000);

#undef INSN

   // Floating-point data-processing (3 source)
  void data_processing(unsigned op31, unsigned type, unsigned o1, unsigned o0,
		       FloatRegister Vd, FloatRegister Vn, FloatRegister Vm,
		       FloatRegister Va) {
    starti;
    f(op31, 31, 29);
    f(0b11111, 28, 24);
    f(type, 23, 22), f(o1, 21), f(o1, 15);
    rf(Vm, 16), rf(Vn, 10), rf(Vn, 5), rf(Vd, 0);
  }

#define INSN(NAME, op31, type, o1, o0)					\
  void NAME(FloatRegister Vd, FloatRegister Vn, FloatRegister Vm,	\
	    FloatRegister Va) {						\
    data_processing(op31, type, o1, o0, Vd, Vn, Vm, Va);		\
  }

  INSN(fmadds, 0b000, 0b00, 0, 0);
  INSN(fmsubs, 0b000, 0b00, 0, 1);
  INSN(fnmadds, 0b000, 0b00, 0, 0);
  INSN(fnmsubs, 0b000, 0b00, 0, 1);

  INSN(fmadd, 0b000, 0b01, 0, 0);
  INSN(fmsubd, 0b000, 0b01, 0, 1);
  INSN(fnmadd, 0b000, 0b01, 0, 0);
  INSN(fnmsub, 0b000, 0b01, 0, 1);

#undef INSN

   // Floating-point<->integer conversions
  void float_int_convert(unsigned op31, unsigned type,
			 unsigned rmode, unsigned opcode,
			 Register Rd, Register Rn) {
    starti;
    f(op31, 31, 29);
    f(0b11110, 28, 24);
    f(type, 23, 22), f(1, 21), f(rmode, 20, 19);
    f(opcode, 18, 16), f(0b000000, 15, 10);
    rf(Rn, 5), rf(Rd, 0);
  }

#define INSN(NAME, op31, type, rmode, opcode)				\
  void NAME(Register Rd, FloatRegister Vn) {				\
    float_int_convert(op31, type, rmode, opcode, Rd, (Register)Vn);	\
  }

  INSN(fcvtszw, 0b000, 0b00, 0b11, 0b000);
  INSN(fcvtzs, 0b000, 0b01, 0b11, 0b000);
  INSN(fcvtzdw, 0b100, 0b00, 0b11, 0b000);
  INSN(fcvtszd, 0b100, 0b01, 0b11, 0b000);

  INSN(fmovs, 0b000, 0b00, 0b00, 0b110);
  INSN(fmovd, 0b100, 0b01, 0b00, 0b110);

  INSN(fmovhid, 0b100, 0b10, 0b01, 0b110);

#undef INSN

#define INSN(NAME, op31, type, rmode, opcode)				\
  void NAME(FloatRegister Vd, Register Rn) {				\
    float_int_convert(op31, type, rmode, opcode, (Register)Vd, Rn);	\
  }

  INSN(fmovs, 0b000, 0b00, 0b00, 0b111);
  INSN(fmovd, 0b100, 0b01, 0b00, 0b111);

  INSN(fmovhid, 0b100, 0b10, 0b01, 0b111);

#undef INSN

  // Floating-point compare
  void float_compare(unsigned op31, unsigned type,
		     unsigned op, unsigned op2,
		     FloatRegister Vn, FloatRegister Vm = (FloatRegister)0) {
    starti;
    f(op31, 31, 29);
    f(0b11110, 28, 24);
    f(type, 23, 22), f(1, 21);
    f(op, 15, 14), f(0b1000, 13, 10), f(op2, 4, 0);
    rf(Vn, 5), rf(Vm, 16);
  }


#define INSN(NAME, op31, type, op, op2)	\
  void NAME(FloatRegister Vn, FloatRegister Vm) {	\
    float_compare(op31, type, op, op2, Vn, Vm);	\
  }

#define INSN1(NAME, op31, type, op, op2)	\
  void NAME(FloatRegister Vn) {	\
    float_compare(op31, type, op, op2, Vn);	\
  }

  INSN(fcmps, 0b000, 0b00, 0b00, 0b00000);
  INSN1(fcmps, 0b000, 0b00, 0b00, 0b01000);
  INSN(fcmpes, 0b000, 0b00, 0b00, 0b10000);
  INSN1(fcmpes, 0b000, 0b00, 0b00, 0b11000);

  INSN(fcmpd, 0b000,   0b01, 0b00, 0b00000);
  INSN1(fcmpd, 0b000,  0b01, 0b00, 0b01000);
  INSN(fcmped, 0b000,  0b01, 0b00, 0b10000);
  INSN1(fcmped, 0b000, 0b01, 0b00, 0b11000);

#undef INSN
#undef INSN1

  Assembler_aarch64(CodeBuffer* code) : AbstractAssembler(code) {
  }

  virtual RegisterOrConstant delayed_value_impl(intptr_t* delayed_value_addr,
                                                Register tmp,
                                                int offset) {
  }

  // Stack overflow checking
  virtual void bang_stack_with_offset(int offset) {
  }

  bool operand_valid_for_logical_immdiate(int is32, uint64_t imm);
};

#undef starti

Instruction::~Instruction() {
  assem->emit();
}


#endif // CPU_AARCH64_VM_ASSEMBLER_AARCH64_HPP
