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

package jdk.nashorn.internal.ir.debug;

import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.Arrays;
import java.util.List;
import jdk.nashorn.internal.codegen.Compiler;
import jdk.nashorn.internal.codegen.CompilerConstants;
import jdk.nashorn.internal.ir.AccessNode;
import jdk.nashorn.internal.ir.BinaryNode;
import jdk.nashorn.internal.ir.Block;
import jdk.nashorn.internal.ir.BreakNode;
import jdk.nashorn.internal.ir.CallNode;
import jdk.nashorn.internal.ir.CaseNode;
import jdk.nashorn.internal.ir.CatchNode;
import jdk.nashorn.internal.ir.ContinueNode;
import jdk.nashorn.internal.ir.DoWhileNode;
import jdk.nashorn.internal.ir.EmptyNode;
import jdk.nashorn.internal.ir.ExecuteNode;
import jdk.nashorn.internal.ir.ForNode;
import jdk.nashorn.internal.ir.FunctionNode;
import jdk.nashorn.internal.ir.IdentNode;
import jdk.nashorn.internal.ir.IfNode;
import jdk.nashorn.internal.ir.IndexNode;
import jdk.nashorn.internal.ir.LabelNode;
import jdk.nashorn.internal.ir.LineNumberNode;
import jdk.nashorn.internal.ir.LiteralNode;
import jdk.nashorn.internal.ir.Node;
import jdk.nashorn.internal.ir.ObjectNode;
import jdk.nashorn.internal.ir.PropertyNode;
import jdk.nashorn.internal.ir.ReturnNode;
import jdk.nashorn.internal.ir.RuntimeNode;
import jdk.nashorn.internal.ir.SplitNode;
import jdk.nashorn.internal.ir.SwitchNode;
import jdk.nashorn.internal.ir.TernaryNode;
import jdk.nashorn.internal.ir.ThrowNode;
import jdk.nashorn.internal.ir.TryNode;
import jdk.nashorn.internal.ir.UnaryNode;
import jdk.nashorn.internal.ir.VarNode;
import jdk.nashorn.internal.ir.WhileNode;
import jdk.nashorn.internal.ir.WithNode;
import jdk.nashorn.internal.ir.visitor.NodeVisitor;
import jdk.nashorn.internal.parser.JSONParser;
import jdk.nashorn.internal.parser.Lexer.RegexToken;
import jdk.nashorn.internal.parser.Parser;
import jdk.nashorn.internal.parser.TokenType;
import jdk.nashorn.internal.runtime.Context;
import jdk.nashorn.internal.runtime.ParserException;
import jdk.nashorn.internal.runtime.ScriptObject;
import jdk.nashorn.internal.runtime.Source;

/**
 * This IR writer produces a JSON string that represents AST as a JSON string.
 */
public final class JSONWriter extends NodeVisitor {
    /**
     * Returns AST as JSON compatible string.
     *
     * @param code code to be parsed
     * @param name name of the code source (used for location)
     * @param includeLoc tells whether to include location information for nodes or not
     * @return JSON string representation of AST of the supplied code
     */
    public static String parse(final String code, final String name, final boolean includeLoc) {
        final ScriptObject global     = Context.getGlobal();
        final Context      context    = AccessController.doPrivileged(
                new PrivilegedAction<Context>() {
                    @Override
                    public Context run() {
                        return Context.getContext();
                    }
                });
        final Compiler     compiler   = Compiler.compiler(new Source(name, code), context, new Context.ThrowErrorManager(), context._strict);
        final Parser       parser     = new Parser(compiler, context._strict);
        final JSONWriter   jsonWriter = new JSONWriter(includeLoc);
        try {
            final FunctionNode functionNode = parser.parse(CompilerConstants.RUN_SCRIPT.tag());
            functionNode.accept(jsonWriter);
            return jsonWriter.getString();
        } catch (final ParserException e) {
            e.throwAsEcmaException(global);
            return null;
        }
    }

    @Override
    protected Node enterDefault(final Node node) {
        objectStart();
        location(node);

        return node;
    }

    @Override
    protected Node leaveDefault(final Node node) {
        objectEnd();
        return null;
    }

