/*
 * Copyright (c) 1999, 2011, Oracle and/or its affiliates. All rights reserved.
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
#include "asm/assembler.hpp"
#include "c1/c1_Defs.hpp"
#include "c1/c1_MacroAssembler.hpp"
#include "c1/c1_Runtime1.hpp"
#include "interpreter/interpreter.hpp"
#include "nativeInst_aarch64.hpp"
#include "oops/compiledICHolder.hpp"
#include "oops/oop.inline.hpp"
#include "prims/jvmtiExport.hpp"
#include "register_aarch64.hpp"
#include "runtime/sharedRuntime.hpp"
#include "runtime/signature.hpp"
#include "runtime/vframeArray.hpp"
#include "vmreg_aarch64.inline.hpp"


// Implementation of StubAssembler

int StubAssembler::call_RT(Register oop_result1, Register metadata_result, address entry, int args_size) {
  // setup registers
  assert(!(oop_result1->is_valid() || metadata_result->is_valid()) || oop_result1 != metadata_result, "registers must be different");
  assert(oop_result1 != rthread && metadata_result != rthread, "registers must be different");
  assert(args_size >= 0, "illegal args_size");
  bool align_stack = false;

  mov(c_rarg0, rthread);
  set_num_rt_args(0); // Nothing on stack

  Label retaddr;
  set_last_Java_frame(sp, rfp, retaddr, rscratch1);

  // do the call
  mov(rscratch1, RuntimeAddress(entry));
  brx86(rscratch1, args_size + 1, 8, 1);
  bind(retaddr);
  int call_offset = offset();
  // verify callee-saved register
#ifdef ASSERT
  push(0b1, sp); // r0
  { Label L;
    get_thread(r0);
    cmp(rthread, r0);
    br(Assembler::EQ, L);
    stop("StubAssembler::call_RT: rthread not callee saved?");
    bind(L);
  }
  pop(0b1, sp);
#endif
  reset_last_Java_frame(true, true);

  // check for pending exceptions
  { Label L;
    // check for pending exceptions (java_thread is set upon return)
    ldr(rscratch1, Address(rthread, in_bytes(Thread::pending_exception_offset())));
    cbz(rscratch1, L);
    // exception pending => remove activation and forward to exception handler
    // make sure that the vm_results are cleared
    if (oop_result1->is_valid()) {
      str(zr, Address(rthread, JavaThread::vm_result_offset()));
    }
    if (metadata_result->is_valid()) {
      str(zr, Address(rthread, JavaThread::vm_result_2_offset()));
    }
    if (frame_size() == no_frame_size) {
      leave();
      b(RuntimeAddress(StubRoutines::forward_exception_entry()));
    } else if (_stub_id == Runtime1::forward_exception_id) {
      should_not_reach_here();
    } else {
      b(RuntimeAddress(Runtime1::entry_for(Runtime1::forward_exception_id)));
    }
    bind(L);
  }
  // get oop results if there are any and reset the values in the thread
  if (oop_result1->is_valid()) {
    get_vm_result(oop_result1, rthread);
  }
  if (metadata_result->is_valid()) {
    get_vm_result_2(metadata_result, rthread);
  }
  return call_offset;
}


int StubAssembler::call_RT(Register oop_result1, Register metadata_result, address entry, Register arg1) {
  mov(c_rarg1, arg1);
  return call_RT(oop_result1, metadata_result, entry, 1);
}


int StubAssembler::call_RT(Register oop_result1, Register metadata_result, address entry, Register arg1, Register arg2) {
  if (c_rarg1 == arg2) {
    if (c_rarg2 == arg1) {
      mov(rscratch1, arg1);
      mov(arg1, arg2);
      mov(arg2, rscratch1);
    } else {
      mov(c_rarg2, arg2);
      mov(c_rarg1, arg1);
    }
  } else {
    mov(c_rarg1, arg1);
    mov(c_rarg2, arg2);
  }
  return call_RT(oop_result1, metadata_result, entry, 2);
}


int StubAssembler::call_RT(Register oop_result1, Register metadata_result, address entry, Register arg1, Register arg2, Register arg3) {
  // if there is any conflict use the stack
  if (arg1 == c_rarg2 || arg1 == c_rarg3 ||
      arg2 == c_rarg1 || arg1 == c_rarg3 ||
      arg3 == c_rarg1 || arg1 == c_rarg2) {
    stp(arg3, arg2, Address(pre(sp, 2 * wordSize)));
    stp(arg1, zr, Address(pre(sp, -2 * wordSize)));
    ldp(c_rarg1, zr, Address(post(sp, 2 * wordSize)));
    ldp(c_rarg3, c_rarg2, Address(post(sp, 2 * wordSize)));
  } else {
    mov(c_rarg1, arg1);
    mov(c_rarg2, arg2);
    mov(c_rarg3, arg3);
  }
  return call_RT(oop_result1, metadata_result, entry, 3);
}

// Implementation of StubFrame

class StubFrame: public StackObj {
 private:
  StubAssembler* _sasm;

 public:
  StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments);
  void load_argument(int offset_in_words, Register reg);

  ~StubFrame();
};;


#define __ _sasm->

StubFrame::StubFrame(StubAssembler* sasm, const char* name, bool must_gc_arguments) {
  _sasm = sasm;
  __ set_info(name, must_gc_arguments);
  __ enter();
}

// load parameters that were stored with LIR_Assembler::store_parameter
// Note: offsets for store_parameter and load_argument must match
void StubFrame::load_argument(int offset_in_words, Register reg) { Unimplemented(); }

StubFrame::~StubFrame() {
  __ leave();
  __ ret(lr);
}

#undef __


// Implementation of Runtime1

#define __ sasm->

const int float_regs_as_doubles_size_in_slots = pd_nof_fpu_regs_frame_map * 2;

// Stack layout for saving/restoring  all the registers needed during a runtime
// call (this includes deoptimization)
// Note: note that users of this frame may well have arguments to some runtime
// while these values are on the stack. These positions neglect those arguments
// but the code in save_live_registers will take the argument count into
// account.
//

enum reg_save_layout {
  reg_save_frame_size = 32 /* float */ + 32 /* integer */
};

