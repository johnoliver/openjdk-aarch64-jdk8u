/*
 * Copyright (c) 2006, 2011, Oracle and/or its affiliates. All rights reserved.
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
 *
 * @bug 6388456
 * @summary Need adjustable TLS max record size for interoperability
 *      with non-compliant
 *
 * @run main/othervm -Djsse.enableCBCProtection=false LargePacket
 *
 * @author Xuelei Fan
 */

import javax.net.ssl.*;
import java.nio.channels.*;
import java.net.*;

public class LargePacket extends SSLEngineService {

    /*
     * =============================================================
     * Set the various variables needed for the tests, then
     * specify what tests to run on each side.
     */

    /*
     * Should we run the client or server in a separate thread?
     * Both sides can throw exceptions, but do you have a preference
     * as to which side should be the main thread.
     */
    static boolean separateServerThread = true;

    // Is the server ready to serve?
    volatile static boolean serverReady = false;

    /*
     * Turn on SSL debugging?
     */
    static boolean debug = false;

    /*
     * Define the server side of the test.
     *
     * If the server prematurely exits, serverReady will be set to true
     * to avoid infinite hangs.
     */
    void doServerSide() throws Exception {
        // create SSLEngine.
        SSLEngine ssle = createSSLEngine(false);

        // Create a server socket channel.
        InetSocketAddress isa =
                new InetSocketAddress(InetAddress.getLocalHost(), serverPort);
        ServerSocketChannel ssc = ServerSocketChannel.open();
        ssc.socket().bind(isa);
        serverPort = ssc.socket().getLocalPort();

        // Signal Client, we're ready for his connect.
        serverReady = true;

        // Accept a socket channel.
        SocketChannel sc = ssc.accept();

        // Complete connection.
        while (!sc.finishConnect() ) {
            // waiting for the connection completed.
        }

        // handshaking
        handshaking(ssle, sc);

        // receive application data
        receive(ssle, sc);

        // send out application data
        deliver(ssle, sc);

        // close the socket channel.
        sc.close();
        ssc.close();
    }

    /*
     * Define the client side of the test.
     *
     * If the server prematurely exits, serverReady will be set to true
     * to avoid infinite hangs.
     */
    void doClientSide() throws Exception {
        // create SSLEngine.
        SSLEngine ssle = createSSLEngine(true);

        /*
         * Wait for server to get started.
         */
        while (!serverReady) {
            Thread.sleep(50);
        }

        // Create a non-blocking socket channel.
        SocketChannel sc = SocketChannel.open();
        sc.configureBlocking(false);
        InetSocketAddress isa =
                new InetSocketAddress(InetAddress.getLocalHost(), serverPort);
        sc.connect(isa);

        // Complete connection.
        while (!sc.finishConnect() ) {
            // waiting for the connection completed.
        }

        // handshaking
        handshaking(ssle, sc);

        // send out application data
        deliver(ssle, sc);

        // receive application data
        receive(ssle, sc);

        // close the socket channel.
        sc.close();
    }

    /*
     * =============================================================
     * The remainder is just support stuff
     */
    volatile Exception serverException = null;
    volatile Exception clientException = null;

    // use any free port by default
    volatile int serverPort = 0;

    public static void main(String args[]) throws Exception {
        if (debug)
            System.setProperty("javax.net.debug", "all");

        new LargePacket();
    }

    Thread clientThread = null;
    Thread serverThread = null;

    /*
     * Primary constructor, used to drive remainder of the test.
     *
     * Fork off the other side, then do your work.
     */
    LargePacket() throws Exception {
        if (separateServerThread) {
            startServer(true);
            startClient(false);
        } else {
            startClient(true);
            startServer(false);
        }

        /*
         * Wait for other side to close down.
         */
        if (separateServerThread) {
            serverThread.join();
        } else {
            clientThread.join();
        }

        /*
         * When we get here, the test is pretty much over.
         *
         * If the main thread excepted, that propagates back
         * immediately.  If the other thread threw an exception, we
         * should report back.
         */
        if (serverException != null) {
            System.out.print("Server Exception:");
            throw serverException;
        }
        if (clientException != null) {
            System.out.print("Client Exception:");
            throw clientException;
        }
    }

    void startServer(boolean newThread) throws Exception {
        if (newThread) {
            serverThread = new Thread() {
                public void run() {
                    try {
                        doServerSide();
                    } catch (Exception e) {
                        /*
                         * Our server thread just died.
                         *
                         * Release the client, if not active already...
                         */
                        System.err.println("Server died...");
                        System.err.println(e);
                        serverReady = true;
                        serverException = e;
                    }
                }
            };
            serverThread.start();
        } else {
            doServerSide();
        }
    }

    void startClient(boolean newThread) throws Exception {
        if (newThread) {
            clientThread = new Thread() {
                public void run() {
                    try {
                        doClientSide();
                    } catch (Exception e) {
                        /*
                         * Our client thread just died.
                         */
                        System.err.println("Client died...");
                        clientException = e;
                    }
                }
            };
            clientThread.start();
        } else {
            doClientSide();
        }
    }
}
