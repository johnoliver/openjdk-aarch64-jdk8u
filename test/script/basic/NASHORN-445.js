/*
 * Copyright (c) 2010, 2012, Oracle and/or its affiliates. All rights reserved.
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
 * NASHORN-445 : Array.prototype.map gets confused by index user accessor properties inherited from Array.prototype
 *
 * @test
 * @run
 */

var arr = [1, 2, 3];

Object.defineProperty(Array.prototype, "0", {
    get: function () {
        return "hello"
    },
    set: function(x) {
        print("setter for '0' called with " + x);
    },
    enumerable: true
});

var res = arr.map(function(kVal, k, arr) {
    return kVal*2;
});

if (res.length !== arr.length) {
    fail("map result array is not of right length");
}

for (var i in res) {
    if (res[i] !== 2*arr[i]) {
        fail("map res[" + i + "] does not have right value");
    }
}

