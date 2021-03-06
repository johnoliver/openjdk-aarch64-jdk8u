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

#ifndef SHARE_VM_GC_SHENANDOAH_SHENANDOAHHEAPREGION_HPP
#define SHARE_VM_GC_SHENANDOAH_SHENANDOAHHEAPREGION_HPP

#include "memory/space.hpp"

class VMStructs;

class ShenandoahHeapRegion : public ContiguousSpace {
  friend class VMStructs;
private:
  /*
    Region state is described by a state machine. Transitions are guarded by
    heap lock, which allows changing the state of several regions atomically.
    Region states can be logically aggregated in groups.

      "Empty":
      .................................................................
      .                                                               .
      .                                                               .
      .         Uncommitted  <-------  Committed <------------------------\
      .              |                     |                          .   |
      .              \---------v-----------/                          .   |
      .                        |                                      .   |
      .........................|.......................................   |
                               |                                          |
      "Active":                |                                          |
      .........................|.......................................   |
      .                        |                                      .   |
      .      /-----------------^-------------------\                  .   |
      .      |                                     |                  .   |
      .      v                                     v    "Humongous":  .   |
      .   Regular ---\-----\     ..................O................  .   |
      .     |  ^     |     |     .                 |               .  .   |
      .     |  |     |     |     .                 *---------\     .  .   |
      .     v  |     |     |     .                 v         v     .  .   |
      .    Pinned  Cset    |     .  HStart <--> H/Start   H/Cont   .  .   |
      .       ^    / |     |     .  Pinned         v         |     .  .   |
      .       |   /  |     |     .                 *<--------/     .  .   |
      .       |  v   |     |     .                 |               .  .   |
      .  CsetPinned  |     |     ..................O................  .   |
      .              |     |                       |                  .   |
      .              \-----\---v-------------------/                  .   |
      .                        |                                      .   |
      .........................|.......................................   |
                               |                                          |
      "Trash":                 |                                          |
      .........................|.......................................   |
      .                        |                                      .   |
      .                        v                                      .   |
      .                      Trash ---------------------------------------/
      .                                                               .
      .                                                               .
      .................................................................

    Transition from "Empty" to "Active" is first allocation. It can go from {Uncommitted, Committed}
    to {Regular, "Humongous"}. The allocation may happen in Regular regions too, but not in Humongous.

    Transition from "Active" to "Trash" is reclamation. It can go from CSet during the normal cycle,
    and from {Regular, "Humongous"} for immediate reclamation. The existence of Trash state allows
    quick reclamation without actual cleaning up.

    Transition from "Trash" to "Empty" is recycling. It cleans up the regions and corresponding metadata.
    Can be done asynchronously and in bulk.

    Note how internal transitions disallow logic bugs:
      a) No region can go Empty, unless properly reclaimed/recycled;
      b) No region can go Uncommitted, unless reclaimed/recycled first;
      c) Only Regular regions can go to CSet;
      d) Pinned cannot go Trash, thus it could never be reclaimed until unpinned;
      e) Pinned cannot go CSet, thus it never moves;
      f) Humongous cannot be used for regular allocations;
      g) Humongous cannot go CSet, thus it never moves;
      h) Humongous start can go pinned, and thus can be protected from moves (humongous continuations should
         follow associated humongous starts, not pinnable/movable by themselves);
      i) Empty cannot go Trash, avoiding useless work;
      j) ...
   */

  enum RegionState {
    _empty_uncommitted,       // region is empty and has memory uncommitted
    _empty_committed,         // region is empty and has memory committed
    _regular,                 // region is for regular allocations
    _humongous_start,         // region is the humongous start
    _humongous_cont,          // region is the humongous continuation
    _pinned_humongous_start,  // region is both humongous start and pinned
    _cset,                    // region is in collection set
    _pinned,                  // region is pinned
    _pinned_cset,             // region is pinned and in cset (evac failure path)
    _trash,                   // region contains only trash
  };

  const char* region_state_to_string(RegionState s) const {
    switch (s) {
      case _empty_uncommitted:       return "Empty Uncommitted";
      case _empty_committed:         return "Empty Committed";
      case _regular:                 return "Regular";
      case _humongous_start:         return "Humongous Start";
      case _humongous_cont:          return "Humongous Continuation";
      case _pinned_humongous_start:  return "Humongous Start, Pinned";
      case _cset:                    return "Collection Set";
      case _pinned:                  return "Pinned";
      case _pinned_cset:             return "Collection Set, Pinned";
      case _trash:                   return "Trash";
      default:
        ShouldNotReachHere();
        return "";
    }
  }

