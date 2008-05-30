/*
 * Copyright 1997-2007 Sun Microsystems, Inc.  All Rights Reserved.
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
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 */

#include "incls/_precompiled.incl"
#include "incls/_compile.cpp.incl"

/// Support for intrinsics.

// Return the index at which m must be inserted (or already exists).
// The sort order is by the address of the ciMethod, with is_virtual as minor key.
int Compile::intrinsic_insertion_index(ciMethod* m, bool is_virtual) {
#ifdef ASSERT
  for (int i = 1; i < _intrinsics->length(); i++) {
    CallGenerator* cg1 = _intrinsics->at(i-1);
    CallGenerator* cg2 = _intrinsics->at(i);
    assert(cg1->method() != cg2->method()
           ? cg1->method()     < cg2->method()
           : cg1->is_virtual() < cg2->is_virtual(),
           "compiler intrinsics list must stay sorted");
  }
#endif
  // Binary search sorted list, in decreasing intervals [lo, hi].
  int lo = 0, hi = _intrinsics->length()-1;
  while (lo <= hi) {
    int mid = (uint)(hi + lo) / 2;
    ciMethod* mid_m = _intrinsics->at(mid)->method();
    if (m < mid_m) {
      hi = mid-1;
    } else if (m > mid_m) {
      lo = mid+1;
    } else {
      // look at minor sort key
      bool mid_virt = _intrinsics->at(mid)->is_virtual();
      if (is_virtual < mid_virt) {
        hi = mid-1;
      } else if (is_virtual > mid_virt) {
        lo = mid+1;
      } else {
        return mid;  // exact match
      }
    }
  }
  return lo;  // inexact match
}

void Compile::register_intrinsic(CallGenerator* cg) {
  if (_intrinsics == NULL) {
    _intrinsics = new GrowableArray<CallGenerator*>(60);
  }
  // This code is stolen from ciObjectFactory::insert.
  // Really, GrowableArray should have methods for
  // insert_at, remove_at, and binary_search.
  int len = _intrinsics->length();
  int index = intrinsic_insertion_index(cg->method(), cg->is_virtual());
  if (index == len) {
    _intrinsics->append(cg);
  } else {
#ifdef ASSERT
    CallGenerator* oldcg = _intrinsics->at(index);
    assert(oldcg->method() != cg->method() || oldcg->is_virtual() != cg->is_virtual(), "don't register twice");
#endif
    _intrinsics->append(_intrinsics->at(len-1));
    int pos;
    for (pos = len-2; pos >= index; pos--) {
      _intrinsics->at_put(pos+1,_intrinsics->at(pos));
    }
    _intrinsics->at_put(index, cg);
  }
  assert(find_intrinsic(cg->method(), cg->is_virtual()) == cg, "registration worked");
}

CallGenerator* Compile::find_intrinsic(ciMethod* m, bool is_virtual) {
  assert(m->is_loaded(), "don't try this on unloaded methods");
  if (_intrinsics != NULL) {
    int index = intrinsic_insertion_index(m, is_virtual);
    if (index < _intrinsics->length()
        && _intrinsics->at(index)->method() == m
        && _intrinsics->at(index)->is_virtual() == is_virtual) {
      return _intrinsics->at(index);
    }
  }
  // Lazily create intrinsics for intrinsic IDs well-known in the runtime.
  if (m->intrinsic_id() != vmIntrinsics::_none) {
    CallGenerator* cg = make_vm_intrinsic(m, is_virtual);
    if (cg != NULL) {
      // Save it for next time:
      register_intrinsic(cg);
      return cg;
    } else {
      gather_intrinsic_statistics(m->intrinsic_id(), is_virtual, _intrinsic_disabled);
    }
  }
  return NULL;
}

// Compile:: register_library_intrinsics and make_vm_intrinsic are defined
// in library_call.cpp.


#ifndef PRODUCT
// statistics gathering...

juint  Compile::_intrinsic_hist_count[vmIntrinsics::ID_LIMIT] = {0};
jubyte Compile::_intrinsic_hist_flags[vmIntrinsics::ID_LIMIT] = {0};

bool Compile::gather_intrinsic_statistics(vmIntrinsics::ID id, bool is_virtual, int flags) {
  assert(id > vmIntrinsics::_none && id < vmIntrinsics::ID_LIMIT, "oob");
  int oflags = _intrinsic_hist_flags[id];
  assert(flags != 0, "what happened?");
  if (is_virtual) {
    flags |= _intrinsic_virtual;
  }
  bool changed = (flags != oflags);
  if ((flags & _intrinsic_worked) != 0) {
    juint count = (_intrinsic_hist_count[id] += 1);
    if (count == 1) {
      changed = true;           // first time
    }
    // increment the overall count also:
    _intrinsic_hist_count[vmIntrinsics::_none] += 1;
  }
  if (changed) {
    if (((oflags ^ flags) & _intrinsic_virtual) != 0) {
      // Something changed about the intrinsic's virtuality.
      if ((flags & _intrinsic_virtual) != 0) {
        // This is the first use of this intrinsic as a virtual call.
        if (oflags != 0) {
          // We already saw it as a non-virtual, so note both cases.
          flags |= _intrinsic_both;
        }
      } else if ((oflags & _intrinsic_both) == 0) {
        // This is the first use of this intrinsic as a non-virtual
        flags |= _intrinsic_both;
      }
    }
    _intrinsic_hist_flags[id] = (jubyte) (oflags | flags);
  }
  // update the overall flags also:
  _intrinsic_hist_flags[vmIntrinsics::_none] |= (jubyte) flags;
  return changed;
}

static char* format_flags(int flags, char* buf) {
  buf[0] = 0;
  if ((flags & Compile::_intrinsic_worked) != 0)    strcat(buf, ",worked");
  if ((flags & Compile::_intrinsic_failed) != 0)    strcat(buf, ",failed");
  if ((flags & Compile::_intrinsic_disabled) != 0)  strcat(buf, ",disabled");
  if ((flags & Compile::_intrinsic_virtual) != 0)   strcat(buf, ",virtual");
  if ((flags & Compile::_intrinsic_both) != 0)      strcat(buf, ",nonvirtual");
  if (buf[0] == 0)  strcat(buf, ",");
  assert(buf[0] == ',', "must be");
  return &buf[1];
}

void Compile::print_intrinsic_statistics() {
  char flagsbuf[100];
  ttyLocker ttyl;
  if (xtty != NULL)  xtty->head("statistics type='intrinsic'");
  tty->print_cr("Compiler intrinsic usage:");
  juint total = _intrinsic_hist_count[vmIntrinsics::_none];
  if (total == 0)  total = 1;  // avoid div0 in case of no successes
  #define PRINT_STAT_LINE(name, c, f) \
    tty->print_cr("  %4d (%4.1f%%) %s (%s)", (int)(c), ((c) * 100.0) / total, name, f);
  for (int index = 1 + (int)vmIntrinsics::_none; index < (int)vmIntrinsics::ID_LIMIT; index++) {
    vmIntrinsics::ID id = (vmIntrinsics::ID) index;
    int   flags = _intrinsic_hist_flags[id];
    juint count = _intrinsic_hist_count[id];
    if ((flags | count) != 0) {
      PRINT_STAT_LINE(vmIntrinsics::name_at(id), count, format_flags(flags, flagsbuf));
    }
  }
  PRINT_STAT_LINE("total", total, format_flags(_intrinsic_hist_flags[vmIntrinsics::_none], flagsbuf));
  if (xtty != NULL)  xtty->tail("statistics");
}

void Compile::print_statistics() {
  { ttyLocker ttyl;
    if (xtty != NULL)  xtty->head("statistics type='opto'");
    Parse::print_statistics();
    PhaseCCP::print_statistics();
    PhaseRegAlloc::print_statistics();
    Scheduling::print_statistics();
    PhasePeephole::print_statistics();
    PhaseIdealLoop::print_statistics();
    if (xtty != NULL)  xtty->tail("statistics");
  }
  if (_intrinsic_hist_flags[vmIntrinsics::_none] != 0) {
    // put this under its own <statistics> element.
    print_intrinsic_statistics();
  }
}
#endif //PRODUCT

// Support for bundling info
Bundle* Compile::node_bundling(const Node *n) {
  assert(valid_bundle_info(n), "oob");
  return &_node_bundling_base[n->_idx];
}

bool Compile::valid_bundle_info(const Node *n) {
  return (_node_bundling_limit > n->_idx);
}


// Identify all nodes that are reachable from below, useful.
// Use breadth-first pass that records state in a Unique_Node_List,
// recursive traversal is slower.
void Compile::identify_useful_nodes(Unique_Node_List &useful) {
  int estimated_worklist_size = unique();
  useful.map( estimated_worklist_size, NULL );  // preallocate space

  // Initialize worklist
  if (root() != NULL)     { useful.push(root()); }
  // If 'top' is cached, declare it useful to preserve cached node
  if( cached_top_node() ) { useful.push(cached_top_node()); }

  // Push all useful nodes onto the list, breadthfirst
  for( uint next = 0; next < useful.size(); ++next ) {
    assert( next < unique(), "Unique useful nodes < total nodes");
    Node *n  = useful.at(next);
    uint max = n->len();
    for( uint i = 0; i < max; ++i ) {
      Node *m = n->in(i);
      if( m == NULL ) continue;
      useful.push(m);
    }
  }
}

// Disconnect all useless nodes by disconnecting those at the boundary.
void Compile::remove_useless_nodes(Unique_Node_List &useful) {
  uint next = 0;
  while( next < useful.size() ) {
    Node *n = useful.at(next++);
    // Use raw traversal of out edges since this code removes out edges
    int max = n->outcnt();
    for (int j = 0; j < max; ++j ) {
      Node* child = n->raw_out(j);
      if( ! useful.member(child) ) {
        assert( !child->is_top() || child != top(),
                "If top is cached in Compile object it is in useful list");
        // Only need to remove this out-edge to the useless node
        n->raw_del_out(j);
        --j;
        --max;
      }
    }
    if (n->outcnt() == 1 && n->has_special_unique_user()) {
      record_for_igvn( n->unique_out() );
    }
  }
  debug_only(verify_graph_edges(true/*check for no_dead_code*/);)
}

//------------------------------frame_size_in_words-----------------------------
// frame_slots in units of words
int Compile::frame_size_in_words() const {
  // shift is 0 in LP32 and 1 in LP64
  const int shift = (LogBytesPerWord - LogBytesPerInt);
  int words = _frame_slots >> shift;
  assert( words << shift == _frame_slots, "frame size must be properly aligned in LP64" );
  return words;
}

// ============================================================================
//------------------------------CompileWrapper---------------------------------
class CompileWrapper : public StackObj {
  Compile *const _compile;
 public:
  CompileWrapper(Compile* compile);

  ~CompileWrapper();
};

CompileWrapper::CompileWrapper(Compile* compile) : _compile(compile) {
  // the Compile* pointer is stored in the current ciEnv:
  ciEnv* env = compile->env();
  assert(env == ciEnv::current(), "must already be a ciEnv active");
  assert(env->compiler_data() == NULL, "compile already active?");
  env->set_compiler_data(compile);
  assert(compile == Compile::current(), "sanity");

  compile->set_type_dict(NULL);
  compile->set_type_hwm(NULL);
  compile->set_type_last_size(0);
  compile->set_last_tf(NULL, NULL);
  compile->set_indexSet_arena(NULL);
  compile->set_indexSet_free_block_list(NULL);
  compile->init_type_arena();
  Type::Initialize(compile);
  _compile->set_scratch_buffer_blob(NULL);
  _compile->begin_method();
}
CompileWrapper::~CompileWrapper() {
  if (_compile->failing()) {
    _compile->print_method("Failed");
  }
  _compile->end_method();
  if (_compile->scratch_buffer_blob() != NULL)
    BufferBlob::free(_compile->scratch_buffer_blob());
  _compile->env()->set_compiler_data(NULL);
}


