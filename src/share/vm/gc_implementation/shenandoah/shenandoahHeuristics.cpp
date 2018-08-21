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

#include "precompiled.hpp"
#include "gc_implementation/shenandoah/brooksPointer.hpp"
#include "gc_implementation/shenandoah/shenandoahCollectorPolicy.hpp"
#include "gc_implementation/shenandoah/shenandoahHeap.inline.hpp"
#include "gc_implementation/shenandoah/shenandoahHeapRegion.hpp"
#include "gc_implementation/shenandoah/shenandoahHeuristics.hpp"
#include "gc_implementation/shenandoah/shenandoahMarkingContext.inline.hpp"

int ShenandoahHeuristics::compare_by_garbage(RegionData a, RegionData b) {
  if (a._garbage > b._garbage)
    return -1;
  else if (a._garbage < b._garbage)
    return 1;
  else return 0;
}

int ShenandoahHeuristics::compare_by_alloc_seq_ascending(RegionData a, RegionData b) {
  if (a._seqnum_last_alloc == b._seqnum_last_alloc)
    return 0;
  else if (a._seqnum_last_alloc < b._seqnum_last_alloc)
    return -1;
  else return 1;
}

int ShenandoahHeuristics::compare_by_alloc_seq_descending(RegionData a, RegionData b) {
  return -compare_by_alloc_seq_ascending(a, b);
}

int ShenandoahHeuristics::compare_by_connects(RegionConnections a, RegionConnections b) {
  if (a._connections == b._connections)
    return 0;
  else if (a._connections < b._connections)
    return -1;
  else return 1;
}

ShenandoahHeuristics::ShenandoahHeuristics() :
  _update_refs_early(false),
  _update_refs_adaptive(false),
  _region_data(NULL),
  _region_data_size(0),
  _region_connects(NULL),
  _region_connects_size(0),
  _degenerated_cycles_in_a_row(0),
  _successful_cycles_in_a_row(0),
  _bytes_in_cset(0),
  _last_cycle_end(0) {
  if (strcmp(ShenandoahUpdateRefsEarly, "on") == 0 ||
      strcmp(ShenandoahUpdateRefsEarly, "true") == 0 ) {
    _update_refs_early = true;
  } else if (strcmp(ShenandoahUpdateRefsEarly, "off") == 0 ||
             strcmp(ShenandoahUpdateRefsEarly, "false") == 0 ) {
    _update_refs_early = false;
  } else if (strcmp(ShenandoahUpdateRefsEarly, "adaptive") == 0) {
    _update_refs_adaptive = true;
    _update_refs_early = true;
  } else {
    vm_exit_during_initialization("Unknown -XX:ShenandoahUpdateRefsEarly option: %s", ShenandoahUpdateRefsEarly);
  }

  // No unloading during concurrent mark? Communicate that to heuristics
  if (!ClassUnloadingWithConcurrentMark) {
    FLAG_SET_DEFAULT(ShenandoahUnloadClassesFrequency, 0);
  }
}

ShenandoahHeuristics::~ShenandoahHeuristics() {
  if (_region_data != NULL) {
    FREE_C_HEAP_ARRAY(RegionGarbage, _region_data, mtGC);
  }
}

ShenandoahHeuristics::RegionData* ShenandoahHeuristics::get_region_data_cache(size_t num) {
  RegionData* res = _region_data;
  if (res == NULL) {
    res = NEW_C_HEAP_ARRAY(RegionData, num, mtGC);
    _region_data = res;
    _region_data_size = num;
  } else if (_region_data_size < num) {
    res = REALLOC_C_HEAP_ARRAY(RegionData, _region_data, num, mtGC);
    _region_data = res;
    _region_data_size = num;
  }
  return res;
}

ShenandoahHeuristics::RegionConnections* ShenandoahHeuristics::get_region_connects_cache(size_t num) {
  RegionConnections* res = _region_connects;
  if (res == NULL) {
    res = NEW_C_HEAP_ARRAY(RegionConnections, num, mtGC);
    _region_connects = res;
    _region_connects_size = num;
  } else if (_region_connects_size < num) {
    res = REALLOC_C_HEAP_ARRAY(RegionConnections, _region_connects, num, mtGC);
    _region_connects = res;
    _region_connects_size = num;
  }
  return res;
}