  // This method protects from accidental changes in enum order:
  int region_state_to_ordinal(RegionState s) const {
    switch (s) {
      case _empty_uncommitted:      return 0;
      case _empty_committed:        return 1;
      case _regular:                return 2;
      case _humongous_start:        return 3;
      case _humongous_cont:         return 4;
      case _cset:                   return 5;
      case _pinned:                 return 6;
      case _trash:                  return 7;
      case _pinned_cset:            return 8;
      case _pinned_humongous_start: return 9;
      default:
        ShouldNotReachHere();
        return -1;
    }
  }

  void report_illegal_transition(const char* method);

public:
  // Allowed transitions from the outside code:
  void make_regular_allocation();
  void make_regular_bypass();
  void make_humongous_start();
  void make_humongous_cont();
  void make_humongous_start_bypass();
  void make_humongous_cont_bypass();
  void make_pinned();
  void make_unpinned();
  void make_cset();
  void make_trash();
  void make_empty();
  void make_uncommitted();
  void make_committed_bypass();

  // Individual states:
  bool is_empty_uncommitted()      const { return _state == _empty_uncommitted; }
  bool is_empty_committed()        const { return _state == _empty_committed; }
  bool is_regular()                const { return _state == _regular; }
  bool is_humongous_continuation() const { return _state == _humongous_cont; }

  // Participation in logical groups:
  bool is_empty()                  const { return is_empty_committed() || is_empty_uncommitted(); }
  bool is_active()                 const { return !is_empty() && !is_trash(); }
  bool is_trash()                  const { return _state == _trash; }
  bool is_humongous_start()        const { return _state == _humongous_start || _state == _pinned_humongous_start; }
  bool is_humongous()              const { return is_humongous_start() || is_humongous_continuation(); }
  bool is_committed()              const { return !is_empty_uncommitted(); }
  bool is_cset()                   const { return _state == _cset   || _state == _pinned_cset; }
  bool is_pinned()                 const { return _state == _pinned || _state == _pinned_cset || _state == _pinned_humongous_start; }

  // Macro-properties:
  bool is_alloc_allowed()          const { return is_empty() || is_regular() || _state == _pinned; }
  bool is_move_allowed()           const { return is_regular() || _state == _cset || (ShenandoahHumongousMoves && _state == _humongous_start); }

  RegionState state()              const { return _state; }
  int  state_ordinal()             const { return region_state_to_ordinal(_state); }

private:
  static size_t RegionCount;
  static size_t RegionSizeBytes;
  static size_t RegionSizeWords;
  static size_t RegionSizeBytesShift;
  static size_t RegionSizeWordsShift;
  static size_t RegionSizeBytesMask;
  static size_t RegionSizeWordsMask;
  static size_t HumongousThresholdBytes;
  static size_t HumongousThresholdWords;
  static size_t MaxTLABSizeBytes;
  static size_t MaxTLABSizeWords;

private:
  ShenandoahHeap* _heap;
  size_t _region_number;
  volatile jint _live_data;
  MemRegion _reserved;

  size_t _tlab_allocs;
  size_t _gclab_allocs;
  size_t _shared_allocs;

  HeapWord* _new_top;

  size_t _critical_pins;

  RegionState _state;
  double _empty_time;

  ShenandoahPacer* _pacer;

public:
  ShenandoahHeapRegion(ShenandoahHeap* heap, HeapWord* start, size_t size_words, size_t index, bool committed);

  static void setup_sizes(size_t initial_heap_size, size_t max_heap_size);

  double empty_time() {
    return _empty_time;
  }

  inline static size_t required_regions(size_t bytes) {
    return (bytes + ShenandoahHeapRegion::region_size_bytes() - 1) >> ShenandoahHeapRegion::region_size_bytes_shift();
  }

  inline static size_t region_count() {
    return ShenandoahHeapRegion::RegionCount;
  }

  inline static size_t region_size_bytes() {
    return ShenandoahHeapRegion::RegionSizeBytes;
  }