//----------------------------print_compile_messages---------------------------
void Compile::print_compile_messages() {
#ifndef PRODUCT
  // Check if recompiling
  if (_subsume_loads == false && PrintOpto) {
    // Recompiling without allowing machine instructions to subsume loads
    tty->print_cr("*********************************************************");
    tty->print_cr("** Bailout: Recompile without subsuming loads          **");
    tty->print_cr("*********************************************************");
  }
  if (_do_escape_analysis != DoEscapeAnalysis && PrintOpto) {
    // Recompiling without escape analysis
    tty->print_cr("*********************************************************");
    tty->print_cr("** Bailout: Recompile without escape analysis          **");
    tty->print_cr("*********************************************************");
  }
  if (env()->break_at_compile()) {
    // Open the debugger when compiing this method.
    tty->print("### Breaking when compiling: ");
    method()->print_short_name();
    tty->cr();
    BREAKPOINT;
  }

  if( PrintOpto ) {
    if (is_osr_compilation()) {
      tty->print("[OSR]%3d", _compile_id);
    } else {
      tty->print("%3d", _compile_id);
    }
  }
#endif
}


void Compile::init_scratch_buffer_blob() {
  if( scratch_buffer_blob() != NULL )  return;

  // Construct a temporary CodeBuffer to have it construct a BufferBlob
  // Cache this BufferBlob for this compile.
  ResourceMark rm;
  int size = (MAX_inst_size + MAX_stubs_size + MAX_const_size);
  BufferBlob* blob = BufferBlob::create("Compile::scratch_buffer", size);
  // Record the buffer blob for next time.
  set_scratch_buffer_blob(blob);
  // Have we run out of code space?
  if (scratch_buffer_blob() == NULL) {
    // Let CompilerBroker disable further compilations.
    record_failure("Not enough space for scratch buffer in CodeCache");
    return;
  }

  // Initialize the relocation buffers
  relocInfo* locs_buf = (relocInfo*) blob->instructions_end() - MAX_locs_size;
  set_scratch_locs_memory(locs_buf);
}


//-----------------------scratch_emit_size-------------------------------------
// Helper function that computes size by emitting code
uint Compile::scratch_emit_size(const Node* n) {
  // Emit into a trash buffer and count bytes emitted.
  // This is a pretty expensive way to compute a size,
  // but it works well enough if seldom used.
  // All common fixed-size instructions are given a size
  // method by the AD file.
  // Note that the scratch buffer blob and locs memory are
  // allocated at the beginning of the compile task, and
  // may be shared by several calls to scratch_emit_size.
  // The allocation of the scratch buffer blob is particularly
  // expensive, since it has to grab the code cache lock.
  BufferBlob* blob = this->scratch_buffer_blob();
  assert(blob != NULL, "Initialize BufferBlob at start");
  assert(blob->size() > MAX_inst_size, "sanity");
  relocInfo* locs_buf = scratch_locs_memory();
  address blob_begin = blob->instructions_begin();
  address blob_end   = (address)locs_buf;
  assert(blob->instructions_contains(blob_end), "sanity");
  CodeBuffer buf(blob_begin, blob_end - blob_begin);
  buf.initialize_consts_size(MAX_const_size);
  buf.initialize_stubs_size(MAX_stubs_size);
  assert(locs_buf != NULL, "sanity");
  int lsize = MAX_locs_size / 2;
  buf.insts()->initialize_shared_locs(&locs_buf[0],     lsize);
  buf.stubs()->initialize_shared_locs(&locs_buf[lsize], lsize);
  n->emit(buf, this->regalloc());
  return buf.code_size();
}


// ============================================================================
//------------------------------Compile standard-------------------------------
debug_only( int Compile::_debug_idx = 100000; )

// Compile a method.  entry_bci is -1 for normal compilations and indicates
// the continuation bci for on stack replacement.


Compile::Compile( ciEnv* ci_env, C2Compiler* compiler, ciMethod* target, int osr_bci, bool subsume_loads, bool do_escape_analysis )
                : Phase(Compiler),
                  _env(ci_env),
                  _log(ci_env->log()),
                  _compile_id(ci_env->compile_id()),
                  _save_argument_registers(false),
                  _stub_name(NULL),
                  _stub_function(NULL),
                  _stub_entry_point(NULL),
                  _method(target),
                  _entry_bci(osr_bci),
                  _initial_gvn(NULL),
                  _for_igvn(NULL),
                  _warm_calls(NULL),
                  _subsume_loads(subsume_loads),
                  _do_escape_analysis(do_escape_analysis),
                  _failure_reason(NULL),
                  _code_buffer("Compile::Fill_buffer"),
                  _orig_pc_slot(0),
                  _orig_pc_slot_offset_in_bytes(0),
                  _node_bundling_limit(0),
                  _node_bundling_base(NULL),
#ifndef PRODUCT
                  _trace_opto_output(TraceOptoOutput || method()->has_option("TraceOptoOutput")),
                  _printer(IdealGraphPrinter::printer()),
#endif
                  _congraph(NULL) {
  C = this;

  CompileWrapper cw(this);
#ifndef PRODUCT
  if (TimeCompiler2) {
    tty->print(" ");
    target->holder()->name()->print();
    tty->print(".");
    target->print_short_name();
    tty->print("  ");
  }
  TraceTime t1("Total compilation time", &_t_totalCompilation, TimeCompiler, TimeCompiler2);
  TraceTime t2(NULL, &_t_methodCompilation, TimeCompiler, false);
  bool print_opto_assembly = PrintOptoAssembly || _method->has_option("PrintOptoAssembly");
  if (!print_opto_assembly) {
    bool print_assembly = (PrintAssembly || _method->should_print_assembly());
    if (print_assembly && !Disassembler::can_decode()) {
      tty->print_cr("PrintAssembly request changed to PrintOptoAssembly");
      print_opto_assembly = true;
    }
  }
  set_print_assembly(print_opto_assembly);
#endif

  if (ProfileTraps) {
    // Make sure the method being compiled gets its own MDO,
    // so we can at least track the decompile_count().
    method()->build_method_data();
  }

  Init(::AliasLevel);


  print_compile_messages();

  if (UseOldInlining || PrintCompilation NOT_PRODUCT( || PrintOpto) )
    _ilt = InlineTree::build_inline_tree_root();
  else
    _ilt = NULL;

  // Even if NO memory addresses are used, MergeMem nodes must have at least 1 slice
  assert(num_alias_types() >= AliasIdxRaw, "");

#define MINIMUM_NODE_HASH  1023
  // Node list that Iterative GVN will start with
  Unique_Node_List for_igvn(comp_arena());
  set_for_igvn(&for_igvn);

  // GVN that will be run immediately on new nodes
  uint estimated_size = method()->code_size()*4+64;
  estimated_size = (estimated_size < MINIMUM_NODE_HASH ? MINIMUM_NODE_HASH : estimated_size);
  PhaseGVN gvn(node_arena(), estimated_size);
  set_initial_gvn(&gvn);

  { // Scope for timing the parser
    TracePhase t3("parse", &_t_parser, true);

    // Put top into the hash table ASAP.
    initial_gvn()->transform_no_reclaim(top());

    // Set up tf(), start(), and find a CallGenerator.
    CallGenerator* cg;
    if (is_osr_compilation()) {
      const TypeTuple *domain = StartOSRNode::osr_domain();
      const TypeTuple *range = TypeTuple::make_range(method()->signature());
      init_tf(TypeFunc::make(domain, range));
      StartNode* s = new (this, 2) StartOSRNode(root(), domain);
      initial_gvn()->set_type_bottom(s);
      init_start(s);
      cg = CallGenerator::for_osr(method(), entry_bci());
    } else {
      // Normal case.
      init_tf(TypeFunc::make(method()));
      StartNode* s = new (this, 2) StartNode(root(), tf()->domain());
      initial_gvn()->set_type_bottom(s);
      init_start(s);
      float past_uses = method()->interpreter_invocation_count();
      float expected_uses = past_uses;
      cg = CallGenerator::for_inline(method(), expected_uses);
    }
    if (failing())  return;
    if (cg == NULL) {
      record_method_not_compilable_all_tiers("cannot parse method");
      return;
    }
    JVMState* jvms = build_start_state(start(), tf());
    if ((jvms = cg->generate(jvms)) == NULL) {
      record_method_not_compilable("method parse failed");
      return;
    }
    GraphKit kit(jvms);

    if (!kit.stopped()) {
      // Accept return values, and transfer control we know not where.
      // This is done by a special, unique ReturnNode bound to root.
      return_values(kit.jvms());
    }

    if (kit.has_exceptions()) {
      // Any exceptions that escape from this call must be rethrown
      // to whatever caller is dynamically above us on the stack.
      // This is done by a special, unique RethrowNode bound to root.
      rethrow_exceptions(kit.transfer_exceptions_into_jvms());
    }

    // Remove clutter produced by parsing.
    if (!failing()) {
      ResourceMark rm;
      PhaseRemoveUseless pru(initial_gvn(), &for_igvn);
    }
  }

  // Note:  Large methods are capped off in do_one_bytecode().
  if (failing())  return;

  // After parsing, node notes are no longer automagic.
  // They must be propagated by register_new_node_with_optimizer(),
  // clone(), or the like.
  set_default_node_notes(NULL);

  for (;;) {
    int successes = Inline_Warm();
    if (failing())  return;
    if (successes == 0)  break;
  }

  // Drain the list.
  Finish_Warm();
#ifndef PRODUCT
  if (_printer) {
    _printer->print_inlining(this);
  }
#endif

  if (failing())  return;
  NOT_PRODUCT( verify_graph_edges(); )

  // Perform escape analysis
  if (_do_escape_analysis)
    _congraph = new ConnectionGraph(this);
  if (_congraph != NULL) {
    NOT_PRODUCT( TracePhase t2("escapeAnalysis", &_t_escapeAnalysis, TimeCompiler); )
    _congraph->compute_escape();
    if (failing())  return;

#ifndef PRODUCT
    if (PrintEscapeAnalysis) {
      _congraph->dump();
    }
#endif
  }
  // Now optimize
  Optimize();
  if (failing())  return;
  NOT_PRODUCT( verify_graph_edges(); )

#ifndef PRODUCT
  if (PrintIdeal) {
    ttyLocker ttyl;  // keep the following output all in one block
    // This output goes directly to the tty, not the compiler log.
    // To enable tools to match it up with the compilation activity,
    // be sure to tag this tty output with the compile ID.
    if (xtty != NULL) {
      xtty->head("ideal compile_id='%d'%s", compile_id(),
                 is_osr_compilation()    ? " compile_kind='osr'" :
                 "");
    }
    root()->dump(9999);
    if (xtty != NULL) {
      xtty->tail("ideal");
    }
  }
#endif

  // Now that we know the size of all the monitors we can add a fixed slot
  // for the original deopt pc.

  _orig_pc_slot =  fixed_slots();
  int next_slot = _orig_pc_slot + (sizeof(address) / VMRegImpl::stack_slot_size);
  set_fixed_slots(next_slot);

  // Now generate code
  Code_Gen();
  if (failing())  return;

  // Check if we want to skip execution of all compiled code.
  {
#ifndef PRODUCT
    if (OptoNoExecute) {
      record_method_not_compilable("+OptoNoExecute");  // Flag as failed
      return;
    }
    TracePhase t2("install_code", &_t_registerMethod, TimeCompiler);
#endif

    if (is_osr_compilation()) {
      _code_offsets.set_value(CodeOffsets::Verified_Entry, 0);
      _code_offsets.set_value(CodeOffsets::OSR_Entry, _first_block_size);
    } else {
      _code_offsets.set_value(CodeOffsets::Verified_Entry, _first_block_size);
      _code_offsets.set_value(CodeOffsets::OSR_Entry, 0);
    }

    env()->register_method(_method, _entry_bci,
                           &_code_offsets,
                           _orig_pc_slot_offset_in_bytes,
                           code_buffer(),
                           frame_size_in_words(), _oop_map_set,
                           &_handler_table, &_inc_table,
                           compiler,
                           env()->comp_level(),
                           true, /*has_debug_info*/
                           has_unsafe_access()
                           );
  }
}

