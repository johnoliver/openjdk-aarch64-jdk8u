/*
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
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

#include "incls/_precompiled.incl"
#include "incls/_ciMethodHandle.cpp.incl"

// ciMethodHandle

// ------------------------------------------------------------------
// ciMethodHandle::get_adapter
//
// Return an adapter for this MethodHandle.
ciMethod* ciMethodHandle::get_adapter(bool is_invokedynamic) const {
  VM_ENTRY_MARK;

  Handle h(get_oop());
  methodHandle callee(_callee->get_methodOop());
  MethodHandleCompiler mhc(h, callee, is_invokedynamic, THREAD);
  methodHandle m = mhc.compile(CHECK_NULL);
  return CURRENT_ENV->get_object(m())->as_method();
}


// ------------------------------------------------------------------
// ciMethodHandle::print_impl
//
// Implementation of the print method.
void ciMethodHandle::print_impl(outputStream* st) {
  st->print(" type=");
  get_oop()->print();
}
