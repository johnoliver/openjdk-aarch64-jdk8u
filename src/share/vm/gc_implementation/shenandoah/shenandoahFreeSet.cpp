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

#include "precompiled.hpp"
#include "gc_implementation/shenandoah/shenandoahFreeSet.hpp"
#include "gc_implementation/shenandoah/shenandoahHeap.inline.hpp"

ShenandoahFreeSet::ShenandoahFreeSet(ShenandoahHeap* heap,size_t max_regions) :
        _heap(heap),
        _mutator_free_bitmap(max_regions, /* in_resource_area = */ false),
        _collector_free_bitmap(max_regions, /* in_resource_area = */ false),
        _max(max_regions)
{
  clear_internal();
}

void ShenandoahFreeSet::increase_used(size_t num_bytes) {
  assert_heaplock_owned_by_current_thread();
  _used += num_bytes;

  assert(_used <= _capacity, err_msg("must not use more than we have: used: "SIZE_FORMAT
                                     ", capacity: "SIZE_FORMAT", num_bytes: "SIZE_FORMAT,
                                     _used, _capacity, num_bytes));
}

bool ShenandoahFreeSet::is_mutator_free(size_t idx) const {
  assert (idx < _max,
          err_msg("index is sane: " SIZE_FORMAT " < " SIZE_FORMAT " (left: " SIZE_FORMAT ", right: " SIZE_FORMAT ")",
                  idx, _max, _mutator_leftmost, _mutator_rightmost));
  return _mutator_free_bitmap.at(idx);
}

bool ShenandoahFreeSet::is_collector_free(size_t idx) const {
  assert (idx < _max,
          err_msg("index is sane: " SIZE_FORMAT " < " SIZE_FORMAT " (left: " SIZE_FORMAT ", right: " SIZE_FORMAT ")",
                  idx, _max, _collector_leftmost, _collector_rightmost));
  return _collector_free_bitmap.at(idx);
}

HeapWord* ShenandoahFreeSet::allocate_single(ShenandoahHeap::ShenandoahAllocationRequest& req, bool& in_new_region) {
  // Scan the bitmap looking for a first fit.
  //
  // Leftmost and rightmost bounds provide enough caching to walk bitmap efficiently. Normally,
  // we would find the region to allocate at right away.
  //
  // Allocations are biased: new application allocs go to beginning of the heap, and GC allocs
  // go to the end. This makes application allocation faster, because we would clear lots
  // of regions from the beginning most of the time.
  //
  // Free set maintains mutator and collector views, and normally they allocate in their views only,
  // unless we special cases for stealing and mixed allocations.

  switch (req.type()) {
    case ShenandoahHeap::_alloc_tlab:
    case ShenandoahHeap::_alloc_shared: {

      // Fast-path: try to allocate in the mutator view first:
      for (size_t idx = _mutator_leftmost; idx <= _mutator_rightmost; idx++) {
        if (is_mutator_free(idx)) {
          HeapWord* result = try_allocate_in(_heap->get_region(idx), req, in_new_region);
          if (result != NULL) {
            return result;
          }
        }
      }

      // Recovery: try to steal the empty region from the collector view:
      for (size_t idx = _collector_leftmost; idx <= _collector_rightmost; idx++) {
        if (is_collector_free(idx)) {
          ShenandoahHeapRegion* r = _heap->get_region(idx);
          if (is_empty_or_trash(r)) {
            flip_to_mutator(idx);
            HeapWord *result = try_allocate_in(r, req, in_new_region);
            if (result != NULL) {
              return result;
            }
          }
        }
      }

      // Recovery: try to mix the allocation into the collector view:
      if (ShenandoahAllowMixedAllocs) {
        for (size_t idx = _collector_leftmost; idx <= _collector_rightmost; idx++) {
          if (is_collector_free(idx)) {
            HeapWord* result = try_allocate_in(_heap->get_region(idx), req, in_new_region);
            if (result != NULL) {
              return result;
            }
          }
        }
      }

      break;
    }
    case ShenandoahHeap::_alloc_gclab:
    case ShenandoahHeap::_alloc_shared_gc: {
      // size_t is unsigned, need to dodge underflow when _leftmost = 0

      // Fast-path: try to allocate in the collector view first:
      for (size_t c = _collector_rightmost + 1; c > _collector_leftmost; c--) {
        size_t idx = c - 1;
        if (is_collector_free(idx)) {
          HeapWord* result = try_allocate_in(_heap->get_region(idx), req, in_new_region);
          if (result != NULL) {
            return result;
          }
        }
      }

      // Recovery: try to steal the empty region from the mutator view:
      for (size_t c = _mutator_rightmost + 1; c > _mutator_leftmost; c--) {
        size_t idx = c - 1;
        if (is_mutator_free(idx)) {
          ShenandoahHeapRegion* r = _heap->get_region(idx);
          if (is_empty_or_trash(r)) {
            flip_to_gc(idx);
            HeapWord *result = try_allocate_in(r, req, in_new_region);
            if (result != NULL) {
              return result;
            }
          }
        }
      }

      // Recovery: try to mix the allocation into the mutator view:
      if (ShenandoahAllowMixedAllocs) {
        for (size_t c = _mutator_rightmost + 1; c > _mutator_leftmost; c--) {
          size_t idx = c - 1;
          if (is_mutator_free(idx)) {
            HeapWord* result = try_allocate_in(_heap->get_region(idx), req, in_new_region);
            if (result != NULL) {
              return result;
            }
          }
        }
      }

      break;
    }
    default:
      ShouldNotReachHere();
  }

  return NULL;
}