//------------------------------Compile----------------------------------------
// Compile a runtime stub
Compile::Compile( ciEnv* ci_env,
                  TypeFunc_generator generator,
                  address stub_function,
                  const char *stub_name,
                  int is_fancy_jump,
                  bool pass_tls,
                  bool save_arg_registers,
                  bool return_pc )
  : Phase(Compiler),
    _env(ci_env),
    _log(ci_env->log()),
    _compile_id(-1),
    _save_argument_registers(save_arg_registers),
    _method(NULL),
    _stub_name(stub_name),
    _stub_function(stub_function),
    _stub_entry_point(NULL),
    _entry_bci(InvocationEntryBci),
    _initial_gvn(NULL),
    _for_igvn(NULL),
    _warm_calls(NULL),
    _orig_pc_slot(0),
    _orig_pc_slot_offset_in_bytes(0),
    _subsume_loads(true),
    _do_escape_analysis(false),
    _failure_reason(NULL),
    _code_buffer("Compile::Fill_buffer"),
    _node_bundling_limit(0),
    _node_bundling_base(NULL),
#ifndef PRODUCT
    _trace_opto_output(TraceOptoOutput),
    _printer(NULL),
#endif
    _congraph(NULL) {
  C = this;

#ifndef PRODUCT
  TraceTime t1(NULL, &_t_totalCompilation, TimeCompiler, false);
  TraceTime t2(NULL, &_t_stubCompilation, TimeCompiler, false);
  set_print_assembly(PrintFrameConverterAssembly);
#endif
  CompileWrapper cw(this);
  Init(/*AliasLevel=*/ 0);
  init_tf((*generator)());

  {
    // The following is a dummy for the sake of GraphKit::gen_stub
    Unique_Node_List for_igvn(comp_arena());
    set_for_igvn(&for_igvn);  // not used, but some GraphKit guys push on this
    PhaseGVN gvn(Thread::current()->resource_area(),255);
    set_initial_gvn(&gvn);    // not significant, but GraphKit guys use it pervasively
    gvn.transform_no_reclaim(top());

    GraphKit kit;
    kit.gen_stub(stub_function, stub_name, is_fancy_jump, pass_tls, return_pc);
  }

  NOT_PRODUCT( verify_graph_edges(); )
  Code_Gen();
  if (failing())  return;


  // Entry point will be accessed using compile->stub_entry_point();
  if (code_buffer() == NULL) {
    Matcher::soft_match_failure();
  } else {
    if (PrintAssembly && (WizardMode || Verbose))
      tty->print_cr("### Stub::%s", stub_name);

    if (!failing()) {
      assert(_fixed_slots == 0, "no fixed slots used for runtime stubs");

      // Make the NMethod
      // For now we mark the frame as never safe for profile stackwalking
      RuntimeStub *rs = RuntimeStub::new_runtime_stub(stub_name,
                                                      code_buffer(),
                                                      CodeOffsets::frame_never_safe,
                                                      // _code_offsets.value(CodeOffsets::Frame_Complete),
                                                      frame_size_in_words(),
                                                      _oop_map_set,
                                                      save_arg_registers);
      assert(rs != NULL && rs->is_runtime_stub(), "sanity check");

      _stub_entry_point = rs->entry_point();
    }
  }
}

#ifndef PRODUCT
void print_opto_verbose_signature( const TypeFunc *j_sig, const char *stub_name ) {
  if(PrintOpto && Verbose) {
    tty->print("%s   ", stub_name); j_sig->print_flattened(); tty->cr();
  }
}
#endif

void Compile::print_codes() {
}

//------------------------------Init-------------------------------------------
// Prepare for a single compilation
void Compile::Init(int aliaslevel) {
  _unique  = 0;
  _regalloc = NULL;

  _tf      = NULL;  // filled in later
  _top     = NULL;  // cached later
  _matcher = NULL;  // filled in later
  _cfg     = NULL;  // filled in later

  set_24_bit_selection_and_mode(Use24BitFP, false);

  _node_note_array = NULL;
  _default_node_notes = NULL;

  _immutable_memory = NULL; // filled in at first inquiry

  // Globally visible Nodes
  // First set TOP to NULL to give safe behavior during creation of RootNode
  set_cached_top_node(NULL);
  set_root(new (this, 3) RootNode());
  // Now that you have a Root to point to, create the real TOP
  set_cached_top_node( new (this, 1) ConNode(Type::TOP) );
  set_recent_alloc(NULL, NULL);

  // Create Debug Information Recorder to record scopes, oopmaps, etc.
  env()->set_oop_recorder(new OopRecorder(comp_arena()));
  env()->set_debug_info(new DebugInformationRecorder(env()->oop_recorder()));
  env()->set_dependencies(new Dependencies(env()));

  _fixed_slots = 0;
  set_has_split_ifs(false);
  set_has_loops(has_method() && method()->has_loops()); // first approximation
  _deopt_happens = true;  // start out assuming the worst
  _trap_can_recompile = false;  // no traps emitted yet
  _major_progress = true; // start out assuming good things will happen
  set_has_unsafe_access(false);
  Copy::zero_to_bytes(_trap_hist, sizeof(_trap_hist));
  set_decompile_count(0);

  // Compilation level related initialization
  if (env()->comp_level() == CompLevel_fast_compile) {
    set_num_loop_opts(Tier1LoopOptsCount);
    set_do_inlining(Tier1Inline != 0);
    set_max_inline_size(Tier1MaxInlineSize);
    set_freq_inline_size(Tier1FreqInlineSize);
    set_do_scheduling(false);
    set_do_count_invocations(Tier1CountInvocations);
    set_do_method_data_update(Tier1UpdateMethodData);
  } else {
    assert(env()->comp_level() == CompLevel_full_optimization, "unknown comp level");
    set_num_loop_opts(LoopOptsCount);
    set_do_inlining(Inline);
    set_max_inline_size(MaxInlineSize);
    set_freq_inline_size(FreqInlineSize);
    set_do_scheduling(OptoScheduling);
    set_do_count_invocations(false);
    set_do_method_data_update(false);
  }

  if (debug_info()->recording_non_safepoints()) {
    set_node_note_array(new(comp_arena()) GrowableArray<Node_Notes*>
                        (comp_arena(), 8, 0, NULL));
    set_default_node_notes(Node_Notes::make(this));
  }

  // // -- Initialize types before each compile --
  // // Update cached type information
  // if( _method && _method->constants() )
  //   Type::update_loaded_types(_method, _method->constants());

  // Init alias_type map.
  if (!_do_escape_analysis && aliaslevel == 3)
    aliaslevel = 2;  // No unique types without escape analysis
  _AliasLevel = aliaslevel;
  const int grow_ats = 16;
  _max_alias_types = grow_ats;
  _alias_types   = NEW_ARENA_ARRAY(comp_arena(), AliasType*, grow_ats);
  AliasType* ats = NEW_ARENA_ARRAY(comp_arena(), AliasType,  grow_ats);
  Copy::zero_to_bytes(ats, sizeof(AliasType)*grow_ats);
  {
    for (int i = 0; i < grow_ats; i++)  _alias_types[i] = &ats[i];
  }
  // Initialize the first few types.
  _alias_types[AliasIdxTop]->Init(AliasIdxTop, NULL);
  _alias_types[AliasIdxBot]->Init(AliasIdxBot, TypePtr::BOTTOM);
  _alias_types[AliasIdxRaw]->Init(AliasIdxRaw, TypeRawPtr::BOTTOM);
  _num_alias_types = AliasIdxRaw+1;
  // Zero out the alias type cache.
  Copy::zero_to_bytes(_alias_cache, sizeof(_alias_cache));
  // A NULL adr_type hits in the cache right away.  Preload the right answer.
  probe_alias_cache(NULL)->_index = AliasIdxTop;

  _intrinsics = NULL;
  _macro_nodes = new GrowableArray<Node*>(comp_arena(), 8,  0, NULL);
  register_library_intrinsics();
}

//---------------------------init_start----------------------------------------
// Install the StartNode on this compile object.
void Compile::init_start(StartNode* s) {
  if (failing())
    return; // already failing
  assert(s == start(), "");
}

StartNode* Compile::start() const {
  assert(!failing(), "");
  for (DUIterator_Fast imax, i = root()->fast_outs(imax); i < imax; i++) {
    Node* start = root()->fast_out(i);
    if( start->is_Start() )
      return start->as_Start();
  }
  ShouldNotReachHere();
  return NULL;
}

//-------------------------------immutable_memory-------------------------------------
// Access immutable memory
Node* Compile::immutable_memory() {
  if (_immutable_memory != NULL) {
    return _immutable_memory;
  }
  StartNode* s = start();
  for (DUIterator_Fast imax, i = s->fast_outs(imax); true; i++) {
    Node *p = s->fast_out(i);
    if (p != s && p->as_Proj()->_con == TypeFunc::Memory) {
      _immutable_memory = p;
      return _immutable_memory;
    }
  }
  ShouldNotReachHere();
  return NULL;
}

//----------------------set_cached_top_node------------------------------------
// Install the cached top node, and make sure Node::is_top works correctly.
void Compile::set_cached_top_node(Node* tn) {
  if (tn != NULL)  verify_top(tn);
  Node* old_top = _top;
  _top = tn;
  // Calling Node::setup_is_top allows the nodes the chance to adjust
  // their _out arrays.
  if (_top != NULL)     _top->setup_is_top();
  if (old_top != NULL)  old_top->setup_is_top();
  assert(_top == NULL || top()->is_top(), "");
}

#ifndef PRODUCT
void Compile::verify_top(Node* tn) const {
  if (tn != NULL) {
    assert(tn->is_Con(), "top node must be a constant");
    assert(((ConNode*)tn)->type() == Type::TOP, "top node must have correct type");
    assert(tn->in(0) != NULL, "must have live top node");
  }
}
#endif


///-------------------Managing Per-Node Debug & Profile Info-------------------

void Compile::grow_node_notes(GrowableArray<Node_Notes*>* arr, int grow_by) {
  guarantee(arr != NULL, "");
  int num_blocks = arr->length();
  if (grow_by < num_blocks)  grow_by = num_blocks;
  int num_notes = grow_by * _node_notes_block_size;
  Node_Notes* notes = NEW_ARENA_ARRAY(node_arena(), Node_Notes, num_notes);
  Copy::zero_to_bytes(notes, num_notes * sizeof(Node_Notes));
  while (num_notes > 0) {
    arr->append(notes);
    notes     += _node_notes_block_size;
    num_notes -= _node_notes_block_size;
  }
  assert(num_notes == 0, "exact multiple, please");
}

bool Compile::copy_node_notes_to(Node* dest, Node* source) {
  if (source == NULL || dest == NULL)  return false;

  if (dest->is_Con())
    return false;               // Do not push debug info onto constants.

#ifdef ASSERT
  // Leave a bread crumb trail pointing to the original node:
  if (dest != NULL && dest != source && dest->debug_orig() == NULL) {
    dest->set_debug_orig(source);
  }
#endif

  if (node_note_array() == NULL)
    return false;               // Not collecting any notes now.

  // This is a copy onto a pre-existing node, which may already have notes.
  // If both nodes have notes, do not overwrite any pre-existing notes.
  Node_Notes* source_notes = node_notes_at(source->_idx);
  if (source_notes == NULL || source_notes->is_clear())  return false;
  Node_Notes* dest_notes   = node_notes_at(dest->_idx);
  if (dest_notes == NULL || dest_notes->is_clear()) {
    return set_node_notes_at(dest->_idx, source_notes);
  }

  Node_Notes merged_notes = (*source_notes);
  // The order of operations here ensures that dest notes will win...
  merged_notes.update_from(dest_notes);
  return set_node_notes_at(dest->_idx, &merged_notes);
}


