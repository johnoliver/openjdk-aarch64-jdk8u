/*
 * Copyright (c) 2015, Red Hat, Inc. and/or its affiliates.
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

#ifndef SHARE_VM_OPTO_SHENANDOAH_SUPPORT_HPP
#define SHARE_VM_OPTO_SHENANDOAH_SUPPORT_HPP

#include "gc_implementation/shenandoah/brooksPointer.hpp"
#include "memory/allocation.hpp"
#include "opto/addnode.hpp"
#include "opto/machnode.hpp"
#include "opto/memnode.hpp"
#include "opto/multnode.hpp"
#include "opto/node.hpp"

class PhaseGVN;


class ShenandoahBarrierNode : public TypeNode {
private:
  bool _allow_fromspace;

#ifdef ASSERT
  enum verify_type {
    ShenandoahLoad,
    ShenandoahStore,
    ShenandoahValue,
    ShenandoahNone,
  };

  static bool verify_helper(Node* in, Node_Stack& phis, VectorSet& visited, verify_type t, bool trace, Unique_Node_List& barriers_used);
#endif

public:

public:
  enum { Control,
         Memory,
         ValueIn
  };

  ShenandoahBarrierNode(Node* ctrl, Node* mem, Node* obj, bool allow_fromspace)
    : TypeNode(obj->bottom_type(), 3),
      _allow_fromspace(allow_fromspace) {

    init_req(Control, ctrl);
    init_req(Memory, mem);
    init_req(ValueIn, obj);

    init_class_id(Class_ShenandoahBarrier);
  }

  static Node* skip_through_barrier(Node* n);

  static const TypeOopPtr* brooks_pointer_type(const Type* t) {
    return t->is_oopptr()->cast_to_nonconst()->add_offset(BrooksPointer::byte_offset())->is_oopptr();
  }

  virtual const TypePtr* adr_type() const {
    if (bottom_type() == Type::TOP) {
      return NULL;
    }
    //const TypePtr* adr_type = in(MemNode::Address)->bottom_type()->is_ptr();
    const TypePtr* adr_type = brooks_pointer_type(bottom_type());
    assert(adr_type->offset() == BrooksPointer::byte_offset(), "sane offset");
    assert(Compile::current()->alias_type(adr_type)->is_rewritable(), "brooks ptr must be rewritable");
    return adr_type;
  }

  virtual uint  ideal_reg() const { return Op_RegP; }
  virtual uint match_edge(uint idx) const {
    return idx >= ValueIn;
  }

  Node* Identity_impl(PhaseTransform* phase);

  virtual const Type* Value(PhaseTransform* phase) const;
  virtual bool depends_only_on_test() const {
    return true;
  };

  static bool needs_barrier(PhaseTransform* phase, ShenandoahBarrierNode* orig, Node* n, Node* rb_mem, bool allow_fromspace);

  static void verify(RootNode* root);
#ifdef ASSERT
  static void verify_raw_mem(RootNode* root);
#endif
#ifndef PRODUCT
  virtual void dump_spec(outputStream *st) const;
#endif

protected:
  uint hash() const;
  uint cmp(const Node& n) const;
  uint size_of() const;

private:
  static bool needs_barrier_impl(PhaseTransform* phase, ShenandoahBarrierNode* orig, Node* n, Node* rb_mem, bool allow_fromspace, Unique_Node_List &visited);


  static bool dominates_memory(PhaseTransform* phase, Node* b1, Node* b2, bool linear);
  static bool dominates_memory_impl(PhaseTransform* phase, Node* b1, Node* b2, Node* current, bool linear);
};

class ShenandoahReadBarrierNode : public ShenandoahBarrierNode {
public:
  ShenandoahReadBarrierNode(Node* ctrl, Node* mem, Node* obj)
    : ShenandoahBarrierNode(ctrl, mem, obj, true) {
  }
  ShenandoahReadBarrierNode(Node* ctrl, Node* mem, Node* obj, bool allow_fromspace)
    : ShenandoahBarrierNode(ctrl, mem, obj, allow_fromspace) {
  }

  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual Node* Identity(PhaseTransform* phase);
  virtual int Opcode() const;

  bool is_independent(Node* mem);

private:
  static bool is_independent(const Type* in_type, const Type* this_type);
  static bool dominates_memory_rb(PhaseTransform* phase, Node* b1, Node* b2, bool linear);
  static bool dominates_memory_rb_impl(PhaseTransform* phase, Node* b1, Node* b2, Node* current, bool linear);
};

class ShenandoahWriteBarrierNode : public ShenandoahBarrierNode {
public:
  ShenandoahWriteBarrierNode(Compile* C, Node* ctrl, Node* mem, Node* obj)
    : ShenandoahBarrierNode(ctrl, mem, obj, false) {
    C->add_shenandoah_barrier(this);
    //tty->print("new wb: "); dump();
  }

  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual Node* Identity(PhaseTransform* phase);
  virtual bool depends_only_on_test() const { return false; }

  // virtual void set_req( uint i, Node *n ) {
  //   if (i == MemNode::Memory) { assert(n == Compiler::current()->immutable_memory(), "set only immutable mem on wb"); }
  //   Node::set_req(i, n);
  // }
};

class ShenandoahWBMemProjNode : public ProjNode {
public:
  enum {SWBMEMPROJCON = (uint)-3};
  ShenandoahWBMemProjNode(Node *src) : ProjNode( src, SWBMEMPROJCON) {
    assert(src->Opcode() == Op_ShenandoahWriteBarrier || src->is_Mach(), "epxect wb");
  }
  virtual Node* Identity(PhaseTransform* phase);

  virtual int Opcode() const;
  virtual bool      is_CFG() const  { return false; }
  virtual const Type *bottom_type() const {return Type::MEMORY;}
  virtual const TypePtr *adr_type() const {
    Node* wb = in(0);
    if (wb == NULL || wb->is_top())  return NULL; // node is dead
    assert(wb->Opcode() == Op_ShenandoahWriteBarrier || (wb->is_Mach() && wb->as_Mach()->ideal_Opcode() == Op_ShenandoahWriteBarrier), "expect wb");
    return ShenandoahBarrierNode::brooks_pointer_type(wb->bottom_type());
  }

  virtual uint ideal_reg() const { return 0;} // memory projections don't have a register
  virtual const Type *Value(PhaseTransform* phase ) const {
    return bottom_type();
  }
#ifndef PRODUCT
  virtual void dump_spec(outputStream *st) const {};
#endif
};

#endif // SHARE_VM_OPTO_SHENANDOAH_SUPPORT_HPP