// Save off registers which might be killed by calls into the runtime.
// Tries to smart of about FP registers.  In particular we separate
// saving and describing the FPU registers for deoptimization since we
// have to save the FPU registers twice if we describe them.  The
// deopt blob is the only thing which needs to describe FPU registers.
// In all other cases it should be sufficient to simply save their
// current value.

static int cpu_reg_save_offsets[FrameMap::nof_cpu_regs];
static int fpu_reg_save_offsets[FrameMap::nof_fpu_regs];
static int reg_save_size_in_words;
static int frame_size_in_bytes = -1;

static OopMap* generate_oop_map(StubAssembler* sasm, bool save_fpu_registers) {
  int frame_size_in_bytes = reg_save_frame_size * BytesPerWord;
  sasm->set_frame_size(frame_size_in_bytes / BytesPerWord);
  int frame_size_in_slots = frame_size_in_bytes / sizeof(jint);
  OopMap* oop_map = new OopMap(frame_size_in_slots, 0);

  for (int i = 0; i < FrameMap::nof_cpu_regs; i++) {
    Register r = as_Register(i);
    if (i <= 18 && i != rscratch1->encoding() && i != rscratch2->encoding()) {
      int sp_offset = cpu_reg_save_offsets[i];
      oop_map->set_callee_saved(VMRegImpl::stack2reg(sp_offset),
                                r->as_VMReg());
    }
  }

  if (save_fpu_registers) {
    for (int i = 0; i < FrameMap::nof_fpu_regs; i++) {
      FloatRegister r = as_FloatRegister(i);
      {
	int sp_offset = fpu_reg_save_offsets[i];
	oop_map->set_callee_saved(VMRegImpl::stack2reg(sp_offset),
				  r->as_VMReg());
      }
    }
  }
  return oop_map;
}

static OopMap* save_live_registers(StubAssembler* sasm,
                                   bool save_fpu_registers = true) {
  __ block_comment("save_live_registers");

  __ push(0x3fffffff, sp);         // integer registers except lr & sp

  if (save_fpu_registers) {
    for (int i = 30; i >= 0; i -= 2)
      __ stpd(as_FloatRegister(i), as_FloatRegister(i+1),
	      Address(__ pre(sp, -2 * wordSize)));
  } else {
    __ add(sp, sp, -32 * wordSize);
  }

  return generate_oop_map(sasm, save_fpu_registers);
}

static void restore_fpu(StubAssembler* sasm, bool restore_fpu_registers = true) { Unimplemented(); }