    @Override
    public Node enter(final AccessNode accessNode) {
        enterDefault(accessNode);

        type("MemberExpression");
        comma();

        property("object");
        accessNode.getBase().accept(this);
        comma();

        property("property");
        accessNode.getProperty().accept(this);
        comma();

        property("computed", false);

        return leaveDefault(accessNode);
    }

    @Override
    public Node enter(final Block block) {
        enterDefault(block);

        type("BlockStatement");
        comma();

        array("body", block.getStatements());

        return leaveDefault(block);
    }

    private static boolean isLogical(final TokenType tt) {
        switch (tt) {
            case AND:
            case OR:
                return true;
            default:
                return false;
        }
    }

    @Override
    public Node enter(final BinaryNode binaryNode) {
        enterDefault(binaryNode);

        final String name;
        if (binaryNode.isAssignment()) {
            name = "AssignmentExpression";
        } else if (isLogical(binaryNode.tokenType())) {
            name = "LogicalExpression";
        } else {
            name = "BinaryExpression";
        }

        type(name);
        comma();

        property("operator", binaryNode.tokenType().getName());
        comma();

        property("left");
        binaryNode.lhs().accept(this);
        comma();

        property("right");
        binaryNode.rhs().accept(this);

        return leaveDefault(binaryNode);
    }

    @Override
    public Node enter(final BreakNode breakNode) {
        enterDefault(breakNode);

        type("BreakStatement");
        comma();

        final LabelNode label = breakNode.getLabel();
        if (label != null) {
            property("label", label.getLabel().getName());
        } else {
            property("label");
            nullValue();
        }

        return leaveDefault(breakNode);
    }

    @Override
    public Node enter(final CallNode callNode) {
        enterDefault(callNode);

        type("CallExpression");
        comma();

        property("callee");
        callNode.getFunction().accept(this);
        comma();

        array("arguments", callNode.getArgs());

        return leaveDefault(callNode);
    }

    @Override
    public Node enter(final CaseNode caseNode) {
        enterDefault(caseNode);

        type("SwitchCase");
        comma();

        final Node test = caseNode.getTest();
        property("test");
        if (test != null) {
            test.accept(this);
        } else {
            nullValue();
        }
        comma();

        array("consequent", caseNode.getBody().getStatements());

        return leaveDefault(caseNode);
    }

    @Override
    public Node enter(final CatchNode catchNode) {
        enterDefault(catchNode);

        type("CatchClause");
        comma();

        property("param");
        catchNode.getException().accept(this);
        comma();

        final Node guard = catchNode.getExceptionCondition();
        property("guard");
        if (guard != null) {
            guard.accept(this);
        } else {
            nullValue();
        }
        comma();

        property("body");
        catchNode.getBody().accept(this);

        return leaveDefault(catchNode);
    }

    @Override
    public Node enter(final ContinueNode continueNode) {
        enterDefault(continueNode);

        type("ContinueStatement");
        comma();

        final LabelNode label = continueNode.getLabel();
        if (label != null) {
            property("label", label.getLabel().getName());
        } else {
            property("label");
            nullValue();
        }

        return leaveDefault(continueNode);
    }

    @Override
    public Node enter(final DoWhileNode doWhileNode) {
        enterDefault(doWhileNode);

        type("DoWhileStatement");
        comma();

        property("body");
        doWhileNode.getBody().accept(this);
        comma();

        property("test");
        doWhileNode.getTest().accept(this);

        return leaveDefault(doWhileNode);
    }

    @Override
    public Node enter(final EmptyNode emptyNode) {
        enterDefault(emptyNode);

        type("EmptyStatement");

        return leaveDefault(emptyNode);
    }

    @Override
    public Node enter(final ExecuteNode executeNode) {
        enterDefault(executeNode);

        type("ExpressionStatement");
        comma();

        property("expression");
        executeNode.getExpression().accept(this);

        return leaveDefault(executeNode);
    }

