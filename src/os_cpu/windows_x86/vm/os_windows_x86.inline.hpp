/*
 * Copyright (c) 2011, 2012, Oracle and/or its affiliates. All rights reserved.
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

#ifndef OS_CPU_WINDOWS_X86_VM_OS_WINDOWS_X86_INLINE_HPP
#define OS_CPU_WINDOWS_X86_VM_OS_WINDOWS_X86_INLINE_HPP

#include "runtime/os.hpp"

inline jlong os::rdtsc() {
  // 32 bit: 64 bit result in edx:eax
  // 64 bit: 64 bit value in rax
  uint64_t res;
  res = (uint64_t)__rdtsc();
  return (jlong)res;
}

#endif // OS_CPU_WINDOWS_X86_VM_OS_WINDOWS_X86_INLINE_HPP