static void restore_live_registers(StubAssembler* sasm, bool restore_fpu_registers = true) {
  if (restore_fpu_registers) {
    for (int i = 0; i < 32; i += 2)
      __ ldpd(as_FloatRegister(i), as_FloatRegister(i+1),
	      Address(__ post(sp, 2 * wordSize)));
  } else {
    __ add(sp, sp, 32 * wordSize);
  }

  __ pop(0x3fffffff, sp);
}

static void restore_live_registers_except_r0(StubAssembler* sasm, bool restore_fpu_registers = true)  {

  if (restore_fpu_registers) {
    for (int i = 0; i < 32; i += 2)
      __ ldpd(as_FloatRegister(i), as_FloatRegister(i+1),
	      Address(__ post(sp, 2 * wordSize)));
  } else {
    __ add(sp, sp, 32 * wordSize);
  }

  __ ldp(zr, r1, Address(__ post(sp, 16)));
  __ pop(0x3ffffffc, sp);
}



void Runtime1::initialize_pd() {
  int i;
  int sp_offset = 0;

  // all float registers are saved explicitly
  assert(FrameMap::nof_fpu_regs == 32, "double registers not handled here");
  for (i = 0; i < FrameMap::nof_fpu_regs; i++) {
    fpu_reg_save_offsets[i] = sp_offset;
    sp_offset += 2;   // SP offsets are in halfwords
  }

  for (i = 0; i < FrameMap::nof_cpu_regs; i++) {
    Register r = as_Register(i);
    cpu_reg_save_offsets[i] = sp_offset;
    sp_offset += 2;   // SP offsets are in halfwords
  }
}


// target: the entry point of the method that creates and posts the exception oop
// has_argument: true if the exception needs an argument (passed on stack because registers must be preserved)

OopMapSet* Runtime1::generate_exception_throw(StubAssembler* sasm, address target, bool has_argument) {
  // make a frame and preserve the caller's caller-save registers
  OopMap* oop_map = save_live_registers(sasm);
  int call_offset;
  if (!has_argument) {
    call_offset = __ call_RT(noreg, noreg, target);
  } else {
    call_offset = __ call_RT(noreg, noreg, target, rscratch1);
  }
  OopMapSet* oop_maps = new OopMapSet();
  oop_maps->add_gc_map(call_offset, oop_map);

  __ should_not_reach_here();
  return oop_maps;
}