    @Override
    public Node enter(final ForNode forNode) {
        enterDefault(forNode);

        if (forNode.isForIn() || forNode.isForEach()) {
            type("ForInStatement");
            comma();

            property("left");
            forNode.getInit().accept(this);
            comma();

            property("right");
            forNode.getModify().accept(this);
            comma();

            property("body");
            forNode.getBody().accept(this);
            comma();

            property("each", forNode.isForEach());
        } else {
            type("ForStatement");
            comma();

            final Node init = forNode.getInit();
            property("init");
            if (init != null) {
                init.accept(this);
            } else {
                nullValue();
            }
            comma();

            final Node test = forNode.getTest();
            property("test");
            if (test != null) {
                test.accept(this);
            } else {
                nullValue();
            }
            comma();

            final Node update = forNode.getModify();
            property("update");
            if (update != null) {
                update.accept(this);
            } else {
                nullValue();
            }
            comma();

            property("body");
            forNode.getBody().accept(this);
        }

        return leaveDefault(forNode);
    }

    @Override
    public Node enter(final FunctionNode functionNode) {
        enterDefault(functionNode);

        final boolean program = functionNode.isScript();
        final String name;
        if (program) {
            name = "Program";
        } else if (functionNode.isStatement()) {
            name = "FunctionDeclaration";
        } else {
            name = "FunctionExpression";
        }
        type(name);
        comma();

        if (! program) {
            property("id");
            if (functionNode.isAnonymous()) {
                nullValue();
            } else {
                functionNode.getIdent().accept(this);
            }
            comma();
        }

        property("rest");
        nullValue();
        comma();

        if (!program) {
            array("params", functionNode.getParameters());
            comma();
        }

        // body consists of nested functions and statements
        final List<FunctionNode> funcs = functionNode.getFunctions();
        final List<Node> stats = functionNode.getStatements();
        final int size = stats.size() + funcs.size();
        int idx = 0;
        arrayStart("body");

        for (final Node func : funcs) {
            func.accept(this);
            if (idx != (size - 1)) {
                comma();
            }
            idx++;
        }

        for (final Node stat : stats) {
            if (! stat.isDebug()) {
                stat.accept(this);
                if (idx != (size - 1)) {
                    comma();
                }
            }
            idx++;
        }
        arrayEnd();

        return leaveDefault(functionNode);
    }

    @Override
    public Node enter(final IdentNode identNode) {
        enterDefault(identNode);

        final String name = identNode.getName();
        if ("this".equals(name)) {
            type("ThisExpression");
        } else {
            type("Identifier");
            comma();
            property("name", identNode.getName());
        }

        return leaveDefault(identNode);
    }

    @Override
    public Node enter(final IfNode ifNode) {
        enterDefault(ifNode);

        type("IfStatement");
        comma();

        property("test");
        ifNode.getTest().accept(this);
        comma();

        property("consequent");
        ifNode.getPass().accept(this);
        final Node elsePart = ifNode.getFail();
        comma();

        property("alternate");
        if (elsePart != null) {
            elsePart.accept(this);
        } else {
            nullValue();
        }

        return leaveDefault(ifNode);
    }

    @Override
    public Node enter(final IndexNode indexNode) {
        enterDefault(indexNode);

        type("MemberExpression");
        comma();

        property("object");
        indexNode.getBase().accept(this);
        comma();

        property("property");
        indexNode.getIndex().accept(this);
        comma();

        property("computed", true);

        return leaveDefault(indexNode);
    }

    @Override
    public Node enter(final LabelNode labelNode) {
        enterDefault(labelNode);

        type("LabeledStatement");
        comma();

        property("label");
        labelNode.getLabel().accept(this);
        comma();

        property("body");
        labelNode.getBody().accept(this);

        return leaveDefault(labelNode);
    }

    @Override
    public Node enter(final LineNumberNode lineNumberNode) {
        return null;
    }

    @SuppressWarnings("rawtypes")
    @Override
    public Node enter(final LiteralNode literalNode) {
        enterDefault(literalNode);

        if (literalNode instanceof LiteralNode.ArrayLiteralNode) {
            type("ArrayExpression");
            comma();

            final Node[] value = (Node[])literalNode.getValue();
            array("elements", Arrays.asList(value));
        } else {
            type("Literal");
            comma();

            property("value");
            final Object value = literalNode.getValue();
            if (value instanceof RegexToken) {
                // encode RegExp literals as Strings of the form /.../<flags>
                final RegexToken regex = (RegexToken)value;
                final StringBuilder regexBuf = new StringBuilder();
                regexBuf.append('/');
                regexBuf.append(regex.getExpression());
                regexBuf.append('/');
                regexBuf.append(regex.getOptions());
                buf.append(quote(regexBuf.toString()));
            } else {
                final String str = literalNode.getString();
                // encode every String literal with prefix '$' so that script
                // can differentiate b/w RegExps as Strings and Strings.
                buf.append(literalNode.isString()? quote("$" + str) : str);
            }
        }

        return leaveDefault(literalNode);
    }

