/*
 * Copyright (c) 1997, 2010, Oracle and/or its affiliates. All rights reserved.
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
#include "assembler_aarch64.inline.hpp"
#include "runtime/icache.hpp"

extern void aarch64TestHook();
extern "C" void setup_arm_sim();

#define __ _masm->

int _flush_icache_stub_dummy(address addr, int lines, int magic)
{
  // no need to do any cache flushing on x86 so just obey th eimplicit
  // contract to return the magic arg

  return magic;
}

void ICacheStubGenerator::generate_icache_flush(ICache::flush_icache_stub_t* flush_icache_stub) {

  aarch64TestHook();

  StubCodeMark mark(this, "ICache", "flush_icache_stub");

  address start = __ pc();

  // generate a c stub prolog which will bootstrap into the ARM code
  // which follows, loading the 3 general registers passed by the
  // caller into the first 3 ARM registers

  __ c_stub_prolog(3, 0, MacroAssembler::ret_type_integral);

  address entry = __ pc();

  // we need to invalidate each cache line and then skip to the next
  // one.
  // TODO : currently use a nop for the flush operation. we also
  // need a mfence before and after the loop.
  address loop = __ pc();
  __ nop();			// should actually invalidate r0
  __ add(r0, r0, NativeCall::cache_line_size);
  __ sub(r1, r1, 1);
  __ cbnz(r1, loop);
  // we need to move r2 to r0
  __ add(r0, r2, 0u);
  __ ret(r30);

  Disassembler::decode(entry, __ pc());

  *flush_icache_stub =  (ICache::flush_icache_stub_t)start;
}

#undef __