OopMapSet* Runtime1::generate_handle_exception(StubID id, StubAssembler *sasm) {
  __ block_comment("generate_handle_exception");

  // incoming parameters
  const Register exception_oop = r0;
  const Register exception_pc  = r3;
  // other registers used in this stub

  // Save registers, if required.
  OopMapSet* oop_maps = new OopMapSet();
  OopMap* oop_map = NULL;
  switch (id) {
  case forward_exception_id:
    // We're handling an exception in the context of a compiled frame.
    // The registers have been saved in the standard places.  Perform
    // an exception lookup in the caller and dispatch to the handler
    // if found.  Otherwise unwind and dispatch to the callers
    // exception handler.
    oop_map = generate_oop_map(sasm, 1 /*thread*/);

    // load and clear pending exception oop into r0
    __ ldr(exception_oop, Address(rthread, Thread::pending_exception_offset()));
    __ str(zr, Address(rthread, Thread::pending_exception_offset()));

    // load issuing PC (the return address for this stub) into r3
    __ ldr(exception_pc, Address(rfp, 1*BytesPerWord));

    // make sure that the vm_results are cleared (may be unnecessary)
    __ str(zr, Address(rthread, JavaThread::vm_result_offset()));
    __ str(zr, Address(rthread, JavaThread::vm_result_2_offset()));
    break;
  case handle_exception_nofpu_id:
  case handle_exception_id:
    // At this point all registers MAY be live.
    oop_map = save_live_registers(sasm, id == handle_exception_nofpu_id);
    break;
  case handle_exception_from_callee_id: {
    // At this point all registers except exception oop (r0) and
    // exception pc (lr) are dead.
    const int frame_size = 2 /*fp, return address*/;
    oop_map = new OopMap(frame_size * VMRegImpl::slots_per_word, 0);
    sasm->set_frame_size(frame_size);
    break;
  }
  default:
    __ should_not_reach_here();
    break;
  }

  // verify that only r0 and r3 are valid at this time
  __ invalidate_registers(false, true, true, false, true, true);
  // verify that r0 contains a valid exception
  __ verify_not_null_oop(exception_oop);

#ifdef ASSERT
  // check that fields in JavaThread for exception oop and issuing pc are
  // empty before writing to them
  Label oop_empty;
  __ ldr(rscratch1, Address(rthread, JavaThread::exception_oop_offset()));
  __ cbz(rscratch1, oop_empty);
  __ stop("exception oop already set");
  __ bind(oop_empty);

  Label pc_empty;
  __ ldr(rscratch1, Address(rthread, JavaThread::exception_pc_offset()));
  __ cbz(rscratch1, pc_empty);
  __ stop("exception pc already set");
  __ bind(pc_empty);
#endif

  // save exception oop and issuing pc into JavaThread
  // (exception handler will load it from here)
  __ str(exception_oop, Address(rthread, JavaThread::exception_oop_offset()));
  __ str(exception_pc, Address(rthread, JavaThread::exception_pc_offset()));

  // patch throwing pc into return address (has bci & oop map)
  __ str(exception_pc, Address(rfp, 1*BytesPerWord));

  // compute the exception handler.
  // the exception oop and the throwing pc are read from the fields in JavaThread
  int call_offset = __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, exception_handler_for_pc));
  oop_maps->add_gc_map(call_offset, oop_map);

  // r0: handler address
  //      will be the deopt blob if nmethod was deoptimized while we looked up
  //      handler regardless of whether handler existed in the nmethod.

  // only r0 is valid at this time, all other registers have been destroyed by the runtime call
  __ invalidate_registers(false, true, true, true, true, true);

  // patch the return address, this stub will directly return to the exception handler
  __ str(r0, Address(rfp, 1*BytesPerWord));

  switch (id) {
  case forward_exception_id:
  case handle_exception_nofpu_id:
  case handle_exception_id:
    // Restore the registers that were saved at the beginning.
    restore_live_registers(sasm, id == handle_exception_nofpu_id);
    break;
  case handle_exception_from_callee_id:
    // WIN64_ONLY: No need to add frame::arg_reg_save_area_bytes to SP
    // since we do a leave anyway.

    // Pop the return address since we are possibly changing SP (restoring from BP).
    __ leave();

    // Restore SP from FP if the exception PC is a method handle call site.
    {
      Label nope;
      __ ldr(rscratch1, Address(rthread, JavaThread::is_method_handle_return_offset()));
      __ cbnz(rscratch1, nope);
      __ call_Unimplemented();
      __ bind(nope);
    }

    __ ret(lr);  // jump to exception handler
    break;
  default:  ShouldNotReachHere();
  }

  return oop_maps;
}


void Runtime1::generate_unwind_exception(StubAssembler *sasm) {
  // incoming parameters
  const Register exception_oop = r0;
  // callee-saved copy of exception_oop during runtime call
  const Register exception_oop_callee_saved = r19;
  // other registers used in this stub
  const Register exception_pc = r3;
  const Register handler_addr = r1;

  // verify that only r0, is valid at this time
  __ invalidate_registers(false, true, true, true, true, true);

#ifdef ASSERT
  // check that fields in JavaThread for exception oop and issuing pc are empty
  Label oop_empty;
  __ ldr(rscratch1, Address(rthread, JavaThread::exception_oop_offset()));
  __ cbz(rscratch1, oop_empty);
  __ stop("exception oop must be empty");
  __ bind(oop_empty);

  Label pc_empty;
  __ ldr(rscratch1, Address(rthread, JavaThread::exception_pc_offset()));
  __ cbz(rscratch1, pc_empty);
  __ stop("exception pc must be empty");
  __ bind(pc_empty);
#endif

  // save exception_oop in callee-saved register to preserve it during runtime calls
  __ verify_not_null_oop(exception_oop);
  __ mov(exception_oop_callee_saved, exception_oop);

  // search the exception handler address of the caller (using the return address)
  __ call_VM_leaf(CAST_FROM_FN_PTR(address, SharedRuntime::exception_handler_for_return_address), rthread, lr);
  // r0: exception handler address of the caller

  // Only R0 and R19 are valid at this time; all other registers have been destroyed by the call.
  __ invalidate_registers(false, false, true, true, false, true);

  // move result of call into correct register
  __ mov(handler_addr, r0);

  // Restore exception oop to R0 (required convention of exception handler).
  __ mov(exception_oop, exception_oop_callee_saved);

  // verify that there is really a valid exception in r0
  __ verify_not_null_oop(exception_oop);

  {
    Label foo;
    __ ldrw(rscratch1, Address(rthread, JavaThread::is_method_handle_return_offset()));
    __ cbz(rscratch1, foo);
    __ mov(sp, rfp);
    __ bind(foo);
  }

  // continue at exception handler (return address removed)
  // note: do *not* remove arguments when unwinding the
  //       activation since the caller assumes having
  //       all arguments on the stack when entering the
  //       runtime to determine the exception handler
  //       (GC happens at call site with arguments!)
  // r0: exception oop
  // r3: throwing pc
  // r1: exception handler
  __ br(handler_addr);
}



