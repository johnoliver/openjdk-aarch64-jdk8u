/*
 * Copyright 2008 Sun Microsystems, Inc.  All Rights Reserved.
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
 */

/**
 * @test
 * @bug 6289149
 * @summary test config (0,0,1,0): declared 2-arg in agent class
 * @author Daniel D. Daugherty, Sun Microsystems
 *
 * @run shell ../MakeJAR3.sh InheritAgent0010
 * @run main/othervm -javaagent:InheritAgent0010.jar DummyMain
 */

import java.lang.instrument.*;

class InheritAgent0010 extends InheritAgent0010Super {

    // This agent does NOT have a single argument premain() method.

    //
    // This agent has a double argument premain() method which
    // is the one that should be called.
    //
    public static void premain (String agentArgs, Instrumentation instArg) {
        System.out.println("Hello from Double-Arg InheritAgent0010!");
    }
}

class InheritAgent0010Super {

    // This agent does NOT have a single argument premain() method.

    // This agent does NOT have a double argument premain() method.
}
