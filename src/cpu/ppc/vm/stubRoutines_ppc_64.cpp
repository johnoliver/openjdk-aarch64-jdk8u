/*
 * Copyright (c) 2002, 2018, Oracle and/or its affiliates. All rights reserved.
 * Copyright (c) 2012, 2018 SAP SE. All rights reserved.
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
 *
 */

#include "asm/macroAssembler.inline.hpp"
#include "runtime/stubRoutines.hpp"

// Implementation of the platform-specific part of StubRoutines - for
// a description of how to extend it, see the stubRoutines.hpp file.


#define __ masm->

// CRC32 Intrinsics.
void StubRoutines::ppc64::generate_load_crc_table_addr(MacroAssembler* masm, Register table) {
  __ load_const(table, StubRoutines::_crc_table_adr);
}

void StubRoutines::ppc64::generate_load_crc_constants_addr(MacroAssembler* masm, Register table) {
  __ load_const_optimized(table, (address)StubRoutines::ppc64::_constants, R0);
}

void StubRoutines::ppc64::generate_load_crc_barret_constants_addr(MacroAssembler* masm, Register table) {
  __ load_const_optimized(table, (address)StubRoutines::ppc64::_barret_constants, R0);
}

juint* StubRoutines::ppc64::generate_crc_constants() {
  juint constants[CRC32_CONSTANTS_SIZE] = {
      // Reduce 262144 kbits to 1024 bits
      0x99ea94a8UL, 0x00000000UL, 0x651797d2UL, 0x00000001UL,       // x^261120 mod p(x)` << 1, x^261184 mod p(x)` << 1
      0x945a8420UL, 0x00000000UL, 0x21e0d56cUL, 0x00000000UL,       // x^260096 mod p(x)` << 1, x^260160 mod p(x)` << 1
      0x30762706UL, 0x00000000UL, 0x0f95ecaaUL, 0x00000000UL,       // x^259072 mod p(x)` << 1, x^259136 mod p(x)` << 1
      0xa52fc582UL, 0x00000001UL, 0xebd224acUL, 0x00000001UL,       // x^258048 mod p(x)` << 1, x^258112 mod p(x)` << 1
      0xa4a7167aUL, 0x00000001UL, 0x0ccb97caUL, 0x00000000UL,       // x^257024 mod p(x)` << 1, x^257088 mod p(x)` << 1
      0x0c18249aUL, 0x00000000UL, 0x006ec8a8UL, 0x00000001UL,       // x^256000 mod p(x)` << 1, x^256064 mod p(x)` << 1
      0xa924ae7cUL, 0x00000000UL, 0x4f58f196UL, 0x00000001UL,       // x^254976 mod p(x)` << 1, x^255040 mod p(x)` << 1
      0xe12ccc12UL, 0x00000001UL, 0xa7192ca6UL, 0x00000001UL,       // x^253952 mod p(x)` << 1, x^254016 mod p(x)` << 1
      0xa0b9d4acUL, 0x00000000UL, 0x9a64bab2UL, 0x00000001UL,       // x^252928 mod p(x)` << 1, x^252992 mod p(x)` << 1
      0x95e8ddfeUL, 0x00000000UL, 0x14f4ed2eUL, 0x00000000UL,       // x^251904 mod p(x)` << 1, x^251968 mod p(x)` << 1
      0x233fddc4UL, 0x00000000UL, 0x1092b6a2UL, 0x00000001UL,       // x^250880 mod p(x)` << 1, x^250944 mod p(x)` << 1
      0xb4529b62UL, 0x00000001UL, 0xc8a1629cUL, 0x00000000UL,       // x^249856 mod p(x)` << 1, x^249920 mod p(x)` << 1
      0xa7fa0e64UL, 0x00000001UL, 0x7bf32e8eUL, 0x00000001UL,       // x^248832 mod p(x)` << 1, x^248896 mod p(x)` << 1
      0xb5334592UL, 0x00000001UL, 0xf8cc6582UL, 0x00000001UL,       // x^247808 mod p(x)` << 1, x^247872 mod p(x)` << 1
      0x1f8ee1b4UL, 0x00000001UL, 0x8631ddf0UL, 0x00000000UL,       // x^246784 mod p(x)` << 1, x^246848 mod p(x)` << 1
      0x6252e632UL, 0x00000000UL, 0x7e5a76d0UL, 0x00000000UL,       // x^245760 mod p(x)` << 1, x^245824 mod p(x)` << 1
      0xab973e84UL, 0x00000000UL, 0x2b09b31cUL, 0x00000000UL,       // x^244736 mod p(x)` << 1, x^244800 mod p(x)` << 1
      0x7734f5ecUL, 0x00000000UL, 0xb2df1f84UL, 0x00000001UL,       // x^243712 mod p(x)` << 1, x^243776 mod p(x)` << 1
      0x7c547798UL, 0x00000000UL, 0xd6f56afcUL, 0x00000001UL,       // x^242688 mod p(x)` << 1, x^242752 mod p(x)` << 1
      0x7ec40210UL, 0x00000000UL, 0xb9b5e70cUL, 0x00000001UL,       // x^241664 mod p(x)` << 1, x^241728 mod p(x)` << 1
      0xab1695a8UL, 0x00000001UL, 0x34b626d2UL, 0x00000000UL,       // x^240640 mod p(x)` << 1, x^240704 mod p(x)` << 1
      0x90494bbaUL, 0x00000000UL, 0x4c53479aUL, 0x00000001UL,       // x^239616 mod p(x)` << 1, x^239680 mod p(x)` << 1
      0x123fb816UL, 0x00000001UL, 0xa6d179a4UL, 0x00000001UL,       // x^238592 mod p(x)` << 1, x^238656 mod p(x)` << 1
      0xe188c74cUL, 0x00000001UL, 0x5abd16b4UL, 0x00000001UL,       // x^237568 mod p(x)` << 1, x^237632 mod p(x)` << 1
      0xc2d3451cUL, 0x00000001UL, 0x018f9852UL, 0x00000000UL,       // x^236544 mod p(x)` << 1, x^236608 mod p(x)` << 1
      0xf55cf1caUL, 0x00000000UL, 0x1fb3084aUL, 0x00000000UL,       // x^235520 mod p(x)` << 1, x^235584 mod p(x)` << 1
      0xa0531540UL, 0x00000001UL, 0xc53dfb04UL, 0x00000000UL,       // x^234496 mod p(x)` << 1, x^234560 mod p(x)` << 1
      0x32cd7ebcUL, 0x00000001UL, 0xe10c9ad6UL, 0x00000000UL,       // x^233472 mod p(x)` << 1, x^233536 mod p(x)` << 1
      0x73ab7f36UL, 0x00000000UL, 0x25aa994aUL, 0x00000000UL,       // x^232448 mod p(x)` << 1, x^232512 mod p(x)` << 1
      0x41aed1c2UL, 0x00000000UL, 0xfa3a74c4UL, 0x00000000UL,       // x^231424 mod p(x)` << 1, x^231488 mod p(x)` << 1
      0x36c53800UL, 0x00000001UL, 0x33eb3f40UL, 0x00000000UL,       // x^230400 mod p(x)` << 1, x^230464 mod p(x)` << 1
      0x26835a30UL, 0x00000001UL, 0x7193f296UL, 0x00000001UL,       // x^229376 mod p(x)` << 1, x^229440 mod p(x)` << 1
      0x6241b502UL, 0x00000000UL, 0x43f6c86aUL, 0x00000000UL,       // x^228352 mod p(x)` << 1, x^228416 mod p(x)` << 1
      0xd5196ad4UL, 0x00000000UL, 0x6b513ec6UL, 0x00000001UL,       // x^227328 mod p(x)` << 1, x^227392 mod p(x)` << 1
      0x9cfa769aUL, 0x00000000UL, 0xc8f25b4eUL, 0x00000000UL,       // x^226304 mod p(x)` << 1, x^226368 mod p(x)` << 1
      0x920e5df4UL, 0x00000000UL, 0xa45048ecUL, 0x00000001UL,       // x^225280 mod p(x)` << 1, x^225344 mod p(x)` << 1
      0x69dc310eUL, 0x00000001UL, 0x0c441004UL, 0x00000000UL,       // x^224256 mod p(x)` << 1, x^224320 mod p(x)` << 1
      0x09fc331cUL, 0x00000000UL, 0x0e17cad6UL, 0x00000000UL,       // x^223232 mod p(x)` << 1, x^223296 mod p(x)` << 1
      0x0d94a81eUL, 0x00000001UL, 0x253ae964UL, 0x00000001UL,       // x^222208 mod p(x)` << 1, x^222272 mod p(x)` << 1
      0x27a20ab2UL, 0x00000000UL, 0xd7c88ebcUL, 0x00000001UL,       // x^221184 mod p(x)` << 1, x^221248 mod p(x)` << 1
      0x14f87504UL, 0x00000001UL, 0xe7ca913aUL, 0x00000001UL,       // x^220160 mod p(x)` << 1, x^220224 mod p(x)` << 1
      0x4b076d96UL, 0x00000000UL, 0x33ed078aUL, 0x00000000UL,       // x^219136 mod p(x)` << 1, x^219200 mod p(x)` << 1
      0xda4d1e74UL, 0x00000000UL, 0xe1839c78UL, 0x00000000UL,       // x^218112 mod p(x)` << 1, x^218176 mod p(x)` << 1
      0x1b81f672UL, 0x00000000UL, 0x322b267eUL, 0x00000001UL,       // x^217088 mod p(x)` << 1, x^217152 mod p(x)` << 1
      0x9367c988UL, 0x00000000UL, 0x638231b6UL, 0x00000000UL,       // x^216064 mod p(x)` << 1, x^216128 mod p(x)` << 1
      0x717214caUL, 0x00000001UL, 0xee7f16f4UL, 0x00000001UL,       // x^215040 mod p(x)` << 1, x^215104 mod p(x)` << 1
      0x9f47d820UL, 0x00000000UL, 0x17d9924aUL, 0x00000001UL,       // x^214016 mod p(x)` << 1, x^214080 mod p(x)` << 1
      0x0d9a47d2UL, 0x00000001UL, 0xe1a9e0c4UL, 0x00000000UL,       // x^212992 mod p(x)` << 1, x^213056 mod p(x)` << 1
      0xa696c58cUL, 0x00000000UL, 0x403731dcUL, 0x00000001UL,       // x^211968 mod p(x)` << 1, x^212032 mod p(x)` << 1
      0x2aa28ec6UL, 0x00000000UL, 0xa5ea9682UL, 0x00000001UL,       // x^210944 mod p(x)` << 1, x^211008 mod p(x)` << 1
      0xfe18fd9aUL, 0x00000001UL, 0x01c5c578UL, 0x00000001UL,       // x^209920 mod p(x)` << 1, x^209984 mod p(x)` << 1
      0x9d4fc1aeUL, 0x00000001UL, 0xdddf6494UL, 0x00000000UL,       // x^208896 mod p(x)` << 1, x^208960 mod p(x)` << 1
      0xba0e3deaUL, 0x00000001UL, 0xf1c3db28UL, 0x00000000UL,       // x^207872 mod p(x)` << 1, x^207936 mod p(x)` << 1
      0x74b59a5eUL, 0x00000000UL, 0x3112fb9cUL, 0x00000001UL,       // x^206848 mod p(x)` << 1, x^206912 mod p(x)` << 1
      0xf2b5ea98UL, 0x00000000UL, 0xb680b906UL, 0x00000000UL,       // x^205824 mod p(x)` << 1, x^205888 mod p(x)` << 1
      0x87132676UL, 0x00000001UL, 0x1a282932UL, 0x00000000UL,       // x^204800 mod p(x)` << 1, x^204864 mod p(x)` << 1
      0x0a8c6ad4UL, 0x00000001UL, 0x89406e7eUL, 0x00000000UL,       // x^203776 mod p(x)` << 1, x^203840 mod p(x)` << 1
      0xe21dfe70UL, 0x00000001UL, 0xdef6be8cUL, 0x00000001UL,       // x^202752 mod p(x)` << 1, x^202816 mod p(x)` << 1
      0xda0050e4UL, 0x00000001UL, 0x75258728UL, 0x00000000UL,       // x^201728 mod p(x)` << 1, x^201792 mod p(x)` << 1
      0x772172aeUL, 0x00000000UL, 0x9536090aUL, 0x00000001UL,       // x^200704 mod p(x)` << 1, x^200768 mod p(x)` << 1
      0xe47724aaUL, 0x00000000UL, 0xf2455bfcUL, 0x00000000UL,       // x^199680 mod p(x)` << 1, x^199744 mod p(x)` << 1
      0x3cd63ac4UL, 0x00000000UL, 0x8c40baf4UL, 0x00000001UL,       // x^198656 mod p(x)` << 1, x^198720 mod p(x)` << 1
      0xbf47d352UL, 0x00000001UL, 0x4cd390d4UL, 0x00000000UL,       // x^197632 mod p(x)` << 1, x^197696 mod p(x)` << 1
      0x8dc1d708UL, 0x00000001UL, 0xe4ece95aUL, 0x00000001UL,       // x^196608 mod p(x)` << 1, x^196672 mod p(x)` << 1
      0x2d4620a4UL, 0x00000000UL, 0x1a3ee918UL, 0x00000000UL,       // x^195584 mod p(x)` << 1, x^195648 mod p(x)` << 1
      0x58fd1740UL, 0x00000000UL, 0x7c652fb8UL, 0x00000000UL,       // x^194560 mod p(x)` << 1, x^194624 mod p(x)` << 1
      0xdadd9bfcUL, 0x00000000UL, 0x1c67842cUL, 0x00000001UL,       // x^193536 mod p(x)` << 1, x^193600 mod p(x)` << 1
      0xea2140beUL, 0x00000001UL, 0x254f759cUL, 0x00000000UL,       // x^192512 mod p(x)` << 1, x^192576 mod p(x)` << 1
      0x9de128baUL, 0x00000000UL, 0x7ece94caUL, 0x00000000UL,       // x^191488 mod p(x)` << 1, x^191552 mod p(x)` << 1
      0x3ac3aa8eUL, 0x00000001UL, 0x38f258c2UL, 0x00000000UL,       // x^190464 mod p(x)` << 1, x^190528 mod p(x)` << 1
      0x99980562UL, 0x00000000UL, 0xcdf17b00UL, 0x00000001UL,       // x^189440 mod p(x)` << 1, x^189504 mod p(x)` << 1
      0xc1579c86UL, 0x00000001UL, 0x1f882c16UL, 0x00000001UL,       // x^188416 mod p(x)` << 1, x^188480 mod p(x)` << 1
      0x68dbbf94UL, 0x00000000UL, 0x00093fc8UL, 0x00000001UL,       // x^187392 mod p(x)` << 1, x^187456 mod p(x)` << 1
      0x4509fb04UL, 0x00000000UL, 0xcd684f16UL, 0x00000001UL,       // x^186368 mod p(x)` << 1, x^186432 mod p(x)` << 1
      0x202f6398UL, 0x00000001UL, 0x4bc6a70aUL, 0x00000000UL,       // x^185344 mod p(x)` << 1, x^185408 mod p(x)` << 1
      0x3aea243eUL, 0x00000001UL, 0x4fc7e8e4UL, 0x00000000UL,       // x^184320 mod p(x)` << 1, x^184384 mod p(x)` << 1
      0xb4052ae6UL, 0x00000001UL, 0x30103f1cUL, 0x00000001UL,       // x^183296 mod p(x)` << 1, x^183360 mod p(x)` << 1
      0xcd2a0ae8UL, 0x00000001UL, 0x11b0024cUL, 0x00000001UL,       // x^182272 mod p(x)` << 1, x^182336 mod p(x)` << 1
      0xfe4aa8b4UL, 0x00000001UL, 0x0b3079daUL, 0x00000001UL,       // x^181248 mod p(x)` << 1, x^181312 mod p(x)` << 1
      0xd1559a42UL, 0x00000001UL, 0x0192bcc2UL, 0x00000001UL,       // x^180224 mod p(x)` << 1, x^180288 mod p(x)` << 1
      0xf3e05eccUL, 0x00000001UL, 0x74838d50UL, 0x00000000UL,       // x^179200 mod p(x)` << 1, x^179264 mod p(x)` << 1
      0x04ddd2ccUL, 0x00000001UL, 0x1b20f520UL, 0x00000000UL,       // x^178176 mod p(x)` << 1, x^178240 mod p(x)` << 1
      0x5393153cUL, 0x00000001UL, 0x50c3590aUL, 0x00000000UL,       // x^177152 mod p(x)` << 1, x^177216 mod p(x)` << 1
      0x57e942c6UL, 0x00000000UL, 0xb41cac8eUL, 0x00000000UL,       // x^176128 mod p(x)` << 1, x^176192 mod p(x)` << 1
      0x2c633850UL, 0x00000001UL, 0x0c72cc78UL, 0x00000000UL,       // x^175104 mod p(x)` << 1, x^175168 mod p(x)` << 1
      0xebcaae4cUL, 0x00000000UL, 0x30cdb032UL, 0x00000000UL,       // x^174080 mod p(x)` << 1, x^174144 mod p(x)` << 1
      0x3ee532a6UL, 0x00000001UL, 0x3e09fc32UL, 0x00000001UL,       // x^173056 mod p(x)` << 1, x^173120 mod p(x)` << 1
      0xbf0cbc7eUL, 0x00000001UL, 0x1ed624d2UL, 0x00000000UL,       // x^172032 mod p(x)` << 1, x^172096 mod p(x)` << 1
      0xd50b7a5aUL, 0x00000000UL, 0x781aee1aUL, 0x00000000UL,       // x^171008 mod p(x)` << 1, x^171072 mod p(x)` << 1
      0x02fca6e8UL, 0x00000000UL, 0xc4d8348cUL, 0x00000001UL,       // x^169984 mod p(x)` << 1, x^170048 mod p(x)` << 1
      0x7af40044UL, 0x00000000UL, 0x57a40336UL, 0x00000000UL,       // x^168960 mod p(x)` << 1, x^169024 mod p(x)` << 1
      0x16178744UL, 0x00000000UL, 0x85544940UL, 0x00000000UL,       // x^167936 mod p(x)` << 1, x^168000 mod p(x)` << 1
      0x4c177458UL, 0x00000001UL, 0x9cd21e80UL, 0x00000001UL,       // x^166912 mod p(x)` << 1, x^166976 mod p(x)` << 1
      0x1b6ddf04UL, 0x00000001UL, 0x3eb95bc0UL, 0x00000001UL,       // x^165888 mod p(x)` << 1, x^165952 mod p(x)` << 1
      0xf3e29cccUL, 0x00000001UL, 0xdfc9fdfcUL, 0x00000001UL,       // x^164864 mod p(x)` << 1, x^164928 mod p(x)` << 1
      0x35ae7562UL, 0x00000001UL, 0xcd028bc2UL, 0x00000000UL,       // x^163840 mod p(x)` << 1, x^163904 mod p(x)` << 1
      0x90ef812cUL, 0x00000001UL, 0x90db8c44UL, 0x00000000UL,       // x^162816 mod p(x)` << 1, x^162880 mod p(x)` << 1
      0x67a2c786UL, 0x00000000UL, 0x0010a4ceUL, 0x00000001UL,       // x^161792 mod p(x)` << 1, x^161856 mod p(x)` << 1
      0x48b9496cUL, 0x00000000UL, 0xc8f4c72cUL, 0x00000001UL,       // x^160768 mod p(x)` << 1, x^160832 mod p(x)` << 1
      0x5a422de6UL, 0x00000001UL, 0x1c26170cUL, 0x00000000UL,       // x^159744 mod p(x)` << 1, x^159808 mod p(x)` << 1
      0xef0e3640UL, 0x00000001UL, 0xe3fccf68UL, 0x00000000UL,       // x^158720 mod p(x)` << 1, x^158784 mod p(x)` << 1
      0x006d2d26UL, 0x00000001UL, 0xd513ed24UL, 0x00000000UL,       // x^157696 mod p(x)` << 1, x^157760 mod p(x)` << 1
      0x170d56d6UL, 0x00000001UL, 0x141beadaUL, 0x00000000UL,       // x^156672 mod p(x)` << 1, x^156736 mod p(x)` << 1
      0xa5fb613cUL, 0x00000000UL, 0x1071aea0UL, 0x00000001UL,       // x^155648 mod p(x)` << 1, x^155712 mod p(x)` << 1
      0x40bbf7fcUL, 0x00000000UL, 0x2e19080aUL, 0x00000001UL,       // x^154624 mod p(x)` << 1, x^154688 mod p(x)` << 1
      0x6ac3a5b2UL, 0x00000001UL, 0x00ecf826UL, 0x00000001UL,       // x^153600 mod p(x)` << 1, x^153664 mod p(x)` << 1
      0xabf16230UL, 0x00000000UL, 0x69b09412UL, 0x00000000UL,       // x^152576 mod p(x)` << 1, x^152640 mod p(x)` << 1
      0xebe23facUL, 0x00000001UL, 0x22297bacUL, 0x00000001UL,       // x^151552 mod p(x)` << 1, x^151616 mod p(x)` << 1
      0x8b6a0894UL, 0x00000000UL, 0xe9e4b068UL, 0x00000000UL,       // x^150528 mod p(x)` << 1, x^150592 mod p(x)` << 1
      0x288ea478UL, 0x00000001UL, 0x4b38651aUL, 0x00000000UL,       // x^149504 mod p(x)` << 1, x^149568 mod p(x)` << 1
      0x6619c442UL, 0x00000001UL, 0x468360e2UL, 0x00000001UL,       // x^148480 mod p(x)` << 1, x^148544 mod p(x)` << 1
      0x86230038UL, 0x00000000UL, 0x121c2408UL, 0x00000000UL,       // x^147456 mod p(x)` << 1, x^147520 mod p(x)` << 1
      0x7746a756UL, 0x00000001UL, 0xda7e7d08UL, 0x00000000UL,       // x^146432 mod p(x)` << 1, x^146496 mod p(x)` << 1
      0x91b8f8f8UL, 0x00000001UL, 0x058d7652UL, 0x00000001UL,       // x^145408 mod p(x)` << 1, x^145472 mod p(x)` << 1
      0x8e167708UL, 0x00000000UL, 0x4a098a90UL, 0x00000001UL,       // x^144384 mod p(x)` << 1, x^144448 mod p(x)` << 1
      0x48b22d54UL, 0x00000001UL, 0x20dbe72eUL, 0x00000000UL,       // x^143360 mod p(x)` << 1, x^143424 mod p(x)` << 1
      0x44ba2c3cUL, 0x00000000UL, 0x1e7323e8UL, 0x00000001UL,       // x^142336 mod p(x)` << 1, x^142400 mod p(x)` << 1
      0xb54d2b52UL, 0x00000000UL, 0xd5d4bf94UL, 0x00000000UL,       // x^141312 mod p(x)` << 1, x^141376 mod p(x)` << 1
      0x05a4fd8aUL, 0x00000000UL, 0x99d8746cUL, 0x00000001UL,       // x^140288 mod p(x)` << 1, x^140352 mod p(x)` << 1
      0x39f9fc46UL, 0x00000001UL, 0xce9ca8a0UL, 0x00000000UL,       // x^139264 mod p(x)` << 1, x^139328 mod p(x)` << 1
      0x5a1fa824UL, 0x00000001UL, 0x136edeceUL, 0x00000000UL,       // x^138240 mod p(x)` << 1, x^138304 mod p(x)` << 1
      0x0a61ae4cUL, 0x00000000UL, 0x9b92a068UL, 0x00000001UL,       // x^137216 mod p(x)` << 1, x^137280 mod p(x)` << 1
      0x45e9113eUL, 0x00000001UL, 0x71d62206UL, 0x00000000UL,       // x^136192 mod p(x)` << 1, x^136256 mod p(x)` << 1
      0x6a348448UL, 0x00000000UL, 0xdfc50158UL, 0x00000000UL,       // x^135168 mod p(x)` << 1, x^135232 mod p(x)` << 1
      0x4d80a08cUL, 0x00000000UL, 0x517626bcUL, 0x00000001UL,       // x^134144 mod p(x)` << 1, x^134208 mod p(x)` << 1
      0x4b6837a0UL, 0x00000001UL, 0x48d1e4faUL, 0x00000001UL,       // x^133120 mod p(x)` << 1, x^133184 mod p(x)` << 1
      0x6896a7fcUL, 0x00000001UL, 0x94d8266eUL, 0x00000000UL,       // x^132096 mod p(x)` << 1, x^132160 mod p(x)` << 1
      0x4f187140UL, 0x00000001UL, 0x606c5e34UL, 0x00000000UL,       // x^131072 mod p(x)` << 1, x^131136 mod p(x)` << 1
      0x9581b9daUL, 0x00000001UL, 0x9766beaaUL, 0x00000001UL,       // x^130048 mod p(x)` << 1, x^130112 mod p(x)` << 1
      0x091bc984UL, 0x00000001UL, 0xd80c506cUL, 0x00000001UL,       // x^129024 mod p(x)` << 1, x^129088 mod p(x)` << 1
      0x1067223cUL, 0x00000000UL, 0x1e73837cUL, 0x00000000UL,       // x^128000 mod p(x)` << 1, x^128064 mod p(x)` << 1
      0xab16ea02UL, 0x00000001UL, 0x64d587deUL, 0x00000000UL,       // x^126976 mod p(x)` << 1, x^127040 mod p(x)` << 1
      0x3c4598a8UL, 0x00000001UL, 0xf4a507b0UL, 0x00000000UL,       // x^125952 mod p(x)` << 1, x^126016 mod p(x)` << 1
      0xb3735430UL, 0x00000000UL, 0x40e342fcUL, 0x00000000UL,       // x^124928 mod p(x)` << 1, x^124992 mod p(x)` << 1
      0xbb3fc0c0UL, 0x00000001UL, 0xd5ad9c3aUL, 0x00000001UL,       // x^123904 mod p(x)` << 1, x^123968 mod p(x)` << 1
      0x570ae19cUL, 0x00000001UL, 0x94a691a4UL, 0x00000000UL,       // x^122880 mod p(x)` << 1, x^122944 mod p(x)` << 1
      0xea910712UL, 0x00000001UL, 0x271ecdfaUL, 0x00000001UL,       // x^121856 mod p(x)` << 1, x^121920 mod p(x)` << 1
      0x67127128UL, 0x00000001UL, 0x9e54475aUL, 0x00000000UL,       // x^120832 mod p(x)` << 1, x^120896 mod p(x)` << 1
      0x19e790a2UL, 0x00000000UL, 0xc9c099eeUL, 0x00000000UL,       // x^119808 mod p(x)` << 1, x^119872 mod p(x)` << 1
      0x3788f710UL, 0x00000000UL, 0x9a2f736cUL, 0x00000000UL,       // x^118784 mod p(x)` << 1, x^118848 mod p(x)` << 1
      0x682a160eUL, 0x00000001UL, 0xbb9f4996UL, 0x00000000UL,       // x^117760 mod p(x)` << 1, x^117824 mod p(x)` << 1
      0x7f0ebd2eUL, 0x00000000UL, 0xdb688050UL, 0x00000001UL,       // x^116736 mod p(x)` << 1, x^116800 mod p(x)` << 1
      0x2b032080UL, 0x00000000UL, 0xe9b10af4UL, 0x00000000UL,       // x^115712 mod p(x)` << 1, x^115776 mod p(x)` << 1
      0xcfd1664aUL, 0x00000000UL, 0x2d4545e4UL, 0x00000001UL,       // x^114688 mod p(x)` << 1, x^114752 mod p(x)` << 1
      0xaa1181c2UL, 0x00000000UL, 0x0361139cUL, 0x00000000UL,       // x^113664 mod p(x)` << 1, x^113728 mod p(x)` << 1
      0xddd08002UL, 0x00000000UL, 0xa5a1a3a8UL, 0x00000001UL,       // x^112640 mod p(x)` << 1, x^112704 mod p(x)` << 1
      0xe8dd0446UL, 0x00000000UL, 0x6844e0b0UL, 0x00000000UL,       // x^111616 mod p(x)` << 1, x^111680 mod p(x)` << 1
      0xbbd94a00UL, 0x00000001UL, 0xc3762f28UL, 0x00000000UL,       // x^110592 mod p(x)` << 1, x^110656 mod p(x)` << 1
      0xab6cd180UL, 0x00000000UL, 0xd26287a2UL, 0x00000001UL,       // x^109568 mod p(x)` << 1, x^109632 mod p(x)` << 1
      0x31803ce2UL, 0x00000000UL, 0xf6f0bba8UL, 0x00000001UL,       // x^108544 mod p(x)` << 1, x^108608 mod p(x)` << 1
      0x24f40b0cUL, 0x00000000UL, 0x2ffabd62UL, 0x00000000UL,       // x^107520 mod p(x)` << 1, x^107584 mod p(x)` << 1
      0xba1d9834UL, 0x00000001UL, 0xfb4516b8UL, 0x00000000UL,       // x^106496 mod p(x)` << 1, x^106560 mod p(x)` << 1
      0x04de61aaUL, 0x00000001UL, 0x8cfa961cUL, 0x00000001UL,       // x^105472 mod p(x)` << 1, x^105536 mod p(x)` << 1
      0x13e40d46UL, 0x00000001UL, 0x9e588d52UL, 0x00000001UL,       // x^104448 mod p(x)` << 1, x^104512 mod p(x)` << 1
      0x415598a0UL, 0x00000001UL, 0x180f0bbcUL, 0x00000001UL,       // x^103424 mod p(x)` << 1, x^103488 mod p(x)` << 1
      0xbf6c8c90UL, 0x00000000UL, 0xe1d9177aUL, 0x00000000UL,       // x^102400 mod p(x)` << 1, x^102464 mod p(x)` << 1
      0x788b0504UL, 0x00000001UL, 0x05abc27cUL, 0x00000001UL,       // x^101376 mod p(x)` << 1, x^101440 mod p(x)` << 1
      0x38385d02UL, 0x00000000UL, 0x972e4a58UL, 0x00000000UL,       // x^100352 mod p(x)` << 1, x^100416 mod p(x)` << 1
      0xb6c83844UL, 0x00000001UL, 0x83499a5eUL, 0x00000001UL,       // x^99328 mod p(x)` << 1, x^99392 mod p(x)` << 1
      0x51061a8aUL, 0x00000000UL, 0xc96a8ccaUL, 0x00000001UL,       // x^98304 mod p(x)` << 1, x^98368 mod p(x)` << 1
      0x7351388aUL, 0x00000001UL, 0xa1a5b60cUL, 0x00000001UL,       // x^97280 mod p(x)` << 1, x^97344 mod p(x)` << 1
      0x32928f92UL, 0x00000001UL, 0xe4b6ac9cUL, 0x00000000UL,       // x^96256 mod p(x)` << 1, x^96320 mod p(x)` << 1
      0xe6b4f48aUL, 0x00000000UL, 0x807e7f5aUL, 0x00000001UL,       // x^95232 mod p(x)` << 1, x^95296 mod p(x)` << 1
      0x39d15e90UL, 0x00000000UL, 0x7a7e3bc8UL, 0x00000001UL,       // x^94208 mod p(x)` << 1, x^94272 mod p(x)` << 1
      0x312d6074UL, 0x00000000UL, 0xd73975daUL, 0x00000000UL,       // x^93184 mod p(x)` << 1, x^93248 mod p(x)` << 1
      0x7bbb2cc4UL, 0x00000001UL, 0x7375d038UL, 0x00000001UL,       // x^92160 mod p(x)` << 1, x^92224 mod p(x)` << 1
      0x6ded3e18UL, 0x00000001UL, 0x193680bcUL, 0x00000000UL,       // x^91136 mod p(x)` << 1, x^91200 mod p(x)` << 1
      0xf1638b16UL, 0x00000000UL, 0x999b06f6UL, 0x00000000UL,       // x^90112 mod p(x)` << 1, x^90176 mod p(x)` << 1
      0xd38b9eccUL, 0x00000001UL, 0xf685d2b8UL, 0x00000001UL,       // x^89088 mod p(x)` << 1, x^89152 mod p(x)` << 1
      0x8b8d09dcUL, 0x00000001UL, 0xf4ecbed2UL, 0x00000001UL,       // x^88064 mod p(x)` << 1, x^88128 mod p(x)` << 1
      0xe7bc27d2UL, 0x00000000UL, 0xba16f1a0UL, 0x00000000UL,       // x^87040 mod p(x)` << 1, x^87104 mod p(x)` << 1
      0x275e1e96UL, 0x00000000UL, 0x15aceac4UL, 0x00000001UL,       // x^86016 mod p(x)` << 1, x^86080 mod p(x)` << 1
      0xe2e3031eUL, 0x00000000UL, 0xaeff6292UL, 0x00000001UL,       // x^84992 mod p(x)` << 1, x^85056 mod p(x)` << 1
      0x041c84d8UL, 0x00000001UL, 0x9640124cUL, 0x00000000UL,       // x^83968 mod p(x)` << 1, x^84032 mod p(x)` << 1
      0x706ce672UL, 0x00000000UL, 0x14f41f02UL, 0x00000001UL,       // x^82944 mod p(x)` << 1, x^83008 mod p(x)` << 1
      0x5d5070daUL, 0x00000001UL, 0x9c5f3586UL, 0x00000000UL,       // x^81920 mod p(x)` << 1, x^81984 mod p(x)` << 1
      0x38f9493aUL, 0x00000000UL, 0x878275faUL, 0x00000001UL,       // x^80896 mod p(x)` << 1, x^80960 mod p(x)` << 1
      0xa3348a76UL, 0x00000000UL, 0xddc42ce8UL, 0x00000000UL,       // x^79872 mod p(x)` << 1, x^79936 mod p(x)` << 1
      0xad0aab92UL, 0x00000001UL, 0x81d2c73aUL, 0x00000001UL,       // x^78848 mod p(x)` << 1, x^78912 mod p(x)` << 1
      0x9e85f712UL, 0x00000001UL, 0x41c9320aUL, 0x00000001UL,       // x^77824 mod p(x)` << 1, x^77888 mod p(x)` << 1
      0x5a871e76UL, 0x00000000UL, 0x5235719aUL, 0x00000001UL,       // x^76800 mod p(x)` << 1, x^76864 mod p(x)` << 1
      0x7249c662UL, 0x00000001UL, 0xbe27d804UL, 0x00000000UL,       // x^75776 mod p(x)` << 1, x^75840 mod p(x)` << 1
      0x3a084712UL, 0x00000000UL, 0x6242d45aUL, 0x00000000UL,       // x^74752 mod p(x)` << 1, x^74816 mod p(x)` << 1
      0xed438478UL, 0x00000000UL, 0x9a53638eUL, 0x00000000UL,       // x^73728 mod p(x)` << 1, x^73792 mod p(x)` << 1
      0xabac34ccUL, 0x00000000UL, 0x001ecfb6UL, 0x00000001UL,       // x^72704 mod p(x)` << 1, x^72768 mod p(x)` << 1
      0x5f35ef3eUL, 0x00000000UL, 0x6d7c2d64UL, 0x00000001UL,       // x^71680 mod p(x)` << 1, x^71744 mod p(x)` << 1
      0x47d6608cUL, 0x00000000UL, 0xd0ce46c0UL, 0x00000001UL,       // x^70656 mod p(x)` << 1, x^70720 mod p(x)` << 1
      0x2d01470eUL, 0x00000000UL, 0x24c907b4UL, 0x00000001UL,       // x^69632 mod p(x)` << 1, x^69696 mod p(x)` << 1
      0x58bbc7b0UL, 0x00000001UL, 0x18a555caUL, 0x00000000UL,       // x^68608 mod p(x)` << 1, x^68672 mod p(x)` << 1
      0xc0a23e8eUL, 0x00000000UL, 0x6b0980bcUL, 0x00000000UL,       // x^67584 mod p(x)` << 1, x^67648 mod p(x)` << 1
      0xebd85c88UL, 0x00000001UL, 0x8bbba964UL, 0x00000000UL,       // x^66560 mod p(x)` << 1, x^66624 mod p(x)` << 1
      0x9ee20bb2UL, 0x00000001UL, 0x070a5a1eUL, 0x00000001UL,       // x^65536 mod p(x)` << 1, x^65600 mod p(x)` << 1
      0xacabf2d6UL, 0x00000001UL, 0x2204322aUL, 0x00000000UL,       // x^64512 mod p(x)` << 1, x^64576 mod p(x)` << 1
      0xb7963d56UL, 0x00000001UL, 0xa27524d0UL, 0x00000000UL,       // x^63488 mod p(x)` << 1, x^63552 mod p(x)` << 1
      0x7bffa1feUL, 0x00000001UL, 0x20b1e4baUL, 0x00000000UL,       // x^62464 mod p(x)` << 1, x^62528 mod p(x)` << 1
      0x1f15333eUL, 0x00000000UL, 0x32cc27fcUL, 0x00000000UL,       // x^61440 mod p(x)` << 1, x^61504 mod p(x)` << 1
      0x8593129eUL, 0x00000001UL, 0x44dd22b8UL, 0x00000000UL,       // x^60416 mod p(x)` << 1, x^60480 mod p(x)` << 1
      0x9cb32602UL, 0x00000001UL, 0xdffc9e0aUL, 0x00000000UL,       // x^59392 mod p(x)` << 1, x^59456 mod p(x)` << 1
      0x42b05cc8UL, 0x00000001UL, 0xb7a0ed14UL, 0x00000001UL,       // x^58368 mod p(x)` << 1, x^58432 mod p(x)` << 1
      0xbe49e7a4UL, 0x00000001UL, 0xc7842488UL, 0x00000000UL,       // x^57344 mod p(x)` << 1, x^57408 mod p(x)` << 1
      0x08f69d6cUL, 0x00000001UL, 0xc02a4feeUL, 0x00000001UL,       // x^56320 mod p(x)` << 1, x^56384 mod p(x)` << 1
      0x6c0971f0UL, 0x00000000UL, 0x3c273778UL, 0x00000000UL,       // x^55296 mod p(x)` << 1, x^55360 mod p(x)` << 1
      0x5b16467aUL, 0x00000000UL, 0xd63f8894UL, 0x00000001UL,       // x^54272 mod p(x)` << 1, x^54336 mod p(x)` << 1
      0x551a628eUL, 0x00000001UL, 0x6be557d6UL, 0x00000000UL,       // x^53248 mod p(x)` << 1, x^53312 mod p(x)` << 1
      0x9e42ea92UL, 0x00000001UL, 0x6a7806eaUL, 0x00000000UL,       // x^52224 mod p(x)` << 1, x^52288 mod p(x)` << 1
      0x2fa83ff2UL, 0x00000001UL, 0x6155aa0cUL, 0x00000001UL,       // x^51200 mod p(x)` << 1, x^51264 mod p(x)` << 1
      0x1ca9cde0UL, 0x00000001UL, 0x908650acUL, 0x00000000UL,       // x^50176 mod p(x)` << 1, x^50240 mod p(x)` << 1
      0xc8e5cd74UL, 0x00000000UL, 0xaa5a8084UL, 0x00000000UL,       // x^49152 mod p(x)` << 1, x^49216 mod p(x)` << 1
      0x96c27f0cUL, 0x00000000UL, 0x91bb500aUL, 0x00000001UL,       // x^48128 mod p(x)` << 1, x^48192 mod p(x)` << 1
      0x2baed926UL, 0x00000000UL, 0x64e9bed0UL, 0x00000000UL,       // x^47104 mod p(x)` << 1, x^47168 mod p(x)` << 1
      0x7c8de8d2UL, 0x00000001UL, 0x9444f302UL, 0x00000000UL,       // x^46080 mod p(x)` << 1, x^46144 mod p(x)` << 1
      0xd43d6068UL, 0x00000000UL, 0x9db07d3cUL, 0x00000001UL,       // x^45056 mod p(x)` << 1, x^45120 mod p(x)` << 1
      0xcb2c4b26UL, 0x00000000UL, 0x359e3e6eUL, 0x00000001UL,       // x^44032 mod p(x)` << 1, x^44096 mod p(x)` << 1
      0x45b8da26UL, 0x00000001UL, 0xe4f10dd2UL, 0x00000001UL,       // x^43008 mod p(x)` << 1, x^43072 mod p(x)` << 1
      0x8fff4b08UL, 0x00000001UL, 0x24f5735eUL, 0x00000001UL,       // x^41984 mod p(x)` << 1, x^42048 mod p(x)` << 1
      0x50b58ed0UL, 0x00000001UL, 0x24760a4cUL, 0x00000001UL,       // x^40960 mod p(x)` << 1, x^41024 mod p(x)` << 1
      0x549f39bcUL, 0x00000001UL, 0x0f1fc186UL, 0x00000000UL,       // x^39936 mod p(x)` << 1, x^40000 mod p(x)` << 1
      0xef4d2f42UL, 0x00000000UL, 0x150e4cc4UL, 0x00000000UL,       // x^38912 mod p(x)` << 1, x^38976 mod p(x)` << 1
      0xb1468572UL, 0x00000001UL, 0x2a6204e8UL, 0x00000000UL,       // x^37888 mod p(x)` << 1, x^37952 mod p(x)` << 1
      0x3d7403b2UL, 0x00000001UL, 0xbeb1d432UL, 0x00000000UL,       // x^36864 mod p(x)` << 1, x^36928 mod p(x)` << 1
      0xa4681842UL, 0x00000001UL, 0x35f3f1f0UL, 0x00000001UL,       // x^35840 mod p(x)` << 1, x^35904 mod p(x)` << 1
      0x67714492UL, 0x00000001UL, 0x74fe2232UL, 0x00000000UL,       // x^34816 mod p(x)` << 1, x^34880 mod p(x)` << 1
      0xe599099aUL, 0x00000001UL, 0x1ac6e2baUL, 0x00000000UL,       // x^33792 mod p(x)` << 1, x^33856 mod p(x)` << 1
      0xfe128194UL, 0x00000000UL, 0x13fca91eUL, 0x00000000UL,       // x^32768 mod p(x)` << 1, x^32832 mod p(x)` << 1
      0x77e8b990UL, 0x00000000UL, 0x83f4931eUL, 0x00000001UL,       // x^31744 mod p(x)` << 1, x^31808 mod p(x)` << 1
      0xa267f63aUL, 0x00000001UL, 0xb6d9b4e4UL, 0x00000000UL,       // x^30720 mod p(x)` << 1, x^30784 mod p(x)` << 1
      0x945c245aUL, 0x00000001UL, 0xb5188656UL, 0x00000000UL,       // x^29696 mod p(x)` << 1, x^29760 mod p(x)` << 1
      0x49002e76UL, 0x00000001UL, 0x27a81a84UL, 0x00000000UL,       // x^28672 mod p(x)` << 1, x^28736 mod p(x)` << 1
      0xbb8310a4UL, 0x00000001UL, 0x25699258UL, 0x00000001UL,       // x^27648 mod p(x)` << 1, x^27712 mod p(x)` << 1
      0x9ec60bccUL, 0x00000001UL, 0xb23de796UL, 0x00000001UL,       // x^26624 mod p(x)` << 1, x^26688 mod p(x)` << 1
      0x2d8590aeUL, 0x00000001UL, 0xfe4365dcUL, 0x00000000UL,       // x^25600 mod p(x)` << 1, x^25664 mod p(x)` << 1
      0x65b00684UL, 0x00000000UL, 0xc68f497aUL, 0x00000000UL,       // x^24576 mod p(x)` << 1, x^24640 mod p(x)` << 1
      0x5e5aeadcUL, 0x00000001UL, 0xfbf521eeUL, 0x00000000UL,       // x^23552 mod p(x)` << 1, x^23616 mod p(x)` << 1
      0xb77ff2b0UL, 0x00000000UL, 0x5eac3378UL, 0x00000001UL,       // x^22528 mod p(x)` << 1, x^22592 mod p(x)` << 1
      0x88da2ff6UL, 0x00000001UL, 0x34914b90UL, 0x00000001UL,       // x^21504 mod p(x)` << 1, x^21568 mod p(x)` << 1
      0x63da929aUL, 0x00000000UL, 0x16335cfeUL, 0x00000000UL,       // x^20480 mod p(x)` << 1, x^20544 mod p(x)` << 1
      0x389caa80UL, 0x00000001UL, 0x0372d10cUL, 0x00000001UL,       // x^19456 mod p(x)` << 1, x^19520 mod p(x)` << 1
      0x3db599d2UL, 0x00000001UL, 0x5097b908UL, 0x00000001UL,       // x^18432 mod p(x)` << 1, x^18496 mod p(x)` << 1
      0x22505a86UL, 0x00000001UL, 0x227a7572UL, 0x00000001UL,       // x^17408 mod p(x)` << 1, x^17472 mod p(x)` << 1
      0x6bd72746UL, 0x00000001UL, 0x9a8f75c0UL, 0x00000000UL,       // x^16384 mod p(x)` << 1, x^16448 mod p(x)` << 1
      0xc3faf1d4UL, 0x00000001UL, 0x682c77a2UL, 0x00000000UL,       // x^15360 mod p(x)` << 1, x^15424 mod p(x)` << 1
      0x111c826cUL, 0x00000001UL, 0x231f091cUL, 0x00000000UL,       // x^14336 mod p(x)` << 1, x^14400 mod p(x)` << 1
      0x153e9fb2UL, 0x00000000UL, 0x7d4439f2UL, 0x00000000UL,       // x^13312 mod p(x)` << 1, x^13376 mod p(x)` << 1
      0x2b1f7b60UL, 0x00000000UL, 0x7e221efcUL, 0x00000001UL,       // x^12288 mod p(x)` << 1, x^12352 mod p(x)` << 1
      0xb1dba570UL, 0x00000000UL, 0x67457c38UL, 0x00000001UL,       // x^11264 mod p(x)` << 1, x^11328 mod p(x)` << 1
      0xf6397b76UL, 0x00000001UL, 0xbdf081c4UL, 0x00000000UL,       // x^10240 mod p(x)` << 1, x^10304 mod p(x)` << 1
      0x56335214UL, 0x00000001UL, 0x6286d6b0UL, 0x00000001UL,       // x^9216 mod p(x)` << 1, x^9280 mod p(x)` << 1
      0xd70e3986UL, 0x00000001UL, 0xc84f001cUL, 0x00000000UL,       // x^8192 mod p(x)` << 1, x^8256 mod p(x)` << 1
      0x3701a774UL, 0x00000000UL, 0x64efe7c0UL, 0x00000000UL,       // x^7168 mod p(x)` << 1, x^7232 mod p(x)` << 1
      0xac81ef72UL, 0x00000000UL, 0x0ac2d904UL, 0x00000000UL,       // x^6144 mod p(x)` << 1, x^6208 mod p(x)` << 1
      0x33212464UL, 0x00000001UL, 0xfd226d14UL, 0x00000000UL,       // x^5120 mod p(x)` << 1, x^5184 mod p(x)` << 1
      0xe4e45610UL, 0x00000000UL, 0x1cfd42e0UL, 0x00000001UL,       // x^4096 mod p(x)` << 1, x^4160 mod p(x)` << 1
      0x0c1bd370UL, 0x00000000UL, 0x6e5a5678UL, 0x00000001UL,       // x^3072 mod p(x)` << 1, x^3136 mod p(x)` << 1
      0xa7b9e7a6UL, 0x00000001UL, 0xd888fe22UL, 0x00000001UL,       // x^2048 mod p(x)` << 1, x^2112 mod p(x)` << 1
      0x7d657a10UL, 0x00000000UL, 0xaf77fcd4UL, 0x00000001UL,       // x^1024 mod p(x)` << 1, x^1088 mod p(x)` << 1

      // Reduce final 1024-2048 bits to 64 bits, shifting 32 bits to include the trailing 32 bits of zeros
      0xec447f11UL, 0x99168a18UL, 0x13e8221eUL, 0xed837b26UL,       // x^2048 mod p(x)`, x^2016 mod p(x)`, x^1984 mod p(x)`, x^1952 mod p(x)`
      0x8fd2cd3cUL, 0xe23e954eUL, 0x47b9ce5aUL, 0xc8acdd81UL,       // x^1920 mod p(x)`, x^1888 mod p(x)`, x^1856 mod p(x)`, x^1824 mod p(x)`
      0x6b1d2b53UL, 0x92f8befeUL, 0xd4277e25UL, 0xd9ad6d87UL,       // x^1792 mod p(x)`, x^1760 mod p(x)`, x^1728 mod p(x)`, x^1696 mod p(x)`
      0x291ea462UL, 0xf38a3556UL, 0x33fbca3bUL, 0xc10ec5e0UL,       // x^1664 mod p(x)`, x^1632 mod p(x)`, x^1600 mod p(x)`, x^1568 mod p(x)`
      0x62b6ca4bUL, 0x974ac562UL, 0x82e02e2fUL, 0xc0b55b0eUL,       // x^1536 mod p(x)`, x^1504 mod p(x)`, x^1472 mod p(x)`, x^1440 mod p(x)`
      0x784d2a56UL, 0x855712b3UL, 0xe172334dUL, 0x71aa1df0UL,       // x^1408 mod p(x)`, x^1376 mod p(x)`, x^1344 mod p(x)`, x^1312 mod p(x)`
      0x0eaee722UL, 0xa5abe9f8UL, 0x3969324dUL, 0xfee3053eUL,       // x^1280 mod p(x)`, x^1248 mod p(x)`, x^1216 mod p(x)`, x^1184 mod p(x)`
      0xdb54814cUL, 0x1fa0943dUL, 0x3eb2bd08UL, 0xf44779b9UL,       // x^1152 mod p(x)`, x^1120 mod p(x)`, x^1088 mod p(x)`, x^1056 mod p(x)`
      0xd7bbfe6aUL, 0xa53ff440UL, 0x00cc3374UL, 0xf5449b3fUL,       // x^1024 mod p(x)`, x^992 mod p(x)`,  x^960 mod p(x)`,  x^928 mod p(x)`
      0x6325605cUL, 0xebe7e356UL, 0xd777606eUL, 0x6f8346e1UL,       // x^896 mod p(x)`,  x^864 mod p(x)`,  x^832 mod p(x)`,  x^800 mod p(x)`
      0xe5b592b8UL, 0xc65a272cUL, 0xc0b95347UL, 0xe3ab4f2aUL,       // x^768 mod p(x)`,  x^736 mod p(x)`,  x^704 mod p(x)`,  x^672 mod p(x)`
      0x4721589fUL, 0x5705a9caUL, 0x329ecc11UL, 0xaa2215eaUL,       // x^640 mod p(x)`,  x^608 mod p(x)`,  x^576 mod p(x)`,  x^544 mod p(x)`
      0x88d14467UL, 0xe3720acbUL, 0xd95efd26UL, 0x1ed8f66eUL,       // x^512 mod p(x)`,  x^480 mod p(x)`,  x^448 mod p(x)`,  x^416 mod p(x)`
      0x15141c31UL, 0xba1aca03UL, 0xa700e96aUL, 0x78ed02d5UL,       // x^384 mod p(x)`,  x^352 mod p(x)`,  x^320 mod p(x)`,  x^288 mod p(x)`
      0xed627daeUL, 0xad2a31b3UL, 0x32b39da3UL, 0xba8ccbe8UL,       // x^256 mod p(x)`,  x^224 mod p(x)`,  x^192 mod p(x)`,  x^160 mod p(x)`
      0xa06a2517UL, 0x6655004fUL, 0xb1e6b092UL, 0xedb88320UL        // x^128 mod p(x)`,  x^96 mod p(x)`,   x^64 mod p(x)`,   x^32 mod p(x)`
  };

  juint* ptr = (juint*) malloc(sizeof(juint) * CRC32_CONSTANTS_SIZE);
  guarantee(((intptr_t)ptr & 0xF) == 0, "16-byte alignment needed");
  guarantee(ptr != NULL, "allocation error of a crc table");
  memcpy((void*)ptr, constants, sizeof(juint) * CRC32_CONSTANTS_SIZE);
  return ptr;
}