OopMapSet* Runtime1::generate_patching(StubAssembler* sasm, address target) {
  // use the maximum number of runtime-arguments here because it is difficult to
  // distinguish each RT-Call.
  // Note: This number affects also the RT-Call in generate_handle_exception because
  //       the oop-map is shared for all calls.
  const int num_rt_args = 2;  // thread + dummy

  DeoptimizationBlob* deopt_blob = SharedRuntime::deopt_blob();
  assert(deopt_blob != NULL, "deoptimization blob must have been created");

  OopMap* oop_map = save_live_registers(sasm, num_rt_args);

  __ mov(c_rarg0, rthread);
  __ set_last_Java_frame(sp, rfp, (address)NULL, rscratch1);
  // do the call
  __ mov(rscratch1, RuntimeAddress(target));
  __ brx86(rscratch1, 1, 0, 1);
  OopMapSet* oop_maps = new OopMapSet();
  oop_maps->add_gc_map(__ offset(), oop_map);
  // verify callee-saved register
#ifdef ASSERT
  { Label L;
    __ get_thread(rscratch1);
    __ cmp(rthread, rscratch1);
    __ br(Assembler::EQ, L);
    __ stop("StubAssembler::call_RT: rthread not callee saved?");
    __ bind(L);
  }
#endif
  __ reset_last_Java_frame(true, false);

  // check for pending exceptions
  { Label L;
    __ ldr(rscratch1, Address(rthread, Thread::pending_exception_offset()));
    __ cbz(rscratch1, L);
    // exception pending => remove activation and forward to exception handler

    { Label L1;
      __ cbnz(r0, L1);                                  // have we deoptimized?
      __ b(RuntimeAddress(Runtime1::entry_for(Runtime1::forward_exception_id)));
      __ bind(L1);
    }

    // the deopt blob expects exceptions in the special fields of
    // JavaThread, so copy and clear pending exception.

    // load and clear pending exception
    __ ldr(r0, Address(rthread, Thread::pending_exception_offset()));
    __ str(zr, Address(rthread, Thread::pending_exception_offset()));

    // check that there is really a valid exception
    __ verify_not_null_oop(r0);

    // load throwing pc: this is the return address of the stub
    __ mov(r3, lr);

#ifdef ASSERT
    // check that fields in JavaThread for exception oop and issuing pc are empty
    Label oop_empty;
    __ ldr(rscratch1, Address(rthread, Thread::pending_exception_offset()));
    __ cbz(rscratch1, oop_empty);
    __ stop("exception oop must be empty");
    __ bind(oop_empty);

    Label pc_empty;
    __ ldr(rscratch1, Address(rthread, JavaThread::exception_pc_offset()));
    __ cbz(rscratch1, pc_empty);
    __ stop("exception pc must be empty");
    __ bind(pc_empty);
#endif

    // store exception oop and throwing pc to JavaThread
    __ str(r0, Address(rthread, JavaThread::exception_oop_offset()));
    __ str(r3, Address(rthread, JavaThread::exception_pc_offset()));

    restore_live_registers(sasm);

    __ leave();

    // Forward the exception directly to deopt blob. We can blow no
    // registers and must leave throwing pc on the stack.  A patch may
    // have values live in registers so the entry point with the
    // exception in tls.
    __ b(RuntimeAddress(deopt_blob->unpack_with_exception_in_tls()));

    __ bind(L);
  }


  // Runtime will return true if the nmethod has been deoptimized during
  // the patching process. In that case we must do a deopt reexecute instead.

  Label reexecuteEntry, cont;

  __ cbz(r0, cont);                                 // have we deoptimized?

  // Will reexecute. Proper return address is already on the stack we just restore
  // registers, pop all of our frame but the return address and jump to the deopt blob
  restore_live_registers(sasm);
  __ leave();
  __ b(RuntimeAddress(deopt_blob->unpack_with_reexecution()));

  __ bind(cont);
  restore_live_registers(sasm);
  __ leave();
  __ ret(lr);

  return oop_maps;
}