//--------------------------allow_range_check_smearing-------------------------
// Gating condition for coalescing similar range checks.
// Sometimes we try 'speculatively' replacing a series of a range checks by a
// single covering check that is at least as strong as any of them.
// If the optimization succeeds, the simplified (strengthened) range check
// will always succeed.  If it fails, we will deopt, and then give up
// on the optimization.
bool Compile::allow_range_check_smearing() const {
  // If this method has already thrown a range-check,
  // assume it was because we already tried range smearing
  // and it failed.
  uint already_trapped = trap_count(Deoptimization::Reason_range_check);
  return !already_trapped;
}


//------------------------------flatten_alias_type-----------------------------
const TypePtr *Compile::flatten_alias_type( const TypePtr *tj ) const {
  int offset = tj->offset();
  TypePtr::PTR ptr = tj->ptr();

  // Process weird unsafe references.
  if (offset == Type::OffsetBot && (tj->isa_instptr() /*|| tj->isa_klassptr()*/)) {
    assert(InlineUnsafeOps, "indeterminate pointers come only from unsafe ops");
    tj = TypeOopPtr::BOTTOM;
    ptr = tj->ptr();
    offset = tj->offset();
  }

  // Array pointers need some flattening
  const TypeAryPtr *ta = tj->isa_aryptr();
  if( ta && _AliasLevel >= 2 ) {
    // For arrays indexed by constant indices, we flatten the alias
    // space to include all of the array body.  Only the header, klass
    // and array length can be accessed un-aliased.
    if( offset != Type::OffsetBot ) {
      if( ta->const_oop() ) { // methodDataOop or methodOop
        offset = Type::OffsetBot;   // Flatten constant access into array body
        tj = ta = TypeAryPtr::make(ptr,ta->const_oop(),ta->ary(),ta->klass(),false,Type::OffsetBot, ta->instance_id());
      } else if( offset == arrayOopDesc::length_offset_in_bytes() ) {
        // range is OK as-is.
        tj = ta = TypeAryPtr::RANGE;
      } else if( offset == oopDesc::klass_offset_in_bytes() ) {
        tj = TypeInstPtr::KLASS; // all klass loads look alike
        ta = TypeAryPtr::RANGE; // generic ignored junk
        ptr = TypePtr::BotPTR;
      } else if( offset == oopDesc::mark_offset_in_bytes() ) {
        tj = TypeInstPtr::MARK;
        ta = TypeAryPtr::RANGE; // generic ignored junk
        ptr = TypePtr::BotPTR;
      } else {                  // Random constant offset into array body
        offset = Type::OffsetBot;   // Flatten constant access into array body
        tj = ta = TypeAryPtr::make(ptr,ta->ary(),ta->klass(),false,Type::OffsetBot, ta->instance_id());
      }
    }
    // Arrays of fixed size alias with arrays of unknown size.
    if (ta->size() != TypeInt::POS) {
      const TypeAry *tary = TypeAry::make(ta->elem(), TypeInt::POS);
      tj = ta = TypeAryPtr::make(ptr,ta->const_oop(),tary,ta->klass(),false,offset, ta->instance_id());
    }
    // Arrays of known objects become arrays of unknown objects.
    if (ta->elem()->isa_narrowoop() && ta->elem() != TypeNarrowOop::BOTTOM) {
      const TypeAry *tary = TypeAry::make(TypeNarrowOop::BOTTOM, ta->size());
      tj = ta = TypeAryPtr::make(ptr,ta->const_oop(),tary,NULL,false,offset, ta->instance_id());
    }
    if (ta->elem()->isa_oopptr() && ta->elem() != TypeInstPtr::BOTTOM) {
      const TypeAry *tary = TypeAry::make(TypeInstPtr::BOTTOM, ta->size());
      tj = ta = TypeAryPtr::make(ptr,ta->const_oop(),tary,NULL,false,offset, ta->instance_id());
    }
    // Arrays of bytes and of booleans both use 'bastore' and 'baload' so
    // cannot be distinguished by bytecode alone.
    if (ta->elem() == TypeInt::BOOL) {
      const TypeAry *tary = TypeAry::make(TypeInt::BYTE, ta->size());
      ciKlass* aklass = ciTypeArrayKlass::make(T_BYTE);
      tj = ta = TypeAryPtr::make(ptr,ta->const_oop(),tary,aklass,false,offset, ta->instance_id());
    }
    // During the 2nd round of IterGVN, NotNull castings are removed.
    // Make sure the Bottom and NotNull variants alias the same.
    // Also, make sure exact and non-exact variants alias the same.
    if( ptr == TypePtr::NotNull || ta->klass_is_exact() ) {
      if (ta->const_oop()) {
        tj = ta = TypeAryPtr::make(TypePtr::Constant,ta->const_oop(),ta->ary(),ta->klass(),false,offset);
      } else {
        tj = ta = TypeAryPtr::make(TypePtr::BotPTR,ta->ary(),ta->klass(),false,offset);
      }
    }
  }

  // Oop pointers need some flattening
  const TypeInstPtr *to = tj->isa_instptr();
  if( to && _AliasLevel >= 2 && to != TypeOopPtr::BOTTOM ) {
    if( ptr == TypePtr::Constant ) {
      // No constant oop pointers (such as Strings); they alias with
      // unknown strings.
      tj = to = TypeInstPtr::make(TypePtr::BotPTR,to->klass(),false,0,offset);
    } else if( to->is_instance_field() ) {
      tj = to; // Keep NotNull and klass_is_exact for instance type
    } else if( ptr == TypePtr::NotNull || to->klass_is_exact() ) {
      // During the 2nd round of IterGVN, NotNull castings are removed.
      // Make sure the Bottom and NotNull variants alias the same.
      // Also, make sure exact and non-exact variants alias the same.
      tj = to = TypeInstPtr::make(TypePtr::BotPTR,to->klass(),false,0,offset, to->instance_id());
    }
    // Canonicalize the holder of this field
    ciInstanceKlass *k = to->klass()->as_instance_klass();
    if (offset >= 0 && offset < instanceOopDesc::base_offset_in_bytes()) {
      // First handle header references such as a LoadKlassNode, even if the
      // object's klass is unloaded at compile time (4965979).
      tj = to = TypeInstPtr::make(TypePtr::BotPTR, env()->Object_klass(), false, NULL, offset, to->instance_id());
    } else if (offset < 0 || offset >= k->size_helper() * wordSize) {
      to = NULL;
      tj = TypeOopPtr::BOTTOM;
      offset = tj->offset();
    } else {
      ciInstanceKlass *canonical_holder = k->get_canonical_holder(offset);
      if (!k->equals(canonical_holder) || tj->offset() != offset) {
        tj = to = TypeInstPtr::make(to->ptr(), canonical_holder, false, NULL, offset, to->instance_id());
      }
    }
  }

  // Klass pointers to object array klasses need some flattening
  const TypeKlassPtr *tk = tj->isa_klassptr();
  if( tk ) {
    // If we are referencing a field within a Klass, we need
    // to assume the worst case of an Object.  Both exact and
    // inexact types must flatten to the same alias class.
    // Since the flattened result for a klass is defined to be
    // precisely java.lang.Object, use a constant ptr.
    if ( offset == Type::OffsetBot || (offset >= 0 && (size_t)offset < sizeof(Klass)) ) {

      tj = tk = TypeKlassPtr::make(TypePtr::Constant,
                                   TypeKlassPtr::OBJECT->klass(),
                                   offset);
    }

    ciKlass* klass = tk->klass();
    if( klass->is_obj_array_klass() ) {
      ciKlass* k = TypeAryPtr::OOPS->klass();
      if( !k || !k->is_loaded() )                  // Only fails for some -Xcomp runs
        k = TypeInstPtr::BOTTOM->klass();
      tj = tk = TypeKlassPtr::make( TypePtr::NotNull, k, offset );
    }

    // Check for precise loads from the primary supertype array and force them
    // to the supertype cache alias index.  Check for generic array loads from
    // the primary supertype array and also force them to the supertype cache
    // alias index.  Since the same load can reach both, we need to merge
    // these 2 disparate memories into the same alias class.  Since the
    // primary supertype array is read-only, there's no chance of confusion
    // where we bypass an array load and an array store.
    uint off2 = offset - Klass::primary_supers_offset_in_bytes();
    if( offset == Type::OffsetBot ||
        off2 < Klass::primary_super_limit()*wordSize ) {
      offset = sizeof(oopDesc) +Klass::secondary_super_cache_offset_in_bytes();
      tj = tk = TypeKlassPtr::make( TypePtr::NotNull, tk->klass(), offset );
    }
  }

  // Flatten all Raw pointers together.
  if (tj->base() == Type::RawPtr)
    tj = TypeRawPtr::BOTTOM;

  if (tj->base() == Type::AnyPtr)
    tj = TypePtr::BOTTOM;      // An error, which the caller must check for.

  // Flatten all to bottom for now
  switch( _AliasLevel ) {
  case 0:
    tj = TypePtr::BOTTOM;
    break;
  case 1:                       // Flatten to: oop, static, field or array
    switch (tj->base()) {
    //case Type::AryPtr: tj = TypeAryPtr::RANGE;    break;
    case Type::RawPtr:   tj = TypeRawPtr::BOTTOM;   break;
    case Type::AryPtr:   // do not distinguish arrays at all
    case Type::InstPtr:  tj = TypeInstPtr::BOTTOM;  break;
    case Type::KlassPtr: tj = TypeKlassPtr::OBJECT; break;
    case Type::AnyPtr:   tj = TypePtr::BOTTOM;      break;  // caller checks it
    default: ShouldNotReachHere();
    }
    break;
  case 2:                       // No collasping at level 2; keep all splits
  case 3:                       // No collasping at level 3; keep all splits
    break;
  default:
    Unimplemented();
  }

  offset = tj->offset();
  assert( offset != Type::OffsetTop, "Offset has fallen from constant" );

  assert( (offset != Type::OffsetBot && tj->base() != Type::AryPtr) ||
          (offset == Type::OffsetBot && tj->base() == Type::AryPtr) ||
          (offset == Type::OffsetBot && tj == TypeOopPtr::BOTTOM) ||
          (offset == Type::OffsetBot && tj == TypePtr::BOTTOM) ||
          (offset == oopDesc::mark_offset_in_bytes() && tj->base() == Type::AryPtr) ||
          (offset == oopDesc::klass_offset_in_bytes() && tj->base() == Type::AryPtr) ||
          (offset == arrayOopDesc::length_offset_in_bytes() && tj->base() == Type::AryPtr)  ,
          "For oops, klasses, raw offset must be constant; for arrays the offset is never known" );
  assert( tj->ptr() != TypePtr::TopPTR &&
          tj->ptr() != TypePtr::AnyNull &&
          tj->ptr() != TypePtr::Null, "No imprecise addresses" );
//    assert( tj->ptr() != TypePtr::Constant ||
//            tj->base() == Type::RawPtr ||
//            tj->base() == Type::KlassPtr, "No constant oop addresses" );

  return tj;
}

void Compile::AliasType::Init(int i, const TypePtr* at) {
  _index = i;
  _adr_type = at;
  _field = NULL;
  _is_rewritable = true; // default
  const TypeOopPtr *atoop = (at != NULL) ? at->isa_oopptr() : NULL;
  if (atoop != NULL && atoop->is_instance()) {
    const TypeOopPtr *gt = atoop->cast_to_instance(TypeOopPtr::UNKNOWN_INSTANCE);
    _general_index = Compile::current()->get_alias_index(gt);
  } else {
    _general_index = 0;
  }
}

