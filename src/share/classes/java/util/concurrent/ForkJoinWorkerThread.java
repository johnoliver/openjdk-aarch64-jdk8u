/*
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
 * This file is available under and governed by the GNU General Public
 * License version 2 only, as published by the Free Software Foundation.
 * However, the following notice accompanied the original version of this
 * file:
 *
 * Written by Doug Lea with assistance from members of JCP JSR-166
 * Expert Group and released to the public domain, as explained at
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

package java.util.concurrent;

/**
 * A thread managed by a {@link ForkJoinPool}, which executes
 * {@link ForkJoinTask}s.
 * This class is subclassable solely for the sake of adding
 * functionality -- there are no overridable methods dealing with
 * scheduling or execution.  However, you can override initialization
 * and termination methods surrounding the main task processing loop.
 * If you do create such a subclass, you will also need to supply a
 * custom {@link ForkJoinPool.ForkJoinWorkerThreadFactory} to
 * {@linkplain ForkJoinPool#ForkJoinPool use it} in a {@code ForkJoinPool}.
 *
 * @since 1.7
 * @author Doug Lea
 */
public class ForkJoinWorkerThread extends Thread {
    /*
     * ForkJoinWorkerThreads are managed by ForkJoinPools and perform
     * ForkJoinTasks. For explanation, see the internal documentation
     * of class ForkJoinPool.
     *
     * This class just maintains links to its pool and WorkQueue.  The
     * pool field is set immediately upon construction, but the
     * workQueue field is not set until a call to registerWorker
     * completes. This leads to a visibility race, that is tolerated
     * by requiring that the workQueue field is only accessed by the
     * owning thread.
     */

    final ForkJoinPool pool;                // the pool this thread works in
    final ForkJoinPool.WorkQueue workQueue; // work-stealing mechanics

    /**
     * Creates a ForkJoinWorkerThread operating in the given pool.
     *
     * @param pool the pool this thread works in
     * @throws NullPointerException if pool is null
     */
    protected ForkJoinWorkerThread(ForkJoinPool pool) {
        // Use a placeholder until a useful name can be set in registerWorker
        super("aForkJoinWorkerThread");
        this.pool = pool;
        this.workQueue = pool.registerWorker(this);
    }

    /**
     * Returns the pool hosting this thread.
     *
     * @return the pool
     */
    public ForkJoinPool getPool() {
        return pool;
    }

    /**
     * Returns the unique index number of this thread in its pool.
     * The returned value ranges from zero to the maximum number of
     * threads (minus one) that may exist in the pool, and does not
     * change during the lifetime of the thread.  This method may be
     * useful for applications that track status or collect results
     * per-worker-thread rather than per-task.
     *
     * @return the index number
     */
    public int getPoolIndex() {
        return workQueue.poolIndex >>> 1; // ignore odd/even tag bit
    }

    /**
     * Initializes internal state after construction but before
     * processing any tasks. If you override this method, you must
     * invoke {@code super.onStart()} at the beginning of the method.
     * Initialization requires care: Most fields must have legal
     * default values, to ensure that attempted accesses from other
     * threads work correctly even before this thread starts
     * processing tasks.
     */
    protected void onStart() {
    }

    /**
     * Performs cleanup associated with termination of this worker
     * thread.  If you override this method, you must invoke
     * {@code super.onTermination} at the end of the overridden method.
     *
     * @param exception the exception causing this thread to abort due
     * to an unrecoverable error, or {@code null} if completed normally
     */
    protected void onTermination(Throwable exception) {
    }

    /**
     * This method is required to be public, but should never be
     * called explicitly. It performs the main run loop to execute
     * {@link ForkJoinTask}s.
     */
    public void run() {
        Throwable exception = null;
        try {
            onStart();
            pool.runWorker(workQueue);
        } catch (Throwable ex) {
            exception = ex;
        } finally {
            try {
                onTermination(exception);
            } catch (Throwable ex) {
                if (exception == null)
                    exception = ex;
            } finally {
                pool.deregisterWorker(this, exception);
            }
        }
    }
}
