/*
 * Copyright (c) 2010, 2012, Oracle and/or its affiliates. All rights reserved.
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

package jdk.nashorn.internal.runtime.arrays;

import jdk.nashorn.internal.runtime.JSType;
import jdk.nashorn.internal.runtime.ScriptObject;

/**
 * Iterator over a map
 */
class MapIterator extends ArrayLikeIterator<Object> {

    protected final ScriptObject obj;
    private final long length;

    MapIterator(final ScriptObject obj, final boolean includeUndefined) {
        super(includeUndefined);
        this.obj    = obj;
        this.length = JSType.toUint32(obj.getLength());
        this.index  = 0;
    }

    protected boolean indexInArray() {
        return index < length;
    }

    @Override
    public int getLength() {
        return (int) length;
    }

    @Override
    public boolean hasNext() {
        if (length == 0L) {
            return false; //return empty string if toUint32(length) == 0
        }

        while (indexInArray()) {
            if (obj.has(index) || includeUndefined) {
                break;
            }
            bumpIndex();
        }

        // special case - balk at iterating to infinity or MAX_UINT
        return (length != JSType.MAX_UINT) && indexInArray();
    }

    @Override
    public Object next() {
        return indexInArray() ? obj.get(bumpIndex()) : null;
    }
}