//---------------------------------print_on------------------------------------
#ifndef PRODUCT
void Compile::AliasType::print_on(outputStream* st) {
  if (index() < 10)
        st->print("@ <%d> ", index());
  else  st->print("@ <%d>",  index());
  st->print(is_rewritable() ? "   " : " RO");
  int offset = adr_type()->offset();
  if (offset == Type::OffsetBot)
        st->print(" +any");
  else  st->print(" +%-3d", offset);
  st->print(" in ");
  adr_type()->dump_on(st);
  const TypeOopPtr* tjp = adr_type()->isa_oopptr();
  if (field() != NULL && tjp) {
    if (tjp->klass()  != field()->holder() ||
        tjp->offset() != field()->offset_in_bytes()) {
      st->print(" != ");
      field()->print();
      st->print(" ***");
    }
  }
}

void print_alias_types() {
  Compile* C = Compile::current();
  tty->print_cr("--- Alias types, AliasIdxBot .. %d", C->num_alias_types()-1);
  for (int idx = Compile::AliasIdxBot; idx < C->num_alias_types(); idx++) {
    C->alias_type(idx)->print_on(tty);
    tty->cr();
  }
}
#endif


//----------------------------probe_alias_cache--------------------------------
Compile::AliasCacheEntry* Compile::probe_alias_cache(const TypePtr* adr_type) {
  intptr_t key = (intptr_t) adr_type;
  key ^= key >> logAliasCacheSize;
  return &_alias_cache[key & right_n_bits(logAliasCacheSize)];
}


//-----------------------------grow_alias_types--------------------------------
void Compile::grow_alias_types() {
  const int old_ats  = _max_alias_types; // how many before?
  const int new_ats  = old_ats;          // how many more?
  const int grow_ats = old_ats+new_ats;  // how many now?
  _max_alias_types = grow_ats;
  _alias_types =  REALLOC_ARENA_ARRAY(comp_arena(), AliasType*, _alias_types, old_ats, grow_ats);
  AliasType* ats =    NEW_ARENA_ARRAY(comp_arena(), AliasType, new_ats);
  Copy::zero_to_bytes(ats, sizeof(AliasType)*new_ats);
  for (int i = 0; i < new_ats; i++)  _alias_types[old_ats+i] = &ats[i];
}


//--------------------------------find_alias_type------------------------------
Compile::AliasType* Compile::find_alias_type(const TypePtr* adr_type, bool no_create) {
  if (_AliasLevel == 0)
    return alias_type(AliasIdxBot);

  AliasCacheEntry* ace = probe_alias_cache(adr_type);
  if (ace->_adr_type == adr_type) {
    return alias_type(ace->_index);
  }

  // Handle special cases.
  if (adr_type == NULL)             return alias_type(AliasIdxTop);
  if (adr_type == TypePtr::BOTTOM)  return alias_type(AliasIdxBot);

  // Do it the slow way.
  const TypePtr* flat = flatten_alias_type(adr_type);

#ifdef ASSERT
  assert(flat == flatten_alias_type(flat), "idempotent");
  assert(flat != TypePtr::BOTTOM,     "cannot alias-analyze an untyped ptr");
  if (flat->isa_oopptr() && !flat->isa_klassptr()) {
    const TypeOopPtr* foop = flat->is_oopptr();
    const TypePtr* xoop = foop->cast_to_exactness(!foop->klass_is_exact())->is_ptr();
    assert(foop == flatten_alias_type(xoop), "exactness must not affect alias type");
  }
  assert(flat == flatten_alias_type(flat), "exact bit doesn't matter");
#endif

  int idx = AliasIdxTop;
  for (int i = 0; i < num_alias_types(); i++) {
    if (alias_type(i)->adr_type() == flat) {
      idx = i;
      break;
    }
  }

  if (idx == AliasIdxTop) {
    if (no_create)  return NULL;
    // Grow the array if necessary.
    if (_num_alias_types == _max_alias_types)  grow_alias_types();
    // Add a new alias type.
    idx = _num_alias_types++;
    _alias_types[idx]->Init(idx, flat);
    if (flat == TypeInstPtr::KLASS)  alias_type(idx)->set_rewritable(false);
    if (flat == TypeAryPtr::RANGE)   alias_type(idx)->set_rewritable(false);
    if (flat->isa_instptr()) {
      if (flat->offset() == java_lang_Class::klass_offset_in_bytes()
          && flat->is_instptr()->klass() == env()->Class_klass())
        alias_type(idx)->set_rewritable(false);
    }
    if (flat->isa_klassptr()) {
      if (flat->offset() == Klass::super_check_offset_offset_in_bytes() + (int)sizeof(oopDesc))
        alias_type(idx)->set_rewritable(false);
      if (flat->offset() == Klass::modifier_flags_offset_in_bytes() + (int)sizeof(oopDesc))
        alias_type(idx)->set_rewritable(false);
      if (flat->offset() == Klass::access_flags_offset_in_bytes() + (int)sizeof(oopDesc))
        alias_type(idx)->set_rewritable(false);
      if (flat->offset() == Klass::java_mirror_offset_in_bytes() + (int)sizeof(oopDesc))
        alias_type(idx)->set_rewritable(false);
    }
    // %%% (We would like to finalize JavaThread::threadObj_offset(),
    // but the base pointer type is not distinctive enough to identify
    // references into JavaThread.)

    // Check for final instance fields.
    const TypeInstPtr* tinst = flat->isa_instptr();
    if (tinst && tinst->offset() >= instanceOopDesc::base_offset_in_bytes()) {
      ciInstanceKlass *k = tinst->klass()->as_instance_klass();
      ciField* field = k->get_field_by_offset(tinst->offset(), false);
      // Set field() and is_rewritable() attributes.
      if (field != NULL)  alias_type(idx)->set_field(field);
    }
    const TypeKlassPtr* tklass = flat->isa_klassptr();
    // Check for final static fields.
    if (tklass && tklass->klass()->is_instance_klass()) {
      ciInstanceKlass *k = tklass->klass()->as_instance_klass();
      ciField* field = k->get_field_by_offset(tklass->offset(), true);
      // Set field() and is_rewritable() attributes.
      if (field != NULL)   alias_type(idx)->set_field(field);
    }
  }

  // Fill the cache for next time.
  ace->_adr_type = adr_type;
  ace->_index    = idx;
  assert(alias_type(adr_type) == alias_type(idx),  "type must be installed");

  // Might as well try to fill the cache for the flattened version, too.
  AliasCacheEntry* face = probe_alias_cache(flat);
  if (face->_adr_type == NULL) {
    face->_adr_type = flat;
    face->_index    = idx;
    assert(alias_type(flat) == alias_type(idx), "flat type must work too");
  }

  return alias_type(idx);
}


Compile::AliasType* Compile::alias_type(ciField* field) {
  const TypeOopPtr* t;
  if (field->is_static())
    t = TypeKlassPtr::make(field->holder());
  else
    t = TypeOopPtr::make_from_klass_raw(field->holder());
  AliasType* atp = alias_type(t->add_offset(field->offset_in_bytes()));
  assert(field->is_final() == !atp->is_rewritable(), "must get the rewritable bits correct");
  return atp;
}


//------------------------------have_alias_type--------------------------------
bool Compile::have_alias_type(const TypePtr* adr_type) {
  AliasCacheEntry* ace = probe_alias_cache(adr_type);
  if (ace->_adr_type == adr_type) {
    return true;
  }

  // Handle special cases.
  if (adr_type == NULL)             return true;
  if (adr_type == TypePtr::BOTTOM)  return true;

  return find_alias_type(adr_type, true) != NULL;
}

//-----------------------------must_alias--------------------------------------
// True if all values of the given address type are in the given alias category.
bool Compile::must_alias(const TypePtr* adr_type, int alias_idx) {
  if (alias_idx == AliasIdxBot)         return true;  // the universal category
  if (adr_type == NULL)                 return true;  // NULL serves as TypePtr::TOP
  if (alias_idx == AliasIdxTop)         return false; // the empty category
  if (adr_type->base() == Type::AnyPtr) return false; // TypePtr::BOTTOM or its twins

  // the only remaining possible overlap is identity
  int adr_idx = get_alias_index(adr_type);
  assert(adr_idx != AliasIdxBot && adr_idx != AliasIdxTop, "");
  assert(adr_idx == alias_idx ||
         (alias_type(alias_idx)->adr_type() != TypeOopPtr::BOTTOM
          && adr_type                       != TypeOopPtr::BOTTOM),
         "should not be testing for overlap with an unsafe pointer");
  return adr_idx == alias_idx;
}

//------------------------------can_alias--------------------------------------
// True if any values of the given address type are in the given alias category.
bool Compile::can_alias(const TypePtr* adr_type, int alias_idx) {
  if (alias_idx == AliasIdxTop)         return false; // the empty category
  if (adr_type == NULL)                 return false; // NULL serves as TypePtr::TOP
  if (alias_idx == AliasIdxBot)         return true;  // the universal category
  if (adr_type->base() == Type::AnyPtr) return true;  // TypePtr::BOTTOM or its twins

  // the only remaining possible overlap is identity
  int adr_idx = get_alias_index(adr_type);
  assert(adr_idx != AliasIdxBot && adr_idx != AliasIdxTop, "");
  return adr_idx == alias_idx;
}



//---------------------------pop_warm_call-------------------------------------
WarmCallInfo* Compile::pop_warm_call() {
  WarmCallInfo* wci = _warm_calls;
  if (wci != NULL)  _warm_calls = wci->remove_from(wci);
  return wci;
}

//----------------------------Inline_Warm--------------------------------------
int Compile::Inline_Warm() {
  // If there is room, try to inline some more warm call sites.
  // %%% Do a graph index compaction pass when we think we're out of space?
  if (!InlineWarmCalls)  return 0;

  int calls_made_hot = 0;
  int room_to_grow   = NodeCountInliningCutoff - unique();
  int amount_to_grow = MIN2(room_to_grow, (int)NodeCountInliningStep);
  int amount_grown   = 0;
  WarmCallInfo* call;
  while (amount_to_grow > 0 && (call = pop_warm_call()) != NULL) {
    int est_size = (int)call->size();
    if (est_size > (room_to_grow - amount_grown)) {
      // This one won't fit anyway.  Get rid of it.
      call->make_cold();
      continue;
    }
    call->make_hot();
    calls_made_hot++;
    amount_grown   += est_size;
    amount_to_grow -= est_size;
  }

  if (calls_made_hot > 0)  set_major_progress();
  return calls_made_hot;
}


//----------------------------Finish_Warm--------------------------------------
void Compile::Finish_Warm() {
  if (!InlineWarmCalls)  return;
  if (failing())  return;
  if (warm_calls() == NULL)  return;

  // Clean up loose ends, if we are out of space for inlining.
  WarmCallInfo* call;
  while ((call = pop_warm_call()) != NULL) {
    call->make_cold();
  }
}


