/*
 * Copyright (c) 2010, 2011, Oracle and/or its affiliates. All rights reserved.
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
 * @bug 4917091
 * @summary javac rejects array over 128 in length
 * @compile/fail/ref=Test256a.out -XDrawDiagnostics Test256a.java
 */

public class Test256a {
            // allocating an array with more than 255 dimensions is not allowed
            static Object expected = (Object)new Object
                [1/*001*/][1/*002*/][1/*003*/][1/*004*/][1/*005*/]
                [1/*006*/][1/*007*/][1/*008*/][1/*009*/][1/*010*/]
                [1/*011*/][1/*012*/][1/*013*/][1/*014*/][1/*015*/]
                [1/*016*/][1/*017*/][1/*018*/][1/*019*/][1/*020*/]
                [1/*021*/][1/*022*/][1/*023*/][1/*024*/][1/*025*/]
                [1/*026*/][1/*027*/][1/*028*/][1/*029*/][1/*030*/]
                [1/*031*/][1/*032*/][1/*033*/][1/*034*/][1/*035*/]
                [1/*036*/][1/*037*/][1/*038*/][1/*039*/][1/*040*/]
                [1/*041*/][1/*042*/][1/*043*/][1/*044*/][1/*045*/]
                [1/*046*/][1/*047*/][1/*048*/][1/*049*/][1/*050*/]
                [1/*051*/][1/*052*/][1/*053*/][1/*054*/][1/*055*/]
                [1/*056*/][1/*057*/][1/*058*/][1/*059*/][1/*060*/]
                [1/*061*/][1/*062*/][1/*063*/][1/*064*/][1/*065*/]
                [1/*066*/][1/*067*/][1/*068*/][1/*069*/][1/*070*/]
                [1/*071*/][1/*072*/][1/*073*/][1/*074*/][1/*075*/]
                [1/*076*/][1/*077*/][1/*078*/][1/*079*/][1/*080*/]
                [1/*081*/][1/*082*/][1/*083*/][1/*084*/][1/*085*/]
                [1/*086*/][1/*087*/][1/*088*/][1/*089*/][1/*090*/]
                [1/*091*/][1/*092*/][1/*093*/][1/*094*/][1/*095*/]
                [1/*096*/][1/*097*/][1/*098*/][1/*099*/][1/*100*/]

                [1/*101*/][1/*102*/][1/*103*/][1/*104*/][1/*105*/]
                [1/*106*/][1/*107*/][1/*108*/][1/*109*/][1/*110*/]
                [1/*111*/][1/*112*/][1/*113*/][1/*114*/][1/*115*/]
                [1/*116*/][1/*117*/][1/*118*/][1/*119*/][1/*120*/]
                [1/*121*/][1/*122*/][1/*123*/][1/*124*/][1/*125*/]
                [1/*126*/][1/*127*/][1/*128*/][1/*129*/][1/*130*/]
                [1/*131*/][1/*132*/][1/*133*/][1/*134*/][1/*135*/]
                [1/*136*/][1/*137*/][1/*138*/][1/*139*/][1/*140*/]
                [1/*141*/][1/*142*/][1/*143*/][1/*144*/][1/*145*/]
                [1/*146*/][1/*147*/][1/*148*/][1/*149*/][1/*150*/]
                [1/*151*/][1/*152*/][1/*153*/][1/*154*/][1/*155*/]
                [1/*156*/][1/*157*/][1/*158*/][1/*159*/][1/*160*/]
                [1/*161*/][1/*162*/][1/*163*/][1/*164*/][1/*165*/]
                [1/*166*/][1/*167*/][1/*168*/][1/*169*/][1/*170*/]
                [1/*171*/][1/*172*/][1/*173*/][1/*174*/][1/*175*/]
                [1/*176*/][1/*177*/][1/*178*/][1/*179*/][1/*180*/]
                [1/*181*/][1/*182*/][1/*183*/][1/*184*/][1/*185*/]
                [1/*186*/][1/*187*/][1/*188*/][1/*189*/][1/*190*/]
                [1/*191*/][1/*192*/][1/*193*/][1/*194*/][1/*195*/]
                [1/*196*/][1/*197*/][1/*198*/][1/*199*/][1/*200*/]

                [1/*201*/][1/*202*/][1/*203*/][1/*204*/][1/*205*/]
                [1/*206*/][1/*207*/][1/*208*/][1/*209*/][1/*210*/]
                [1/*211*/][1/*212*/][1/*213*/][1/*214*/][1/*215*/]
                [1/*216*/][1/*217*/][1/*218*/][1/*219*/][1/*220*/]
                [1/*221*/][1/*222*/][1/*223*/][1/*224*/][1/*225*/]
                [1/*226*/][1/*227*/][1/*228*/][1/*229*/][1/*230*/]
                [1/*231*/][1/*232*/][1/*233*/][1/*234*/][1/*235*/]
                [1/*236*/][1/*237*/][1/*238*/][1/*239*/][1/*240*/]
                [1/*241*/][1/*242*/][1/*243*/][1/*244*/][1/*245*/]
                [1/*246*/][1/*247*/][1/*248*/][1/*249*/][1/*250*/]
                [1/*251*/][1/*252*/][1/*253*/][1/*254*/][1/*255*/]
                [1/*256*/];
}
