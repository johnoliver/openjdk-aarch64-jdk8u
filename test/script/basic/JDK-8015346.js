/*
 * Copyright (c) 2010, 2013, Oracle and/or its affiliates. All rights reserved.
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

/**
 * JDK-8015346: JSON parsing issues with escaped strings, octal, decimal numbers *
 * @test
 * @run
 */

function checkJSON(str) {
    try {
        JSON.parse(str);
        fail("should have thrown SyntaxError for JSON.parse on " + str);
    } catch (e) {
        if (! (e instanceof SyntaxError)) {
            fail("Expected SyntaxError, but got " + e);
        }
    }
}

// invalid escape in a string
checkJSON('"\\a"')

// invalid floating point number patterns
checkJSON("1.")
checkJSON(".8")
checkJSON("2.3e+")
checkJSON("0.3E+")

// octal, hexadecimal not allowed
checkJSON("08")
checkJSON("06")
checkJSON('0x3')
