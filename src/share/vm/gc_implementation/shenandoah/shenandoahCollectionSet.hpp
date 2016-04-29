/*
 * Copyright (c) 2016, Red Hat, Inc. and/or its affiliates.
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

#include "gc_implementation/shenandoah/shenandoahHeapRegionSet.hpp"

#ifndef SHARE_VM_GC_SHENANDOAH_SHENANDOAHCOLLECTIONSET_HPP
#define SHARE_VM_GC_SHENANDOAH_SHENANDOAHCOLLECTIONSET_HPP

class ShenandoahCollectionSet: public ShenandoahHeapRegionSet {
private:
  size_t _garbage;
  size_t _live_data;
public:
  ShenandoahCollectionSet(size_t max_regions);
  ~ShenandoahCollectionSet();
  void add_region(ShenandoahHeapRegion* r);
  size_t live_data();
  size_t garbage();
  void clear();
};

#endif //SHARE_VM_GC_SHENANDOAH_SHENANDOAHCOLLECTIONSET_HPP