HeapWord* ShenandoahFreeSet::try_allocate_in(ShenandoahHeapRegion* r, ShenandoahHeap::ShenandoahAllocationRequest& req, bool& in_new_region) {
  assert (!has_no_alloc_capacity(r), err_msg("Performance: should avoid full regions on this path: " SIZE_FORMAT, r->region_number()));

  try_recycle_trashed(r);

  in_new_region = r->is_empty();

  HeapWord* result = NULL;
  size_t size = req.size();

  if (ShenandoahElasticTLAB && req.is_lab_alloc()) {
    size_t free = align_size_down(r->free() >> LogHeapWordSize, MinObjAlignment);
    if (size > free) {
      size = free;
    }
    if (size >= req.min_size()) {
      result = r->allocate(size, req.type());
      assert (result != NULL, err_msg("Allocation must succeed: free " SIZE_FORMAT ", actual " SIZE_FORMAT, free, size));
    }
  } else {
    result = r->allocate(size, req.type());
  }

  if (result != NULL) {
    // Allocation successful, bump stats:
    increase_used(size * HeapWordSize);

    // Record actual allocation size
    req.set_actual_size(size);
  }

  if (result == NULL || has_no_alloc_capacity(r)) {
    // Region cannot afford this or future allocations. Retire it.
    //
    // While this seems a bit harsh, especially in the case when this large allocation does not
    // fit, but the next small one would, we are risking to inflate scan times when lots of
    // almost-full regions precede the fully-empty region where we want allocate the entire TLAB.
    // TODO: Record first fully-empty region, and use that for large allocations

    // Record the remainder as allocation waste
    size_t waste = r->free();
    if (waste > 0) {
      increase_used(waste);
      _heap->notify_alloc_words(waste >> LogHeapWordSize, true);
    }

    size_t num = r->region_number();
    _collector_free_bitmap.clear_bit(num);
    _mutator_free_bitmap.clear_bit(num);
    // Touched the bounds? Need to update:
    if (touches_bounds(num)) {
      adjust_bounds();
    }
    assert_bounds();
  }
  return result;
}

bool ShenandoahFreeSet::touches_bounds(size_t num) const {
  return num == _collector_leftmost || num == _collector_rightmost || num == _mutator_leftmost || num == _mutator_rightmost;
}

void ShenandoahFreeSet::adjust_bounds() {
  // Rewind both mutator bounds until the next bit.
  while (_mutator_leftmost < _max && !is_mutator_free(_mutator_leftmost)) {
    _mutator_leftmost++;
  }
  while (_mutator_rightmost > 0 && !is_mutator_free(_mutator_rightmost)) {
    _mutator_rightmost--;
  }
  // Rewind both collector bounds until the next bit.
  while (_collector_leftmost < _max && !is_collector_free(_collector_leftmost)) {
    _collector_leftmost++;
  }
  while (_collector_rightmost > 0 && !is_collector_free(_collector_rightmost)) {
    _collector_rightmost--;
  }
}

