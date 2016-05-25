/*
 * Copyright (c) 2015, Oracle and/or its affiliates. All rights reserved.
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
 */

/*
 * @test
 * @bug 8129740 8133111 8157142
 * @summary Incorrect class file created when passing lambda in inner class constructor
 * @run main CaptureInCtorChainingTest
 */

import java.util.function.Consumer;
import java.util.function.Function;

public class CaptureInCtorChainingTest {

    CaptureInCtorChainingTest(Function<Function<Function<Consumer<Void>, Void>, Void>, Void> innerClass) {
        new InnerClass(innerClass);
    }

    void foo(Void v) { }

    class InnerClass {

        InnerClass(Function<Function<Function<Consumer<Void>, Void>, Void>, Void> factory) {
            this(factory.apply(o -> o.apply(CaptureInCtorChainingTest.this::foo)));
        }

        InnerClass(Void unused) { }
    }

    public static void main(String[] args) {
        new CaptureInCtorChainingTest(o -> null);
    }
}