void ShenandoahHeuristics::choose_collection_set(ShenandoahCollectionSet* collection_set) {
  assert(collection_set->count() == 0, "Must be empty");
  start_choose_collection_set();

  ShenandoahHeap* heap = ShenandoahHeap::heap();

  // Step 1. Build up the region candidates we care about, rejecting losers and accepting winners right away.

  size_t num_regions = heap->num_regions();

  RegionData* candidates = get_region_data_cache(num_regions);

  size_t cand_idx = 0;

  size_t total_garbage = 0;

  size_t immediate_garbage = 0;
  size_t immediate_regions = 0;

  size_t free = 0;
  size_t free_regions = 0;

  ShenandoahMarkingContext* const ctx = heap->complete_marking_context();

  for (size_t i = 0; i < num_regions; i++) {
    ShenandoahHeapRegion* region = heap->get_region(i);

    size_t garbage = region->garbage();
    total_garbage += garbage;

    if (region->is_empty()) {
      free_regions++;
      free += ShenandoahHeapRegion::region_size_bytes();
    } else if (region->is_regular()) {
      if (!region->has_live()) {
        // We can recycle it right away and put it in the free set.
        immediate_regions++;
        immediate_garbage += garbage;
        region->make_trash();
      } else {
        // This is our candidate for later consideration.
        candidates[cand_idx]._region = region;
        candidates[cand_idx]._garbage = garbage;
        cand_idx++;
      }
    } else if (region->is_humongous_start()) {
      // Reclaim humongous regions here, and count them as the immediate garbage
#ifdef ASSERT
      bool reg_live = region->has_live();
      bool bm_live = ctx->is_marked(oop(region->bottom() + BrooksPointer::word_size()));
      assert(reg_live == bm_live,
             err_msg("Humongous liveness and marks should agree. Region live: %s; Bitmap live: %s; Region Live Words: " SIZE_FORMAT,
                     BOOL_TO_STR(reg_live), BOOL_TO_STR(bm_live), region->get_live_data_words()));
#endif
      if (!region->has_live()) {
        heap->trash_humongous_region_at(region);

        // Count only the start. Continuations would be counted on "trash" path
        immediate_regions++;
        immediate_garbage += garbage;
      }
    } else if (region->is_trash()) {
      // Count in just trashed collection set, during coalesced CM-with-UR
      immediate_regions++;
      immediate_garbage += garbage;
    }
  }

  // Step 2. Look back at garbage statistics, and decide if we want to collect anything,
  // given the amount of immediately reclaimable garbage. If we do, figure out the collection set.

  assert (immediate_garbage <= total_garbage,
          err_msg("Cannot have more immediate garbage than total garbage: " SIZE_FORMAT "M vs " SIZE_FORMAT "M",
                  immediate_garbage / M, total_garbage / M));

  size_t immediate_percent = total_garbage == 0 ? 0 : (immediate_garbage * 100 / total_garbage);

  if (immediate_percent <= ShenandoahImmediateThreshold) {
    choose_collection_set_from_regiondata(collection_set, candidates, cand_idx, immediate_garbage + free);
    collection_set->update_region_status();

    size_t cset_percent = total_garbage == 0 ? 0 : (collection_set->garbage() * 100 / total_garbage);
    log_info(gc, ergo)("Collectable Garbage: " SIZE_FORMAT "M (" SIZE_FORMAT "%% of total), " SIZE_FORMAT "M CSet, " SIZE_FORMAT " CSet regions",
                       collection_set->garbage() / M, cset_percent, collection_set->live_data() / M, collection_set->count());
  }
  end_choose_collection_set();

  log_info(gc, ergo)("Immediate Garbage: " SIZE_FORMAT "M (" SIZE_FORMAT "%% of total), " SIZE_FORMAT " regions",
                     immediate_garbage / M, immediate_percent, immediate_regions);
}

void ShenandoahHeuristics::record_gc_start() {
  // Do nothing
}

void ShenandoahHeuristics::record_gc_end() {
  // Do nothing
}

void ShenandoahHeuristics::record_cycle_start() {
  // Do nothing
}

void ShenandoahHeuristics::record_cycle_end() {
  _last_cycle_end = os::elapsedTime();
}

void ShenandoahHeuristics::record_phase_time(ShenandoahPhaseTimings::Phase phase, double secs) {
  // Do nothing
}

void ShenandoahHeuristics::print_thresholds() {
  // Do nothing
}

bool ShenandoahHeuristics::should_start_update_refs() {
  return _update_refs_early;
}

bool ShenandoahHeuristics::should_degenerate_cycle() {
  return _degenerated_cycles_in_a_row <= ShenandoahFullGCThreshold;
}

void ShenandoahHeuristics::record_success_concurrent() {
  _degenerated_cycles_in_a_row = 0;
  _successful_cycles_in_a_row++;
}

void ShenandoahHeuristics::record_success_degenerated() {
  _degenerated_cycles_in_a_row++;
  _successful_cycles_in_a_row = 0;
}

void ShenandoahHeuristics::record_success_full() {
  _degenerated_cycles_in_a_row = 0;
  _successful_cycles_in_a_row++;
}

void ShenandoahHeuristics::record_allocation_failure_gc() {
  _bytes_in_cset = 0;
}

void ShenandoahHeuristics::record_explicit_gc() {
  _bytes_in_cset = 0;
}

void ShenandoahHeuristics::record_peak_occupancy() {
  // Nothing to do by default.
}

bool ShenandoahHeuristics::should_process_references() {
  if (ShenandoahRefProcFrequency == 0) return false;
  size_t cycle = ShenandoahHeap::heap()->shenandoahPolicy()->cycle_counter();
  // Process references every Nth GC cycle.
  return cycle % ShenandoahRefProcFrequency == 0;
}

bool ShenandoahHeuristics::should_unload_classes() {
  if (ShenandoahUnloadClassesFrequency == 0) return false;
  size_t cycle = ShenandoahHeap::heap()->shenandoahPolicy()->cycle_counter();
  // Unload classes every Nth GC cycle.
  // This should not happen in the same cycle as process_references to amortize costs.
  // Offsetting by one is enough to break the rendezvous when periods are equal.
  // When periods are not equal, offsetting by one is just as good as any other guess.
  return (cycle + 1) % ShenandoahUnloadClassesFrequency == 0;
}

void ShenandoahHeuristics::initialize() {
  // Nothing to do by default.
}

bool ShenandoahHeuristics::should_start_normal_gc() const {
  double last_time_ms = (os::elapsedTime() - _last_cycle_end) * 1000;
  bool periodic_gc = (last_time_ms > ShenandoahGuaranteedGCInterval);
  if (periodic_gc) {
    log_info(gc)("Trigger: Time since last GC (%.0f ms) is larger than guaranteed interval (" UINTX_FORMAT " ms)",
                  last_time_ms, ShenandoahGuaranteedGCInterval);
  }
  return periodic_gc;
}
