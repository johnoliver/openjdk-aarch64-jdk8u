/*
 * Copyright (c) 2018, Red Hat, Inc. and/or its affiliates.
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
#ifndef SHARE_VM_GC_SHENANDOAH_VMSTRUCTS_SHENANDOAH_HPP
#define SHARE_VM_GC_SHENANDOAH_VMSTRUCTS_SHENANDOAH_HPP

#include "gc_implementation/shenandoah/shenandoahHeap.hpp"
#include "gc_implementation/shenandoah/shenandoahHeapRegion.hpp"
#include "gc_implementation/shenandoah/shenandoahMonitoringSupport.hpp"

#define VM_STRUCTS_SHENANDOAH(nonstatic_field, volatile_nonstatic_field, static_field)  \
  static_field(ShenandoahHeapRegion, RegionSizeBytes,        size_t)                    \
  nonstatic_field(ShenandoahHeap, _num_regions,              size_t)                    \
  volatile_nonstatic_field(ShenandoahHeap, _used,            jlong)                     \
  volatile_nonstatic_field(ShenandoahHeap, _committed,       size_t)                    \
  nonstatic_field(ShenandoahHeap,          _regions, ShenandoahHeapRegion**)            \
  nonstatic_field(ShenandoahHeapRegion,    _state,  ShenandoahHeapRegion::RegionState)  \
  nonstatic_field(ShenandoahHeapRegion,    _region_number,   size_t)

#define VM_INT_CONSTANTS_SHENANDOAH(declare_constant, declare_constant_with_value)    \
  declare_constant(ShenandoahHeapRegion::_empty_uncommitted)                          \
  declare_constant(ShenandoahHeapRegion::_empty_committed)                            \
  declare_constant(ShenandoahHeapRegion::_regular)                                    \
  declare_constant(ShenandoahHeapRegion::_humongous_start)                            \
  declare_constant(ShenandoahHeapRegion::_humongous_cont)                             \
  declare_constant(ShenandoahHeapRegion::_pinned_humongous_start)                     \
  declare_constant(ShenandoahHeapRegion::_cset)                                       \
  declare_constant(ShenandoahHeapRegion::_pinned)                                     \
  declare_constant(ShenandoahHeapRegion::_pinned_cset)                                \
  declare_constant(ShenandoahHeapRegion::_trash)

#define VM_TYPES_SHENANDOAH(declare_type,                                     \
                            declare_toplevel_type,                            \
                            declare_integer_type)                             \
  declare_type(ShenandoahHeap, CollectedHeap)                                 \
  declare_type(ShenandoahHeapRegion, ContiguousSpace)                         \
  declare_toplevel_type(ShenandoahHeap*)                                      \
  declare_toplevel_type(ShenandoahHeapRegion*)                                \
  declare_integer_type(ShenandoahHeapRegion::RegionState)                     \

#endif // SHARE_VM_GC_SHENANDOAH_VMSTRUCTS_SHENANDOAH_HPP
