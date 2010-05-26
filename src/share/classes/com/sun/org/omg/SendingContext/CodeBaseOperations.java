/*
 * Copyright (c) 1999, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
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
 */
package com.sun.org.omg.SendingContext;


/**
* com/sun/org/omg/SendingContext/CodeBaseOperations.java
* Generated by the IDL-to-Java compiler (portable), version "3.0"
* from rt.idl
* Thursday, May 6, 1999 1:52:08 AM PDT
*/

// Edited to leave RunTime in org.omg.CORBA

public interface CodeBaseOperations  extends org.omg.SendingContext.RunTimeOperations
{

    // Operation to obtain the IR from the sending context
    com.sun.org.omg.CORBA.Repository get_ir ();

    // Operations to obtain a URL to the implementation code
    String implementation (String x);
    String[] implementations (String[] x);

    // the same information
    com.sun.org.omg.CORBA.ValueDefPackage.FullValueDescription meta (String x);
    com.sun.org.omg.CORBA.ValueDefPackage.FullValueDescription[] metas (String[] x);

    // information
    String[] bases (String x);
} // interface CodeBaseOperations