juint* StubRoutines::ppc64::generate_crc_barret_constants() {
  juint barret_constants[CRC32_BARRET_CONSTANTS] = {
      0xf7011641UL, 0x00000001UL, 0x00000000UL, 0x00000000UL,
      0xdb710641UL, 0x00000001UL, 0x00000000UL, 0x00000000UL
  };
  juint* ptr = (juint*) malloc(sizeof(juint) * CRC32_CONSTANTS_SIZE);
  guarantee(((intptr_t)ptr & 0xF) == 0, "16-byte alignment needed");
  guarantee(ptr != NULL, "allocation error of a crc table");
  memcpy((void*) ptr, barret_constants, sizeof(juint) * CRC32_BARRET_CONSTANTS);
  return ptr;
}

// CRC32 Intrinsics.
/**
 *  crc_table[] from jdk/src/share/native/java/util/zip/zlib-1.2.8/crc32.h
 */
juint StubRoutines::ppc64::_crc_table[CRC32_TABLES][CRC32_COLUMN_SIZE] = {
  {
    0x00000000UL, 0x77073096UL, 0xee0e612cUL, 0x990951baUL, 0x076dc419UL,
    0x706af48fUL, 0xe963a535UL, 0x9e6495a3UL, 0x0edb8832UL, 0x79dcb8a4UL,
    0xe0d5e91eUL, 0x97d2d988UL, 0x09b64c2bUL, 0x7eb17cbdUL, 0xe7b82d07UL,
    0x90bf1d91UL, 0x1db71064UL, 0x6ab020f2UL, 0xf3b97148UL, 0x84be41deUL,
    0x1adad47dUL, 0x6ddde4ebUL, 0xf4d4b551UL, 0x83d385c7UL, 0x136c9856UL,
    0x646ba8c0UL, 0xfd62f97aUL, 0x8a65c9ecUL, 0x14015c4fUL, 0x63066cd9UL,
    0xfa0f3d63UL, 0x8d080df5UL, 0x3b6e20c8UL, 0x4c69105eUL, 0xd56041e4UL,
    0xa2677172UL, 0x3c03e4d1UL, 0x4b04d447UL, 0xd20d85fdUL, 0xa50ab56bUL,
    0x35b5a8faUL, 0x42b2986cUL, 0xdbbbc9d6UL, 0xacbcf940UL, 0x32d86ce3UL,
    0x45df5c75UL, 0xdcd60dcfUL, 0xabd13d59UL, 0x26d930acUL, 0x51de003aUL,
    0xc8d75180UL, 0xbfd06116UL, 0x21b4f4b5UL, 0x56b3c423UL, 0xcfba9599UL,
    0xb8bda50fUL, 0x2802b89eUL, 0x5f058808UL, 0xc60cd9b2UL, 0xb10be924UL,
    0x2f6f7c87UL, 0x58684c11UL, 0xc1611dabUL, 0xb6662d3dUL, 0x76dc4190UL,
    0x01db7106UL, 0x98d220bcUL, 0xefd5102aUL, 0x71b18589UL, 0x06b6b51fUL,
    0x9fbfe4a5UL, 0xe8b8d433UL, 0x7807c9a2UL, 0x0f00f934UL, 0x9609a88eUL,
    0xe10e9818UL, 0x7f6a0dbbUL, 0x086d3d2dUL, 0x91646c97UL, 0xe6635c01UL,
    0x6b6b51f4UL, 0x1c6c6162UL, 0x856530d8UL, 0xf262004eUL, 0x6c0695edUL,
    0x1b01a57bUL, 0x8208f4c1UL, 0xf50fc457UL, 0x65b0d9c6UL, 0x12b7e950UL,
    0x8bbeb8eaUL, 0xfcb9887cUL, 0x62dd1ddfUL, 0x15da2d49UL, 0x8cd37cf3UL,
    0xfbd44c65UL, 0x4db26158UL, 0x3ab551ceUL, 0xa3bc0074UL, 0xd4bb30e2UL,
    0x4adfa541UL, 0x3dd895d7UL, 0xa4d1c46dUL, 0xd3d6f4fbUL, 0x4369e96aUL,
    0x346ed9fcUL, 0xad678846UL, 0xda60b8d0UL, 0x44042d73UL, 0x33031de5UL,
    0xaa0a4c5fUL, 0xdd0d7cc9UL, 0x5005713cUL, 0x270241aaUL, 0xbe0b1010UL,
    0xc90c2086UL, 0x5768b525UL, 0x206f85b3UL, 0xb966d409UL, 0xce61e49fUL,
    0x5edef90eUL, 0x29d9c998UL, 0xb0d09822UL, 0xc7d7a8b4UL, 0x59b33d17UL,
    0x2eb40d81UL, 0xb7bd5c3bUL, 0xc0ba6cadUL, 0xedb88320UL, 0x9abfb3b6UL,
    0x03b6e20cUL, 0x74b1d29aUL, 0xead54739UL, 0x9dd277afUL, 0x04db2615UL,
    0x73dc1683UL, 0xe3630b12UL, 0x94643b84UL, 0x0d6d6a3eUL, 0x7a6a5aa8UL,
    0xe40ecf0bUL, 0x9309ff9dUL, 0x0a00ae27UL, 0x7d079eb1UL, 0xf00f9344UL,
    0x8708a3d2UL, 0x1e01f268UL, 0x6906c2feUL, 0xf762575dUL, 0x806567cbUL,
    0x196c3671UL, 0x6e6b06e7UL, 0xfed41b76UL, 0x89d32be0UL, 0x10da7a5aUL,
    0x67dd4accUL, 0xf9b9df6fUL, 0x8ebeeff9UL, 0x17b7be43UL, 0x60b08ed5UL,
    0xd6d6a3e8UL, 0xa1d1937eUL, 0x38d8c2c4UL, 0x4fdff252UL, 0xd1bb67f1UL,
    0xa6bc5767UL, 0x3fb506ddUL, 0x48b2364bUL, 0xd80d2bdaUL, 0xaf0a1b4cUL,
    0x36034af6UL, 0x41047a60UL, 0xdf60efc3UL, 0xa867df55UL, 0x316e8eefUL,
    0x4669be79UL, 0xcb61b38cUL, 0xbc66831aUL, 0x256fd2a0UL, 0x5268e236UL,
    0xcc0c7795UL, 0xbb0b4703UL, 0x220216b9UL, 0x5505262fUL, 0xc5ba3bbeUL,
    0xb2bd0b28UL, 0x2bb45a92UL, 0x5cb36a04UL, 0xc2d7ffa7UL, 0xb5d0cf31UL,
    0x2cd99e8bUL, 0x5bdeae1dUL, 0x9b64c2b0UL, 0xec63f226UL, 0x756aa39cUL,
    0x026d930aUL, 0x9c0906a9UL, 0xeb0e363fUL, 0x72076785UL, 0x05005713UL,
    0x95bf4a82UL, 0xe2b87a14UL, 0x7bb12baeUL, 0x0cb61b38UL, 0x92d28e9bUL,
    0xe5d5be0dUL, 0x7cdcefb7UL, 0x0bdbdf21UL, 0x86d3d2d4UL, 0xf1d4e242UL,
    0x68ddb3f8UL, 0x1fda836eUL, 0x81be16cdUL, 0xf6b9265bUL, 0x6fb077e1UL,
    0x18b74777UL, 0x88085ae6UL, 0xff0f6a70UL, 0x66063bcaUL, 0x11010b5cUL,
    0x8f659effUL, 0xf862ae69UL, 0x616bffd3UL, 0x166ccf45UL, 0xa00ae278UL,
    0xd70dd2eeUL, 0x4e048354UL, 0x3903b3c2UL, 0xa7672661UL, 0xd06016f7UL,
    0x4969474dUL, 0x3e6e77dbUL, 0xaed16a4aUL, 0xd9d65adcUL, 0x40df0b66UL,
    0x37d83bf0UL, 0xa9bcae53UL, 0xdebb9ec5UL, 0x47b2cf7fUL, 0x30b5ffe9UL,
    0xbdbdf21cUL, 0xcabac28aUL, 0x53b39330UL, 0x24b4a3a6UL, 0xbad03605UL,
    0xcdd70693UL, 0x54de5729UL, 0x23d967bfUL, 0xb3667a2eUL, 0xc4614ab8UL,
    0x5d681b02UL, 0x2a6f2b94UL, 0xb40bbe37UL, 0xc30c8ea1UL, 0x5a05df1bUL,
    0x2d02ef8dUL
#ifdef  CRC32_BYFOUR
  },
  {
    0x00000000UL, 0x191b3141UL, 0x32366282UL, 0x2b2d53c3UL, 0x646cc504UL,
    0x7d77f445UL, 0x565aa786UL, 0x4f4196c7UL, 0xc8d98a08UL, 0xd1c2bb49UL,
    0xfaefe88aUL, 0xe3f4d9cbUL, 0xacb54f0cUL, 0xb5ae7e4dUL, 0x9e832d8eUL,
    0x87981ccfUL, 0x4ac21251UL, 0x53d92310UL, 0x78f470d3UL, 0x61ef4192UL,
    0x2eaed755UL, 0x37b5e614UL, 0x1c98b5d7UL, 0x05838496UL, 0x821b9859UL,
    0x9b00a918UL, 0xb02dfadbUL, 0xa936cb9aUL, 0xe6775d5dUL, 0xff6c6c1cUL,
    0xd4413fdfUL, 0xcd5a0e9eUL, 0x958424a2UL, 0x8c9f15e3UL, 0xa7b24620UL,
    0xbea97761UL, 0xf1e8e1a6UL, 0xe8f3d0e7UL, 0xc3de8324UL, 0xdac5b265UL,
    0x5d5daeaaUL, 0x44469febUL, 0x6f6bcc28UL, 0x7670fd69UL, 0x39316baeUL,
    0x202a5aefUL, 0x0b07092cUL, 0x121c386dUL, 0xdf4636f3UL, 0xc65d07b2UL,
    0xed705471UL, 0xf46b6530UL, 0xbb2af3f7UL, 0xa231c2b6UL, 0x891c9175UL,
    0x9007a034UL, 0x179fbcfbUL, 0x0e848dbaUL, 0x25a9de79UL, 0x3cb2ef38UL,
    0x73f379ffUL, 0x6ae848beUL, 0x41c51b7dUL, 0x58de2a3cUL, 0xf0794f05UL,
    0xe9627e44UL, 0xc24f2d87UL, 0xdb541cc6UL, 0x94158a01UL, 0x8d0ebb40UL,
    0xa623e883UL, 0xbf38d9c2UL, 0x38a0c50dUL, 0x21bbf44cUL, 0x0a96a78fUL,
    0x138d96ceUL, 0x5ccc0009UL, 0x45d73148UL, 0x6efa628bUL, 0x77e153caUL,
    0xbabb5d54UL, 0xa3a06c15UL, 0x888d3fd6UL, 0x91960e97UL, 0xded79850UL,
    0xc7cca911UL, 0xece1fad2UL, 0xf5facb93UL, 0x7262d75cUL, 0x6b79e61dUL,
    0x4054b5deUL, 0x594f849fUL, 0x160e1258UL, 0x0f152319UL, 0x243870daUL,
    0x3d23419bUL, 0x65fd6ba7UL, 0x7ce65ae6UL, 0x57cb0925UL, 0x4ed03864UL,
    0x0191aea3UL, 0x188a9fe2UL, 0x33a7cc21UL, 0x2abcfd60UL, 0xad24e1afUL,
    0xb43fd0eeUL, 0x9f12832dUL, 0x8609b26cUL, 0xc94824abUL, 0xd05315eaUL,
    0xfb7e4629UL, 0xe2657768UL, 0x2f3f79f6UL, 0x362448b7UL, 0x1d091b74UL,
    0x04122a35UL, 0x4b53bcf2UL, 0x52488db3UL, 0x7965de70UL, 0x607eef31UL,
    0xe7e6f3feUL, 0xfefdc2bfUL, 0xd5d0917cUL, 0xcccba03dUL, 0x838a36faUL,
    0x9a9107bbUL, 0xb1bc5478UL, 0xa8a76539UL, 0x3b83984bUL, 0x2298a90aUL,
    0x09b5fac9UL, 0x10aecb88UL, 0x5fef5d4fUL, 0x46f46c0eUL, 0x6dd93fcdUL,
    0x74c20e8cUL, 0xf35a1243UL, 0xea412302UL, 0xc16c70c1UL, 0xd8774180UL,
    0x9736d747UL, 0x8e2de606UL, 0xa500b5c5UL, 0xbc1b8484UL, 0x71418a1aUL,
    0x685abb5bUL, 0x4377e898UL, 0x5a6cd9d9UL, 0x152d4f1eUL, 0x0c367e5fUL,
    0x271b2d9cUL, 0x3e001cddUL, 0xb9980012UL, 0xa0833153UL, 0x8bae6290UL,
    0x92b553d1UL, 0xddf4c516UL, 0xc4eff457UL, 0xefc2a794UL, 0xf6d996d5UL,
    0xae07bce9UL, 0xb71c8da8UL, 0x9c31de6bUL, 0x852aef2aUL, 0xca6b79edUL,
    0xd37048acUL, 0xf85d1b6fUL, 0xe1462a2eUL, 0x66de36e1UL, 0x7fc507a0UL,
    0x54e85463UL, 0x4df36522UL, 0x02b2f3e5UL, 0x1ba9c2a4UL, 0x30849167UL,
    0x299fa026UL, 0xe4c5aeb8UL, 0xfdde9ff9UL, 0xd6f3cc3aUL, 0xcfe8fd7bUL,
    0x80a96bbcUL, 0x99b25afdUL, 0xb29f093eUL, 0xab84387fUL, 0x2c1c24b0UL,
    0x350715f1UL, 0x1e2a4632UL, 0x07317773UL, 0x4870e1b4UL, 0x516bd0f5UL,
    0x7a468336UL, 0x635db277UL, 0xcbfad74eUL, 0xd2e1e60fUL, 0xf9ccb5ccUL,
    0xe0d7848dUL, 0xaf96124aUL, 0xb68d230bUL, 0x9da070c8UL, 0x84bb4189UL,
    0x03235d46UL, 0x1a386c07UL, 0x31153fc4UL, 0x280e0e85UL, 0x674f9842UL,
    0x7e54a903UL, 0x5579fac0UL, 0x4c62cb81UL, 0x8138c51fUL, 0x9823f45eUL,
    0xb30ea79dUL, 0xaa1596dcUL, 0xe554001bUL, 0xfc4f315aUL, 0xd7626299UL,
    0xce7953d8UL, 0x49e14f17UL, 0x50fa7e56UL, 0x7bd72d95UL, 0x62cc1cd4UL,
    0x2d8d8a13UL, 0x3496bb52UL, 0x1fbbe891UL, 0x06a0d9d0UL, 0x5e7ef3ecUL,
    0x4765c2adUL, 0x6c48916eUL, 0x7553a02fUL, 0x3a1236e8UL, 0x230907a9UL,
    0x0824546aUL, 0x113f652bUL, 0x96a779e4UL, 0x8fbc48a5UL, 0xa4911b66UL,
    0xbd8a2a27UL, 0xf2cbbce0UL, 0xebd08da1UL, 0xc0fdde62UL, 0xd9e6ef23UL,
    0x14bce1bdUL, 0x0da7d0fcUL, 0x268a833fUL, 0x3f91b27eUL, 0x70d024b9UL,
    0x69cb15f8UL, 0x42e6463bUL, 0x5bfd777aUL, 0xdc656bb5UL, 0xc57e5af4UL,
    0xee530937UL, 0xf7483876UL, 0xb809aeb1UL, 0xa1129ff0UL, 0x8a3fcc33UL,
    0x9324fd72UL
  },
  {
    0x00000000UL, 0x01c26a37UL, 0x0384d46eUL, 0x0246be59UL, 0x0709a8dcUL,
    0x06cbc2ebUL, 0x048d7cb2UL, 0x054f1685UL, 0x0e1351b8UL, 0x0fd13b8fUL,
    0x0d9785d6UL, 0x0c55efe1UL, 0x091af964UL, 0x08d89353UL, 0x0a9e2d0aUL,
    0x0b5c473dUL, 0x1c26a370UL, 0x1de4c947UL, 0x1fa2771eUL, 0x1e601d29UL,
    0x1b2f0bacUL, 0x1aed619bUL, 0x18abdfc2UL, 0x1969b5f5UL, 0x1235f2c8UL,
    0x13f798ffUL, 0x11b126a6UL, 0x10734c91UL, 0x153c5a14UL, 0x14fe3023UL,
    0x16b88e7aUL, 0x177ae44dUL, 0x384d46e0UL, 0x398f2cd7UL, 0x3bc9928eUL,
    0x3a0bf8b9UL, 0x3f44ee3cUL, 0x3e86840bUL, 0x3cc03a52UL, 0x3d025065UL,
    0x365e1758UL, 0x379c7d6fUL, 0x35dac336UL, 0x3418a901UL, 0x3157bf84UL,
    0x3095d5b3UL, 0x32d36beaUL, 0x331101ddUL, 0x246be590UL, 0x25a98fa7UL,
    0x27ef31feUL, 0x262d5bc9UL, 0x23624d4cUL, 0x22a0277bUL, 0x20e69922UL,
    0x2124f315UL, 0x2a78b428UL, 0x2bbade1fUL, 0x29fc6046UL, 0x283e0a71UL,
    0x2d711cf4UL, 0x2cb376c3UL, 0x2ef5c89aUL, 0x2f37a2adUL, 0x709a8dc0UL,
    0x7158e7f7UL, 0x731e59aeUL, 0x72dc3399UL, 0x7793251cUL, 0x76514f2bUL,
    0x7417f172UL, 0x75d59b45UL, 0x7e89dc78UL, 0x7f4bb64fUL, 0x7d0d0816UL,
    0x7ccf6221UL, 0x798074a4UL, 0x78421e93UL, 0x7a04a0caUL, 0x7bc6cafdUL,
    0x6cbc2eb0UL, 0x6d7e4487UL, 0x6f38fadeUL, 0x6efa90e9UL, 0x6bb5866cUL,
    0x6a77ec5bUL, 0x68315202UL, 0x69f33835UL, 0x62af7f08UL, 0x636d153fUL,
    0x612bab66UL, 0x60e9c151UL, 0x65a6d7d4UL, 0x6464bde3UL, 0x662203baUL,
    0x67e0698dUL, 0x48d7cb20UL, 0x4915a117UL, 0x4b531f4eUL, 0x4a917579UL,
    0x4fde63fcUL, 0x4e1c09cbUL, 0x4c5ab792UL, 0x4d98dda5UL, 0x46c49a98UL,
    0x4706f0afUL, 0x45404ef6UL, 0x448224c1UL, 0x41cd3244UL, 0x400f5873UL,
    0x4249e62aUL, 0x438b8c1dUL, 0x54f16850UL, 0x55330267UL, 0x5775bc3eUL,
    0x56b7d609UL, 0x53f8c08cUL, 0x523aaabbUL, 0x507c14e2UL, 0x51be7ed5UL,
    0x5ae239e8UL, 0x5b2053dfUL, 0x5966ed86UL, 0x58a487b1UL, 0x5deb9134UL,
    0x5c29fb03UL, 0x5e6f455aUL, 0x5fad2f6dUL, 0xe1351b80UL, 0xe0f771b7UL,
    0xe2b1cfeeUL, 0xe373a5d9UL, 0xe63cb35cUL, 0xe7fed96bUL, 0xe5b86732UL,
    0xe47a0d05UL, 0xef264a38UL, 0xeee4200fUL, 0xeca29e56UL, 0xed60f461UL,
    0xe82fe2e4UL, 0xe9ed88d3UL, 0xebab368aUL, 0xea695cbdUL, 0xfd13b8f0UL,
    0xfcd1d2c7UL, 0xfe976c9eUL, 0xff5506a9UL, 0xfa1a102cUL, 0xfbd87a1bUL,
    0xf99ec442UL, 0xf85cae75UL, 0xf300e948UL, 0xf2c2837fUL, 0xf0843d26UL,
    0xf1465711UL, 0xf4094194UL, 0xf5cb2ba3UL, 0xf78d95faUL, 0xf64fffcdUL,
    0xd9785d60UL, 0xd8ba3757UL, 0xdafc890eUL, 0xdb3ee339UL, 0xde71f5bcUL,
    0xdfb39f8bUL, 0xddf521d2UL, 0xdc374be5UL, 0xd76b0cd8UL, 0xd6a966efUL,
    0xd4efd8b6UL, 0xd52db281UL, 0xd062a404UL, 0xd1a0ce33UL, 0xd3e6706aUL,
    0xd2241a5dUL, 0xc55efe10UL, 0xc49c9427UL, 0xc6da2a7eUL, 0xc7184049UL,
    0xc25756ccUL, 0xc3953cfbUL, 0xc1d382a2UL, 0xc011e895UL, 0xcb4dafa8UL,
    0xca8fc59fUL, 0xc8c97bc6UL, 0xc90b11f1UL, 0xcc440774UL, 0xcd866d43UL,
    0xcfc0d31aUL, 0xce02b92dUL, 0x91af9640UL, 0x906dfc77UL, 0x922b422eUL,
    0x93e92819UL, 0x96a63e9cUL, 0x976454abUL, 0x9522eaf2UL, 0x94e080c5UL,
    0x9fbcc7f8UL, 0x9e7eadcfUL, 0x9c381396UL, 0x9dfa79a1UL, 0x98b56f24UL,
    0x99770513UL, 0x9b31bb4aUL, 0x9af3d17dUL, 0x8d893530UL, 0x8c4b5f07UL,
    0x8e0de15eUL, 0x8fcf8b69UL, 0x8a809decUL, 0x8b42f7dbUL, 0x89044982UL,
    0x88c623b5UL, 0x839a6488UL, 0x82580ebfUL, 0x801eb0e6UL, 0x81dcdad1UL,
    0x8493cc54UL, 0x8551a663UL, 0x8717183aUL, 0x86d5720dUL, 0xa9e2d0a0UL,
    0xa820ba97UL, 0xaa6604ceUL, 0xaba46ef9UL, 0xaeeb787cUL, 0xaf29124bUL,
    0xad6fac12UL, 0xacadc625UL, 0xa7f18118UL, 0xa633eb2fUL, 0xa4755576UL,
    0xa5b73f41UL, 0xa0f829c4UL, 0xa13a43f3UL, 0xa37cfdaaUL, 0xa2be979dUL,
    0xb5c473d0UL, 0xb40619e7UL, 0xb640a7beUL, 0xb782cd89UL, 0xb2cddb0cUL,
    0xb30fb13bUL, 0xb1490f62UL, 0xb08b6555UL, 0xbbd72268UL, 0xba15485fUL,
    0xb853f606UL, 0xb9919c31UL, 0xbcde8ab4UL, 0xbd1ce083UL, 0xbf5a5edaUL,
    0xbe9834edUL
  },
  {
    0x00000000UL, 0xb8bc6765UL, 0xaa09c88bUL, 0x12b5afeeUL, 0x8f629757UL,
    0x37def032UL, 0x256b5fdcUL, 0x9dd738b9UL, 0xc5b428efUL, 0x7d084f8aUL,
    0x6fbde064UL, 0xd7018701UL, 0x4ad6bfb8UL, 0xf26ad8ddUL, 0xe0df7733UL,
    0x58631056UL, 0x5019579fUL, 0xe8a530faUL, 0xfa109f14UL, 0x42acf871UL,
    0xdf7bc0c8UL, 0x67c7a7adUL, 0x75720843UL, 0xcdce6f26UL, 0x95ad7f70UL,
    0x2d111815UL, 0x3fa4b7fbUL, 0x8718d09eUL, 0x1acfe827UL, 0xa2738f42UL,
    0xb0c620acUL, 0x087a47c9UL, 0xa032af3eUL, 0x188ec85bUL, 0x0a3b67b5UL,
    0xb28700d0UL, 0x2f503869UL, 0x97ec5f0cUL, 0x8559f0e2UL, 0x3de59787UL,
    0x658687d1UL, 0xdd3ae0b4UL, 0xcf8f4f5aUL, 0x7733283fUL, 0xeae41086UL,
    0x525877e3UL, 0x40edd80dUL, 0xf851bf68UL, 0xf02bf8a1UL, 0x48979fc4UL,
    0x5a22302aUL, 0xe29e574fUL, 0x7f496ff6UL, 0xc7f50893UL, 0xd540a77dUL,
    0x6dfcc018UL, 0x359fd04eUL, 0x8d23b72bUL, 0x9f9618c5UL, 0x272a7fa0UL,
    0xbafd4719UL, 0x0241207cUL, 0x10f48f92UL, 0xa848e8f7UL, 0x9b14583dUL,
    0x23a83f58UL, 0x311d90b6UL, 0x89a1f7d3UL, 0x1476cf6aUL, 0xaccaa80fUL,
    0xbe7f07e1UL, 0x06c36084UL, 0x5ea070d2UL, 0xe61c17b7UL, 0xf4a9b859UL,
    0x4c15df3cUL, 0xd1c2e785UL, 0x697e80e0UL, 0x7bcb2f0eUL, 0xc377486bUL,
    0xcb0d0fa2UL, 0x73b168c7UL, 0x6104c729UL, 0xd9b8a04cUL, 0x446f98f5UL,
    0xfcd3ff90UL, 0xee66507eUL, 0x56da371bUL, 0x0eb9274dUL, 0xb6054028UL,
    0xa4b0efc6UL, 0x1c0c88a3UL, 0x81dbb01aUL, 0x3967d77fUL, 0x2bd27891UL,
    0x936e1ff4UL, 0x3b26f703UL, 0x839a9066UL, 0x912f3f88UL, 0x299358edUL,
    0xb4446054UL, 0x0cf80731UL, 0x1e4da8dfUL, 0xa6f1cfbaUL, 0xfe92dfecUL,
    0x462eb889UL, 0x549b1767UL, 0xec277002UL, 0x71f048bbUL, 0xc94c2fdeUL,
    0xdbf98030UL, 0x6345e755UL, 0x6b3fa09cUL, 0xd383c7f9UL, 0xc1366817UL,
    0x798a0f72UL, 0xe45d37cbUL, 0x5ce150aeUL, 0x4e54ff40UL, 0xf6e89825UL,
    0xae8b8873UL, 0x1637ef16UL, 0x048240f8UL, 0xbc3e279dUL, 0x21e91f24UL,
    0x99557841UL, 0x8be0d7afUL, 0x335cb0caUL, 0xed59b63bUL, 0x55e5d15eUL,
    0x47507eb0UL, 0xffec19d5UL, 0x623b216cUL, 0xda874609UL, 0xc832e9e7UL,
    0x708e8e82UL, 0x28ed9ed4UL, 0x9051f9b1UL, 0x82e4565fUL, 0x3a58313aUL,
    0xa78f0983UL, 0x1f336ee6UL, 0x0d86c108UL, 0xb53aa66dUL, 0xbd40e1a4UL,
    0x05fc86c1UL, 0x1749292fUL, 0xaff54e4aUL, 0x322276f3UL, 0x8a9e1196UL,
    0x982bbe78UL, 0x2097d91dUL, 0x78f4c94bUL, 0xc048ae2eUL, 0xd2fd01c0UL,
    0x6a4166a5UL, 0xf7965e1cUL, 0x4f2a3979UL, 0x5d9f9697UL, 0xe523f1f2UL,
    0x4d6b1905UL, 0xf5d77e60UL, 0xe762d18eUL, 0x5fdeb6ebUL, 0xc2098e52UL,
    0x7ab5e937UL, 0x680046d9UL, 0xd0bc21bcUL, 0x88df31eaUL, 0x3063568fUL,
    0x22d6f961UL, 0x9a6a9e04UL, 0x07bda6bdUL, 0xbf01c1d8UL, 0xadb46e36UL,
    0x15080953UL, 0x1d724e9aUL, 0xa5ce29ffUL, 0xb77b8611UL, 0x0fc7e174UL,
    0x9210d9cdUL, 0x2aacbea8UL, 0x38191146UL, 0x80a57623UL, 0xd8c66675UL,
    0x607a0110UL, 0x72cfaefeUL, 0xca73c99bUL, 0x57a4f122UL, 0xef189647UL,
    0xfdad39a9UL, 0x45115eccUL, 0x764dee06UL, 0xcef18963UL, 0xdc44268dUL,
    0x64f841e8UL, 0xf92f7951UL, 0x41931e34UL, 0x5326b1daUL, 0xeb9ad6bfUL,
    0xb3f9c6e9UL, 0x0b45a18cUL, 0x19f00e62UL, 0xa14c6907UL, 0x3c9b51beUL,
    0x842736dbUL, 0x96929935UL, 0x2e2efe50UL, 0x2654b999UL, 0x9ee8defcUL,
    0x8c5d7112UL, 0x34e11677UL, 0xa9362eceUL, 0x118a49abUL, 0x033fe645UL,
    0xbb838120UL, 0xe3e09176UL, 0x5b5cf613UL, 0x49e959fdUL, 0xf1553e98UL,
    0x6c820621UL, 0xd43e6144UL, 0xc68bceaaUL, 0x7e37a9cfUL, 0xd67f4138UL,
    0x6ec3265dUL, 0x7c7689b3UL, 0xc4caeed6UL, 0x591dd66fUL, 0xe1a1b10aUL,
    0xf3141ee4UL, 0x4ba87981UL, 0x13cb69d7UL, 0xab770eb2UL, 0xb9c2a15cUL,
    0x017ec639UL, 0x9ca9fe80UL, 0x241599e5UL, 0x36a0360bUL, 0x8e1c516eUL,
    0x866616a7UL, 0x3eda71c2UL, 0x2c6fde2cUL, 0x94d3b949UL, 0x090481f0UL,
    0xb1b8e695UL, 0xa30d497bUL, 0x1bb12e1eUL, 0x43d23e48UL, 0xfb6e592dUL,
    0xe9dbf6c3UL, 0x516791a6UL, 0xccb0a91fUL, 0x740cce7aUL, 0x66b96194UL,
    0xde0506f1UL
  },
  {
    0x00000000UL, 0x96300777UL, 0x2c610eeeUL, 0xba510999UL, 0x19c46d07UL,
    0x8ff46a70UL, 0x35a563e9UL, 0xa395649eUL, 0x3288db0eUL, 0xa4b8dc79UL,
    0x1ee9d5e0UL, 0x88d9d297UL, 0x2b4cb609UL, 0xbd7cb17eUL, 0x072db8e7UL,
    0x911dbf90UL, 0x6410b71dUL, 0xf220b06aUL, 0x4871b9f3UL, 0xde41be84UL,
    0x7dd4da1aUL, 0xebe4dd6dUL, 0x51b5d4f4UL, 0xc785d383UL, 0x56986c13UL,
    0xc0a86b64UL, 0x7af962fdUL, 0xecc9658aUL, 0x4f5c0114UL, 0xd96c0663UL,
    0x633d0ffaUL, 0xf50d088dUL, 0xc8206e3bUL, 0x5e10694cUL, 0xe44160d5UL,
    0x727167a2UL, 0xd1e4033cUL, 0x47d4044bUL, 0xfd850dd2UL, 0x6bb50aa5UL,
    0xfaa8b535UL, 0x6c98b242UL, 0xd6c9bbdbUL, 0x40f9bcacUL, 0xe36cd832UL,
    0x755cdf45UL, 0xcf0dd6dcUL, 0x593dd1abUL, 0xac30d926UL, 0x3a00de51UL,
    0x8051d7c8UL, 0x1661d0bfUL, 0xb5f4b421UL, 0x23c4b356UL, 0x9995bacfUL,
    0x0fa5bdb8UL, 0x9eb80228UL, 0x0888055fUL, 0xb2d90cc6UL, 0x24e90bb1UL,
    0x877c6f2fUL, 0x114c6858UL, 0xab1d61c1UL, 0x3d2d66b6UL, 0x9041dc76UL,
    0x0671db01UL, 0xbc20d298UL, 0x2a10d5efUL, 0x8985b171UL, 0x1fb5b606UL,
    0xa5e4bf9fUL, 0x33d4b8e8UL, 0xa2c90778UL, 0x34f9000fUL, 0x8ea80996UL,
    0x18980ee1UL, 0xbb0d6a7fUL, 0x2d3d6d08UL, 0x976c6491UL, 0x015c63e6UL,
    0xf4516b6bUL, 0x62616c1cUL, 0xd8306585UL, 0x4e0062f2UL, 0xed95066cUL,
    0x7ba5011bUL, 0xc1f40882UL, 0x57c40ff5UL, 0xc6d9b065UL, 0x50e9b712UL,
    0xeab8be8bUL, 0x7c88b9fcUL, 0xdf1ddd62UL, 0x492dda15UL, 0xf37cd38cUL,
    0x654cd4fbUL, 0x5861b24dUL, 0xce51b53aUL, 0x7400bca3UL, 0xe230bbd4UL,
    0x41a5df4aUL, 0xd795d83dUL, 0x6dc4d1a4UL, 0xfbf4d6d3UL, 0x6ae96943UL,
    0xfcd96e34UL, 0x468867adUL, 0xd0b860daUL, 0x732d0444UL, 0xe51d0333UL,
    0x5f4c0aaaUL, 0xc97c0dddUL, 0x3c710550UL, 0xaa410227UL, 0x10100bbeUL,
    0x86200cc9UL, 0x25b56857UL, 0xb3856f20UL, 0x09d466b9UL, 0x9fe461ceUL,
    0x0ef9de5eUL, 0x98c9d929UL, 0x2298d0b0UL, 0xb4a8d7c7UL, 0x173db359UL,
    0x810db42eUL, 0x3b5cbdb7UL, 0xad6cbac0UL, 0x2083b8edUL, 0xb6b3bf9aUL,
    0x0ce2b603UL, 0x9ad2b174UL, 0x3947d5eaUL, 0xaf77d29dUL, 0x1526db04UL,
    0x8316dc73UL, 0x120b63e3UL, 0x843b6494UL, 0x3e6a6d0dUL, 0xa85a6a7aUL,
    0x0bcf0ee4UL, 0x9dff0993UL, 0x27ae000aUL, 0xb19e077dUL, 0x44930ff0UL,
    0xd2a30887UL, 0x68f2011eUL, 0xfec20669UL, 0x5d5762f7UL, 0xcb676580UL,
    0x71366c19UL, 0xe7066b6eUL, 0x761bd4feUL, 0xe02bd389UL, 0x5a7ada10UL,
    0xcc4add67UL, 0x6fdfb9f9UL, 0xf9efbe8eUL, 0x43beb717UL, 0xd58eb060UL,
    0xe8a3d6d6UL, 0x7e93d1a1UL, 0xc4c2d838UL, 0x52f2df4fUL, 0xf167bbd1UL,
    0x6757bca6UL, 0xdd06b53fUL, 0x4b36b248UL, 0xda2b0dd8UL, 0x4c1b0aafUL,
    0xf64a0336UL, 0x607a0441UL, 0xc3ef60dfUL, 0x55df67a8UL, 0xef8e6e31UL,
    0x79be6946UL, 0x8cb361cbUL, 0x1a8366bcUL, 0xa0d26f25UL, 0x36e26852UL,
    0x95770cccUL, 0x03470bbbUL, 0xb9160222UL, 0x2f260555UL, 0xbe3bbac5UL,
    0x280bbdb2UL, 0x925ab42bUL, 0x046ab35cUL, 0xa7ffd7c2UL, 0x31cfd0b5UL,
    0x8b9ed92cUL, 0x1daede5bUL, 0xb0c2649bUL, 0x26f263ecUL, 0x9ca36a75UL,
    0x0a936d02UL, 0xa906099cUL, 0x3f360eebUL, 0x85670772UL, 0x13570005UL,
    0x824abf95UL, 0x147ab8e2UL, 0xae2bb17bUL, 0x381bb60cUL, 0x9b8ed292UL,
    0x0dbed5e5UL, 0xb7efdc7cUL, 0x21dfdb0bUL, 0xd4d2d386UL, 0x42e2d4f1UL,
    0xf8b3dd68UL, 0x6e83da1fUL, 0xcd16be81UL, 0x5b26b9f6UL, 0xe177b06fUL,
    0x7747b718UL, 0xe65a0888UL, 0x706a0fffUL, 0xca3b0666UL, 0x5c0b0111UL,
    0xff9e658fUL, 0x69ae62f8UL, 0xd3ff6b61UL, 0x45cf6c16UL, 0x78e20aa0UL,
    0xeed20dd7UL, 0x5483044eUL, 0xc2b30339UL, 0x612667a7UL, 0xf71660d0UL,
    0x4d476949UL, 0xdb776e3eUL, 0x4a6ad1aeUL, 0xdc5ad6d9UL, 0x660bdf40UL,
    0xf03bd837UL, 0x53aebca9UL, 0xc59ebbdeUL, 0x7fcfb247UL, 0xe9ffb530UL,
    0x1cf2bdbdUL, 0x8ac2bacaUL, 0x3093b353UL, 0xa6a3b424UL, 0x0536d0baUL,
    0x9306d7cdUL, 0x2957de54UL, 0xbf67d923UL, 0x2e7a66b3UL, 0xb84a61c4UL,
    0x021b685dUL, 0x942b6f2aUL, 0x37be0bb4UL, 0xa18e0cc3UL, 0x1bdf055aUL,
    0x8def022dUL
  },
  {
    0x00000000UL, 0x41311b19UL, 0x82623632UL, 0xc3532d2bUL, 0x04c56c64UL,
    0x45f4777dUL, 0x86a75a56UL, 0xc796414fUL, 0x088ad9c8UL, 0x49bbc2d1UL,
    0x8ae8effaUL, 0xcbd9f4e3UL, 0x0c4fb5acUL, 0x4d7eaeb5UL, 0x8e2d839eUL,
    0xcf1c9887UL, 0x5112c24aUL, 0x1023d953UL, 0xd370f478UL, 0x9241ef61UL,
    0x55d7ae2eUL, 0x14e6b537UL, 0xd7b5981cUL, 0x96848305UL, 0x59981b82UL,
    0x18a9009bUL, 0xdbfa2db0UL, 0x9acb36a9UL, 0x5d5d77e6UL, 0x1c6c6cffUL,
    0xdf3f41d4UL, 0x9e0e5acdUL, 0xa2248495UL, 0xe3159f8cUL, 0x2046b2a7UL,
    0x6177a9beUL, 0xa6e1e8f1UL, 0xe7d0f3e8UL, 0x2483dec3UL, 0x65b2c5daUL,
    0xaaae5d5dUL, 0xeb9f4644UL, 0x28cc6b6fUL, 0x69fd7076UL, 0xae6b3139UL,
    0xef5a2a20UL, 0x2c09070bUL, 0x6d381c12UL, 0xf33646dfUL, 0xb2075dc6UL,
    0x715470edUL, 0x30656bf4UL, 0xf7f32abbUL, 0xb6c231a2UL, 0x75911c89UL,
    0x34a00790UL, 0xfbbc9f17UL, 0xba8d840eUL, 0x79dea925UL, 0x38efb23cUL,
    0xff79f373UL, 0xbe48e86aUL, 0x7d1bc541UL, 0x3c2ade58UL, 0x054f79f0UL,
    0x447e62e9UL, 0x872d4fc2UL, 0xc61c54dbUL, 0x018a1594UL, 0x40bb0e8dUL,
    0x83e823a6UL, 0xc2d938bfUL, 0x0dc5a038UL, 0x4cf4bb21UL, 0x8fa7960aUL,
    0xce968d13UL, 0x0900cc5cUL, 0x4831d745UL, 0x8b62fa6eUL, 0xca53e177UL,
    0x545dbbbaUL, 0x156ca0a3UL, 0xd63f8d88UL, 0x970e9691UL, 0x5098d7deUL,
    0x11a9ccc7UL, 0xd2fae1ecUL, 0x93cbfaf5UL, 0x5cd76272UL, 0x1de6796bUL,
    0xdeb55440UL, 0x9f844f59UL, 0x58120e16UL, 0x1923150fUL, 0xda703824UL,
    0x9b41233dUL, 0xa76bfd65UL, 0xe65ae67cUL, 0x2509cb57UL, 0x6438d04eUL,
    0xa3ae9101UL, 0xe29f8a18UL, 0x21cca733UL, 0x60fdbc2aUL, 0xafe124adUL,
    0xeed03fb4UL, 0x2d83129fUL, 0x6cb20986UL, 0xab2448c9UL, 0xea1553d0UL,
    0x29467efbUL, 0x687765e2UL, 0xf6793f2fUL, 0xb7482436UL, 0x741b091dUL,
    0x352a1204UL, 0xf2bc534bUL, 0xb38d4852UL, 0x70de6579UL, 0x31ef7e60UL,
    0xfef3e6e7UL, 0xbfc2fdfeUL, 0x7c91d0d5UL, 0x3da0cbccUL, 0xfa368a83UL,
    0xbb07919aUL, 0x7854bcb1UL, 0x3965a7a8UL, 0x4b98833bUL, 0x0aa99822UL,
    0xc9fab509UL, 0x88cbae10UL, 0x4f5def5fUL, 0x0e6cf446UL, 0xcd3fd96dUL,
    0x8c0ec274UL, 0x43125af3UL, 0x022341eaUL, 0xc1706cc1UL, 0x804177d8UL,
    0x47d73697UL, 0x06e62d8eUL, 0xc5b500a5UL, 0x84841bbcUL, 0x1a8a4171UL,
    0x5bbb5a68UL, 0x98e87743UL, 0xd9d96c5aUL, 0x1e4f2d15UL, 0x5f7e360cUL,
    0x9c2d1b27UL, 0xdd1c003eUL, 0x120098b9UL, 0x533183a0UL, 0x9062ae8bUL,
    0xd153b592UL, 0x16c5f4ddUL, 0x57f4efc4UL, 0x94a7c2efUL, 0xd596d9f6UL,
    0xe9bc07aeUL, 0xa88d1cb7UL, 0x6bde319cUL, 0x2aef2a85UL, 0xed796bcaUL,
    0xac4870d3UL, 0x6f1b5df8UL, 0x2e2a46e1UL, 0xe136de66UL, 0xa007c57fUL,
    0x6354e854UL, 0x2265f34dUL, 0xe5f3b202UL, 0xa4c2a91bUL, 0x67918430UL,
    0x26a09f29UL, 0xb8aec5e4UL, 0xf99fdefdUL, 0x3accf3d6UL, 0x7bfde8cfUL,
    0xbc6ba980UL, 0xfd5ab299UL, 0x3e099fb2UL, 0x7f3884abUL, 0xb0241c2cUL,
    0xf1150735UL, 0x32462a1eUL, 0x73773107UL, 0xb4e17048UL, 0xf5d06b51UL,
    0x3683467aUL, 0x77b25d63UL, 0x4ed7facbUL, 0x0fe6e1d2UL, 0xccb5ccf9UL,
    0x8d84d7e0UL, 0x4a1296afUL, 0x0b238db6UL, 0xc870a09dUL, 0x8941bb84UL,
    0x465d2303UL, 0x076c381aUL, 0xc43f1531UL, 0x850e0e28UL, 0x42984f67UL,
    0x03a9547eUL, 0xc0fa7955UL, 0x81cb624cUL, 0x1fc53881UL, 0x5ef42398UL,
    0x9da70eb3UL, 0xdc9615aaUL, 0x1b0054e5UL, 0x5a314ffcUL, 0x996262d7UL,
    0xd85379ceUL, 0x174fe149UL, 0x567efa50UL, 0x952dd77bUL, 0xd41ccc62UL,
    0x138a8d2dUL, 0x52bb9634UL, 0x91e8bb1fUL, 0xd0d9a006UL, 0xecf37e5eUL,
    0xadc26547UL, 0x6e91486cUL, 0x2fa05375UL, 0xe836123aUL, 0xa9070923UL,
    0x6a542408UL, 0x2b653f11UL, 0xe479a796UL, 0xa548bc8fUL, 0x661b91a4UL,
    0x272a8abdUL, 0xe0bccbf2UL, 0xa18dd0ebUL, 0x62defdc0UL, 0x23efe6d9UL,
    0xbde1bc14UL, 0xfcd0a70dUL, 0x3f838a26UL, 0x7eb2913fUL, 0xb924d070UL,
    0xf815cb69UL, 0x3b46e642UL, 0x7a77fd5bUL, 0xb56b65dcUL, 0xf45a7ec5UL,
    0x370953eeUL, 0x763848f7UL, 0xb1ae09b8UL, 0xf09f12a1UL, 0x33cc3f8aUL,
    0x72fd2493UL
  },
  {
    0x00000000UL, 0x376ac201UL, 0x6ed48403UL, 0x59be4602UL, 0xdca80907UL,
    0xebc2cb06UL, 0xb27c8d04UL, 0x85164f05UL, 0xb851130eUL, 0x8f3bd10fUL,
    0xd685970dUL, 0xe1ef550cUL, 0x64f91a09UL, 0x5393d808UL, 0x0a2d9e0aUL,
    0x3d475c0bUL, 0x70a3261cUL, 0x47c9e41dUL, 0x1e77a21fUL, 0x291d601eUL,
    0xac0b2f1bUL, 0x9b61ed1aUL, 0xc2dfab18UL, 0xf5b56919UL, 0xc8f23512UL,
    0xff98f713UL, 0xa626b111UL, 0x914c7310UL, 0x145a3c15UL, 0x2330fe14UL,
    0x7a8eb816UL, 0x4de47a17UL, 0xe0464d38UL, 0xd72c8f39UL, 0x8e92c93bUL,
    0xb9f80b3aUL, 0x3cee443fUL, 0x0b84863eUL, 0x523ac03cUL, 0x6550023dUL,
    0x58175e36UL, 0x6f7d9c37UL, 0x36c3da35UL, 0x01a91834UL, 0x84bf5731UL,
    0xb3d59530UL, 0xea6bd332UL, 0xdd011133UL, 0x90e56b24UL, 0xa78fa925UL,
    0xfe31ef27UL, 0xc95b2d26UL, 0x4c4d6223UL, 0x7b27a022UL, 0x2299e620UL,
    0x15f32421UL, 0x28b4782aUL, 0x1fdeba2bUL, 0x4660fc29UL, 0x710a3e28UL,
    0xf41c712dUL, 0xc376b32cUL, 0x9ac8f52eUL, 0xada2372fUL, 0xc08d9a70UL,
    0xf7e75871UL, 0xae591e73UL, 0x9933dc72UL, 0x1c259377UL, 0x2b4f5176UL,
    0x72f11774UL, 0x459bd575UL, 0x78dc897eUL, 0x4fb64b7fUL, 0x16080d7dUL,
    0x2162cf7cUL, 0xa4748079UL, 0x931e4278UL, 0xcaa0047aUL, 0xfdcac67bUL,
    0xb02ebc6cUL, 0x87447e6dUL, 0xdefa386fUL, 0xe990fa6eUL, 0x6c86b56bUL,
    0x5bec776aUL, 0x02523168UL, 0x3538f369UL, 0x087faf62UL, 0x3f156d63UL,
    0x66ab2b61UL, 0x51c1e960UL, 0xd4d7a665UL, 0xe3bd6464UL, 0xba032266UL,
    0x8d69e067UL, 0x20cbd748UL, 0x17a11549UL, 0x4e1f534bUL, 0x7975914aUL,
    0xfc63de4fUL, 0xcb091c4eUL, 0x92b75a4cUL, 0xa5dd984dUL, 0x989ac446UL,
    0xaff00647UL, 0xf64e4045UL, 0xc1248244UL, 0x4432cd41UL, 0x73580f40UL,
    0x2ae64942UL, 0x1d8c8b43UL, 0x5068f154UL, 0x67023355UL, 0x3ebc7557UL,
    0x09d6b756UL, 0x8cc0f853UL, 0xbbaa3a52UL, 0xe2147c50UL, 0xd57ebe51UL,
    0xe839e25aUL, 0xdf53205bUL, 0x86ed6659UL, 0xb187a458UL, 0x3491eb5dUL,
    0x03fb295cUL, 0x5a456f5eUL, 0x6d2fad5fUL, 0x801b35e1UL, 0xb771f7e0UL,
    0xeecfb1e2UL, 0xd9a573e3UL, 0x5cb33ce6UL, 0x6bd9fee7UL, 0x3267b8e5UL,
    0x050d7ae4UL, 0x384a26efUL, 0x0f20e4eeUL, 0x569ea2ecUL, 0x61f460edUL,
    0xe4e22fe8UL, 0xd388ede9UL, 0x8a36abebUL, 0xbd5c69eaUL, 0xf0b813fdUL,
    0xc7d2d1fcUL, 0x9e6c97feUL, 0xa90655ffUL, 0x2c101afaUL, 0x1b7ad8fbUL,
    0x42c49ef9UL, 0x75ae5cf8UL, 0x48e900f3UL, 0x7f83c2f2UL, 0x263d84f0UL,
    0x115746f1UL, 0x944109f4UL, 0xa32bcbf5UL, 0xfa958df7UL, 0xcdff4ff6UL,
    0x605d78d9UL, 0x5737bad8UL, 0x0e89fcdaUL, 0x39e33edbUL, 0xbcf571deUL,
    0x8b9fb3dfUL, 0xd221f5ddUL, 0xe54b37dcUL, 0xd80c6bd7UL, 0xef66a9d6UL,
    0xb6d8efd4UL, 0x81b22dd5UL, 0x04a462d0UL, 0x33cea0d1UL, 0x6a70e6d3UL,
    0x5d1a24d2UL, 0x10fe5ec5UL, 0x27949cc4UL, 0x7e2adac6UL, 0x494018c7UL,
    0xcc5657c2UL, 0xfb3c95c3UL, 0xa282d3c1UL, 0x95e811c0UL, 0xa8af4dcbUL,
    0x9fc58fcaUL, 0xc67bc9c8UL, 0xf1110bc9UL, 0x740744ccUL, 0x436d86cdUL,
    0x1ad3c0cfUL, 0x2db902ceUL, 0x4096af91UL, 0x77fc6d90UL, 0x2e422b92UL,
    0x1928e993UL, 0x9c3ea696UL, 0xab546497UL, 0xf2ea2295UL, 0xc580e094UL,
    0xf8c7bc9fUL, 0xcfad7e9eUL, 0x9613389cUL, 0xa179fa9dUL, 0x246fb598UL,
    0x13057799UL, 0x4abb319bUL, 0x7dd1f39aUL, 0x3035898dUL, 0x075f4b8cUL,
    0x5ee10d8eUL, 0x698bcf8fUL, 0xec9d808aUL, 0xdbf7428bUL, 0x82490489UL,
    0xb523c688UL, 0x88649a83UL, 0xbf0e5882UL, 0xe6b01e80UL, 0xd1dadc81UL,
    0x54cc9384UL, 0x63a65185UL, 0x3a181787UL, 0x0d72d586UL, 0xa0d0e2a9UL,
    0x97ba20a8UL, 0xce0466aaUL, 0xf96ea4abUL, 0x7c78ebaeUL, 0x4b1229afUL,
    0x12ac6fadUL, 0x25c6adacUL, 0x1881f1a7UL, 0x2feb33a6UL, 0x765575a4UL,
    0x413fb7a5UL, 0xc429f8a0UL, 0xf3433aa1UL, 0xaafd7ca3UL, 0x9d97bea2UL,
    0xd073c4b5UL, 0xe71906b4UL, 0xbea740b6UL, 0x89cd82b7UL, 0x0cdbcdb2UL,
    0x3bb10fb3UL, 0x620f49b1UL, 0x55658bb0UL, 0x6822d7bbUL, 0x5f4815baUL,
    0x06f653b8UL, 0x319c91b9UL, 0xb48adebcUL, 0x83e01cbdUL, 0xda5e5abfUL,
    0xed3498beUL
  },
  {
    0x00000000UL, 0x6567bcb8UL, 0x8bc809aaUL, 0xeeafb512UL, 0x5797628fUL,
    0x32f0de37UL, 0xdc5f6b25UL, 0xb938d79dUL, 0xef28b4c5UL, 0x8a4f087dUL,
    0x64e0bd6fUL, 0x018701d7UL, 0xb8bfd64aUL, 0xddd86af2UL, 0x3377dfe0UL,
    0x56106358UL, 0x9f571950UL, 0xfa30a5e8UL, 0x149f10faUL, 0x71f8ac42UL,
    0xc8c07bdfUL, 0xada7c767UL, 0x43087275UL, 0x266fcecdUL, 0x707fad95UL,
    0x1518112dUL, 0xfbb7a43fUL, 0x9ed01887UL, 0x27e8cf1aUL, 0x428f73a2UL,
    0xac20c6b0UL, 0xc9477a08UL, 0x3eaf32a0UL, 0x5bc88e18UL, 0xb5673b0aUL,
    0xd00087b2UL, 0x6938502fUL, 0x0c5fec97UL, 0xe2f05985UL, 0x8797e53dUL,
    0xd1878665UL, 0xb4e03addUL, 0x5a4f8fcfUL, 0x3f283377UL, 0x8610e4eaUL,
    0xe3775852UL, 0x0dd8ed40UL, 0x68bf51f8UL, 0xa1f82bf0UL, 0xc49f9748UL,
    0x2a30225aUL, 0x4f579ee2UL, 0xf66f497fUL, 0x9308f5c7UL, 0x7da740d5UL,
    0x18c0fc6dUL, 0x4ed09f35UL, 0x2bb7238dUL, 0xc518969fUL, 0xa07f2a27UL,
    0x1947fdbaUL, 0x7c204102UL, 0x928ff410UL, 0xf7e848a8UL, 0x3d58149bUL,
    0x583fa823UL, 0xb6901d31UL, 0xd3f7a189UL, 0x6acf7614UL, 0x0fa8caacUL,
    0xe1077fbeUL, 0x8460c306UL, 0xd270a05eUL, 0xb7171ce6UL, 0x59b8a9f4UL,
    0x3cdf154cUL, 0x85e7c2d1UL, 0xe0807e69UL, 0x0e2fcb7bUL, 0x6b4877c3UL,
    0xa20f0dcbUL, 0xc768b173UL, 0x29c70461UL, 0x4ca0b8d9UL, 0xf5986f44UL,
    0x90ffd3fcUL, 0x7e5066eeUL, 0x1b37da56UL, 0x4d27b90eUL, 0x284005b6UL,
    0xc6efb0a4UL, 0xa3880c1cUL, 0x1ab0db81UL, 0x7fd76739UL, 0x9178d22bUL,
    0xf41f6e93UL, 0x03f7263bUL, 0x66909a83UL, 0x883f2f91UL, 0xed589329UL,
    0x546044b4UL, 0x3107f80cUL, 0xdfa84d1eUL, 0xbacff1a6UL, 0xecdf92feUL,
    0x89b82e46UL, 0x67179b54UL, 0x027027ecUL, 0xbb48f071UL, 0xde2f4cc9UL,
    0x3080f9dbUL, 0x55e74563UL, 0x9ca03f6bUL, 0xf9c783d3UL, 0x176836c1UL,
    0x720f8a79UL, 0xcb375de4UL, 0xae50e15cUL, 0x40ff544eUL, 0x2598e8f6UL,
    0x73888baeUL, 0x16ef3716UL, 0xf8408204UL, 0x9d273ebcUL, 0x241fe921UL,
    0x41785599UL, 0xafd7e08bUL, 0xcab05c33UL, 0x3bb659edUL, 0x5ed1e555UL,
    0xb07e5047UL, 0xd519ecffUL, 0x6c213b62UL, 0x094687daUL, 0xe7e932c8UL,
    0x828e8e70UL, 0xd49eed28UL, 0xb1f95190UL, 0x5f56e482UL, 0x3a31583aUL,
    0x83098fa7UL, 0xe66e331fUL, 0x08c1860dUL, 0x6da63ab5UL, 0xa4e140bdUL,
    0xc186fc05UL, 0x2f294917UL, 0x4a4ef5afUL, 0xf3762232UL, 0x96119e8aUL,
    0x78be2b98UL, 0x1dd99720UL, 0x4bc9f478UL, 0x2eae48c0UL, 0xc001fdd2UL,
    0xa566416aUL, 0x1c5e96f7UL, 0x79392a4fUL, 0x97969f5dUL, 0xf2f123e5UL,
    0x05196b4dUL, 0x607ed7f5UL, 0x8ed162e7UL, 0xebb6de5fUL, 0x528e09c2UL,
    0x37e9b57aUL, 0xd9460068UL, 0xbc21bcd0UL, 0xea31df88UL, 0x8f566330UL,
    0x61f9d622UL, 0x049e6a9aUL, 0xbda6bd07UL, 0xd8c101bfUL, 0x366eb4adUL,
    0x53090815UL, 0x9a4e721dUL, 0xff29cea5UL, 0x11867bb7UL, 0x74e1c70fUL,
    0xcdd91092UL, 0xa8beac2aUL, 0x46111938UL, 0x2376a580UL, 0x7566c6d8UL,
    0x10017a60UL, 0xfeaecf72UL, 0x9bc973caUL, 0x22f1a457UL, 0x479618efUL,
    0xa939adfdUL, 0xcc5e1145UL, 0x06ee4d76UL, 0x6389f1ceUL, 0x8d2644dcUL,
    0xe841f864UL, 0x51792ff9UL, 0x341e9341UL, 0xdab12653UL, 0xbfd69aebUL,
    0xe9c6f9b3UL, 0x8ca1450bUL, 0x620ef019UL, 0x07694ca1UL, 0xbe519b3cUL,
    0xdb362784UL, 0x35999296UL, 0x50fe2e2eUL, 0x99b95426UL, 0xfcdee89eUL,
    0x12715d8cUL, 0x7716e134UL, 0xce2e36a9UL, 0xab498a11UL, 0x45e63f03UL,
    0x208183bbUL, 0x7691e0e3UL, 0x13f65c5bUL, 0xfd59e949UL, 0x983e55f1UL,
    0x2106826cUL, 0x44613ed4UL, 0xaace8bc6UL, 0xcfa9377eUL, 0x38417fd6UL,
    0x5d26c36eUL, 0xb389767cUL, 0xd6eecac4UL, 0x6fd61d59UL, 0x0ab1a1e1UL,
    0xe41e14f3UL, 0x8179a84bUL, 0xd769cb13UL, 0xb20e77abUL, 0x5ca1c2b9UL,
    0x39c67e01UL, 0x80fea99cUL, 0xe5991524UL, 0x0b36a036UL, 0x6e511c8eUL,
    0xa7166686UL, 0xc271da3eUL, 0x2cde6f2cUL, 0x49b9d394UL, 0xf0810409UL,
    0x95e6b8b1UL, 0x7b490da3UL, 0x1e2eb11bUL, 0x483ed243UL, 0x2d596efbUL,
    0xc3f6dbe9UL, 0xa6916751UL, 0x1fa9b0ccUL, 0x7ace0c74UL, 0x9461b966UL,
    0xf10605deUL
#endif
  }
};

juint* StubRoutines::ppc64::_constants = StubRoutines::ppc64::generate_crc_constants();

juint* StubRoutines::ppc64::_barret_constants = StubRoutines::ppc64::generate_crc_barret_constants();