//------------------------------Optimize---------------------------------------
// Given a graph, optimize it.
void Compile::Optimize() {
  TracePhase t1("optimizer", &_t_optimizer, true);

#ifndef PRODUCT
  if (env()->break_at_compile()) {
    BREAKPOINT;
  }

#endif

  ResourceMark rm;
  int          loop_opts_cnt;

  NOT_PRODUCT( verify_graph_edges(); )

  print_method("Start");

 {
  // Iterative Global Value Numbering, including ideal transforms
  // Initialize IterGVN with types and values from parse-time GVN
  PhaseIterGVN igvn(initial_gvn());
  {
    NOT_PRODUCT( TracePhase t2("iterGVN", &_t_iterGVN, TimeCompiler); )
    igvn.optimize();
  }

  print_method("Iter GVN 1", 2);

  if (failing())  return;

  // get rid of the connection graph since it's information is not
  // updated by optimizations
  _congraph = NULL;


  // Loop transforms on the ideal graph.  Range Check Elimination,
  // peeling, unrolling, etc.

  // Set loop opts counter
  loop_opts_cnt = num_loop_opts();
  if((loop_opts_cnt > 0) && (has_loops() || has_split_ifs())) {
    {
      TracePhase t2("idealLoop", &_t_idealLoop, true);
      PhaseIdealLoop ideal_loop( igvn, NULL, true );
      loop_opts_cnt--;
      if (major_progress()) print_method("PhaseIdealLoop 1", 2);
      if (failing())  return;
    }
    // Loop opts pass if partial peeling occurred in previous pass
    if(PartialPeelLoop && major_progress() && (loop_opts_cnt > 0)) {
      TracePhase t3("idealLoop", &_t_idealLoop, true);
      PhaseIdealLoop ideal_loop( igvn, NULL, false );
      loop_opts_cnt--;
      if (major_progress()) print_method("PhaseIdealLoop 2", 2);
      if (failing())  return;
    }
    // Loop opts pass for loop-unrolling before CCP
    if(major_progress() && (loop_opts_cnt > 0)) {
      TracePhase t4("idealLoop", &_t_idealLoop, true);
      PhaseIdealLoop ideal_loop( igvn, NULL, false );
      loop_opts_cnt--;
      if (major_progress()) print_method("PhaseIdealLoop 3", 2);
    }
  }
  if (failing())  return;

  // Conditional Constant Propagation;
  PhaseCCP ccp( &igvn );
  assert( true, "Break here to ccp.dump_nodes_and_types(_root,999,1)");
  {
    TracePhase t2("ccp", &_t_ccp, true);
    ccp.do_transform();
  }
  print_method("PhaseCPP 1", 2);

  assert( true, "Break here to ccp.dump_old2new_map()");

  // Iterative Global Value Numbering, including ideal transforms
  {
    NOT_PRODUCT( TracePhase t2("iterGVN2", &_t_iterGVN2, TimeCompiler); )
    igvn = ccp;
    igvn.optimize();
  }

  print_method("Iter GVN 2", 2);

  if (failing())  return;

  // Loop transforms on the ideal graph.  Range Check Elimination,
  // peeling, unrolling, etc.
  if(loop_opts_cnt > 0) {
    debug_only( int cnt = 0; );
    while(major_progress() && (loop_opts_cnt > 0)) {
      TracePhase t2("idealLoop", &_t_idealLoop, true);
      assert( cnt++ < 40, "infinite cycle in loop optimization" );
      PhaseIdealLoop ideal_loop( igvn, NULL, true );
      loop_opts_cnt--;
      if (major_progress()) print_method("PhaseIdealLoop iterations", 2);
      if (failing())  return;
    }
  }
  {
    NOT_PRODUCT( TracePhase t2("macroExpand", &_t_macroExpand, TimeCompiler); )
    PhaseMacroExpand  mex(igvn);
    if (mex.expand_macro_nodes()) {
      assert(failing(), "must bail out w/ explicit message");
      return;
    }
  }

 } // (End scope of igvn; run destructor if necessary for asserts.)

  // A method with only infinite loops has no edges entering loops from root
  {
    NOT_PRODUCT( TracePhase t2("graphReshape", &_t_graphReshaping, TimeCompiler); )
    if (final_graph_reshaping()) {
      assert(failing(), "must bail out w/ explicit message");
      return;
    }
  }

  print_method("Optimize finished", 2);
}


//------------------------------Code_Gen---------------------------------------
// Given a graph, generate code for it
void Compile::Code_Gen() {
  if (failing())  return;

  // Perform instruction selection.  You might think we could reclaim Matcher
  // memory PDQ, but actually the Matcher is used in generating spill code.
  // Internals of the Matcher (including some VectorSets) must remain live
  // for awhile - thus I cannot reclaim Matcher memory lest a VectorSet usage
  // set a bit in reclaimed memory.

  // In debug mode can dump m._nodes.dump() for mapping of ideal to machine
  // nodes.  Mapping is only valid at the root of each matched subtree.
  NOT_PRODUCT( verify_graph_edges(); )

  Node_List proj_list;
  Matcher m(proj_list);
  _matcher = &m;
  {
    TracePhase t2("matcher", &_t_matcher, true);
    m.match();
  }
  // In debug mode can dump m._nodes.dump() for mapping of ideal to machine
  // nodes.  Mapping is only valid at the root of each matched subtree.
  NOT_PRODUCT( verify_graph_edges(); )

  // If you have too many nodes, or if matching has failed, bail out
  check_node_count(0, "out of nodes matching instructions");
  if (failing())  return;

  // Build a proper-looking CFG
  PhaseCFG cfg(node_arena(), root(), m);
  _cfg = &cfg;
  {
    NOT_PRODUCT( TracePhase t2("scheduler", &_t_scheduler, TimeCompiler); )
    cfg.Dominators();
    if (failing())  return;

    NOT_PRODUCT( verify_graph_edges(); )

    cfg.Estimate_Block_Frequency();
    cfg.GlobalCodeMotion(m,unique(),proj_list);

    print_method("Global code motion", 2);

    if (failing())  return;
    NOT_PRODUCT( verify_graph_edges(); )

    debug_only( cfg.verify(); )
  }
  NOT_PRODUCT( verify_graph_edges(); )

  PhaseChaitin regalloc(unique(),cfg,m);
  _regalloc = &regalloc;
  {
    TracePhase t2("regalloc", &_t_registerAllocation, true);
    // Perform any platform dependent preallocation actions.  This is used,
    // for example, to avoid taking an implicit null pointer exception
    // using the frame pointer on win95.
    _regalloc->pd_preallocate_hook();

    // Perform register allocation.  After Chaitin, use-def chains are
    // no longer accurate (at spill code) and so must be ignored.
    // Node->LRG->reg mappings are still accurate.
    _regalloc->Register_Allocate();

    // Bail out if the allocator builds too many nodes
    if (failing())  return;
  }

  // Prior to register allocation we kept empty basic blocks in case the
  // the allocator needed a place to spill.  After register allocation we
  // are not adding any new instructions.  If any basic block is empty, we
  // can now safely remove it.
  {
    NOT_PRODUCT( TracePhase t2("removeEmpty", &_t_removeEmptyBlocks, TimeCompiler); )
    cfg.RemoveEmpty();
  }

  // Perform any platform dependent postallocation verifications.
  debug_only( _regalloc->pd_postallocate_verify_hook(); )

  // Apply peephole optimizations
  if( OptoPeephole ) {
    NOT_PRODUCT( TracePhase t2("peephole", &_t_peephole, TimeCompiler); )
    PhasePeephole peep( _regalloc, cfg);
    peep.do_transform();
  }

  // Convert Nodes to instruction bits in a buffer
  {
    // %%%% workspace merge brought two timers together for one job
    TracePhase t2a("output", &_t_output, true);
    NOT_PRODUCT( TraceTime t2b(NULL, &_t_codeGeneration, TimeCompiler, false); )
    Output();
  }

  print_method("End");

  // He's dead, Jim.
  _cfg     = (PhaseCFG*)0xdeadbeef;
  _regalloc = (PhaseChaitin*)0xdeadbeef;
}


//------------------------------dump_asm---------------------------------------
// Dump formatted assembly
#ifndef PRODUCT
void Compile::dump_asm(int *pcs, uint pc_limit) {
  bool cut_short = false;
  tty->print_cr("#");
  tty->print("#  ");  _tf->dump();  tty->cr();
  tty->print_cr("#");

  // For all blocks
  int pc = 0x0;                 // Program counter
  char starts_bundle = ' ';
  _regalloc->dump_frame();

  Node *n = NULL;
  for( uint i=0; i<_cfg->_num_blocks; i++ ) {
    if (VMThread::should_terminate()) { cut_short = true; break; }
    Block *b = _cfg->_blocks[i];
    if (b->is_connector() && !Verbose) continue;
    n = b->_nodes[0];
    if (pcs && n->_idx < pc_limit)
      tty->print("%3.3x   ", pcs[n->_idx]);
    else
      tty->print("      ");
    b->dump_head( &_cfg->_bbs );
    if (b->is_connector()) {
      tty->print_cr("        # Empty connector block");
    } else if (b->num_preds() == 2 && b->pred(1)->is_CatchProj() && b->pred(1)->as_CatchProj()->_con == CatchProjNode::fall_through_index) {
      tty->print_cr("        # Block is sole successor of call");
    }

    // For all instructions
    Node *delay = NULL;
    for( uint j = 0; j<b->_nodes.size(); j++ ) {
      if (VMThread::should_terminate()) { cut_short = true; break; }
      n = b->_nodes[j];
      if (valid_bundle_info(n)) {
        Bundle *bundle = node_bundling(n);
        if (bundle->used_in_unconditional_delay()) {
          delay = n;
          continue;
        }
        if (bundle->starts_bundle())
          starts_bundle = '+';
      }

      if (WizardMode) n->dump();

      if( !n->is_Region() &&    // Dont print in the Assembly
          !n->is_Phi() &&       // a few noisely useless nodes
          !n->is_Proj() &&
          !n->is_MachTemp() &&
          !n->is_Catch() &&     // Would be nice to print exception table targets
          !n->is_MergeMem() &&  // Not very interesting
          !n->is_top() &&       // Debug info table constants
          !(n->is_Con() && !n->is_Mach())// Debug info table constants
          ) {
        if (pcs && n->_idx < pc_limit)
          tty->print("%3.3x", pcs[n->_idx]);
        else
          tty->print("   ");
        tty->print(" %c ", starts_bundle);
        starts_bundle = ' ';
        tty->print("\t");
        n->format(_regalloc, tty);
        tty->cr();
      }

      // If we have an instruction with a delay slot, and have seen a delay,
      // then back up and print it
      if (valid_bundle_info(n) && node_bundling(n)->use_unconditional_delay()) {
        assert(delay != NULL, "no unconditional delay instruction");
        if (WizardMode) delay->dump();

        if (node_bundling(delay)->starts_bundle())
          starts_bundle = '+';
        if (pcs && n->_idx < pc_limit)
          tty->print("%3.3x", pcs[n->_idx]);
        else
          tty->print("   ");
        tty->print(" %c ", starts_bundle);
        starts_bundle = ' ';
        tty->print("\t");
        delay->format(_regalloc, tty);
        tty->print_cr("");
        delay = NULL;
      }

      // Dump the exception table as well
      if( n->is_Catch() && (Verbose || WizardMode) ) {
        // Print the exception table for this offset
        _handler_table.print_subtable_for(pc);
      }
    }

    if (pcs && n->_idx < pc_limit)
      tty->print_cr("%3.3x", pcs[n->_idx]);
    else
      tty->print_cr("");

    assert(cut_short || delay == NULL, "no unconditional delay branch");

  } // End of per-block dump
  tty->print_cr("");

  if (cut_short)  tty->print_cr("*** disassembly is cut short ***");
}
#endif

//------------------------------Final_Reshape_Counts---------------------------
// This class defines counters to help identify when a method
// may/must be executed using hardware with only 24-bit precision.
struct Final_Reshape_Counts : public StackObj {
  int  _call_count;             // count non-inlined 'common' calls
  int  _float_count;            // count float ops requiring 24-bit precision
  int  _double_count;           // count double ops requiring more precision
  int  _java_call_count;        // count non-inlined 'java' calls
  VectorSet _visited;           // Visitation flags
  Node_List _tests;             // Set of IfNodes & PCTableNodes

  Final_Reshape_Counts() :
    _call_count(0), _float_count(0), _double_count(0), _java_call_count(0),
    _visited( Thread::current()->resource_area() ) { }

  void inc_call_count  () { _call_count  ++; }
  void inc_float_count () { _float_count ++; }
  void inc_double_count() { _double_count++; }
  void inc_java_call_count() { _java_call_count++; }