    @Override
    public Node enter(final ObjectNode objectNode) {
        enterDefault(objectNode);

        type("ObjectExpression");
        comma();

        array("properties", objectNode.getElements());

        return leaveDefault(objectNode);
    }

    @Override
    public Node enter(final PropertyNode propertyNode) {
        final Node key = propertyNode.getKey();

        final Node value = propertyNode.getValue();
        if (value != null) {
            objectStart();
            location(propertyNode);

            property("key");
            key.accept(this);
            comma();

            property("value");
            value.accept(this);
            comma();

            property("kind", "init");

            objectEnd();
        } else {
            // getter
            final Node getter = propertyNode.getGetter();
            if (getter != null) {
                objectStart();
                location(propertyNode);

                property("key");
                key.accept(this);
                comma();

                property("value");
                getter.accept(this);
                comma();

                property("kind", "get");

                objectEnd();
            }

            // setter
            final Node setter = propertyNode.getSetter();
            if (setter != null) {
                objectStart();
                location(propertyNode);

                property("key");
                key.accept(this);
                comma();

                property("value");
                setter.accept(this);
                comma();

                property("kind", "set");

                objectEnd();
            }
        }

        return null;
    }

    @Override
    public Node enter(final ReturnNode returnNode) {
        enterDefault(returnNode);

        type("ReturnStatement");
        comma();

        final Node arg = returnNode.getExpression();
        property("argument");
        if (arg != null) {
            arg.accept(this);
        } else {
            nullValue();
        }

        return leaveDefault(returnNode);
    }

    @Override
    public Node enter(final RuntimeNode runtimeNode) {
        final RuntimeNode.Request req = runtimeNode.getRequest();

        if (req == RuntimeNode.Request.DEBUGGER) {
            enterDefault(runtimeNode);

            type("DebuggerStatement");

            return leaveDefault(runtimeNode);
        }

        return null;
    }

    @Override
    public Node enter(final SplitNode splitNode) {
        return null;
    }

    @Override
    public Node enter(final SwitchNode switchNode) {
        enterDefault(switchNode);

        type("SwitchStatement");
        comma();

        property("discriminant");
        switchNode.getExpression().accept(this);
        comma();

        array("cases", switchNode.getCases());

        return leaveDefault(switchNode);
    }

    @Override
    public Node enter(final TernaryNode ternaryNode) {
        enterDefault(ternaryNode);

        type("ConditionalExpression");
        comma();

        property("test");
        ternaryNode.lhs().accept(this);
        comma();

        property("consequent");
        ternaryNode.rhs().accept(this);
        comma();

        property("alternate");
        ternaryNode.third().accept(this);

        return leaveDefault(ternaryNode);
    }

    @Override
    public Node enter(final ThrowNode throwNode) {
        enterDefault(throwNode);

        type("ThrowStatement");
        comma();

        property("argument");
        throwNode.getExpression().accept(this);

        return leaveDefault(throwNode);
    }

    @Override
    public Node enter(final TryNode tryNode) {
        enterDefault(tryNode);

        type("TryStatement");
        comma();

        property("block");
        tryNode.getBody().accept(this);
        comma();

        array("handlers", tryNode.getCatches());
        comma();

        property("finalizer");
        final Node finallyNode = tryNode.getFinallyBody();
        if (finallyNode != null) {
            finallyNode.accept(this);
        } else {
            nullValue();
        }

        return leaveDefault(tryNode);
    }

