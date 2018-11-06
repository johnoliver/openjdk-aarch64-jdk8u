/*
 * Copyright (c) 2001, 2014, Oracle and/or its affiliates. All rights reserved.
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

#include "precompiled.hpp"
#include "memory/allocation.inline.hpp"
#include "runtime/arguments.hpp"
#include "runtime/java.hpp"
#include "runtime/mutex.hpp"
#include "runtime/mutexLocker.hpp"
#include "runtime/orderAccess.inline.hpp"
#include "runtime/os.hpp"
#include "runtime/perfData.hpp"
#include "runtime/perfMemory.hpp"
#include "runtime/statSampler.hpp"
#include "utilities/globalDefinitions.hpp"

PRAGMA_FORMAT_MUTE_WARNINGS_FOR_GCC

// Prefix of performance data file.
const char               PERFDATA_NAME[] = "hsperfdata";

// Add 1 for the '_' character between PERFDATA_NAME and pid. The '\0' terminating
// character will be included in the sizeof(PERFDATA_NAME) operation.
static const size_t PERFDATA_FILENAME_LEN = sizeof(PERFDATA_NAME) +
                                            UINT_CHARS + 1;

char*                    PerfMemory::_start = NULL;
char*                    PerfMemory::_end = NULL;
char*                    PerfMemory::_top = NULL;
size_t                   PerfMemory::_capacity = 0;
jint                     PerfMemory::_initialized = false;
PerfDataPrologue*        PerfMemory::_prologue = NULL;

void perfMemory_init() {

  if (!UsePerfData) return;

  PerfMemory::initialize();
}

void perfMemory_exit() {

  if (!UsePerfData) return;
  if (!PerfMemory::is_initialized()) return;

  // if the StatSampler is active, then we don't want to remove
  // resources it may be dependent on. Typically, the StatSampler
  // is disengaged from the watcher thread when this method is called,
  // but it is not disengaged if this method is invoked during a
  // VM abort.
  //
  if (!StatSampler::is_active())
    PerfDataManager::destroy();

  // remove the persistent external resources, if any. this method
  // does not unmap or invalidate any virtual memory allocated during
  // initialization.
  //
  PerfMemory::destroy();
}

void PerfMemory::initialize() {

  if (_prologue != NULL)
    // initialization already performed
    return;

  size_t capacity = align_size_up(PerfDataMemorySize,
                                  os::vm_allocation_granularity());

  if (PerfTraceMemOps) {
    tty->print("PerfDataMemorySize = " SIZE_FORMAT ","
               " os::vm_allocation_granularity = " SIZE_FORMAT ","
               " adjusted size = " SIZE_FORMAT "\n",
               PerfDataMemorySize,
               os::vm_allocation_granularity(),
               capacity);
  }

  // allocate PerfData memory region
  create_memory_region(capacity);

  if (_start == NULL) {

    // the PerfMemory region could not be created as desired. Rather
    // than terminating the JVM, we revert to creating the instrumentation
    // on the C heap. When running in this mode, external monitoring
    // clients cannot attach to and monitor this JVM.
    //
    // the warning is issued only in debug mode in order to avoid
    // additional output to the stdout or stderr output streams.
    //
    if (PrintMiscellaneous && Verbose) {
      warning("Could not create PerfData Memory region, reverting to malloc");
    }

    _prologue = NEW_C_HEAP_OBJ(PerfDataPrologue, mtInternal);
  }
  else {

    // the PerfMemory region was created as expected.

    if (PerfTraceMemOps) {
      tty->print("PerfMemory created: address = " INTPTR_FORMAT ","
                 " size = " SIZE_FORMAT "\n",
                 (void*)_start,
                 _capacity);
    }

    _prologue = (PerfDataPrologue *)_start;
    _end = _start + _capacity;
    _top = _start + sizeof(PerfDataPrologue);
  }

  assert(_prologue != NULL, "prologue pointer must be initialized");

#ifdef VM_LITTLE_ENDIAN
  _prologue->magic = (jint)0xc0c0feca;
  _prologue->byte_order = PERFDATA_LITTLE_ENDIAN;
#else
  _prologue->magic = (jint)0xcafec0c0;
  _prologue->byte_order = PERFDATA_BIG_ENDIAN;
#endif

  _prologue->major_version = PERFDATA_MAJOR_VERSION;
  _prologue->minor_version = PERFDATA_MINOR_VERSION;
  _prologue->accessible = 0;

  _prologue->entry_offset = sizeof(PerfDataPrologue);
  _prologue->num_entries = 0;
  _prologue->used = 0;
  _prologue->overflow = 0;
  _prologue->mod_time_stamp = 0;

  OrderAccess::release_store(&_initialized, 1);
}

void PerfMemory::destroy() {

  if (_prologue == NULL) return;

  if (_start != NULL && _prologue->overflow != 0) {

    // This state indicates that the contiguous memory region exists and
    // that it wasn't large enough to hold all the counters. In this case,
    // we output a warning message to the user on exit if the -XX:+Verbose
    // flag is set (a debug only flag). External monitoring tools can detect
    // this condition by monitoring the _prologue->overflow word.
    //
    // There are two tunables that can help resolve this issue:
    //   - increase the size of the PerfMemory with -XX:PerfDataMemorySize=<n>
    //   - decrease the maximum string constant length with
    //     -XX:PerfMaxStringConstLength=<n>
    //
    if (PrintMiscellaneous && Verbose) {
      warning("PerfMemory Overflow Occurred.\n"
              "\tCapacity = " SIZE_FORMAT " bytes"
              "  Used = " SIZE_FORMAT " bytes"
              "  Overflow = " INT32_FORMAT " bytes"
              "\n\tUse -XX:PerfDataMemorySize=<size> to specify larger size.",
              PerfMemory::capacity(),
              PerfMemory::used(),
              _prologue->overflow);
    }
  }

  if (_start != NULL) {

    // this state indicates that the contiguous memory region was successfully
    // and that persistent resources may need to be cleaned up. This is
    // expected to be the typical condition.
    //
    delete_memory_region();
  }

  _start = NULL;
  _end = NULL;
  _top = NULL;
  _prologue = NULL;
  _capacity = 0;
}

// allocate an aligned block of memory from the PerfData memory
// region. This method assumes that the PerfData memory region
// was aligned on a double word boundary when created.
//
char* PerfMemory::alloc(size_t size) {

  if (!UsePerfData) return NULL;

  MutexLocker ml(PerfDataMemAlloc_lock);

  assert(_prologue != NULL, "called before initialization");

  // check that there is enough memory for this request
  if ((_top + size) >= _end) {

    _prologue->overflow += (jint)size;

    return NULL;
  }

  char* result = _top;

  _top += size;

  assert(contains(result), "PerfData memory pointer out of range");

  _prologue->used = (jint)used();
  _prologue->num_entries = _prologue->num_entries + 1;

  return result;
}

void PerfMemory::mark_updated() {
  if (!UsePerfData) return;

  _prologue->mod_time_stamp = os::elapsed_counter();
}

// Returns the complete path including the file name of performance data file.
// Caller is expected to release the allocated memory.
char* PerfMemory::get_perfdata_file_path() {
  char* dest_file = NULL;

  if (PerfDataSaveFile != NULL) {
    // dest_file_name stores the validated file name if file_name
    // contains %p which will be replaced by pid.
    dest_file = NEW_C_HEAP_ARRAY(char, JVM_MAXPATHLEN, mtInternal);
    if(!Arguments::copy_expand_pid(PerfDataSaveFile, strlen(PerfDataSaveFile),
                                   dest_file, JVM_MAXPATHLEN)) {
      FREE_C_HEAP_ARRAY(char, dest_file, mtInternal);
      if (PrintMiscellaneous && Verbose) {
        warning("Invalid performance data file path name specified, "\
                "fall back to a default name");
      }
    } else {
      return dest_file;
    }
  }
  // create the name of the file for retaining the instrumentation memory.
  dest_file = NEW_C_HEAP_ARRAY(char, PERFDATA_FILENAME_LEN, mtInternal);
  jio_snprintf(dest_file, PERFDATA_FILENAME_LEN,
               "%s_%d", PERFDATA_NAME, os::current_process_id());

  return dest_file;
}
