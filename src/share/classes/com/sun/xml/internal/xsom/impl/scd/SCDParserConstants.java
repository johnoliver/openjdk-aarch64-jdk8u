/*
 * Copyright 2005-2006 Sun Microsystems, Inc.  All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Sun designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Sun in the LICENSE file that accompanied this code.
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
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 */
/* Generated By:JavaCC: Do not edit this line. SCDParserConstants.java */
package com.sun.xml.internal.xsom.impl.scd;

public interface SCDParserConstants {

  int EOF = 0;
  int Letter = 6;
  int BaseChar = 7;
  int Ideographic = 8;
  int CombiningChar = 9;
  int UnicodeDigit = 10;
  int Extender = 11;
  int NCNAME = 12;
  int NUMBER = 13;
  int FACETNAME = 14;

  int DEFAULT = 0;

  String[] tokenImage = {
    "<EOF>",
    "\" \"",
    "\"\\t\"",
    "\"\\n\"",
    "\"\\r\"",
    "\"\\f\"",
    "<Letter>",
    "<BaseChar>",
    "<Ideographic>",
    "<CombiningChar>",
    "<UnicodeDigit>",
    "<Extender>",
    "<NCNAME>",
    "<NUMBER>",
    "<FACETNAME>",
    "\":\"",
    "\"/\"",
    "\"//\"",
    "\"attribute::\"",
    "\"@\"",
    "\"element::\"",
    "\"substitutionGroup::\"",
    "\"type::\"",
    "\"~\"",
    "\"baseType::\"",
    "\"primitiveType::\"",
    "\"itemType::\"",
    "\"memberType::\"",
    "\"scope::\"",
    "\"attributeGroup::\"",
    "\"group::\"",
    "\"identityContraint::\"",
    "\"key::\"",
    "\"notation::\"",
    "\"model::sequence\"",
    "\"model::choice\"",
    "\"model::all\"",
    "\"model::*\"",
    "\"any::*\"",
    "\"anyAttribute::*\"",
    "\"facet::*\"",
    "\"facet::\"",
    "\"component::*\"",
    "\"x-schema::\"",
    "\"x-schema::*\"",
    "\"*\"",
    "\"0\"",
  };

}