  int  get_call_count  () const { return _call_count  ; }
  int  get_float_count () const { return _float_count ; }
  int  get_double_count() const { return _double_count; }
  int  get_java_call_count() const { return _java_call_count; }
};

static bool oop_offset_is_sane(const TypeInstPtr* tp) {
  ciInstanceKlass *k = tp->klass()->as_instance_klass();
  // Make sure the offset goes inside the instance layout.
  return k->contains_field_offset(tp->offset());
  // Note that OffsetBot and OffsetTop are very negative.
}

//------------------------------final_graph_reshaping_impl----------------------
// Implement items 1-5 from final_graph_reshaping below.
static void final_graph_reshaping_impl( Node *n, Final_Reshape_Counts &fpu ) {

  if ( n->outcnt() == 0 ) return; // dead node
  uint nop = n->Opcode();

  // Check for 2-input instruction with "last use" on right input.
  // Swap to left input.  Implements item (2).
  if( n->req() == 3 &&          // two-input instruction
      n->in(1)->outcnt() > 1 && // left use is NOT a last use
      (!n->in(1)->is_Phi() || n->in(1)->in(2) != n) && // it is not data loop
      n->in(2)->outcnt() == 1 &&// right use IS a last use
      !n->in(2)->is_Con() ) {   // right use is not a constant
    // Check for commutative opcode
    switch( nop ) {
    case Op_AddI:  case Op_AddF:  case Op_AddD:  case Op_AddL:
    case Op_MaxI:  case Op_MinI:
    case Op_MulI:  case Op_MulF:  case Op_MulD:  case Op_MulL:
    case Op_AndL:  case Op_XorL:  case Op_OrL:
    case Op_AndI:  case Op_XorI:  case Op_OrI: {
      // Move "last use" input to left by swapping inputs
      n->swap_edges(1, 2);
      break;
    }
    default:
      break;
    }
  }

  // Count FPU ops and common calls, implements item (3)
  switch( nop ) {
  // Count all float operations that may use FPU
  case Op_AddF:
  case Op_SubF:
  case Op_MulF:
  case Op_DivF:
  case Op_NegF:
  case Op_ModF:
  case Op_ConvI2F:
  case Op_ConF:
  case Op_CmpF:
  case Op_CmpF3:
  // case Op_ConvL2F: // longs are split into 32-bit halves
    fpu.inc_float_count();
    break;

  case Op_ConvF2D:
  case Op_ConvD2F:
    fpu.inc_float_count();
    fpu.inc_double_count();
    break;

  // Count all double operations that may use FPU
  case Op_AddD:
  case Op_SubD:
  case Op_MulD:
  case Op_DivD:
  case Op_NegD:
  case Op_ModD:
  case Op_ConvI2D:
  case Op_ConvD2I:
  // case Op_ConvL2D: // handled by leaf call
  // case Op_ConvD2L: // handled by leaf call
  case Op_ConD:
  case Op_CmpD:
  case Op_CmpD3:
    fpu.inc_double_count();
    break;
  case Op_Opaque1:              // Remove Opaque Nodes before matching
  case Op_Opaque2:              // Remove Opaque Nodes before matching
    n->subsume_by(n->in(1));
    break;
  case Op_CallStaticJava:
  case Op_CallJava:
  case Op_CallDynamicJava:
    fpu.inc_java_call_count(); // Count java call site;
  case Op_CallRuntime:
  case Op_CallLeaf:
  case Op_CallLeafNoFP: {
    assert( n->is_Call(), "" );
    CallNode *call = n->as_Call();
    // Count call sites where the FP mode bit would have to be flipped.
    // Do not count uncommon runtime calls:
    // uncommon_trap, _complete_monitor_locking, _complete_monitor_unlocking,
    // _new_Java, _new_typeArray, _new_objArray, _rethrow_Java, ...
    if( !call->is_CallStaticJava() || !call->as_CallStaticJava()->_name ) {
      fpu.inc_call_count();   // Count the call site
    } else {                  // See if uncommon argument is shared
      Node *n = call->in(TypeFunc::Parms);
      int nop = n->Opcode();
      // Clone shared simple arguments to uncommon calls, item (1).
      if( n->outcnt() > 1 &&
          !n->is_Proj() &&
          nop != Op_CreateEx &&
          nop != Op_CheckCastPP &&
          !n->is_Mem() ) {
        Node *x = n->clone();
        call->set_req( TypeFunc::Parms, x );
      }
    }
    break;
  }

  case Op_StoreD:
  case Op_LoadD:
  case Op_LoadD_unaligned:
    fpu.inc_double_count();
    goto handle_mem;
  case Op_StoreF:
  case Op_LoadF:
    fpu.inc_float_count();
    goto handle_mem;

  case Op_StoreB:
  case Op_StoreC:
  case Op_StoreCM:
  case Op_StorePConditional:
  case Op_StoreI:
  case Op_StoreL:
  case Op_StoreLConditional:
  case Op_CompareAndSwapI:
  case Op_CompareAndSwapL:
  case Op_CompareAndSwapP:
  case Op_CompareAndSwapN:
  case Op_StoreP:
  case Op_StoreN:
  case Op_LoadB:
  case Op_LoadC:
  case Op_LoadI:
  case Op_LoadKlass:
  case Op_LoadNKlass:
  case Op_LoadL:
  case Op_LoadL_unaligned:
  case Op_LoadPLocked:
  case Op_LoadLLocked:
  case Op_LoadP:
  case Op_LoadN:
  case Op_LoadRange:
  case Op_LoadS: {
  handle_mem:
#ifdef ASSERT
    if( VerifyOptoOopOffsets ) {
      assert( n->is_Mem(), "" );
      MemNode *mem  = (MemNode*)n;
      // Check to see if address types have grounded out somehow.
      const TypeInstPtr *tp = mem->in(MemNode::Address)->bottom_type()->isa_instptr();
      assert( !tp || oop_offset_is_sane(tp), "" );
    }
#endif
    break;
  }

  case Op_AddP: {               // Assert sane base pointers
    const Node *addp = n->in(AddPNode::Address);
    assert( !addp->is_AddP() ||
            addp->in(AddPNode::Base)->is_top() || // Top OK for allocation
            addp->in(AddPNode::Base) == n->in(AddPNode::Base),
            "Base pointers must match" );
    break;
  }

#ifdef _LP64
  case Op_CmpP:
    // Do this transformation here to preserve CmpPNode::sub() and
    // other TypePtr related Ideal optimizations (for example, ptr nullness).
    if( n->in(1)->is_DecodeN() ) {
      Compile* C = Compile::current();
      Node* in2 = NULL;
      if( n->in(2)->is_DecodeN() ) {
        in2 = n->in(2)->in(1);
      } else if ( n->in(2)->Opcode() == Op_ConP ) {
        const Type* t = n->in(2)->bottom_type();
        if (t == TypePtr::NULL_PTR) {
          Node *in1 = n->in(1);
          if (Matcher::clone_shift_expressions) {
            // x86, ARM and friends can handle 2 adds in addressing mode.
            // Decode a narrow oop and do implicit NULL check in address
            // [R12 + narrow_oop_reg<<3 + offset]
            in2 = ConNode::make(C, TypeNarrowOop::NULL_PTR);
          } else {
            // Don't replace CmpP(o ,null) if 'o' is used in AddP
            // to generate implicit NULL check on Sparc where
            // narrow oops can't be used in address.
            uint i = 0;
            for (; i < in1->outcnt(); i++) {
              if (in1->raw_out(i)->is_AddP())
                break;
            }
            if (i >= in1->outcnt()) {
              in2 = ConNode::make(C, TypeNarrowOop::NULL_PTR);
            }
          }
        } else if (t->isa_oopptr()) {
          in2 = ConNode::make(C, t->is_oopptr()->make_narrowoop());
        }
      }
      if( in2 != NULL ) {
        Node* cmpN = new (C, 3) CmpNNode(n->in(1)->in(1), in2);
        n->subsume_by( cmpN );
      }
    }
#endif

  case Op_ModI:
    if (UseDivMod) {
      // Check if a%b and a/b both exist
      Node* d = n->find_similar(Op_DivI);
      if (d) {
        // Replace them with a fused divmod if supported
        Compile* C = Compile::current();
        if (Matcher::has_match_rule(Op_DivModI)) {
          DivModINode* divmod = DivModINode::make(C, n);
          d->subsume_by(divmod->div_proj());
          n->subsume_by(divmod->mod_proj());
        } else {
          // replace a%b with a-((a/b)*b)
          Node* mult = new (C, 3) MulINode(d, d->in(2));
          Node* sub  = new (C, 3) SubINode(d->in(1), mult);
          n->subsume_by( sub );
        }
      }
    }
    break;

  case Op_ModL:
    if (UseDivMod) {
      // Check if a%b and a/b both exist
      Node* d = n->find_similar(Op_DivL);
      if (d) {
        // Replace them with a fused divmod if supported
        Compile* C = Compile::current();
        if (Matcher::has_match_rule(Op_DivModL)) {
          DivModLNode* divmod = DivModLNode::make(C, n);
          d->subsume_by(divmod->div_proj());
          n->subsume_by(divmod->mod_proj());
        } else {
          // replace a%b with a-((a/b)*b)
          Node* mult = new (C, 3) MulLNode(d, d->in(2));
          Node* sub  = new (C, 3) SubLNode(d->in(1), mult);
          n->subsume_by( sub );
        }
      }
    }
    break;

  case Op_Load16B:
  case Op_Load8B:
  case Op_Load4B:
  case Op_Load8S:
  case Op_Load4S:
  case Op_Load2S:
  case Op_Load8C:
  case Op_Load4C:
  case Op_Load2C:
  case Op_Load4I:
  case Op_Load2I:
  case Op_Load2L:
  case Op_Load4F:
  case Op_Load2F:
  case Op_Load2D:
  case Op_Store16B:
  case Op_Store8B:
  case Op_Store4B:
  case Op_Store8C:
  case Op_Store4C:
  case Op_Store2C:
  case Op_Store4I:
  case Op_Store2I:
  case Op_Store2L:
  case Op_Store4F:
  case Op_Store2F:
  case Op_Store2D:
    break;

  case Op_PackB:
  case Op_PackS:
  case Op_PackC:
  case Op_PackI:
  case Op_PackF:
  case Op_PackL:
  case Op_PackD:
    if (n->req()-1 > 2) {
      // Replace many operand PackNodes with a binary tree for matching
      PackNode* p = (PackNode*) n;
      Node* btp = p->binaryTreePack(Compile::current(), 1, n->req());
      n->subsume_by(btp);
    }
    break;
  default:
    assert( !n->is_Call(), "" );
    assert( !n->is_Mem(), "" );
    break;
  }

  // Collect CFG split points
  if (n->is_MultiBranch())
    fpu._tests.push(n);
}

//------------------------------final_graph_reshaping_walk---------------------
// Replacing Opaque nodes with their input in final_graph_reshaping_impl(),
// requires that the walk visits a node's inputs before visiting the node.
static void final_graph_reshaping_walk( Node_Stack &nstack, Node *root, Final_Reshape_Counts &fpu ) {
  fpu._visited.set(root->_idx); // first, mark node as visited
  uint cnt = root->req();
  Node *n = root;
  uint  i = 0;
  while (true) {
    if (i < cnt) {
      // Place all non-visited non-null inputs onto stack
      Node* m = n->in(i);
      ++i;
      if (m != NULL && !fpu._visited.test_set(m->_idx)) {
        cnt = m->req();
        nstack.push(n, i); // put on stack parent and next input's index
        n = m;
        i = 0;
      }
    } else {
      // Now do post-visit work
      final_graph_reshaping_impl( n, fpu );
      if (nstack.is_empty())
        break;             // finished
      n = nstack.node();   // Get node from stack
      cnt = n->req();
      i = nstack.index();
      nstack.pop();        // Shift to the next node on stack
    }
  }
}

