/*
 * Copyright (c) 2015, 2016, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_GC_G1_WORKERDATAARRAY_INLINE_HPP
#define SHARE_VM_GC_G1_WORKERDATAARRAY_INLINE_HPP

#include "gc_implementation/shenandoah/shenandoahWorkerDataArray.hpp"
#include "memory/allocation.inline.hpp"
#include "utilities/ostream.hpp"

template <typename T>
ShenandoahWorkerDataArray<T>::ShenandoahWorkerDataArray(uint length, const char* title) :
 _title(title),
 _length(0),
 _thread_work_items(NULL) {
  assert(length > 0, "Must have some workers to store data for");
  _length = length;
  _data = NEW_C_HEAP_ARRAY(T, _length, mtGC);
  reset();
}

template <typename T>
void ShenandoahWorkerDataArray<T>::set(uint worker_i, T value) {
  assert(worker_i < _length, err_msg("Worker %d is greater than max: %d", worker_i, _length));
  assert(_data[worker_i] == uninitialized(), err_msg("Overwriting data for worker %d in %s", worker_i, _title));
  _data[worker_i] = value;
}

template <typename T>
T ShenandoahWorkerDataArray<T>::get(uint worker_i) const {
  assert(worker_i < _length, err_msg("Worker %d is greater than max: %d", worker_i, _length));
  return _data[worker_i];
}

template <typename T>
ShenandoahWorkerDataArray<T>::~ShenandoahWorkerDataArray() {
  FREE_C_HEAP_ARRAY(T, _data, mtGC);
}

template <typename T>
void ShenandoahWorkerDataArray<T>::link_thread_work_items(ShenandoahWorkerDataArray<size_t>* thread_work_items) {
  _thread_work_items = thread_work_items;
}

template <typename T>
void ShenandoahWorkerDataArray<T>::set_thread_work_item(uint worker_i, size_t value) {
  assert(_thread_work_items != NULL, "No sub count");
  _thread_work_items->set(worker_i, value);
}

template <typename T>
void ShenandoahWorkerDataArray<T>::add(uint worker_i, T value) {
  assert(worker_i < _length, err_msg("Worker %d is greater than max: %d", worker_i, _length));
  assert(_data[worker_i] != uninitialized(), err_msg("No data to add to for worker %d", worker_i));
  _data[worker_i] += value;
}

template <typename T>
double ShenandoahWorkerDataArray<T>::average() const {
  uint contributing_threads = 0;
  for (uint i = 0; i < _length; ++i) {
    if (get(i) != uninitialized()) {
      contributing_threads++;
    }
  }
  if (contributing_threads == 0) {
    return 0.0;
  }
  return sum() / (double) contributing_threads;
}

template <typename T>
T ShenandoahWorkerDataArray<T>::sum() const {
  T s = 0;
  for (uint i = 0; i < _length; ++i) {
    if (get(i) != uninitialized()) {
      s += get(i);
    }
  }
  return s;
}

template <typename T>
void ShenandoahWorkerDataArray<T>::set_all(T value) {
  for (uint i = 0; i < _length; i++) {
    _data[i] = value;
  }
}

template <class T>
void ShenandoahWorkerDataArray<T>::print_summary_on(outputStream* out, bool print_sum) const {
  out->print("%-25s", title());
  uint start = 0;
  while (start < _length && get(start) == uninitialized()) {
    start++;
  }
  if (start < _length) {
    T min = get(start);
    T max = min;
    T sum = 0;
    uint contributing_threads = 0;
    for (uint i = start; i < _length; ++i) {
      T value = get(i);
      if (value != uninitialized()) {
        max = MAX2(max, value);
        min = MIN2(min, value);
        sum += value;
        contributing_threads++;
      }
    }
    T diff = max - min;
    assert(contributing_threads != 0, "Must be since we found a used value for the start index");
    double avg = sum / (double) contributing_threads;
    WDAPrinter::summary(out, min, avg, max, diff, sum, print_sum);
    out->print_cr(", Workers: %d", contributing_threads);
  } else {
    // No data for this phase.
    out->print_cr(" skipped");
  }
}

template <class T>
void ShenandoahWorkerDataArray<T>::print_details_on(outputStream* out) const {
  WDAPrinter::details(this, out);
}

template <typename T>
void ShenandoahWorkerDataArray<T>::reset() {
  set_all(uninitialized());
  if (_thread_work_items != NULL) {
    _thread_work_items->reset();
  }
}

#endif // SHARE_VM_GC_G1_WORKERDATAARRAY_INLINE_HPP
