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
#include "gc_implementation/shenandoah/heuristics/shenandoahAggressiveHeuristics.hpp"
#include "gc_implementation/shenandoah/shenandoahCollectionSet.hpp"
#include "gc_implementation/shenandoah/shenandoahHeapRegion.hpp"
#include "gc_implementation/shenandoah/shenandoahLogging.hpp"

ShenandoahAggressiveHeuristics::ShenandoahAggressiveHeuristics() : ShenandoahHeuristics() {
  // Do not shortcut evacuation
  SHENANDOAH_ERGO_OVERRIDE_DEFAULT(ShenandoahImmediateThreshold, 100);

  // Aggressive runs with max speed for allocation, to capture races against mutator
  SHENANDOAH_ERGO_DISABLE_FLAG(ShenandoahPacing);

  // Aggressive evacuates everything, so it needs as much evac space as it can get
  SHENANDOAH_ERGO_ENABLE_FLAG(ShenandoahEvacReserveOverflow);

  // If class unloading is globally enabled, aggressive does unloading even with
  // concurrent cycles.
  if (ClassUnloading) {
    SHENANDOAH_ERGO_OVERRIDE_DEFAULT(ShenandoahUnloadClassesFrequency, 1);
  }
}

void ShenandoahAggressiveHeuristics::choose_collection_set_from_regiondata(ShenandoahCollectionSet* cset,
                                                                           RegionData* data, size_t size,
                                                                           size_t free) {
  for (size_t idx = 0; idx < size; idx++) {
    ShenandoahHeapRegion* r = data[idx]._region;
    if (r->garbage() > 0) {
      cset->add_region(r);
    }
  }
}

bool ShenandoahAggressiveHeuristics::should_start_normal_gc() const {
  log_info(gc)("Trigger: Start next cycle immediately");
  return true;
}

bool ShenandoahAggressiveHeuristics::should_process_references() {
  if (ShenandoahRefProcFrequency == 0) return false;
  // Randomly process refs with 50% chance.
  return (os::random() & 1) == 1;
}

bool ShenandoahAggressiveHeuristics::should_unload_classes() {
  if (ShenandoahUnloadClassesFrequency == 0) return false;
  // Randomly unload classes with 50% chance.
  return (os::random() & 1) == 1;
}

const char* ShenandoahAggressiveHeuristics::name() {
  return "aggressive";
}

bool ShenandoahAggressiveHeuristics::is_diagnostic() {
  return true;
}

bool ShenandoahAggressiveHeuristics::is_experimental() {
  return false;
}