HeapWord* ShenandoahFreeSet::allocate_contiguous(ShenandoahHeap::ShenandoahAllocationRequest& req) {
  assert_heaplock_owned_by_current_thread();

  size_t words_size = req.size();
  size_t num = ShenandoahHeapRegion::required_regions(words_size * HeapWordSize);

  // No regions left to satisfy allocation, bye.
  if (num > mutator_count()) {
    return NULL;
  }

  // Find the continuous interval of $num regions, starting from $beg and ending in $end,
  // inclusive. Contiguous allocations are biased to the beginning.

  size_t beg = _mutator_leftmost;
  size_t end = beg;

  while (true) {
    if (end >= _max) {
      // Hit the end, goodbye
      return NULL;
    }

    // If regions are not adjacent, then current [beg; end] is useless, and we may fast-forward.
    // If region is not completely free, the current [beg; end] is useless, and we may fast-forward.
    if (!is_mutator_free(end) || !is_empty_or_trash(_heap->get_region(end))) {
      end++;
      beg = end;
      continue;
    }

    if ((end - beg + 1) == num) {
      // found the match
      break;
    }

    end++;
  };

  size_t remainder = words_size & ShenandoahHeapRegion::region_size_words_mask();

  // Initialize regions:
  for (size_t i = beg; i <= end; i++) {
    ShenandoahHeapRegion* r = _heap->get_region(i);
    try_recycle_trashed(r);

    assert(i == beg || _heap->get_region(i-1)->region_number() + 1 == r->region_number(), "Should be contiguous");
    assert(r->is_empty(), "Should be empty");

    if (i == beg) {
      r->make_humongous_start();
    } else {
      r->make_humongous_cont();
    }

    // Trailing region may be non-full, record the remainder there
    size_t used_words;
    if ((i == end) && (remainder != 0)) {
      used_words = remainder;
    } else {
      used_words = ShenandoahHeapRegion::region_size_words();
    }

    r->set_top(r->bottom() + used_words);
    r->reset_alloc_metadata_to_shared();

    _mutator_free_bitmap.clear_bit(r->region_number());
  }

  // While individual regions report their true use, all humongous regions are
  // marked used in the free set.
  increase_used(ShenandoahHeapRegion::region_size_bytes() * num);

  if (remainder != 0) {
    // Record this remainder as allocation waste
    _heap->notify_alloc_words(ShenandoahHeapRegion::region_size_words() - remainder, true);
  }

  // Allocated at left/rightmost? Move the bounds appropriately.
  if (beg == _mutator_leftmost || end == _mutator_rightmost) {
    adjust_bounds();
  }
  assert_bounds();

  req.set_actual_size(words_size);
  return _heap->get_region(beg)->bottom();
}

bool ShenandoahFreeSet::is_empty_or_trash(ShenandoahHeapRegion *r) {
  return r->is_empty() || r->is_trash();
}

size_t ShenandoahFreeSet::alloc_capacity(ShenandoahHeapRegion *r) {
  if (r->is_trash()) {
    // This would be recycled on allocation path
    return ShenandoahHeapRegion::region_size_bytes();
  } else {
    return r->free();
  }
}

bool ShenandoahFreeSet::has_no_alloc_capacity(ShenandoahHeapRegion *r) {
  return alloc_capacity(r) == 0;
}

void ShenandoahFreeSet::try_recycle_trashed(ShenandoahHeapRegion *r) {
  if (r->is_trash()) {
    _heap->decrease_used(r->used());
    r->recycle();
  }
}

void ShenandoahFreeSet::recycle_trash() {
  // lock is not reentrable, check we don't have it
  assert_heaplock_not_owned_by_current_thread();

  for (size_t i = 0; i < _heap->num_regions(); i++) {
    ShenandoahHeapRegion* r = _heap->get_region(i);
    if (r->is_trash()) {
      ShenandoahHeapLocker locker(_heap->lock());
      try_recycle_trashed(r);
    }
    SpinPause(); // allow allocators to take the lock
  }
}

void ShenandoahFreeSet::flip_to_gc(size_t idx) {
  _mutator_free_bitmap.clear_bit(idx);
  _collector_free_bitmap.set_bit(idx);
  _collector_leftmost = MIN2(idx, _collector_leftmost);
  _collector_rightmost = MAX2(idx, _collector_rightmost);
  if (touches_bounds(idx)) {
    adjust_bounds();
  }
  assert_bounds();
}

void ShenandoahFreeSet::flip_to_mutator(size_t idx) {
  _collector_free_bitmap.clear_bit(idx);
  _mutator_free_bitmap.set_bit(idx);
  _mutator_leftmost = MIN2(idx, _mutator_leftmost);
  _mutator_rightmost = MAX2(idx, _mutator_rightmost);
  if (touches_bounds(idx)) {
    adjust_bounds();
  }
  assert_bounds();
}

void ShenandoahFreeSet::clear() {
  assert_heaplock_owned_by_current_thread();
  clear_internal();
}

void ShenandoahFreeSet::clear_internal() {
  _mutator_free_bitmap.clear();
  _collector_free_bitmap.clear();
  _mutator_leftmost = _max;
  _mutator_rightmost = 0;
  _collector_leftmost = _max;
  _collector_rightmost = 0;
  _capacity = 0;
  _used = 0;
}

