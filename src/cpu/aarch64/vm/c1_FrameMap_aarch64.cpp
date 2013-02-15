/*
 * Copyright (c) 1999, 2010, Oracle and/or its affiliates. All rights reserved.
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

#include "precompiled.hpp"
#include "c1/c1_FrameMap.hpp"
#include "c1/c1_LIR.hpp"
#include "runtime/sharedRuntime.hpp"
#include "vmreg_aarch64.inline.hpp"

LIR_Opr FrameMap::map_to_opr(BasicType type, VMRegPair* reg, bool) {
  LIR_Opr opr = LIR_OprFact::illegalOpr;
  VMReg r_1 = reg->first();
  VMReg r_2 = reg->second();
  if (r_1->is_stack()) {
    // Convert stack slot to an SP offset
    // The calling convention does not count the SharedRuntime::out_preserve_stack_slots() value
    // so we must add it in here.
    int st_off = (r_1->reg2stack() + SharedRuntime::out_preserve_stack_slots()) * VMRegImpl::stack_slot_size;
    opr = LIR_OprFact::address(new LIR_Address(sp_opr, st_off, type));
  } else if (r_1->is_Register()) {
    Register reg = r_1->as_Register();
    if (r_2->is_Register() && (type == T_LONG || type == T_DOUBLE)) {
      Register reg2 = r_2->as_Register();
#ifdef _LP64
      assert(reg2 == reg, "must be same register");
      opr = as_long_opr(reg);
#else
      opr = as_long_opr(reg2, reg);
#endif // _LP64
    } else if (type == T_OBJECT || type == T_ARRAY) {
      opr = as_oop_opr(reg);
    } else {
      opr = as_opr(reg);
    }
  } else if (r_1->is_FloatRegister()) {
    assert(type == T_DOUBLE || type == T_FLOAT, "wrong type");
    int num = r_1->as_FloatRegister()->encoding();
    if (type == T_FLOAT) {
      opr = LIR_OprFact::single_fpu(num);
    } else {
      opr = LIR_OprFact::double_fpu(num);
    }
  } else {
    ShouldNotReachHere();
  }
  return opr;
}

LIR_Opr FrameMap::r0_opr;
LIR_Opr FrameMap::r1_opr;
LIR_Opr FrameMap::r2_opr;
LIR_Opr FrameMap::r3_opr;
LIR_Opr FrameMap::r4_opr;
LIR_Opr FrameMap::r5_opr;
LIR_Opr FrameMap::r6_opr;
LIR_Opr FrameMap::r7_opr;
LIR_Opr FrameMap::r8_opr;
LIR_Opr FrameMap::r9_opr;
LIR_Opr FrameMap::r10_opr;
LIR_Opr FrameMap::r11_opr;
LIR_Opr FrameMap::r12_opr;
LIR_Opr FrameMap::r13_opr;
LIR_Opr FrameMap::r14_opr;
LIR_Opr FrameMap::r15_opr;
LIR_Opr FrameMap::r16_opr;
LIR_Opr FrameMap::r17_opr;
LIR_Opr FrameMap::r18_opr;
LIR_Opr FrameMap::r19_opr;
LIR_Opr FrameMap::r20_opr;
LIR_Opr FrameMap::r21_opr;
LIR_Opr FrameMap::r22_opr;
LIR_Opr FrameMap::r23_opr;
LIR_Opr FrameMap::r24_opr;
LIR_Opr FrameMap::r25_opr;
LIR_Opr FrameMap::r26_opr;
LIR_Opr FrameMap::r27_opr;
LIR_Opr FrameMap::r28_opr;
LIR_Opr FrameMap::r29_opr;
LIR_Opr FrameMap::r30_opr;

LIR_Opr FrameMap::rfp_opr;
LIR_Opr FrameMap::sp_opr;

LIR_Opr FrameMap::receiver_opr;

LIR_Opr FrameMap::r0_oop_opr;
LIR_Opr FrameMap::r1_oop_opr;
LIR_Opr FrameMap::r2_oop_opr;
LIR_Opr FrameMap::r3_oop_opr;
LIR_Opr FrameMap::r4_oop_opr;
LIR_Opr FrameMap::r5_oop_opr;
LIR_Opr FrameMap::r6_oop_opr;
LIR_Opr FrameMap::r7_oop_opr;
LIR_Opr FrameMap::r8_oop_opr;
LIR_Opr FrameMap::r9_oop_opr;
LIR_Opr FrameMap::r10_oop_opr;
LIR_Opr FrameMap::r11_oop_opr;
LIR_Opr FrameMap::r12_oop_opr;
LIR_Opr FrameMap::r13_oop_opr;
LIR_Opr FrameMap::r14_oop_opr;
LIR_Opr FrameMap::r15_oop_opr;
LIR_Opr FrameMap::r16_oop_opr;
LIR_Opr FrameMap::r17_oop_opr;
LIR_Opr FrameMap::r18_oop_opr;
LIR_Opr FrameMap::r19_oop_opr;
LIR_Opr FrameMap::r20_oop_opr;
LIR_Opr FrameMap::r21_oop_opr;
LIR_Opr FrameMap::r22_oop_opr;
LIR_Opr FrameMap::r23_oop_opr;
LIR_Opr FrameMap::r24_oop_opr;
LIR_Opr FrameMap::r25_oop_opr;
LIR_Opr FrameMap::r26_oop_opr;
LIR_Opr FrameMap::r27_oop_opr;
LIR_Opr FrameMap::r28_oop_opr;
LIR_Opr FrameMap::r29_oop_opr;
LIR_Opr FrameMap::r30_oop_opr;

LIR_Opr FrameMap::r0_metadata_opr;
LIR_Opr FrameMap::r1_metadata_opr;
LIR_Opr FrameMap::r2_metadata_opr;
LIR_Opr FrameMap::r3_metadata_opr;
LIR_Opr FrameMap::r4_metadata_opr;
LIR_Opr FrameMap::r5_metadata_opr;

LIR_Opr FrameMap::long0_opr;
LIR_Opr FrameMap::long1_opr;
LIR_Opr FrameMap::fpu0_float_opr;
LIR_Opr FrameMap::fpu0_double_opr;

LIR_Opr FrameMap::_caller_save_cpu_regs[] = { 0, };
LIR_Opr FrameMap::_caller_save_fpu_regs[] = { 0, };

//--------------------------------------------------------
//               FrameMap
//--------------------------------------------------------

void FrameMap::initialize() {
  assert(!_init_done, "once");

  map_register(0, r0); r0_opr = LIR_OprFact::single_cpu(0);
  map_register(1, r1); r1_opr = LIR_OprFact::single_cpu(1);
  map_register(2, r2); r2_opr = LIR_OprFact::single_cpu(2);
  map_register(3, r3); r3_opr = LIR_OprFact::single_cpu(3);
  map_register(4, r4); r4_opr = LIR_OprFact::single_cpu(4);
  map_register(5, r5); r5_opr = LIR_OprFact::single_cpu(5);
  map_register(6, r6); r6_opr = LIR_OprFact::single_cpu(6);
  map_register(7, r7); r7_opr = LIR_OprFact::single_cpu(7);
  map_register(8, r8); r8_opr = LIR_OprFact::single_cpu(8);
  map_register(9, r9); r9_opr = LIR_OprFact::single_cpu(9);
  map_register(10, r10); r10_opr = LIR_OprFact::single_cpu(10);
  map_register(11, r11); r11_opr = LIR_OprFact::single_cpu(11);
  map_register(12, r12); r12_opr = LIR_OprFact::single_cpu(12);
  map_register(13, r13); r13_opr = LIR_OprFact::single_cpu(13);
  map_register(14, r14); r14_opr = LIR_OprFact::single_cpu(14);
  map_register(15, r15); r15_opr = LIR_OprFact::single_cpu(15);
  map_register(16, r16); r16_opr = LIR_OprFact::single_cpu(16);
  map_register(17, r17); r17_opr = LIR_OprFact::single_cpu(17);
  map_register(18, r18); r18_opr = LIR_OprFact::single_cpu(18);
  map_register(19, r19); r19_opr = LIR_OprFact::single_cpu(19);
  map_register(20, r20); r20_opr = LIR_OprFact::single_cpu(20);
  map_register(21, r21); r21_opr = LIR_OprFact::single_cpu(21);
  map_register(22, r22); r22_opr = LIR_OprFact::single_cpu(22);
  map_register(23, r23); r23_opr = LIR_OprFact::single_cpu(23);
  map_register(24, r24); r24_opr = LIR_OprFact::single_cpu(24);
  map_register(25, r25); r25_opr = LIR_OprFact::single_cpu(25);
  map_register(26, r26); r26_opr = LIR_OprFact::single_cpu(26);
  map_register(27, r27); r27_opr = LIR_OprFact::single_cpu(27);
  map_register(28, r28); r28_opr = LIR_OprFact::single_cpu(28);
  map_register(29, r29); r29_opr = LIR_OprFact::single_cpu(29);
  map_register(30, r30); r30_opr = LIR_OprFact::single_cpu(30);

  long0_opr = LIR_OprFact::double_cpu(0, 0);
  long1_opr = LIR_OprFact::double_cpu(1, 1);

  fpu0_float_opr   = LIR_OprFact::single_fpu(0);
  fpu0_double_opr  = LIR_OprFact::double_fpu(0);

  _caller_save_cpu_regs[0] = r0_opr;
  _caller_save_cpu_regs[1] = r1_opr;
  _caller_save_cpu_regs[2] = r2_opr;
  _caller_save_cpu_regs[3] = r3_opr;
  _caller_save_cpu_regs[4] = r4_opr;
  _caller_save_cpu_regs[5] = r5_opr;

  _caller_save_cpu_regs[6]  = r6_opr;
  _caller_save_cpu_regs[7]  = r7_opr;
  _caller_save_cpu_regs[8]  = r8_opr;
  _caller_save_cpu_regs[9]  = r9_opr;
  _caller_save_cpu_regs[10] = r10_opr;
  _caller_save_cpu_regs[11] = r11_opr;

  for (int i = 0; i < 8; i++) {
    _caller_save_fpu_regs[i] = LIR_OprFact::single_fpu(i);
  }

  _init_done = true;

  r0_oop_opr = as_oop_opr(r0);
  r1_oop_opr = as_oop_opr(r1);
  r2_oop_opr = as_oop_opr(r2);
  r3_oop_opr = as_oop_opr(r3);
  r4_oop_opr = as_oop_opr(r4);
  r5_oop_opr = as_oop_opr(r5);
  r6_oop_opr = as_oop_opr(r6);
  r7_oop_opr = as_oop_opr(r7);
  r8_oop_opr = as_oop_opr(r8);
  r9_oop_opr = as_oop_opr(r9);
  r10_oop_opr = as_oop_opr(r10);
  r11_oop_opr = as_oop_opr(r11);
  r12_oop_opr = as_oop_opr(r12);
  r13_oop_opr = as_oop_opr(r13);
  r14_oop_opr = as_oop_opr(r14);
  r15_oop_opr = as_oop_opr(r15);
  r16_oop_opr = as_oop_opr(r16);
  r17_oop_opr = as_oop_opr(r17);
  r18_oop_opr = as_oop_opr(r18);
  r19_oop_opr = as_oop_opr(r19);
  r20_oop_opr = as_oop_opr(r20);
  r21_oop_opr = as_oop_opr(r21);
  r22_oop_opr = as_oop_opr(r22);
  r23_oop_opr = as_oop_opr(r23);
  r24_oop_opr = as_oop_opr(r24);
  r25_oop_opr = as_oop_opr(r25);
  r26_oop_opr = as_oop_opr(r26);
  r27_oop_opr = as_oop_opr(r27);
  r28_oop_opr = as_oop_opr(r28);
  r29_oop_opr = as_oop_opr(r29);
  r30_oop_opr = as_oop_opr(r30);

  r0_metadata_opr = as_metadata_opr(r0);
  r1_metadata_opr = as_metadata_opr(r1);
  r2_metadata_opr = as_metadata_opr(r2);
  r3_metadata_opr = as_metadata_opr(r3);
  r4_metadata_opr = as_metadata_opr(r4);
  r5_metadata_opr = as_metadata_opr(r5);

  sp_opr = as_pointer_opr(r31_sp);
  rfp_opr = as_pointer_opr(rfp);

  VMRegPair regs;
  BasicType sig_bt = T_OBJECT;
  SharedRuntime::java_calling_convention(&sig_bt, &regs, 1, true);
  receiver_opr = as_oop_opr(regs.first()->as_Register());

  for (int i = 0; i < nof_caller_save_fpu_regs; i++) {
    _caller_save_fpu_regs[i] = LIR_OprFact::single_fpu(i + 16);
  }
  for (int i = 0; i < nof_caller_save_cpu_regs(); i++) {
    _caller_save_cpu_regs[i] = LIR_OprFact::single_cpu(i + 19);
  }
}


Address FrameMap::make_new_address(ByteSize sp_offset) const {
  // for rbp, based address use this:
  // return Address(rbp, in_bytes(sp_offset) - (framesize() - 2) * 4);
  return Address(sp, in_bytes(sp_offset));
}


// ----------------mapping-----------------------
// all mapping is based on rfp addressing, except for simple leaf methods where we access
// the locals sp based (and no frame is built)


// Frame for simple leaf methods (quick entries)
//
//   +----------+
//   | ret addr |   <- TOS
//   +----------+
//   | args     |
//   | ......   |

// Frame for standard methods
//
//   | .........|  <- TOS
//   | locals   |
//   +----------+
//   | old rbp,  |  <- EBP
//   +----------+
//   | ret addr |
//   +----------+
//   |  args    |
//   | .........|


// For OopMaps, map a local variable or spill index to an VMRegImpl name.
// This is the offset from sp() in the frame of the slot for the index,
// skewed by VMRegImpl::stack0 to indicate a stack location (vs.a register.)
//
//           framesize +
//           stack0         stack0          0  <- VMReg
//             |              | <registers> |
//  ...........|..............|.............|
//      0 1 2 3 x x 4 5 6 ... |                <- local indices
//      ^           ^        sp()                 ( x x indicate link
//      |           |                               and return addr)
//  arguments   non-argument locals


VMReg FrameMap::fpu_regname (int n) {
  // Return the OptoReg name for the fpu stack slot "n"
  // A spilled fpu stack slot comprises to two single-word OptoReg's.
  return as_FloatRegister(n)->as_VMReg();
}

LIR_Opr FrameMap::stack_pointer() {
  return FrameMap::sp_opr;
}


// JSR 292
LIR_Opr FrameMap::method_handle_invoke_SP_save_opr() {
  // assert(rfp == rbp_mh_SP_save, "must be same register");
  return rfp_opr;
}


bool FrameMap::validate_frame() {
  return true;
}
