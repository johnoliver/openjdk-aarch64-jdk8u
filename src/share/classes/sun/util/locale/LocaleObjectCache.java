/*
 * Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
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

/*
 *******************************************************************************
 * Copyright (C) 2009-2010, International Business Machines Corporation and    *
 * others. All Rights Reserved.                                                *
 *******************************************************************************
 */
package sun.util.locale;

import java.lang.ref.ReferenceQueue;
import java.lang.ref.SoftReference;
import java.util.concurrent.ConcurrentHashMap;

public abstract class LocaleObjectCache<K, V> {
    private ConcurrentHashMap<K, CacheEntry<K, V>> _map;
    private ReferenceQueue<V> _queue = new ReferenceQueue<V>();

    public LocaleObjectCache() {
        this(16, 0.75f, 16);
    }

    public LocaleObjectCache(int initialCapacity, float loadFactor, int concurrencyLevel) {
        _map = new ConcurrentHashMap<K, CacheEntry<K, V>>(initialCapacity, loadFactor, concurrencyLevel);
    }

    public V get(K key) {
        V value = null;

        cleanStaleEntries();
        CacheEntry<K, V> entry = _map.get(key);
        if (entry != null) {
            value = entry.get();
        }
        if (value == null) {
            key = normalizeKey(key);
            V newVal = createObject(key);
            if (key == null || newVal == null) {
                // subclass must return non-null key/value object
                return null;
            }

            CacheEntry<K, V> newEntry = new CacheEntry<K, V>(key, newVal, _queue);

            while (value == null) {
                cleanStaleEntries();
                entry = _map.putIfAbsent(key, newEntry);
                if (entry == null) {
                    value = newVal;
                    break;
                } else {
                    value = entry.get();
                }
            }
        }
        return value;
    }

    @SuppressWarnings("unchecked")
    private void cleanStaleEntries() {
        CacheEntry<K, V> entry;
        while ((entry = (CacheEntry<K, V>)_queue.poll()) != null) {
            _map.remove(entry.getKey());
        }
    }

    protected abstract V createObject(K key);

    protected K normalizeKey(K key) {
        return key;
    }

    private static class CacheEntry<K, V> extends SoftReference<V> {
        private K _key;

        CacheEntry(K key, V value, ReferenceQueue<V> queue) {
            super(value, queue);
            _key = key;
        }

        K getKey() {
            return _key;
        }
    }
}
