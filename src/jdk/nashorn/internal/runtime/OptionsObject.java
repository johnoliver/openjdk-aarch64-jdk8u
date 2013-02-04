/*
 * Copyright (c) 2010, 2013, Oracle and/or its affiliates. All rights reserved.
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

package jdk.nashorn.internal.runtime;

import java.util.TimeZone;

/**
 * A convenience object to expose only command line options from a Context.
 */
public final class OptionsObject {
    /** Always allow functions as statements */
    public final boolean _anon_functions;

    /** Size of the per-global Class cache size */
    public final int     _class_cache_size;

    /** Only compile script, do not run it or generate other ScriptObjects */
    public final boolean _compile_only;

    /** Accumulated callsite flags that will be used when bootstrapping script callsites */
    public final int     _callsite_flags;

    /** Generate line number table in class files */
    public final boolean _debug_lines;

    /** Package to which generated class files are added */
    public final String  _dest_dir;

    /** Display stack trace upon error, default is false */
    public final boolean _dump_on_error;

    /** Invalid lvalue expressions should be reported as early errors */
    public final boolean _early_lvalue_error;

    /** Empty statements should be preserved in the AST */
    public final boolean _empty_statements;

    /** Show full Nashorn version */
    public final boolean _fullversion;

    /** Create a new class loaded for each compilation */
    public final boolean _loader_per_compile;

    /** Package to which generated class files are added */
    public final String  _package;

    /** Only parse the source code, do not compile */
    public final boolean _parse_only;

    /** Print the AST before lowering */
    public final boolean _print_ast;

    /** Print the AST after lowering */
    public final boolean _print_lower_ast;

    /** Print resulting bytecode for script */
    public final boolean _print_code;

    /** Print function will no print newline characters */
    public final boolean _print_no_newline;

    /** Print AST in more human readable form */
    public final boolean _print_parse;

    /** Print AST in more human readable form after Lowering */
    public final boolean _print_lower_parse;

    /** print symbols and their contents for the script */
    public final boolean _print_symbols;

    /** is this context in scripting mode? */
    public final boolean _scripting;

    /** is this context in strict mode? */
    public final boolean _strict;

    /** print version info of Nashorn */
    public final boolean _version;

    /** should code verification be done of generated bytecode */
    public final boolean _verify_code;

    /** time zone for this context */
    public final TimeZone _timezone;

    /**
     * Constructor
     *
     * @param context a context
     */
    public OptionsObject(final Context context) {
        this._anon_functions = context._anon_functions;
        this._callsite_flags = context._callsite_flags;
        this._class_cache_size = context._class_cache_size;
        this._compile_only = context._compile_only;
        this._debug_lines = context._debug_lines;
        this._dest_dir = context._dest_dir;
        this._dump_on_error = context._dump_on_error;
        this._early_lvalue_error = context._early_lvalue_error;
        this._empty_statements = context._empty_statements;
        this._fullversion = context._fullversion;
        this._loader_per_compile = context._loader_per_compile;
        this._package = context._package;
        this._parse_only = context._parse_only;
        this._print_ast = context._print_ast;
        this._print_code = context._print_code;
        this._print_lower_ast = context._print_lower_ast;
        this._print_lower_parse = context._print_lower_parse;
        this._print_no_newline = context._print_no_newline;
        this._print_parse = context._print_parse;
        this._print_symbols = context._print_symbols;
        this._scripting = context._scripting;
        this._strict = context._strict;
        this._timezone = context._timezone;
        this._verify_code = context._verify_code;
        this._version = context._version;
    }
}