  inline static size_t region_size_words() {
    return ShenandoahHeapRegion::RegionSizeWords;
  }

  inline static size_t region_size_bytes_shift() {
    return ShenandoahHeapRegion::RegionSizeBytesShift;
  }

  inline static size_t region_size_words_shift() {
    return ShenandoahHeapRegion::RegionSizeWordsShift;
  }

  inline static size_t region_size_bytes_mask() {
    return ShenandoahHeapRegion::RegionSizeBytesMask;
  }

  inline static size_t region_size_words_mask() {
    return ShenandoahHeapRegion::RegionSizeWordsMask;
  }

  // Convert to jint with sanity checking
  inline static jint region_size_bytes_jint() {
    assert (ShenandoahHeapRegion::RegionSizeBytes <= (size_t)max_jint, "sanity");
    return (jint)ShenandoahHeapRegion::RegionSizeBytes;
  }

  // Convert to jint with sanity checking
  inline static jint region_size_words_jint() {
    assert (ShenandoahHeapRegion::RegionSizeWords <= (size_t)max_jint, "sanity");
    return (jint)ShenandoahHeapRegion::RegionSizeWords;
  }

  // Convert to jint with sanity checking
  inline static jint region_size_bytes_shift_jint() {
    assert (ShenandoahHeapRegion::RegionSizeBytesShift <= (size_t)max_jint, "sanity");
    return (jint)ShenandoahHeapRegion::RegionSizeBytesShift;
  }

  // Convert to jint with sanity checking
  inline static jint region_size_words_shift_jint() {
    assert (ShenandoahHeapRegion::RegionSizeWordsShift <= (size_t)max_jint, "sanity");
    return (jint)ShenandoahHeapRegion::RegionSizeWordsShift;
  }

  inline static size_t humongous_threshold_bytes() {
    return ShenandoahHeapRegion::HumongousThresholdBytes;
  }

  inline static size_t humongous_threshold_words() {
    return ShenandoahHeapRegion::HumongousThresholdWords;
  }

  inline static size_t max_tlab_size_bytes() {
    return ShenandoahHeapRegion::MaxTLABSizeBytes;
  }

  inline static size_t max_tlab_size_words() {
    return ShenandoahHeapRegion::MaxTLABSizeWords;
  }

  size_t region_number() const;

  // Allocation (return NULL if full)
  inline HeapWord* allocate(size_t word_size, ShenandoahHeap::AllocType type);
  HeapWord* allocate(size_t word_size) {
    // ContiguousSpace wants us to have this method. But it is an error to call this with Shenandoah.
    ShouldNotCallThis();
    return NULL;
  }

  // Roll back the previous allocation of an object with specified size.
  // Returns TRUE when successful, FALSE if not successful or not supported.
  bool rollback_allocation(uint size);

  void clear_live_data();
  void set_live_data(size_t s);

  // Increase live data for newly allocated region
  inline void increase_live_data_alloc_words(size_t s);

  // Increase live data for region scanned with GC
  inline void increase_live_data_gc_words(size_t s);

  bool has_live() const;
  size_t get_live_data_bytes() const;
  size_t get_live_data_words() const;

  void print_on(outputStream* st) const;

  size_t garbage() const;

  void recycle();

  void oop_iterate_skip_unreachable(ExtendedOopClosure* cl, bool skip_unreachable_objects);

  HeapWord* object_iterate_careful(ObjectClosureCareful* cl);

  HeapWord* block_start_const(const void* p) const;

  // Just before GC we need to fill the current region.
  void fill_region();

  bool in_collection_set() const;

  // Find humongous start region that this region belongs to
  ShenandoahHeapRegion* humongous_start_region() const;

  void set_new_top(HeapWord* new_top) { _new_top = new_top; }
  HeapWord* new_top() const { return _new_top; }

  inline void adjust_alloc_metadata(ShenandoahHeap::AllocType type, size_t);
  void reset_alloc_metadata_to_shared();
  void reset_alloc_metadata();
  size_t get_shared_allocs() const;
  size_t get_tlab_allocs() const;
  size_t get_gclab_allocs() const;

private:
  void do_commit();
  void do_uncommit();

  inline void internal_increase_live_data(size_t s);
};

#endif // SHARE_VM_GC_SHENANDOAH_SHENANDOAHHEAPREGION_HPP