void ShenandoahFreeSet::rebuild() {
  assert_heaplock_owned_by_current_thread();
  clear();

  for (size_t idx = 0; idx < _heap->num_regions(); idx++) {
    ShenandoahHeapRegion* region = _heap->get_region(idx);
    if (region->is_alloc_allowed() || region->is_trash()) {
      assert(!region->in_collection_set(), "Shouldn't be adding those to the free set");

      // Do not add regions that would surely fail allocation
      if (has_no_alloc_capacity(region)) continue;

      _capacity += alloc_capacity(region);
      assert(_used <= _capacity, "must not use more than we have");

      assert(!is_mutator_free(idx), "We are about to add it, it shouldn't be there already");
      _mutator_free_bitmap.set_bit(idx);
      _mutator_leftmost  = MIN2(_mutator_leftmost, idx);
      _mutator_rightmost = MAX2(_mutator_rightmost, idx);
    }
  }

  assert_bounds();
  log_status();
}

void ShenandoahFreeSet::log_status() {
  log_info(gc, ergo)("Free: " SIZE_FORMAT "M, Regions: " SIZE_FORMAT " mutator, " SIZE_FORMAT " collector",
                     available() / M, mutator_count(), collector_count());
}

void ShenandoahFreeSet::log_status_verbose() {
  assert_heaplock_owned_by_current_thread();

  if (ShenandoahLogInfo || PrintGCDetails) {
    ResourceMark rm;
    outputStream* ls = gclog_or_tty;

    ls->print_cr("Free set: Used: " SIZE_FORMAT "M of " SIZE_FORMAT "M, Regions: " SIZE_FORMAT " mutator, " SIZE_FORMAT " collector",
                used() / M, capacity() / M, mutator_count(), collector_count());

    {
      ls->print("Free set: Mutator view: ");

      size_t last_idx = 0;
      size_t max = 0;
      size_t max_contig = 0;
      size_t empty_contig = 0;

      size_t total_used = 0;
      size_t total_free = 0;

      for (size_t idx = _mutator_leftmost; idx <= _mutator_rightmost; idx++) {
        if (is_mutator_free(idx)) {
          ShenandoahHeapRegion *r = _heap->get_region(idx);
          size_t free = r->free();

          max = MAX2(max, free);

          if (r->is_empty() && (last_idx + 1 == idx)) {
            empty_contig++;
          } else {
            empty_contig = 0;
          }

          total_used += r->used();
          total_free += free;

          max_contig = MAX2(max_contig, empty_contig);
          last_idx = idx;
        }
      }

      size_t max_humongous = max_contig * ShenandoahHeapRegion::region_size_bytes();
      size_t free = capacity() - used();

      ls->print("Max regular: " SIZE_FORMAT "K, Max humongous: " SIZE_FORMAT "K, ", max / K, max_humongous / K);

      size_t frag_ext;
      if (free > 0) {
        frag_ext = 100 - (100 * max_humongous / free);
      } else {
        frag_ext = 0;
      }
      ls->print("External frag: " SIZE_FORMAT "%%, ", frag_ext);

      size_t frag_int;
      if (mutator_count() > 0) {
        frag_int = (100 * (total_used / mutator_count()) / ShenandoahHeapRegion::region_size_bytes());
      } else {
        frag_int = 0;
      }
      ls->print("Internal frag: " SIZE_FORMAT "%%", frag_int);
      ls->cr();
    }

    {
      ls->print("Free set: Collector view: ");

      size_t max = 0;
      for (size_t idx = _collector_leftmost; idx <= _collector_rightmost; idx++) {
        if (is_collector_free(idx)) {
          ShenandoahHeapRegion *r = _heap->get_region(idx);
          max = MAX2(max, r->free());
        }
      }

      ls->print("Max regular: " SIZE_FORMAT "K", max / K);
      ls->cr();
    }
  }
}

HeapWord* ShenandoahFreeSet::allocate(ShenandoahHeap::ShenandoahAllocationRequest& req, bool& in_new_region) {
  assert_heaplock_owned_by_current_thread();
  assert_bounds();

  if (req.size() > ShenandoahHeapRegion::humongous_threshold_words()) {
    switch (req.type()) {
      case ShenandoahHeap::_alloc_shared:
      case ShenandoahHeap::_alloc_shared_gc:
        in_new_region = true;
        return allocate_contiguous(req);
      case ShenandoahHeap::_alloc_gclab:
      case ShenandoahHeap::_alloc_tlab:
        in_new_region = false;
        assert(false, err_msg("Trying to allocate TLAB larger than the humongous threshold: " SIZE_FORMAT " > " SIZE_FORMAT,
                              req.size(), ShenandoahHeapRegion::humongous_threshold_words()));
        return NULL;
      default:
        ShouldNotReachHere();
        return NULL;
    }
  } else {
    return allocate_single(req, in_new_region);
  }
}