    @Override
    public Node enter(final UnaryNode unaryNode) {
        enterDefault(unaryNode);

        final TokenType tokenType = unaryNode.tokenType();
        if (tokenType == TokenType.NEW) {
            type("NewExpression");
            comma();

            final CallNode callNode = (CallNode)unaryNode.rhs();
            property("callee");
            callNode.getFunction().accept(this);
            comma();

            array("arguments", callNode.getArgs());
        } else {
            final boolean prefix;
            final String operator;
            switch (tokenType) {
                case DECPOSTFIX:
                    prefix = false;
                    operator = "++";
                    break;
                case INCPOSTFIX:
                    prefix = false;
                    operator = "--";
                    break;
                case INCPREFIX:
                    operator = "++";
                    prefix = true;
                    break;
                case DECPREFIX:
                    operator = "--";
                    prefix = true;
                    break;
                default:
                    prefix = false;
                    operator = tokenType.getName();
            }

            type(unaryNode.isAssignment()? "UpdateExpression" : "UnaryExpression");
            comma();

            property("operator", operator);
            comma();

            property("prefix", prefix);
            comma();

            property("argument");
            unaryNode.rhs().accept(this);
        }

        return leaveDefault(unaryNode);
    }

    @Override
    public Node enter(final VarNode varNode) {
        enterDefault(varNode);

        type("VariableDeclaration");
        comma();

        arrayStart("declarations");

        // VariableDeclarator
        objectStart();
        location(varNode.getName());

        type("VariableDeclarator");
        comma();

        property("id", varNode.getName().toString());
        comma();

        property("init");
        final Node init = varNode.getInit();
        if (init != null) {
            init.accept(this);
        } else {
            nullValue();
        }

        // VariableDeclarator
        objectEnd();

        // declarations
        arrayEnd();

        return leaveDefault(varNode);
    }

    @Override
    public Node enter(final WhileNode whileNode) {
        enterDefault(whileNode);

        type("WhileStatement");
        comma();

        property("test");
        whileNode.getTest().accept(this);
        comma();

        property("block");
        whileNode.getBody().accept(this);

        return leaveDefault(whileNode);
    }

    @Override
    public Node enter(final WithNode withNode) {
        enterDefault(withNode);

        type("WithStatement");
        comma();

        property("object");
        withNode.getExpression().accept(this);
        comma();

        property("body");
        withNode.getBody().accept(this);

        return leaveDefault(withNode);
    }

    // Internals below

    private JSONWriter(final boolean includeLocation) {
        this.buf = new StringBuilder();
        this.includeLocation = includeLocation;
    }

    private final StringBuilder buf;
    private final boolean includeLocation;

    private String getString() {
        return buf.toString();
    }

    private void property(final String key, final String value) {
        buf.append('"');
        buf.append(key);
        buf.append("\":");
        if (value != null) {
            buf.append('"');
            buf.append(value);
            buf.append('"');
        }
    }

    private void property(final String key, final boolean value) {
        property(key, Boolean.toString(value));
    }

    private void property(final String key, final int value) {
        property(key, Integer.toString(value));
    }

    private void property(final String key) {
        property(key, null);
    }

    private void type(final String value) {
        property("type", value);
    }

    private void objectStart(final String name) {
        buf.append('"');
        buf.append(name);
        buf.append("\":{");
    }

    private void objectStart() {
        buf.append('{');
    }

    private void objectEnd() {
        buf.append('}');
    }

    private void array(final String name, final List<? extends Node> nodes) {
        // The size, idx comparison is just to avoid trailing comma..
        final int size = nodes.size();
        int idx = 0;
        arrayStart(name);
        for (final Node node : nodes) {
            if (! node.isDebug()) {
                node.accept(this);
                if (idx != (size - 1)) {
                    comma();
                }
            }
            idx++;
        }
        arrayEnd();
    }

    private void arrayStart(final String name) {
        buf.append('"');
        buf.append(name);
        buf.append('"');
        buf.append(':');
        buf.append('[');
    }

    private void arrayEnd() {
        buf.append(']');
    }

    private void comma() {
        buf.append(',');
    }

    private void nullValue() {
        buf.append("null");
    }

    private void location(final Node node) {
        if (includeLocation) {
            objectStart("loc");

            // source name
            final Source src = node.getSource();
            property("source", src.getName());
            comma();

            // start position
            objectStart("start");
            final int start = node.getStart();
            property("line", src.getLine(start));
            comma();
            property("column", src.getColumn(start));
            objectEnd();
            comma();

            // end position
            objectStart("end");
            final int end = node.getFinish();
            property("line", src.getLine(end));
            comma();
            property("column", src.getColumn(end));
            objectEnd();

            // end 'loc'
            objectEnd();

            comma();
        }
    }

    private static String quote(final String str) {
        return JSONParser.quote(str);
    }
}
