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


#ifndef SHARE_VM_GC_SHENANDOAH_SHENANDOAHCOLLECTIONSET_HPP
#define SHARE_VM_GC_SHENANDOAH_SHENANDOAHCOLLECTIONSET_HPP

#include "memory/allocation.hpp"
#include "utilities/ostream.hpp"

class ShenandoahHeap;
class ShenandoahHeapRegion;

class ShenandoahCollectionSet : public CHeapObj<mtGC> {
  friend class ShenandoahHeap;
private:
  jbyte*                _cset_map;
  jbyte*                _biased_cset_map;
  size_t                _map_size;

  ShenandoahHeap* const _heap;

  size_t                _garbage;
  size_t                _live_data;
  size_t                _region_count;

  volatile jint         _current_index;
public:
  ShenandoahCollectionSet(ShenandoahHeap* heap, HeapWord* heap_base);

  // Add region to collection set
  void add_region(ShenandoahHeapRegion* r);

  // Bring per-region statuses to consistency with this collection.
  // TODO: This is a transitional interface that bridges the gap between
  // region statuses and this collection. Should go away after we merge them.
  void update_region_status();

  // Remove region from collection set
  void remove_region(ShenandoahHeapRegion* r);

  // MT version
  ShenandoahHeapRegion* claim_next();

  // Single-thread version
  ShenandoahHeapRegion* next();

  size_t count()  const { return _region_count; }
  bool is_empty() const { return _region_count == 0; }

  void clear_current_index() {
    _current_index = 0;
  }

  inline bool is_in(ShenandoahHeapRegion* r) const;
  inline bool is_in(size_t region_number)    const;
  inline bool is_in(HeapWord* p)             const;

  void print(outputStream* out = tty) const;

  size_t live_data() const { return _live_data; }
  size_t garbage()   const { return _garbage;   }
  void clear();

private:
  jbyte* biased_map_address() const {
    return _biased_cset_map;
  }
};

#endif //SHARE_VM_GC_SHENANDOAH_SHENANDOAHCOLLECTIONSET_HPP