OopMapSet* Runtime1::generate_code_for(StubID id, StubAssembler* sasm) {

  const Register exception_oop = r0;
  const Register exception_pc  = r3;

  // for better readability
  const bool must_gc_arguments = true;
  const bool dont_gc_arguments = false;

  // default value; overwritten for some optimized stubs that are called from methods that do not use the fpu
  bool save_fpu_registers = true;

  // stub code & info for the different stubs
  OopMapSet* oop_maps = NULL;
  OopMap* oop_map = NULL;
  switch (id) {
    {
    case forward_exception_id:
      {
        oop_maps = generate_handle_exception(id, sasm);
        __ leave();
        __ ret(lr);
      }
      break;

    case throw_div0_exception_id:
      { StubFrame f(sasm, "throw_div0_exception", dont_gc_arguments);
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address, throw_div0_exception), false);
      }
      break;

    case new_instance_id:
    case fast_new_instance_id:
    case fast_new_instance_init_check_id:
      {
        Register klass = r3; // Incoming
        Register obj   = r0; // Result

        if (id == new_instance_id) {
          __ set_info("new_instance", dont_gc_arguments);
        } else if (id == fast_new_instance_id) {
          __ set_info("fast new_instance", dont_gc_arguments);
        } else {
          assert(id == fast_new_instance_init_check_id, "bad StubID");
          __ set_info("fast new_instance init check", dont_gc_arguments);
        }

        if ((id == fast_new_instance_id || id == fast_new_instance_init_check_id) &&
            UseTLAB && FastTLABRefill) {
          Label slow_path;
          Register obj_size = r2;
          Register t1       = r19;
          Register t2       = r4;
          assert_different_registers(klass, obj, obj_size, t1, t2);

          __ stp(r5, r19, Address(__ pre(sp, -2 * wordSize)));

          if (id == fast_new_instance_init_check_id) {
            // make sure the klass is initialized
            __ ldrb(rscratch1, Address(klass, InstanceKlass::init_state_offset()));
            __ cmpw(rscratch1, InstanceKlass::fully_initialized);
            __ br(Assembler::NE, slow_path);
          }

#ifdef ASSERT
          // assert object can be fast path allocated
          {
            Label ok, not_ok;
            __ ldrw(obj_size, Address(klass, Klass::layout_helper_offset()));
            __ cmp(obj_size, 0u);
            __ br(Assembler::LE, not_ok);  // make sure it's an instance (LH > 0)
            __ tstw(obj_size, Klass::_lh_instance_slow_path_bit);
            __ br(Assembler::EQ, ok);
            __ bind(not_ok);
            __ stop("assert(can be fast path allocated)");
            __ should_not_reach_here();
            __ bind(ok);
          }
#endif // ASSERT

          // if we got here then the TLAB allocation failed, so try
          // refilling the TLAB or allocating directly from eden.
          Label retry_tlab, try_eden;
          const Register thread =
            __ tlab_refill(retry_tlab, try_eden, slow_path); // does not destroy r3 (klass), returns r5

          __ bind(retry_tlab);

          // get the instance size (size is postive so movl is fine for 64bit)
          __ ldrw(obj_size, Address(klass, Klass::layout_helper_offset()));

          __ tlab_allocate(obj, obj_size, 0, t1, t2, slow_path);

          __ initialize_object(obj, klass, obj_size, 0, t1, t2);
          __ verify_oop(obj);
          __ ldp(r5, r19, Address(__ post(sp, 2 * wordSize)));
          __ ret(lr);

          __ bind(try_eden);
          // get the instance size (size is postive so movl is fine for 64bit)
          __ ldrw(obj_size, Address(klass, Klass::layout_helper_offset()));

          __ eden_allocate(obj, obj_size, 0, t1, slow_path);
          __ incr_allocated_bytes(thread, obj_size, 0, rscratch1);

          __ initialize_object(obj, klass, obj_size, 0, t1, t2);
          __ verify_oop(obj);
          __ ldp(r5, r19, Address(__ post(sp, 2 * wordSize)));
          __ ret(lr);

          __ bind(slow_path);
          __ ldp(r5, r19, Address(__ post(sp, 2 * wordSize)));
        }

        __ enter();
        OopMap* map = save_live_registers(sasm, 2);
        int call_offset = __ call_RT(obj, noreg, CAST_FROM_FN_PTR(address, new_instance), klass);
        oop_maps = new OopMapSet();
        oop_maps->add_gc_map(call_offset, map);
        restore_live_registers_except_r0(sasm);
        __ verify_oop(obj);
        __ leave();
        __ ret(lr);

        // r0,: new instance
      }

      break;

    case new_type_array_id:
    case new_object_array_id:
      {
        Register length   = r19; // Incoming
        Register klass    = r3; // Incoming
        Register obj      = r0; // Result

        if (id == new_type_array_id) {
          __ set_info("new_type_array", dont_gc_arguments);
        } else {
          __ set_info("new_object_array", dont_gc_arguments);
        }

#ifdef ASSERT
        // assert object type is really an array of the proper kind
        {
          Label ok;
          Register t0 = obj;
          __ ldrw(t0, Address(klass, Klass::layout_helper_offset()));
          __ asrw(t0, t0, Klass::_lh_array_tag_shift);
          int tag = ((id == new_type_array_id)
                     ? Klass::_lh_array_tag_type_value
                     : Klass::_lh_array_tag_obj_value);
	  __ mov(rscratch1, tag);
          __ cmpw(t0, rscratch1);
          __ br(Assembler::EQ, ok);
          __ stop("assert(is an array klass)");
          __ should_not_reach_here();
          __ bind(ok);
        }
#endif // ASSERT

        if (UseTLAB && FastTLABRefill) {
          Register arr_size = r4;
          Register t1       = r2;
          Register t2       = r5;
          Label slow_path;
          assert_different_registers(length, klass, obj, arr_size, t1, t2);

          // check that array length is small enough for fast path.
	  __ mov(rscratch1, C1_MacroAssembler::max_array_allocation_length);
          __ cmpw(length, rscratch1);
          __ br(Assembler::HI, slow_path);

          // if we got here then the TLAB allocation failed, so try
          // refilling the TLAB or allocating directly from eden.
          Label retry_tlab, try_eden;
          const Register thread =
            __ tlab_refill(retry_tlab, try_eden, slow_path); // preserves rbx & rdx, returns rdi

          __ bind(retry_tlab);

          // get the allocation size: round_up(hdr + length << (layout_helper & 0x1F))
          // since size is positive ldrw does right thing on 64bit
          __ ldrw(t1, Address(klass, Klass::layout_helper_offset()));
          __ lslvw(arr_size, length, t1);
	  __ ubfx(t1, t1, Klass::_lh_header_size_shift,
		  exact_log2(Klass::_lh_header_size_mask + 1));
          __ add(arr_size, arr_size, t1);
          __ add(arr_size, arr_size, MinObjAlignmentInBytesMask); // align up
          __ andr(arr_size, arr_size, ~MinObjAlignmentInBytesMask);

          __ tlab_allocate(obj, arr_size, 0, t1, t2, slow_path);  // preserves arr_size

          __ initialize_header(obj, klass, length, t1, t2);
          __ ldrb(t1, Address(klass, in_bytes(Klass::layout_helper_offset()) + (Klass::_lh_header_size_shift / BitsPerByte)));
          assert(Klass::_lh_header_size_shift % BitsPerByte == 0, "bytewise");
          assert(Klass::_lh_header_size_mask <= 0xFF, "bytewise");
          __ andr(t1, t1, Klass::_lh_header_size_mask);
          __ sub(arr_size, arr_size, t1);  // body length
          __ add(t1, t1, obj);       // body start
          __ initialize_body(t1, arr_size, 0, t2);
          __ verify_oop(obj);

          __ ret(lr);

          __ bind(try_eden);
          // get the allocation size: round_up(hdr + length << (layout_helper & 0x1F))
          // since size is positive ldrw does right thing on 64bit
          __ ldrw(t1, Address(klass, Klass::layout_helper_offset()));
          // since size is postive movw does right thing on 64bit
          __ movw(arr_size, length);
          __ lslvw(arr_size, length, t1);
	  __ ubfx(t1, t1, Klass::_lh_header_size_shift,
		  exact_log2(Klass::_lh_header_size_mask + 1));
          __ add(arr_size, arr_size, t1);
          __ add(arr_size, arr_size, MinObjAlignmentInBytesMask); // align up
          __ andr(arr_size, arr_size, ~MinObjAlignmentInBytesMask);

          __ eden_allocate(obj, arr_size, 0, t1, slow_path);  // preserves arr_size
          __ incr_allocated_bytes(thread, arr_size, 0, rscratch1);

          __ initialize_header(obj, klass, length, t1, t2);
          __ ldrb(t1, Address(klass, in_bytes(Klass::layout_helper_offset()) + (Klass::_lh_header_size_shift / BitsPerByte)));
          assert(Klass::_lh_header_size_shift % BitsPerByte == 0, "bytewise");
          assert(Klass::_lh_header_size_mask <= 0xFF, "bytewise");
          __ andr(t1, t1, Klass::_lh_header_size_mask);
          __ sub(arr_size, arr_size, t1);  // body length
          __ add(t1, t1, obj);       // body start
          __ initialize_body(t1, arr_size, 0, t2);
          __ verify_oop(obj);

          __ ret(lr);

          __ bind(slow_path);
        }

        __ enter();
        OopMap* map = save_live_registers(sasm, 3);
        int call_offset;
	if (id == new_type_array_id) {
          call_offset = __ call_RT(obj, noreg, CAST_FROM_FN_PTR(address, new_type_array), klass, length);
        } else {
          call_offset = __ call_RT(obj, noreg, CAST_FROM_FN_PTR(address, new_object_array), klass, length);
        }

        oop_maps = new OopMapSet();
        oop_maps->add_gc_map(call_offset, map);
        restore_live_registers_except_r0(sasm);

        __ verify_oop(obj);
        __ leave();
        __ ret(lr);

        // r0: new array
      }
      break;

    case register_finalizer_id:
      {
        __ set_info("register_finalizer", dont_gc_arguments);

        // This is called via call_runtime so the arguments
        // will be place in C abi locations

        __ verify_oop(c_rarg0);

        // load the klass and check the has finalizer flag
        Label register_finalizer;
        Register t = r5;
        __ load_klass(t, r0);
        __ ldrw(t, Address(t, Klass::access_flags_offset()));
        __ tst(t, JVM_ACC_HAS_FINALIZER);
        __ br(Assembler::NE, register_finalizer);
        __ ret(lr);

        __ bind(register_finalizer);
        __ enter();
        OopMap* oop_map = save_live_registers(sasm, 2 /*num_rt_args */);
        int call_offset = __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, SharedRuntime::register_finalizer), r0);
        oop_maps = new OopMapSet();
        oop_maps->add_gc_map(call_offset, oop_map);

        // Now restore all the live registers
        restore_live_registers(sasm);

        __ leave();
        __ ret(lr);
      }
      break;

    case throw_range_check_failed_id:
      { StubFrame f(sasm, "range_check_failed", dont_gc_arguments);
        oop_maps = generate_exception_throw(sasm, CAST_FROM_FN_PTR(address, throw_range_check_exception), true);
      }
      break;

    case unwind_exception_id:
      { __ set_info("unwind_exception", dont_gc_arguments);
        // note: no stubframe since we are about to leave the current
        //       activation and we are calling a leaf VM function only.
        generate_unwind_exception(sasm);
      }
      break;

    case access_field_patching_id:
      { StubFrame f(sasm, "access_field_patching", dont_gc_arguments);
        // we should set up register map
        oop_maps = generate_patching(sasm, CAST_FROM_FN_PTR(address, access_field_patching));
      }
      break;

    case load_klass_patching_id:
      { StubFrame f(sasm, "load_klass_patching", dont_gc_arguments);
        // we should set up register map
        oop_maps = generate_patching(sasm, CAST_FROM_FN_PTR(address, move_klass_patching));
      }
      break;

    case load_mirror_patching_id:
      { StubFrame f(sasm, "load_mirror_patching", dont_gc_arguments);
        // we should set up register map
        oop_maps = generate_patching(sasm, CAST_FROM_FN_PTR(address, move_mirror_patching));
      }
      break;

    case handle_exception_nofpu_id:
    case handle_exception_id:
      { StubFrame f(sasm, "handle_exception", dont_gc_arguments);
        oop_maps = generate_handle_exception(id, sasm);
      }
      break;

    case handle_exception_from_callee_id:
      { StubFrame f(sasm, "handle_exception_from_callee", dont_gc_arguments);
        oop_maps = generate_handle_exception(id, sasm);
      }
      break;

    default:
      { StubFrame f(sasm, "unimplemented entry", dont_gc_arguments);
        __ mov(r0, (int)id);
        __ call_RT(noreg, noreg, CAST_FROM_FN_PTR(address, unimplemented_entry), r0);
        __ should_not_reach_here();
      }
      break;
    }
  }
  return oop_maps;
}

#undef __

const char *Runtime1::pd_name_for_address(address entry) { Unimplemented(); return 0; }
