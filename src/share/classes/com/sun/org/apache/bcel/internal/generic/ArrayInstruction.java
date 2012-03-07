/*
 * reserved comment block
 * DO NOT REMOVE OR ALTER!
 */
package com.sun.org.apache.bcel.internal.generic;

/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2001 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" and
 *    "Apache BCEL" must not be used to endorse or promote products
 *    derived from this software without prior written permission. For
 *    written permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    "Apache BCEL", nor may "Apache" appear in their name, without
 *    prior written permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/**
 * Super class for instructions dealing with array access such as IALOAD.
 *
 * @author  <A HREF="mailto:markus.dahm@berlin.de">M. Dahm</A>
 */
public abstract class ArrayInstruction extends Instruction
  implements ExceptionThrower, TypedInstruction {
  /**
   * Empty constructor needed for the Class.newInstance() statement in
   * Instruction.readInstruction(). Not to be used otherwise.
   */
  ArrayInstruction() {}

  /**
   * @param opcode of instruction
   */
  protected ArrayInstruction(short opcode) {
    super(opcode, (short)1);
  }

  public Class[] getExceptions() {
    return com.sun.org.apache.bcel.internal.ExceptionConstants.EXCS_ARRAY_EXCEPTION;
  }

  /** @return type associated with the instruction
   */
  public Type getType(ConstantPoolGen cp) {
    switch(opcode) {
    case com.sun.org.apache.bcel.internal.Constants.IALOAD: case com.sun.org.apache.bcel.internal.Constants.IASTORE:
      return Type.INT;
    case com.sun.org.apache.bcel.internal.Constants.CALOAD: case com.sun.org.apache.bcel.internal.Constants.CASTORE:
      return Type.CHAR;
    case com.sun.org.apache.bcel.internal.Constants.BALOAD: case com.sun.org.apache.bcel.internal.Constants.BASTORE:
      return Type.BYTE;
    case com.sun.org.apache.bcel.internal.Constants.SALOAD: case com.sun.org.apache.bcel.internal.Constants.SASTORE:
      return Type.SHORT;
    case com.sun.org.apache.bcel.internal.Constants.LALOAD: case com.sun.org.apache.bcel.internal.Constants.LASTORE:
      return Type.LONG;
    case com.sun.org.apache.bcel.internal.Constants.DALOAD: case com.sun.org.apache.bcel.internal.Constants.DASTORE:
      return Type.DOUBLE;
    case com.sun.org.apache.bcel.internal.Constants.FALOAD: case com.sun.org.apache.bcel.internal.Constants.FASTORE:
      return Type.FLOAT;
    case com.sun.org.apache.bcel.internal.Constants.AALOAD: case com.sun.org.apache.bcel.internal.Constants.AASTORE:
      return Type.OBJECT;

    default: throw new ClassGenException("Oops: unknown case in switch" + opcode);
    }
  }
}