size_t ShenandoahFreeSet::unsafe_peek_free() const {
  // Deliberately not locked, this method is unsafe when free set is modified.

  for (size_t index = _mutator_leftmost; index <= _mutator_rightmost; index++) {
    if (index < _max && is_mutator_free(index)) {
      ShenandoahHeapRegion* r = _heap->get_region(index);
      if (r->free() >= MinTLABSize) {
        return r->free();
      }
    }
  }

  // It appears that no regions left
  return 0;
}

void ShenandoahFreeSet::print_on(outputStream* out) const {
  out->print_cr("Mutator Free Set: " SIZE_FORMAT "", mutator_count());
  for (size_t index = _mutator_leftmost; index <= _mutator_rightmost; index++) {
    if (is_mutator_free(index)) {
      _heap->get_region(index)->print_on(out);
    }
  }
  out->print_cr("Collector Free Set: " SIZE_FORMAT "", collector_count());
  for (size_t index = _collector_leftmost; index <= _collector_rightmost; index++) {
    if (is_collector_free(index)) {
      _heap->get_region(index)->print_on(out);
    }
  }
}

#ifdef ASSERT
void ShenandoahFreeSet::assert_heaplock_owned_by_current_thread() const {
  _heap->assert_heaplock_owned_by_current_thread();
}



void ShenandoahFreeSet::assert_heaplock_not_owned_by_current_thread() const {
  _heap->assert_heaplock_not_owned_by_current_thread();
}

void ShenandoahFreeSet::assert_bounds() const {
  // Performance invariants. Failing these would not break the free set, but performance
  // would suffer.
  assert (_mutator_leftmost <= _max, err_msg("leftmost in bounds: "  SIZE_FORMAT " < " SIZE_FORMAT, _mutator_leftmost,  _max));
  assert (_mutator_rightmost < _max, err_msg("rightmost in bounds: " SIZE_FORMAT " < " SIZE_FORMAT, _mutator_rightmost, _max));

  assert (_mutator_leftmost == _max || is_mutator_free(_mutator_leftmost),  err_msg("leftmost region should be free: " SIZE_FORMAT,  _mutator_leftmost));
  assert (_mutator_rightmost == 0   || is_mutator_free(_mutator_rightmost), err_msg("rightmost region should be free: " SIZE_FORMAT, _mutator_rightmost));

  size_t beg_off = _mutator_free_bitmap.get_next_one_offset(0);
  size_t end_off = _mutator_free_bitmap.get_next_one_offset(_mutator_rightmost + 1);
  assert (beg_off >= _mutator_leftmost, err_msg("free regions before the leftmost: " SIZE_FORMAT ", bound " SIZE_FORMAT, beg_off, _mutator_leftmost));
  assert (end_off == _max,      err_msg("free regions past the rightmost: " SIZE_FORMAT ", bound " SIZE_FORMAT,  end_off, _mutator_rightmost));

  assert (_collector_leftmost <= _max, err_msg("leftmost in bounds: "  SIZE_FORMAT " < " SIZE_FORMAT, _collector_leftmost,  _max));
  assert (_collector_rightmost < _max, err_msg("rightmost in bounds: " SIZE_FORMAT " < " SIZE_FORMAT, _collector_rightmost, _max));

  assert (_collector_leftmost == _max || is_collector_free(_collector_leftmost),  err_msg("leftmost region should be free: " SIZE_FORMAT,  _collector_leftmost));
  assert (_collector_rightmost == 0   || is_collector_free(_collector_rightmost), err_msg("rightmost region should be free: " SIZE_FORMAT, _collector_rightmost));

  beg_off = _collector_free_bitmap.get_next_one_offset(0);
  end_off = _collector_free_bitmap.get_next_one_offset(_collector_rightmost + 1);
  assert (beg_off >= _collector_leftmost, err_msg("free regions before the leftmost: " SIZE_FORMAT ", bound " SIZE_FORMAT, beg_off, _collector_leftmost));
  assert (end_off == _max,      err_msg("free regions past the rightmost: " SIZE_FORMAT ", bound " SIZE_FORMAT,  end_off, _collector_rightmost));
}
#endif
