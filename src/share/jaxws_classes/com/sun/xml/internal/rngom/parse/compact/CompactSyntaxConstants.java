/*
 * Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.
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
 * Copyright (C) 2004-2011
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/* Generated By:JavaCC: Do not edit this line. CompactSyntaxConstants.java */
package com.sun.xml.internal.rngom.parse.compact;


/**
 * Token literal values and constants.
 * Generated by org.javacc.parser.OtherFilesGen#start()
 */
public interface CompactSyntaxConstants {

  /** End of File. */
  int EOF = 0;
  /** RegularExpression Id. */
  int NEWLINE = 37;
  /** RegularExpression Id. */
  int NOT_NEWLINE = 38;
  /** RegularExpression Id. */
  int WS = 39;
  /** RegularExpression Id. */
  int DOCUMENTATION = 40;
  /** RegularExpression Id. */
  int DOCUMENTATION_CONTINUE = 41;
  /** RegularExpression Id. */
  int SINGLE_LINE_COMMENT = 42;
  /** RegularExpression Id. */
  int DOCUMENTATION_AFTER_SINGLE_LINE_COMMENT = 43;
  /** RegularExpression Id. */
  int SINGLE_LINE_COMMENT_CONTINUE = 44;
  /** RegularExpression Id. */
  int BASE_CHAR = 45;
  /** RegularExpression Id. */
  int IDEOGRAPHIC = 46;
  /** RegularExpression Id. */
  int LETTER = 47;
  /** RegularExpression Id. */
  int COMBINING_CHAR = 48;
  /** RegularExpression Id. */
  int DIGIT = 49;
  /** RegularExpression Id. */
  int EXTENDER = 50;
  /** RegularExpression Id. */
  int NMSTART = 51;
  /** RegularExpression Id. */
  int NMCHAR = 52;
  /** RegularExpression Id. */
  int NCNAME = 53;
  /** RegularExpression Id. */
  int IDENTIFIER = 54;
  /** RegularExpression Id. */
  int ESCAPED_IDENTIFIER = 55;
  /** RegularExpression Id. */
  int PREFIX_STAR = 56;
  /** RegularExpression Id. */
  int PREFIXED_NAME = 57;
  /** RegularExpression Id. */
  int LITERAL = 58;
  /** RegularExpression Id. */
  int FANNOTATE = 59;
  /** RegularExpression Id. */
  int ILLEGAL_CHAR = 60;

  /** Lexical state. */
  int DEFAULT = 0;
  /** Lexical state. */
  int AFTER_SINGLE_LINE_COMMENT = 1;
  /** Lexical state. */
  int AFTER_DOCUMENTATION = 2;

  /** Literal token values. */
  String[] tokenImage = {
    "<EOF>",
    "\"[\"",
    "\"=\"",
    "\"&=\"",
    "\"|=\"",
    "\"start\"",
    "\"div\"",
    "\"include\"",
    "\"~\"",
    "\"]\"",
    "\"grammar\"",
    "\"{\"",
    "\"}\"",
    "\"namespace\"",
    "\"default\"",
    "\"inherit\"",
    "\"datatypes\"",
    "\"empty\"",
    "\"text\"",
    "\"notAllowed\"",
    "\"|\"",
    "\"&\"",
    "\",\"",
    "\"+\"",
    "\"?\"",
    "\"*\"",
    "\"element\"",
    "\"attribute\"",
    "\"(\"",
    "\")\"",
    "\"-\"",
    "\"list\"",
    "\"mixed\"",
    "\"external\"",
    "\"parent\"",
    "\"string\"",
    "\"token\"",
    "<NEWLINE>",
    "<NOT_NEWLINE>",
    "<WS>",
    "<DOCUMENTATION>",
    "<DOCUMENTATION_CONTINUE>",
    "<SINGLE_LINE_COMMENT>",
    "<DOCUMENTATION_AFTER_SINGLE_LINE_COMMENT>",
    "<SINGLE_LINE_COMMENT_CONTINUE>",
    "<BASE_CHAR>",
    "<IDEOGRAPHIC>",
    "<LETTER>",
    "<COMBINING_CHAR>",
    "<DIGIT>",
    "<EXTENDER>",
    "<NMSTART>",
    "<NMCHAR>",
    "<NCNAME>",
    "<IDENTIFIER>",
    "<ESCAPED_IDENTIFIER>",
    "<PREFIX_STAR>",
    "<PREFIXED_NAME>",
    "<LITERAL>",
    "\">>\"",
    "<ILLEGAL_CHAR>",
  };

}
