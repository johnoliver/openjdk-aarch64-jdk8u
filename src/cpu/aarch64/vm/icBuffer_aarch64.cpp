/*
 * Copyright (c) 2013, Red Hat Inc.
 * Copyright (c) 1997, 2010, Oracle and/or its affiliates.
 * All rights reserved.
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
#include "asm/macroAssembler.hpp"
#include "asm/macroAssembler.inline.hpp"
#include "code/icBuffer.hpp"
#include "gc_interface/collectedHeap.inline.hpp"
#include "interpreter/bytecodes.hpp"
#include "memory/resourceArea.hpp"
#include "nativeInst_aarch64.hpp"
#include "oops/oop.inline.hpp"
#include "oops/oop.inline2.hpp"

int InlineCacheBuffer::ic_stub_code_size() {
  return NativeInstruction::instruction_size * 5;
}


void InlineCacheBuffer::assemble_ic_buffer_code(address code_begin, void* cached_value, address entry_point) {
  ResourceMark rm;
  CodeBuffer      code(code_begin, ic_stub_code_size());
  MacroAssembler* masm            = new MacroAssembler(&code);
  // note: even though the code contains an embedded value, we do not need reloc info
  // because
  // (1) the value is old (i.e., doesn't matter for scavenges)
  // (2) these ICStubs are removed *before* a GC happens, so the roots disappear
  // assert(cached_value == NULL || cached_oop->is_perm(), "must be perm oop");

  Label l;
  masm->ldr(rscratch2, l);
  masm->b(ExternalAddress(entry_point));
  masm->bind(l);
  masm->emit_int64((int64_t)cached_value);
  0;
}

address InlineCacheBuffer::ic_buffer_entry_point(address code_begin) {
  NativeMovConstReg* move = nativeMovConstReg_at(code_begin);   // creation also verifies the object
  NativeJump* jump = nativeJump_at(code_begin + 4);
  return jump->jump_destination();
}


void* InlineCacheBuffer::ic_buffer_cached_value(address code_begin) {
  // creation also verifies the object
  uintptr_t *p = (uintptr_t *)(code_begin + 8);
  void* o = (void*)*p;
  return o;
}
