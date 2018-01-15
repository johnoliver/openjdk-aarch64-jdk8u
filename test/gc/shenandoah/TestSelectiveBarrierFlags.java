/*
 * Copyright (c) 2017 Red Hat, Inc. and/or its affiliates.
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
 *
 */

/* @test
 * @summary Test selective barrier enabling works, by aggressively compiling HelloWorld with combinations
 *          of barrier flags
 * @library /testlibrary
 * @run main/othervm TestSelectiveBarrierFlags -Xint
 * @run main/othervm TestSelectiveBarrierFlags -Xbatch -XX:CompileThreshold=100 -XX:TieredStopAtLevel=1
 * @run main/othervm TestSelectiveBarrierFlags -Xbatch -XX:CompileThreshold=100 -XX:-TieredCompilation -XX:+IgnoreUnrecognizedVMOptions -XX:+ShenandoahVerifyOptoBarriers
 */

import java.util.*;
import java.util.concurrent.*;
import com.oracle.java.testlibrary.*;

public class TestSelectiveBarrierFlags {

    public static void main(String[] args) throws Exception {
        String[][] opts = {
            new String[]{ "ShenandoahSATBBarrier"  },
            new String[]{ "ShenandoahWriteBarrier" },
            new String[]{ "ShenandoahReadBarrier" },
            new String[]{ "ShenandoahCASBarrier" },
            new String[]{ "ShenandoahAcmpBarrier" },
            new String[]{ "ShenandoahCloneBarrier" },
        };

        int size = 1;
        for (String[] l : opts) {
            size *= (l.length + 1);
        }

        ExecutorService pool = Executors.newFixedThreadPool(Runtime.getRuntime().availableProcessors());

        for (int c = 0; c < size; c++) {
            int t = c;

            List<String> conf = new ArrayList<>();
            conf.addAll(Arrays.asList(args));
            conf.add("-Xmx128m");
            conf.add("-XX:+UseShenandoahGC");
            conf.add("-XX:+UnlockDiagnosticVMOptions");
            conf.add("-XX:+UnlockExperimentalVMOptions");
            conf.add("-XX:ShenandoahGCHeuristics=passive");

            StringBuilder sb = new StringBuilder();
            for (String[] l : opts) {
                int f = t % (l.length + 1);
                conf.add("-XX:" + ((f & 1) == 1 ? "+" : "-") + l[0]);
                if (l.length > 1) {
                    conf.add("-XX:" + ((f & 2) == 2 ? "+" : "-") + l[1]);
                }
                t = t / (l.length + 1);
            }

            conf.add("TestSelectiveBarrierFlags$Test");

            pool.submit(() -> {
                try {
                    ProcessBuilder pb = ProcessTools.createJavaProcessBuilder(conf.toArray(new String[0]));
                    OutputAnalyzer output = new OutputAnalyzer(pb.start());
                    output.shouldHaveExitValue(0);
                } catch (Exception e) {
                    e.printStackTrace();
                    System.exit(1);
                }
            });
        }

        pool.shutdown();
        pool.awaitTermination(1, TimeUnit.HOURS);
    }

    public static class Test {
        public static void main(String... args) {
            System.out.println("HelloWorld");
        }
    }

}
