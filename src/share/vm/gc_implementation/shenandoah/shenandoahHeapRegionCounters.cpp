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

#include "gc_implementation/shenandoah/shenandoahHeap.inline.hpp"
#include "gc_implementation/shenandoah/shenandoahHeapRegion.hpp"
#include "gc_implementation/shenandoah/shenandoahHeapRegionSet.hpp"
#include "gc_implementation/shenandoah/shenandoahHeapRegionCounters.hpp"
#include "runtime/perfData.hpp"

ShenandoahHeapRegionCounters::ShenandoahHeapRegionCounters() {

  if (UsePerfData) {
    EXCEPTION_MARK;
    ResourceMark rm;
    ShenandoahHeap* heap = ShenandoahHeap::heap();
    size_t max_regions = heap->max_regions();
    const char* cns = PerfDataManager::name_space("shenandoah", "regions");
    _name_space = NEW_C_HEAP_ARRAY(char, strlen(cns)+1, mtGC);
    strcpy(_name_space, cns);

    const char* cname = PerfDataManager::counter_name(_name_space, "timestamp");
    _timestamp = PerfDataManager::create_long_variable(SUN_GC, cname, PerfData::U_None, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "max_regions");
    PerfDataManager::create_constant(SUN_GC, cname, PerfData::U_None, max_regions, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "region_size");
    PerfDataManager::create_constant(SUN_GC, cname, PerfData::U_None, ShenandoahHeapRegion::RegionSizeBytes >> 10, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "status");
    _status = PerfDataManager::create_long_variable(SUN_GC, cname,
                                                    PerfData::U_None, CHECK);

    _regions_data = NEW_C_HEAP_ARRAY(PerfVariable*, max_regions, mtGC);
    for (uint i = 0; i < max_regions; i++) {
      const char* reg_name = PerfDataManager::name_space(_name_space, "region", i);
      const char* data_name = PerfDataManager::counter_name(reg_name, "data");
      const char* ns = PerfDataManager::ns_to_string(SUN_GC);
      const char* fullname = PerfDataManager::counter_name(ns, data_name);
      assert(!PerfDataManager::exists(fullname), "must not exist");
      _regions_data[i] = PerfDataManager::create_long_variable(SUN_GC, data_name,
                                                               PerfData::U_None, CHECK);

    }
  }
}

ShenandoahHeapRegionCounters::~ShenandoahHeapRegionCounters() {
  if (_name_space != NULL) FREE_C_HEAP_ARRAY(char, _name_space, mtGC);
}

void ShenandoahHeapRegionCounters::update() {
  if (ShenandoahRegionSampling) {
    jlong current = os::javaTimeMillis();
    if (current - _last_sample_millis > ShenandoahRegionSamplingRate) {
      ShenandoahHeap* heap = ShenandoahHeap::heap();
      jlong status = 0;
      if (heap->concurrent_mark_in_progress()) status |= 1;
      if (heap->is_evacuation_in_progress()) status |= 2;
      _status->set_value(status);

      _timestamp->set_value(os::elapsed_counter());

      size_t num_regions = heap->num_regions();
      size_t max_regions = heap->max_regions();
      ShenandoahHeapRegionSet* regions = heap->regions();
      for (uint i = 0; i < max_regions; i++) {
        if (i < num_regions) {
          ShenandoahHeapRegion* r = regions->get(i);
          jlong data = ((r->used() >> 10) & USED_MASK) << USED_SHIFT;
          data |= ((r->get_live_data_bytes() >> 10) & LIVE_MASK) << LIVE_SHIFT;
          jlong flags = 0;
          if (r->in_collection_set())     flags |= 1 << 1;
          if (r->is_humongous())          flags |= 1 << 2;
          if (r->is_recently_allocated()) flags |= 1 << 3;
          if (r->is_pinned())             flags |= 1 << 4;
          data |= (flags & FLAGS_MASK) << FLAGS_SHIFT;
          _regions_data[i]->set_value(data);
        } else {
          jlong flags = 1 << 0;
          flags = (flags & FLAGS_MASK) << FLAGS_SHIFT;
          _regions_data[i]->set_value(flags);
        }
      }
      _last_sample_millis = current;
    }
  }
}