//------------------------------final_graph_reshaping--------------------------
// Final Graph Reshaping.
//
// (1) Clone simple inputs to uncommon calls, so they can be scheduled late
//     and not commoned up and forced early.  Must come after regular
//     optimizations to avoid GVN undoing the cloning.  Clone constant
//     inputs to Loop Phis; these will be split by the allocator anyways.
//     Remove Opaque nodes.
// (2) Move last-uses by commutative operations to the left input to encourage
//     Intel update-in-place two-address operations and better register usage
//     on RISCs.  Must come after regular optimizations to avoid GVN Ideal
//     calls canonicalizing them back.
// (3) Count the number of double-precision FP ops, single-precision FP ops
//     and call sites.  On Intel, we can get correct rounding either by
//     forcing singles to memory (requires extra stores and loads after each
//     FP bytecode) or we can set a rounding mode bit (requires setting and
//     clearing the mode bit around call sites).  The mode bit is only used
//     if the relative frequency of single FP ops to calls is low enough.
//     This is a key transform for SPEC mpeg_audio.
// (4) Detect infinite loops; blobs of code reachable from above but not
//     below.  Several of the Code_Gen algorithms fail on such code shapes,
//     so we simply bail out.  Happens a lot in ZKM.jar, but also happens
//     from time to time in other codes (such as -Xcomp finalizer loops, etc).
//     Detection is by looking for IfNodes where only 1 projection is
//     reachable from below or CatchNodes missing some targets.
// (5) Assert for insane oop offsets in debug mode.

bool Compile::final_graph_reshaping() {
  // an infinite loop may have been eliminated by the optimizer,
  // in which case the graph will be empty.
  if (root()->req() == 1) {
    record_method_not_compilable("trivial infinite loop");
    return true;
  }

  Final_Reshape_Counts fpu;

  // Visit everybody reachable!
  // Allocate stack of size C->unique()/2 to avoid frequent realloc
  Node_Stack nstack(unique() >> 1);
  final_graph_reshaping_walk(nstack, root(), fpu);

  // Check for unreachable (from below) code (i.e., infinite loops).
  for( uint i = 0; i < fpu._tests.size(); i++ ) {
    MultiBranchNode *n = fpu._tests[i]->as_MultiBranch();
    // Get number of CFG targets.
    // Note that PCTables include exception targets after calls.
    uint required_outcnt = n->required_outcnt();
    if (n->outcnt() != required_outcnt) {
      // Check for a few special cases.  Rethrow Nodes never take the
      // 'fall-thru' path, so expected kids is 1 less.
      if (n->is_PCTable() && n->in(0) && n->in(0)->in(0)) {
        if (n->in(0)->in(0)->is_Call()) {
          CallNode *call = n->in(0)->in(0)->as_Call();
          if (call->entry_point() == OptoRuntime::rethrow_stub()) {
            required_outcnt--;      // Rethrow always has 1 less kid
          } else if (call->req() > TypeFunc::Parms &&
                     call->is_CallDynamicJava()) {
            // Check for null receiver. In such case, the optimizer has
            // detected that the virtual call will always result in a null
            // pointer exception. The fall-through projection of this CatchNode
            // will not be populated.
            Node *arg0 = call->in(TypeFunc::Parms);
            if (arg0->is_Type() &&
                arg0->as_Type()->type()->higher_equal(TypePtr::NULL_PTR)) {
              required_outcnt--;
            }
          } else if (call->entry_point() == OptoRuntime::new_array_Java() &&
                     call->req() > TypeFunc::Parms+1 &&
                     call->is_CallStaticJava()) {
            // Check for negative array length. In such case, the optimizer has
            // detected that the allocation attempt will always result in an
            // exception. There is no fall-through projection of this CatchNode .
            Node *arg1 = call->in(TypeFunc::Parms+1);
            if (arg1->is_Type() &&
                arg1->as_Type()->type()->join(TypeInt::POS)->empty()) {
              required_outcnt--;
            }
          }
        }
      }
      // Recheck with a better notion of 'required_outcnt'
      if (n->outcnt() != required_outcnt) {
        record_method_not_compilable("malformed control flow");
        return true;            // Not all targets reachable!
      }
    }
    // Check that I actually visited all kids.  Unreached kids
    // must be infinite loops.
    for (DUIterator_Fast jmax, j = n->fast_outs(jmax); j < jmax; j++)
      if (!fpu._visited.test(n->fast_out(j)->_idx)) {
        record_method_not_compilable("infinite loop");
        return true;            // Found unvisited kid; must be unreach
      }
  }

  // If original bytecodes contained a mixture of floats and doubles
  // check if the optimizer has made it homogenous, item (3).
  if( Use24BitFPMode && Use24BitFP &&
      fpu.get_float_count() > 32 &&
      fpu.get_double_count() == 0 &&
      (10 * fpu.get_call_count() < fpu.get_float_count()) ) {
    set_24_bit_selection_and_mode( false,  true );
  }

  set_has_java_calls(fpu.get_java_call_count() > 0);

  // No infinite loops, no reason to bail out.
  return false;
}

//-----------------------------too_many_traps----------------------------------
// Report if there are too many traps at the current method and bci.
// Return true if there was a trap, and/or PerMethodTrapLimit is exceeded.
bool Compile::too_many_traps(ciMethod* method,
                             int bci,
                             Deoptimization::DeoptReason reason) {
  ciMethodData* md = method->method_data();
  if (md->is_empty()) {
    // Assume the trap has not occurred, or that it occurred only
    // because of a transient condition during start-up in the interpreter.
    return false;
  }
  if (md->has_trap_at(bci, reason) != 0) {
    // Assume PerBytecodeTrapLimit==0, for a more conservative heuristic.
    // Also, if there are multiple reasons, or if there is no per-BCI record,
    // assume the worst.
    if (log())
      log()->elem("observe trap='%s' count='%d'",
                  Deoptimization::trap_reason_name(reason),
                  md->trap_count(reason));
    return true;
  } else {
    // Ignore method/bci and see if there have been too many globally.
    return too_many_traps(reason, md);
  }
}

// Less-accurate variant which does not require a method and bci.
bool Compile::too_many_traps(Deoptimization::DeoptReason reason,
                             ciMethodData* logmd) {
 if (trap_count(reason) >= (uint)PerMethodTrapLimit) {
    // Too many traps globally.
    // Note that we use cumulative trap_count, not just md->trap_count.
    if (log()) {
      int mcount = (logmd == NULL)? -1: (int)logmd->trap_count(reason);
      log()->elem("observe trap='%s' count='0' mcount='%d' ccount='%d'",
                  Deoptimization::trap_reason_name(reason),
                  mcount, trap_count(reason));
    }
    return true;
  } else {
    // The coast is clear.
    return false;
  }
}

//--------------------------too_many_recompiles--------------------------------
// Report if there are too many recompiles at the current method and bci.
// Consults PerBytecodeRecompilationCutoff and PerMethodRecompilationCutoff.
// Is not eager to return true, since this will cause the compiler to use
// Action_none for a trap point, to avoid too many recompilations.
bool Compile::too_many_recompiles(ciMethod* method,
                                  int bci,
                                  Deoptimization::DeoptReason reason) {
  ciMethodData* md = method->method_data();
  if (md->is_empty()) {
    // Assume the trap has not occurred, or that it occurred only
    // because of a transient condition during start-up in the interpreter.
    return false;
  }
  // Pick a cutoff point well within PerBytecodeRecompilationCutoff.
  uint bc_cutoff = (uint) PerBytecodeRecompilationCutoff / 8;
  uint m_cutoff  = (uint) PerMethodRecompilationCutoff / 2 + 1;  // not zero
  Deoptimization::DeoptReason per_bc_reason
    = Deoptimization::reason_recorded_per_bytecode_if_any(reason);
  if ((per_bc_reason == Deoptimization::Reason_none
       || md->has_trap_at(bci, reason) != 0)
      // The trap frequency measure we care about is the recompile count:
      && md->trap_recompiled_at(bci)
      && md->overflow_recompile_count() >= bc_cutoff) {
    // Do not emit a trap here if it has already caused recompilations.
    // Also, if there are multiple reasons, or if there is no per-BCI record,
    // assume the worst.
    if (log())
      log()->elem("observe trap='%s recompiled' count='%d' recompiles2='%d'",
                  Deoptimization::trap_reason_name(reason),
                  md->trap_count(reason),
                  md->overflow_recompile_count());
    return true;
  } else if (trap_count(reason) != 0
             && decompile_count() >= m_cutoff) {
    // Too many recompiles globally, and we have seen this sort of trap.
    // Use cumulative decompile_count, not just md->decompile_count.
    if (log())
      log()->elem("observe trap='%s' count='%d' mcount='%d' decompiles='%d' mdecompiles='%d'",
                  Deoptimization::trap_reason_name(reason),
                  md->trap_count(reason), trap_count(reason),
                  md->decompile_count(), decompile_count());
    return true;
  } else {
    // The coast is clear.
    return false;
  }
}


#ifndef PRODUCT
//------------------------------verify_graph_edges---------------------------
// Walk the Graph and verify that there is a one-to-one correspondence
// between Use-Def edges and Def-Use edges in the graph.
void Compile::verify_graph_edges(bool no_dead_code) {
  if (VerifyGraphEdges) {
    ResourceArea *area = Thread::current()->resource_area();
    Unique_Node_List visited(area);
    // Call recursive graph walk to check edges
    _root->verify_edges(visited);
    if (no_dead_code) {
      // Now make sure that no visited node is used by an unvisited node.
      bool dead_nodes = 0;
      Unique_Node_List checked(area);
      while (visited.size() > 0) {
        Node* n = visited.pop();
        checked.push(n);
        for (uint i = 0; i < n->outcnt(); i++) {
          Node* use = n->raw_out(i);
          if (checked.member(use))  continue;  // already checked
          if (visited.member(use))  continue;  // already in the graph
          if (use->is_Con())        continue;  // a dead ConNode is OK
          // At this point, we have found a dead node which is DU-reachable.
          if (dead_nodes++ == 0)
            tty->print_cr("*** Dead nodes reachable via DU edges:");
          use->dump(2);
          tty->print_cr("---");
          checked.push(use);  // No repeats; pretend it is now checked.
        }
      }
      assert(dead_nodes == 0, "using nodes must be reachable from root");
    }
  }
}
#endif

// The Compile object keeps track of failure reasons separately from the ciEnv.
// This is required because there is not quite a 1-1 relation between the
// ciEnv and its compilation task and the Compile object.  Note that one
// ciEnv might use two Compile objects, if C2Compiler::compile_method decides
// to backtrack and retry without subsuming loads.  Other than this backtracking
// behavior, the Compile's failure reason is quietly copied up to the ciEnv
// by the logic in C2Compiler.
void Compile::record_failure(const char* reason) {
  if (log() != NULL) {
    log()->elem("failure reason='%s' phase='compile'", reason);
  }
  if (_failure_reason == NULL) {
    // Record the first failure reason.
    _failure_reason = reason;
  }
  _root = NULL;  // flush the graph, too
}

Compile::TracePhase::TracePhase(const char* name, elapsedTimer* accumulator, bool dolog)
  : TraceTime(NULL, accumulator, false NOT_PRODUCT( || TimeCompiler ), false)
{
  if (dolog) {
    C = Compile::current();
    _log = C->log();
  } else {
    C = NULL;
    _log = NULL;
  }
  if (_log != NULL) {
    _log->begin_head("phase name='%s' nodes='%d'", name, C->unique());
    _log->stamp();
    _log->end_head();
  }
}

Compile::TracePhase::~TracePhase() {
  if (_log != NULL) {
    _log->done("phase nodes='%d'", C->unique());
  }
}
