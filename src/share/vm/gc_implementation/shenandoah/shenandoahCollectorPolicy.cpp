/*
 * Copyright (c) 2013, 2017, Red Hat, Inc. and/or its affiliates.
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
#include "gc_implementation/shared/gcTimer.hpp"
#include "gc_implementation/shenandoah/shenandoahCollectionSet.hpp"
#include "gc_implementation/shenandoah/shenandoahFreeSet.hpp"
#include "gc_implementation/shenandoah/shenandoahCollectorPolicy.hpp"
#include "gc_implementation/shenandoah/shenandoahHeap.inline.hpp"
#include "gc_implementation/shenandoah/shenandoahLogging.hpp"
#include "utilities/quickSort.hpp"

#define SHENANDOAH_ERGO_DISABLE_FLAG(name)                                  \
  do {                                                                      \
    if (FLAG_IS_DEFAULT(name) && (name)) {                                  \
      log_info(gc)("Heuristics ergonomically sets -XX:-" #name);            \
      FLAG_SET_DEFAULT(name, false);                                        \
    }                                                                       \
  } while (0)

#define SHENANDOAH_ERGO_ENABLE_FLAG(name)                                   \
  do {                                                                      \
    if (FLAG_IS_DEFAULT(name) && !(name)) {                                 \
      log_info(gc)("Heuristics ergonomically sets -XX:+" #name);            \
      FLAG_SET_DEFAULT(name, true);                                         \
    }                                                                       \
  } while (0)

#define SHENANDOAH_ERGO_OVERRIDE_DEFAULT(name, value)                       \
  do {                                                                      \
    if (FLAG_IS_DEFAULT(name)) {                                            \
      log_info(gc)("Heuristics ergonomically sets -XX:" #name "=" #value);  \
      FLAG_SET_DEFAULT(name, value);                                        \
    }                                                                       \
  } while (0)

class ShenandoahHeuristics : public CHeapObj<mtGC> {
protected:
  bool _update_refs_early;
  bool _update_refs_adaptive;

  typedef struct {
    ShenandoahHeapRegion* _region;
    size_t _garbage;
    uint64_t _seqnum_last_alloc;
  } RegionData;

  static int compare_by_garbage(RegionData a, RegionData b) {
    if (a._garbage > b._garbage)
      return -1;
    else if (a._garbage < b._garbage)
      return 1;
    else return 0;
  }

  RegionData* get_region_data_cache(size_t num) {
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

  RegionData* _region_data;
  size_t _region_data_size;
  
  size_t _bytes_allocated_after_last_gc;

  uint _degenerated_cycles_in_a_row;
  uint _successful_cycles_in_a_row;

  size_t _bytes_in_cset;

  double _last_cycle_end;

public:

  ShenandoahHeuristics();
  virtual ~ShenandoahHeuristics();

  void record_gc_start() {
    // Do nothing.
  }

  void record_gc_end() {
    // Do nothing.
  }

  virtual void record_cycle_start() {
    // Do nothing
  }

  virtual void record_cycle_end() {
    _last_cycle_end = os::elapsedTime();
  }

  virtual void record_phase_time(ShenandoahPhaseTimings::Phase phase, double secs) {
    // Do nothing
  }

  virtual void print_thresholds() {
  }

  virtual bool should_start_normal_gc() const = 0;

  virtual bool should_start_update_refs() {
    return _update_refs_early;
  }

  virtual bool update_refs() const {
    return _update_refs_early;
  }

  virtual bool should_degenerate_cycle() {
    return _degenerated_cycles_in_a_row <= ShenandoahFullGCThreshold;
  }

  virtual void record_success_concurrent() {
    _degenerated_cycles_in_a_row = 0;
    _successful_cycles_in_a_row++;
  }

  virtual void record_success_degenerated() {
    _degenerated_cycles_in_a_row++;
    _successful_cycles_in_a_row = 0;
  }

  virtual void record_success_full() {
    _degenerated_cycles_in_a_row = 0;
    _successful_cycles_in_a_row++;
  }

  virtual void record_allocation_failure_gc() {
    _bytes_in_cset = 0;
  }

  virtual void record_explicit_gc() {
    _bytes_in_cset = 0;
  }

  virtual void record_peak_occupancy() {
  }

  virtual void start_choose_collection_set() {
  }
  virtual void end_choose_collection_set() {
  }

  virtual void choose_collection_set(ShenandoahCollectionSet* collection_set, int* connections=NULL);

  virtual bool should_process_references() {
    if (ShenandoahRefProcFrequency == 0) return false;
    size_t cycle = ShenandoahHeap::heap()->shenandoahPolicy()->cycle_counter();
    // Process references every Nth GC cycle.
    return cycle % ShenandoahRefProcFrequency == 0;
  }

  virtual bool should_unload_classes() {
    if (ShenandoahUnloadClassesFrequency == 0) return false;
    size_t cycle = ShenandoahHeap::heap()->shenandoahPolicy()->cycle_counter();
    // Unload classes every Nth GC cycle.
    // This should not happen in the same cycle as process_references to amortize costs.
    // Offsetting by one is enough to break the rendezvous when periods are equal.
    // When periods are not equal, offsetting by one is just as good as any other guess.
    return (cycle + 1) % ShenandoahUnloadClassesFrequency == 0;
  }

  virtual const char* name() = 0;
  virtual bool is_diagnostic() = 0;
  virtual bool is_experimental() = 0;

protected:
  virtual void choose_collection_set_from_regiondata(ShenandoahCollectionSet* set,
                                                     RegionData* data, size_t data_size,
                                                     size_t free) = 0;
};

ShenandoahHeuristics::ShenandoahHeuristics() :
  _bytes_in_cset(0),
  _degenerated_cycles_in_a_row(0),
  _successful_cycles_in_a_row(0),
  _region_data(NULL),
  _region_data_size(0),
  _update_refs_early(false),
  _update_refs_adaptive(false),
  _last_cycle_end(0)
{
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
}

ShenandoahHeuristics::~ShenandoahHeuristics() {
  if (_region_data != NULL) {
    FREE_C_HEAP_ARRAY(RegionGarbage, _region_data, mtGC);
  }
}

void ShenandoahHeuristics::choose_collection_set(ShenandoahCollectionSet* collection_set, int* connections) {
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
        bool bm_live = heap->is_marked_complete(oop(region->bottom() + BrooksPointer::word_size()));
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
    log_info(gc, ergo)("Collectable Garbage: "SIZE_FORMAT"M ("SIZE_FORMAT"%% of total), "SIZE_FORMAT"M CSet, "SIZE_FORMAT" CSet regions",
                       collection_set->garbage() / M, cset_percent, collection_set->live_data() / M, collection_set->count());
  }
  end_choose_collection_set();

  log_info(gc, ergo)("Immediate Garbage: "SIZE_FORMAT"M ("SIZE_FORMAT"%% of total), "SIZE_FORMAT" regions",
                     immediate_garbage / M, immediate_percent, immediate_regions);
}

void ShenandoahCollectorPolicy::record_gc_start() {
  _heuristics->record_gc_start();
}

void ShenandoahCollectorPolicy::record_gc_end() {
  _heuristics->record_gc_end();
}

class ShenandoahPassiveHeuristics : public ShenandoahHeuristics {
public:
  ShenandoahPassiveHeuristics() : ShenandoahHeuristics() {
    // Do not allow concurrent cycles.
    FLAG_SET_DEFAULT(ExplicitGCInvokesConcurrent, false);

    // Passive runs with max speed, reacts on allocation failure.
    FLAG_SET_DEFAULT(ShenandoahPacing, false);

    // Disable known barriers by default.
    SHENANDOAH_ERGO_DISABLE_FLAG(ShenandoahSATBBarrier);
    SHENANDOAH_ERGO_DISABLE_FLAG(ShenandoahWriteBarrier);
    SHENANDOAH_ERGO_DISABLE_FLAG(ShenandoahReadBarrier);
    SHENANDOAH_ERGO_DISABLE_FLAG(ShenandoahCASBarrier);
    SHENANDOAH_ERGO_DISABLE_FLAG(ShenandoahAcmpBarrier);
    SHENANDOAH_ERGO_DISABLE_FLAG(ShenandoahCloneBarrier);
  }

  virtual void choose_collection_set_from_regiondata(ShenandoahCollectionSet* cset,
                                                     RegionData* data, size_t size,
                                                     size_t free) {
    for (size_t idx = 0; idx < size; idx++) {
      ShenandoahHeapRegion* r = data[idx]._region;
      if (r->garbage() > 0) {
        cset->add_region(r);
      }
    }
  }

  virtual bool should_start_normal_gc() const {
    // Never do concurrent GCs.
    return false;
  }

  virtual bool should_process_references() {
    if (ShenandoahRefProcFrequency == 0) return false;
    // Always process references.
    return true;
  }

  virtual bool should_unload_classes() {
    if (ShenandoahUnloadClassesFrequency == 0) return false;
    // Always unload classes.
    return true;
  }

  virtual bool should_degenerate_cycle() {
    // Always fail to Full GC
    return false;
  }

  virtual const char* name() {
    return "passive";
  }

  virtual bool is_diagnostic() {
    return true;
  }

  virtual bool is_experimental() {
    return false;
  }
};

class ShenandoahAggressiveHeuristics : public ShenandoahHeuristics {
public:
  ShenandoahAggressiveHeuristics() : ShenandoahHeuristics() {
    // Do not shortcut evacuation
    SHENANDOAH_ERGO_OVERRIDE_DEFAULT(ShenandoahImmediateThreshold, 100);

    // Aggressive runs with max speed for allocation, to capture races against mutator
    SHENANDOAH_ERGO_DISABLE_FLAG(ShenandoahPacing);
  }

  virtual void choose_collection_set_from_regiondata(ShenandoahCollectionSet* cset,
                                                     RegionData* data, size_t size,
                                                     size_t free) {
    for (size_t idx = 0; idx < size; idx++) {
      ShenandoahHeapRegion* r = data[idx]._region;
      if (r->garbage() > 0) {
        cset->add_region(r);
      }
    }
  }

  virtual bool should_start_normal_gc() const {
    return true;
  }

  virtual bool should_process_references() {
    if (ShenandoahRefProcFrequency == 0) return false;
    // Randomly process refs with 50% chance.
    return (os::random() & 1) == 1;
  }

  virtual bool should_unload_classes() {
    if (ShenandoahUnloadClassesFrequency == 0) return false;
    // Randomly unload classes with 50% chance.
    return (os::random() & 1) == 1;
  }

  virtual const char* name() {
    return "aggressive";
  }

  virtual bool is_diagnostic() {
    return true;
  }

  virtual bool is_experimental() {
    return false;
  }
};

class ShenandoahStaticHeuristics : public ShenandoahHeuristics {
public:
  ShenandoahStaticHeuristics() : ShenandoahHeuristics() {
    // Static heuristics may degrade to continuous if live data is larger
    // than free threshold. ShenandoahAllocationThreshold is supposed to break this,
    // but it only works if it is non-zero.
    SHENANDOAH_ERGO_OVERRIDE_DEFAULT(ShenandoahImmediateThreshold, 1);
  }

  void print_thresholds() {
    log_info(gc, init)("Shenandoah heuristics thresholds: allocation "SIZE_FORMAT", free "SIZE_FORMAT", garbage "SIZE_FORMAT,
                       ShenandoahAllocationThreshold,
                       ShenandoahFreeThreshold,
                       ShenandoahGarbageThreshold);
  }

  virtual ~ShenandoahStaticHeuristics() {}

  virtual bool should_start_normal_gc() const {
    ShenandoahHeap* heap = ShenandoahHeap::heap();

    size_t capacity = heap->capacity();
    size_t available = heap->free_set()->available();
    size_t threshold_available = (capacity * ShenandoahFreeThreshold) / 100;
    size_t threshold_bytes_allocated = heap->capacity() * ShenandoahAllocationThreshold / 100;
    size_t bytes_allocated = heap->bytes_allocated_since_gc_start();

    double last_time_ms = (os::elapsedTime() - _last_cycle_end) * 1000;
    bool periodic_gc = (last_time_ms > ShenandoahGuaranteedGCInterval);

    if (available < threshold_available &&
            bytes_allocated > threshold_bytes_allocated) {
      // Need to check that an appropriate number of regions have
      // been allocated since last concurrent mark too.
      log_info(gc,ergo)("Concurrent marking triggered. Free: " SIZE_FORMAT "M, Free Threshold: " SIZE_FORMAT
                                "M; Allocated: " SIZE_FORMAT "M, Alloc Threshold: " SIZE_FORMAT "M",
                        available / M, threshold_available / M, bytes_allocated / M, threshold_bytes_allocated / M);
      return true;
    } else if (periodic_gc) {
      log_info(gc,ergo)("Periodic GC triggered. Time since last GC: %.0f ms, Guaranteed Interval: " UINTX_FORMAT " ms",
                        last_time_ms, ShenandoahGuaranteedGCInterval);
      return true;
    }

    return false;
  }

  virtual void choose_collection_set_from_regiondata(ShenandoahCollectionSet* cset,
                                                     RegionData* data, size_t size,
                                                     size_t free) {
    size_t threshold = ShenandoahHeapRegion::region_size_bytes() * ShenandoahGarbageThreshold / 100;

    for (size_t idx = 0; idx < size; idx++) {
      ShenandoahHeapRegion* r = data[idx]._region;
      if (r->garbage() > threshold) {
        cset->add_region(r);
      }
    }
  }

  virtual const char* name() {
    return "dynamic";
  }

  virtual bool is_diagnostic() {
    return false;
  }

  virtual bool is_experimental() {
    return false;
  }
};

class ShenandoahCompactHeuristics : public ShenandoahHeuristics {
public:
  ShenandoahCompactHeuristics() : ShenandoahHeuristics() {
    SHENANDOAH_ERGO_ENABLE_FLAG(ShenandoahUncommit);
    SHENANDOAH_ERGO_OVERRIDE_DEFAULT(ShenandoahAllocationThreshold,  10);
    SHENANDOAH_ERGO_OVERRIDE_DEFAULT(ShenandoahImmediateThreshold,   100);
    SHENANDOAH_ERGO_OVERRIDE_DEFAULT(ShenandoahUncommitDelay,        5000);
    SHENANDOAH_ERGO_OVERRIDE_DEFAULT(ShenandoahGuaranteedGCInterval, 30000);
    SHENANDOAH_ERGO_OVERRIDE_DEFAULT(ShenandoahGarbageThreshold,     20);
  }

  virtual bool should_start_normal_gc() const {
    ShenandoahHeap* heap = ShenandoahHeap::heap();

    size_t available = heap->free_set()->available();
    double last_time_ms = (os::elapsedTime() - _last_cycle_end) * 1000;
    bool periodic_gc = (last_time_ms > ShenandoahGuaranteedGCInterval);
    size_t bytes_allocated = heap->bytes_allocated_since_gc_start();
    size_t threshold_bytes_allocated = heap->capacity() * ShenandoahAllocationThreshold / 100;

    if (available < threshold_bytes_allocated || bytes_allocated > threshold_bytes_allocated) {
      log_info(gc,ergo)("Concurrent marking triggered. Free: " SIZE_FORMAT "M, Allocated: " SIZE_FORMAT "M, Alloc Threshold: " SIZE_FORMAT "M",
                         available / M, bytes_allocated / M, threshold_bytes_allocated / M);
      return true;
    } else if (periodic_gc) {
      log_info(gc,ergo)("Periodic GC triggered. Time since last GC: %.0f ms, Guaranteed Interval: " UINTX_FORMAT " ms",
                        last_time_ms, ShenandoahGuaranteedGCInterval);
      return true;
    }

    return false;
  }

  virtual void choose_collection_set_from_regiondata(ShenandoahCollectionSet* cset,
                                                     RegionData* data, size_t size,
                                                     size_t actual_free) {

    // Do not select too large CSet that would overflow the available free space
    size_t max_cset = actual_free * 3 / 4;

    log_info(gc, ergo)("CSet Selection. Actual Free: " SIZE_FORMAT "M, Max CSet: " SIZE_FORMAT "M",
                       actual_free / M, max_cset / M);

    size_t threshold = ShenandoahHeapRegion::region_size_bytes() * ShenandoahGarbageThreshold / 100;

    size_t live_cset = 0;
    for (size_t idx = 0; idx < size; idx++) {
      ShenandoahHeapRegion* r = data[idx]._region;
      size_t new_cset = live_cset + r->get_live_data_bytes();
      if (new_cset < max_cset && r->garbage() > threshold) {
        live_cset = new_cset;
        cset->add_region(r);
      }
    }
  }

  virtual const char* name() {
    return "compact";
  }

  virtual bool is_diagnostic() {
    return false;
  }

  virtual bool is_experimental() {
    return false;
  }
};

class ShenandoahAdaptiveHeuristics : public ShenandoahHeuristics {
private:
  uintx _free_threshold;
  size_t _peak_occupancy;
  TruncatedSeq* _cycle_gap_history;
  TruncatedSeq* _conc_mark_duration_history;
  TruncatedSeq* _conc_uprefs_duration_history;
public:
  ShenandoahAdaptiveHeuristics() :
    ShenandoahHeuristics(),
    _free_threshold(ShenandoahInitFreeThreshold),
    _peak_occupancy(0),
    _conc_mark_duration_history(new TruncatedSeq(5)),
    _conc_uprefs_duration_history(new TruncatedSeq(5)),
    _cycle_gap_history(new TruncatedSeq(5)) {
  }

  virtual ~ShenandoahAdaptiveHeuristics() {}

  virtual void choose_collection_set_from_regiondata(ShenandoahCollectionSet* cset,
                                                     RegionData* data, size_t size,
                                                     size_t actual_free) {
    size_t garbage_threshold = ShenandoahHeapRegion::region_size_bytes() * ShenandoahGarbageThreshold / 100;

    // The logic for cset selection in adaptive is as follows:
    //
    //   1. We cannot get cset larger than available free space. Otherwise we guarantee OOME
    //      during evacuation, and thus guarantee full GC. In practice, we also want to let
    //      application to allocate something. This is why we limit CSet to some fraction of
    //      available space. In non-overloaded heap, max_cset would contain all plausible candidates
    //      over garbage threshold.
    //
    //   2. We should not get cset too low so that free threshold would not be met right
    //      after the cycle. Otherwise we get back-to-back cycles for no reason if heap is
    //      too fragmented. In non-overloaded non-fragmented heap min_cset would be around zero.
    //
    // Therefore, we start by sorting the regions by garbage. Then we unconditionally add the best candidates
    // before we meet min_cset. Then we add all candidates that fit with a garbage threshold before
    // we hit max_cset. When max_cset is hit, we terminate the cset selection. Note that in this scheme,
    // ShenandoahGarbageThreshold is the soft threshold which would be ignored until min_cset is hit.

    size_t free_target = MIN2<size_t>(_free_threshold + MaxNormalStep, 100) * ShenandoahHeap::heap()->capacity() / 100;
    size_t min_cset = free_target > actual_free ? (free_target - actual_free) : 0;
    size_t max_cset = actual_free * 3 / 4;
    min_cset = MIN2(min_cset, max_cset);

    log_info(gc, ergo)("Adaptive CSet Selection. Target Free: " SIZE_FORMAT "M, Actual Free: "
                               SIZE_FORMAT "M, Target CSet: [" SIZE_FORMAT "M, " SIZE_FORMAT "M]",
                       free_target / M, actual_free / M, min_cset / M, max_cset / M);

    // Better select garbage-first regions
    QuickSort::sort<RegionData>(data, (int)size, compare_by_garbage, false);

    size_t live_cset = 0;
    _bytes_in_cset = 0;
    for (size_t idx = 0; idx < size; idx++) {
      ShenandoahHeapRegion* r = data[idx]._region;

      size_t new_cset = live_cset + r->get_live_data_bytes();

      if (new_cset < min_cset) {
        cset->add_region(r);
        _bytes_in_cset += r->used();
        live_cset = new_cset;
      } else if (new_cset <= max_cset) {
        if (r->garbage() > garbage_threshold) {
          cset->add_region(r);
          _bytes_in_cset += r->used();
          live_cset = new_cset;
        }
      } else {
        break;
      }
    }
  }

  static const intx MaxNormalStep = 5;      // max step towards goal under normal conditions
  static const intx DegeneratedGC_Hit = 10; // how much to step on degenerated GC
  static const intx AllocFailure_Hit = 20;  // how much to step on allocation failure full GC
  static const intx UserRequested_Hit = 0;  // how much to step on user requested full GC

  void handle_cycle_success() {
    ShenandoahHeap* heap = ShenandoahHeap::heap();
    size_t capacity = heap->capacity();

    size_t current_threshold = (capacity - _peak_occupancy) * 100 / capacity;
    size_t min_threshold = ShenandoahMinFreeThreshold;
    intx step = min_threshold - current_threshold;
    step = MAX2(step, (intx) -MaxNormalStep);
    step = MIN2(step, (intx) MaxNormalStep);

    log_info(gc, ergo)("Capacity: " SIZE_FORMAT "M, Peak Occupancy: " SIZE_FORMAT
                              "M, Lowest Free: " SIZE_FORMAT "M, Free Threshold: " UINTX_FORMAT "M",
                       capacity / M, _peak_occupancy / M,
                       (capacity - _peak_occupancy) / M, ShenandoahMinFreeThreshold * capacity / 100 / M);

    if (step > 0) {
      // Pessimize
      adjust_free_threshold(step);
    } else if (step < 0) {
      // Optimize, if enough happy cycles happened
      if (_successful_cycles_in_a_row > ShenandoahHappyCyclesThreshold &&
          _free_threshold > 0) {
        adjust_free_threshold(step);
        _successful_cycles_in_a_row = 0;
      }
    } else {
      // do nothing
    }
    _peak_occupancy = 0;
  }

  void record_cycle_start() {
    ShenandoahHeuristics::record_cycle_start();
    double last_cycle_gap = (os::elapsedTime() - _last_cycle_end);
    _cycle_gap_history->add(last_cycle_gap);
  }

  virtual void record_phase_time(ShenandoahPhaseTimings::Phase phase, double secs) {
    if (phase == ShenandoahPhaseTimings::conc_mark) {
      _conc_mark_duration_history->add(secs);
    } else if (phase == ShenandoahPhaseTimings::conc_update_refs) {
      _conc_uprefs_duration_history->add(secs);
    } // Else ignore
  }

  void adjust_free_threshold(intx adj) {
    intx new_value = adj + _free_threshold;
    uintx new_threshold = (uintx)MAX2<intx>(new_value, 0);
    new_threshold = MAX2(new_threshold, ShenandoahMinFreeThreshold);
    new_threshold = MIN2(new_threshold, ShenandoahMaxFreeThreshold);
    if (new_threshold != _free_threshold) {
      _free_threshold = new_threshold;
      log_info(gc,ergo)("Adjusting free threshold to: " UINTX_FORMAT "%% (" SIZE_FORMAT "M)",
                        _free_threshold, _free_threshold * ShenandoahHeap::heap()->capacity() / 100 / M);
    }
  }

  virtual void record_success_concurrent() {
    ShenandoahHeuristics::record_success_concurrent();
    handle_cycle_success();
  }

  virtual void record_success_degenerated() {
    ShenandoahHeuristics::record_success_degenerated();
    adjust_free_threshold(DegeneratedGC_Hit);
  }

  virtual void record_success_full() {
    ShenandoahHeuristics::record_success_full();
    adjust_free_threshold(AllocFailure_Hit);
  }

  virtual void record_explicit_gc() {
    ShenandoahHeuristics::record_explicit_gc();
    adjust_free_threshold(UserRequested_Hit);
  }

  virtual void record_peak_occupancy() {
    _peak_occupancy = MAX2(_peak_occupancy, ShenandoahHeap::heap()->used());
  }

  virtual bool should_start_normal_gc() const {
    ShenandoahHeap* heap = ShenandoahHeap::heap();
    size_t capacity = heap->capacity();
    size_t available = heap->free_set()->available();

    double last_time_ms = (os::elapsedTime() - _last_cycle_end) * 1000;
    bool periodic_gc = (last_time_ms > ShenandoahGuaranteedGCInterval);
    size_t threshold_available = (capacity * _free_threshold) / 100;
    size_t bytes_allocated = heap->bytes_allocated_since_gc_start();
    size_t threshold_bytes_allocated = heap->capacity() * ShenandoahAllocationThreshold / 100;

    if (available < threshold_available &&
            bytes_allocated > threshold_bytes_allocated) {
      log_info(gc,ergo)("Concurrent marking triggered. Free: " SIZE_FORMAT "M, Free Threshold: " SIZE_FORMAT
                                "M; Allocated: " SIZE_FORMAT "M, Alloc Threshold: " SIZE_FORMAT "M",
                        available / M, threshold_available / M, bytes_allocated / M, threshold_bytes_allocated / M);
      // Need to check that an appropriate number of regions have
      // been allocated since last concurrent mark too.
      return true;
    } else if (periodic_gc) {
      log_info(gc,ergo)("Periodic GC triggered. Time since last GC: %.0f ms, Guaranteed Interval: " UINTX_FORMAT " ms",
          last_time_ms, ShenandoahGuaranteedGCInterval);
      return true;
    }

    return false;
  }

  virtual bool should_start_update_refs() {
    if (! _update_refs_adaptive) {
      return _update_refs_early;
    }

    double cycle_gap_avg = _cycle_gap_history->avg();
    double conc_mark_avg = _conc_mark_duration_history->avg();
    double conc_uprefs_avg = _conc_uprefs_duration_history->avg();

    if (_update_refs_early) {
      double threshold = ShenandoahMergeUpdateRefsMinGap / 100.0;
      if (conc_mark_avg + conc_uprefs_avg > cycle_gap_avg * threshold) {
        _update_refs_early = false;
      }
    } else {
      double threshold = ShenandoahMergeUpdateRefsMaxGap / 100.0;
      if (conc_mark_avg + conc_uprefs_avg < cycle_gap_avg * threshold) {
        _update_refs_early = true;
      }
    }
    return _update_refs_early;
  }

  virtual const char* name() {
    return "adaptive";
  }

  virtual bool is_diagnostic() {
    return false;
  }

  virtual bool is_experimental() {
    return false;
  }
};

ShenandoahCollectorPolicy::ShenandoahCollectorPolicy() :
  _cycle_counter(0),
  _success_concurrent_gcs(0),
  _success_partial_gcs(0),
  _success_degenerated_gcs(0),
  _success_full_gcs(0),
  _explicit_concurrent(0),
  _explicit_full(0),
  _alloc_failure_degenerated(0),
  _alloc_failure_full(0),
  _alloc_failure_degenerated_upgrade_to_full(0)
{
  Copy::zero_to_bytes(_degen_points, sizeof(size_t) * ShenandoahHeap::_DEGENERATED_LIMIT);

  ShenandoahHeapRegion::setup_heap_region_size(initial_heap_byte_size(), max_heap_byte_size());

  initialize_all();

  _tracer = new (ResourceObj::C_HEAP, mtGC) ShenandoahTracer();

  if (ShenandoahGCHeuristics != NULL) {
    if (strcmp(ShenandoahGCHeuristics, "aggressive") == 0) {
      _heuristics = new ShenandoahAggressiveHeuristics();
    } else if (strcmp(ShenandoahGCHeuristics, "static") == 0) {
      _heuristics = new ShenandoahStaticHeuristics();
    } else if (strcmp(ShenandoahGCHeuristics, "adaptive") == 0) {
      _heuristics = new ShenandoahAdaptiveHeuristics();
    } else if (strcmp(ShenandoahGCHeuristics, "passive") == 0) {
      _heuristics = new ShenandoahPassiveHeuristics();
    } else if (strcmp(ShenandoahGCHeuristics, "compact") == 0) {
      _heuristics = new ShenandoahCompactHeuristics();
    } else {
      vm_exit_during_initialization("Unknown -XX:ShenandoahGCHeuristics option");
    }

    if (_heuristics->is_diagnostic() && !UnlockDiagnosticVMOptions) {
      vm_exit_during_initialization(
              err_msg("Heuristics \"%s\" is diagnostic, and must be enabled via -XX:+UnlockDiagnosticVMOptions.",
                      _heuristics->name()));
    }
    if (_heuristics->is_experimental() && !UnlockExperimentalVMOptions) {
      vm_exit_during_initialization(
              err_msg("Heuristics \"%s\" is experimental, and must be enabled via -XX:+UnlockExperimentalVMOptions.",
                      _heuristics->name()));
    }
    log_info(gc, init)("Shenandoah heuristics: %s",
                       _heuristics->name());
    _heuristics->print_thresholds();
  } else {
      ShouldNotReachHere();
  }
}

ShenandoahCollectorPolicy* ShenandoahCollectorPolicy::as_pgc_policy() {
  return this;
}

BarrierSet::Name ShenandoahCollectorPolicy::barrier_set_name() {
  return BarrierSet::ShenandoahBarrierSet;
}

HeapWord* ShenandoahCollectorPolicy::mem_allocate_work(size_t size,
                                                       bool is_tlab,
                                                       bool* gc_overhead_limit_was_exceeded) {
  guarantee(false, "Not using this policy feature yet.");
  return NULL;
}

HeapWord* ShenandoahCollectorPolicy::satisfy_failed_allocation(size_t size, bool is_tlab) {
  guarantee(false, "Not using this policy feature yet.");
  return NULL;
}

void ShenandoahCollectorPolicy::initialize_alignments() {

  // This is expected by our algorithm for ShenandoahHeap::heap_region_containing().
  _space_alignment = ShenandoahHeapRegion::region_size_bytes();
  _heap_alignment = ShenandoahHeapRegion::region_size_bytes();
}

void ShenandoahCollectorPolicy::post_heap_initialize() {
  // Nothing to do here (yet).
}

void ShenandoahCollectorPolicy::record_explicit_to_concurrent() {
  _heuristics->record_explicit_gc();
  _explicit_concurrent++;
}

void ShenandoahCollectorPolicy::record_explicit_to_full() {
  _heuristics->record_explicit_gc();
  _explicit_full++;
}

void ShenandoahCollectorPolicy::record_alloc_failure_to_full() {
  _heuristics->record_allocation_failure_gc();
  _alloc_failure_full++;
}

void ShenandoahCollectorPolicy::record_alloc_failure_to_degenerated(ShenandoahHeap::ShenandoahDegenPoint point) {
  assert(point < ShenandoahHeap::_DEGENERATED_LIMIT, "sanity");
  _heuristics->record_allocation_failure_gc();
  _alloc_failure_degenerated++;
  _degen_points[point]++;
}

void ShenandoahCollectorPolicy::record_degenerated_upgrade_to_full() {
  _alloc_failure_degenerated_upgrade_to_full++;
}

void ShenandoahCollectorPolicy::record_success_concurrent() {
  _heuristics->record_success_concurrent();
  _success_concurrent_gcs++;
}

void ShenandoahCollectorPolicy::record_success_partial() {
  _success_partial_gcs++;
}

void ShenandoahCollectorPolicy::record_success_degenerated() {
  _heuristics->record_success_degenerated();
  _success_degenerated_gcs++;
}

void ShenandoahCollectorPolicy::record_success_full() {
  _heuristics->record_success_full();
  _success_full_gcs++;
}

bool ShenandoahCollectorPolicy::should_start_normal_gc() {
  return _heuristics->should_start_normal_gc();
}

bool ShenandoahCollectorPolicy::should_degenerate_cycle() {
  return _heuristics->should_degenerate_cycle();
}

bool ShenandoahCollectorPolicy::update_refs() {
  return _heuristics->update_refs();
}

bool ShenandoahCollectorPolicy::should_start_update_refs() {
  return _heuristics->should_start_update_refs();
}

void ShenandoahCollectorPolicy::record_peak_occupancy() {
  _heuristics->record_peak_occupancy();
}

void ShenandoahCollectorPolicy::choose_collection_set(ShenandoahCollectionSet* collection_set, int* connections) {
  _heuristics->choose_collection_set(collection_set, connections);
}

bool ShenandoahCollectorPolicy::should_process_references() {
  return _heuristics->should_process_references();
}

bool ShenandoahCollectorPolicy::should_unload_classes() {
  return _heuristics->should_unload_classes();
}

size_t ShenandoahCollectorPolicy::cycle_counter() const {
  return _cycle_counter;
}

void ShenandoahCollectorPolicy::record_phase_time(ShenandoahPhaseTimings::Phase phase, double secs) {
  _heuristics->record_phase_time(phase, secs);
}

void ShenandoahCollectorPolicy::record_cycle_start() {
  _cycle_counter++;
  _heuristics->record_cycle_start();
}

void ShenandoahCollectorPolicy::record_cycle_end() {
  _heuristics->record_cycle_end();
}

void ShenandoahCollectorPolicy::record_shutdown() {
  _in_shutdown.set();
}

bool ShenandoahCollectorPolicy::is_at_shutdown() {
  return _in_shutdown.is_set();
}

void ShenandoahCollectorPolicy::print_gc_stats(outputStream* out) const {
  out->print_cr("Under allocation pressure, concurrent cycles may cancel, and either continue cycle");
  out->print_cr("under stop-the-world pause or result in stop-the-world Full GC. Increase heap size,");
  out->print_cr("tune GC heuristics, set more aggressive pacing delay, or lower allocation rate");
  out->print_cr("to avoid Degenerated and Full GC cycles.");
  out->cr();

  out->print_cr(SIZE_FORMAT_W(5) " successful partial concurrent GCs", _success_partial_gcs);
  out->cr();

  out->print_cr(SIZE_FORMAT_W(5) " successful concurrent GCs",         _success_concurrent_gcs);
  out->print_cr("  " SIZE_FORMAT_W(5) " invoked explicitly",           _explicit_concurrent);
  out->cr();

  out->print_cr(SIZE_FORMAT_W(5) " Degenerated GCs",                   _success_degenerated_gcs);
  out->print_cr("  " SIZE_FORMAT_W(5) " caused by allocation failure", _alloc_failure_degenerated);
  for (int c = 0; c < ShenandoahHeap::_DEGENERATED_LIMIT; c++) {
    if (_degen_points[c] > 0) {
      const char* desc = ShenandoahHeap::degen_point_to_string((ShenandoahHeap::ShenandoahDegenPoint)c);
      out->print_cr("    " SIZE_FORMAT_W(5) " happened at %s",         _degen_points[c], desc);
    }
  }
  out->print_cr("  " SIZE_FORMAT_W(5) " upgraded to Full GC",          _alloc_failure_degenerated_upgrade_to_full);
  out->cr();

  out->print_cr(SIZE_FORMAT_W(5) " Full GCs",                          _success_full_gcs + _alloc_failure_degenerated_upgrade_to_full);
  out->print_cr("  " SIZE_FORMAT_W(5) " invoked explicitly",           _explicit_full);
  out->print_cr("  " SIZE_FORMAT_W(5) " caused by allocation failure", _alloc_failure_full);
  out->print_cr("  " SIZE_FORMAT_W(5) " upgraded from Degenerated GC", _alloc_failure_degenerated_upgrade_to_full);
}
