/*
 * Copyright (c) 2000, 2011, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.oops;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

//  ConstantPoolCache : A constant pool cache (constantPoolCacheOopDesc).
//  See cpCacheOop.hpp for details about this class.
//
public class ConstantPoolCache extends Oop {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type      = db.lookupType("constantPoolCacheOopDesc");
    constants      = new OopField(type.getOopField("_constant_pool"), 0);
    baseOffset     = type.getSize();
    Type elType    = db.lookupType("ConstantPoolCacheEntry");
    elementSize    = elType.getSize();
    length         = new CIntField(type.getCIntegerField("_length"), 0);
  }

  ConstantPoolCache(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  public boolean isConstantPoolCache() { return true; }

  private static OopField constants;

  private static long baseOffset;
  private static long elementSize;
  private static CIntField length;


  public ConstantPool getConstants() { return (ConstantPool) constants.getValue(this); }

  public long getObjectSize() {
    return alignObjectSize(baseOffset + getLength() * elementSize);
  }

  public ConstantPoolCacheEntry getEntryAt(int i) {
    if (i < 0 || i >= getLength()) throw new IndexOutOfBoundsException(i + " " + getLength());
    return new ConstantPoolCacheEntry(this, i);
  }

  public static boolean isSecondaryIndex(int i)     { return (i < 0); }
  public static int     decodeSecondaryIndex(int i) { return  isSecondaryIndex(i) ? ~i : i; }
  public static int     encodeSecondaryIndex(int i) { return !isSecondaryIndex(i) ? ~i : i; }

  // secondary entries hold invokedynamic call site bindings
  public ConstantPoolCacheEntry getSecondaryEntryAt(int i) {
    int rawIndex = i;
    if (isSecondaryIndex(i)) {
      rawIndex = decodeSecondaryIndex(i);
    }
    ConstantPoolCacheEntry e = getEntryAt(rawIndex);
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(e.isSecondaryEntry(), "must be a secondary entry:" + rawIndex);
    }
    return e;
  }

  public ConstantPoolCacheEntry getMainEntryAt(int i) {
    int primaryIndex = i;
    if (isSecondaryIndex(i)) {
      // run through an extra level of indirection:
      int rawIndex = decodeSecondaryIndex(i);
      primaryIndex = getEntryAt(rawIndex).getMainEntryIndex();
    }
    ConstantPoolCacheEntry e = getEntryAt(primaryIndex);
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(!e.isSecondaryEntry(), "must not be a secondary entry:" + primaryIndex);
    }
    return e;
  }

  public int getIntAt(int entry, int fld) {
    //alignObjectSize ?
    long offset = baseOffset + /*alignObjectSize*/entry * elementSize + fld* getHeap().getIntSize();
    return (int) getHandle().getCIntegerAt(offset, getHeap().getIntSize(), true );
  }


  public void printValueOn(PrintStream tty) {
    tty.print("ConstantPoolCache for " + getConstants().getPoolHolder().getName().asString());
  }

  public int getLength() {
    return (int) length.getValue(this);
  }

  public void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    if (doVMFields) {
      visitor.doOop(constants, true);
      for (int i = 0; i < getLength(); i++) {
        ConstantPoolCacheEntry entry = getEntryAt(i);
        entry.iterateFields(visitor);
      }
    }
  }
};
