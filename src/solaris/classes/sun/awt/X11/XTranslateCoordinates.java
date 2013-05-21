/*
 * Copyright (c) 2003, 2013, Oracle and/or its affiliates. All rights reserved.
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

// This file is an automatically generated file, please do not edit this file, modify the WrapperGenerator.java file instead !

package sun.awt.X11;

import sun.misc.Unsafe;

public class XTranslateCoordinates {
        private static Unsafe unsafe = XlibWrapper.unsafe;
        private boolean __executed = false;
        long _scr_w;
        long _dest_w;
        int _src_x;
        int _src_y;
        long dest_x_ptr = unsafe.allocateMemory(Native.getIntSize());
        long dest_y_ptr = unsafe.allocateMemory(Native.getIntSize());
        long child_ptr = unsafe.allocateMemory(Native.getLongSize());
        public XTranslateCoordinates(
                long scr_w,
                long dest_w,
                int src_x,
                int src_y       )
        {
                set_scr_w(scr_w);
                set_dest_w(dest_w);
                set_src_x(src_x);
                set_src_y(src_y);

                sun.java2d.Disposer.addRecord(this, disposer = new UnsafeXDisposerRecord("XTranslateCoordinates",
                                                                                         dest_x_ptr, dest_y_ptr, child_ptr));
        }
    UnsafeXDisposerRecord disposer;
        public int execute() {
                return execute(null);
        }
        public int execute(XErrorHandler errorHandler) {
                XToolkit.awtLock();
                try {
                if (isDisposed()) {
                    throw new IllegalStateException("Disposed");
                }
                        if (__executed) {
                            throw new IllegalStateException("Already executed");
                        }
                        __executed = true;
                        if (errorHandler != null) {
                            XErrorHandlerUtil.WITH_XERROR_HANDLER(errorHandler);
                        }
                        int status =
                        XlibWrapper.XTranslateCoordinates(XToolkit.getDisplay(),
                                get_scr_w(),
                                get_dest_w(),
                                get_src_x(),
                                get_src_y(),
                                dest_x_ptr,
                                dest_y_ptr,
                                child_ptr                       );
                        if (errorHandler != null) {
                            XErrorHandlerUtil.RESTORE_XERROR_HANDLER();
                        }
                        return status;
                } finally {
                    XToolkit.awtUnlock();
                }
        }
        public boolean isExecuted() {
            return __executed;
        }

        public boolean isDisposed() {
            return disposer.disposed;
        }
        public void dispose() {
            XToolkit.awtLock();
            try {
                if (isDisposed()) {
                    return;
                }
                disposer.dispose();
            } finally {
                XToolkit.awtUnlock();
            }
        }
        public long get_scr_w() {
                if (isDisposed()) {
                    throw new IllegalStateException("Disposed");
                }
                if (!__executed) {
                    throw new IllegalStateException("Not executed");
                }
                return _scr_w;
        }
        public void set_scr_w(long data) {
                _scr_w = data;
        }
        public long get_dest_w() {
                if (isDisposed()) {
                    throw new IllegalStateException("Disposed");
                }
                if (!__executed) {
                    throw new IllegalStateException("Not executed");
                }
                return _dest_w;
        }
        public void set_dest_w(long data) {
                _dest_w = data;
        }
        public int get_src_x() {
                if (isDisposed()) {
                    throw new IllegalStateException("Disposed");
                }
                if (!__executed) {
                    throw new IllegalStateException("Not executed");
                }
                return _src_x;
        }
        public void set_src_x(int data) {
                _src_x = data;
        }
        public int get_src_y() {
                if (isDisposed()) {
                    throw new IllegalStateException("Disposed");
                }
                if (!__executed) {
                    throw new IllegalStateException("Not executed");
                }
                return _src_y;
        }
        public void set_src_y(int data) {
                _src_y = data;
        }
        public int get_dest_x() {
                if (isDisposed()) {
                    throw new IllegalStateException("Disposed");
                }
                if (!__executed) {
                    throw new IllegalStateException("Not executed");
                }
                return Native.getInt(dest_x_ptr);
        }
        public void set_dest_x(int data) {
                Native.putInt(dest_x_ptr, data);
        }
        public int get_dest_y() {
                if (isDisposed()) {
                    throw new IllegalStateException("Disposed");
                }
                if (!__executed) {
                    throw new IllegalStateException("Not executed");
                }
                return Native.getInt(dest_y_ptr);
        }
        public void set_dest_y(int data) {
                Native.putInt(dest_y_ptr, data);
        }
        public long get_child() {
                if (isDisposed()) {
                    throw new IllegalStateException("Disposed");
                }
                if (!__executed) {
                    throw new IllegalStateException("Not executed");
                }
                return Native.getLong(child_ptr);
        }
        public void set_child(long data) {
                Native.putLong(child_ptr, data);
        }
}
