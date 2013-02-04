/*
 * Copyright (c) 2012, Oracle and/or its affiliates. All rights reserved.
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
 * @bug 8002099
 * @summary Add support for intersection types in cast expression
 */

import com.sun.source.util.JavacTask;
import com.sun.tools.javac.util.List;
import com.sun.tools.javac.util.ListBuffer;
import java.net.URI;
import java.util.Arrays;
import javax.tools.Diagnostic;
import javax.tools.JavaCompiler;
import javax.tools.JavaFileObject;
import javax.tools.SimpleJavaFileObject;
import javax.tools.StandardJavaFileManager;
import javax.tools.ToolProvider;

public class IntersectionTargetTypeTest {

    static int checkCount = 0;

    enum BoundKind {
        INTF,
        CLASS,
        SAM,
        ZAM;
    }

    enum MethodKind {
        NONE,
        ABSTRACT,
        DEFAULT;
    }

    enum TypeKind {
        A("interface A { }\n", "A", BoundKind.ZAM),
        B("interface B { default void m() { } }\n", "B", BoundKind.ZAM),
        C("interface C { void m(); }\n", "C", BoundKind.SAM),
        D("interface D extends B { }\n", "D", BoundKind.ZAM),
        E("interface E extends C { }\n", "E", BoundKind.SAM),
        F("interface F extends C { void g(); }\n", "F", BoundKind.INTF),
        G("interface G extends B { void g(); }\n", "G", BoundKind.SAM),
        H("interface H extends A { void g(); }\n", "H", BoundKind.SAM),
        OBJECT("", "Object", BoundKind.CLASS),
        STRING("", "String", BoundKind.CLASS);

        String declStr;
        String typeStr;
        BoundKind boundKind;

        private TypeKind(String declStr, String typeStr, BoundKind boundKind) {
            this.declStr = declStr;
            this.typeStr = typeStr;
            this.boundKind = boundKind;
        }

        boolean compatibleSupertype(TypeKind tk) {
            if (tk == this) return true;
            switch (tk) {
                case B:
                    return this != C && this != E && this != F;
                case C:
                    return this != B && this != C && this != D && this != G;
                case D: return compatibleSupertype(B);
                case E:
                case F: return compatibleSupertype(C);
                case G: return compatibleSupertype(B);
                case H: return compatibleSupertype(A);
                default:
                    return true;
            }
        }
    }

    enum CastKind {
        ONE_ARY("(#B0)", 1),
        TWO_ARY("(#B0 & #B1)", 2),
        THREE_ARY("(#B0 & #B1 & #B2)", 3);

        String castTemplate;
        int nbounds;

        CastKind(String castTemplate, int nbounds) {
            this.castTemplate = castTemplate;
            this.nbounds = nbounds;
        }
    }

    enum ExpressionKind {
        LAMBDA("()->{}", true),
        MREF("this::m", true),
        //COND_LAMBDA("(true ? ()->{} : ()->{})", true), re-enable if spec allows this
        //COND_MREF("(true ? this::m : this::m)", true),
        STANDALONE("null", false);

        String exprString;
        boolean isFunctional;

        private ExpressionKind(String exprString, boolean isFunctional) {
            this.exprString = exprString;
            this.isFunctional = isFunctional;
        }
    }

    static class CastInfo {
        CastKind kind;
        TypeKind[] types;

        CastInfo(CastKind kind, TypeKind... types) {
            this.kind = kind;
            this.types = types;
        }

        String getCast() {
            String temp = kind.castTemplate;
            for (int i = 0; i < kind.nbounds ; i++) {
                temp = temp.replace(String.format("#B%d", i), types[i].typeStr);
            }
            return temp;
        }

        boolean wellFormed() {
            //check for duplicate types
            for (int i = 0 ; i < types.length ; i++) {
                for (int j = 0 ; j < types.length ; j++) {
                    if (i != j && types[i] == types[j]) {
                        return false;
                    }
                }
            }
            //check that classes only appear as first bound
            boolean classOk = true;
            for (int i = 0 ; i < types.length ; i++) {
                if (types[i].boundKind == BoundKind.CLASS &&
                        !classOk) {
                    return false;
                }
                classOk = false;
            }
            //check that supertypes are mutually compatible
            for (int i = 0 ; i < types.length ; i++) {
                for (int j = 0 ; j < types.length ; j++) {
                    if (!types[i].compatibleSupertype(types[j]) && i != j) {
                        return false;
                    }
                }
            }
            return true;
        }
    }

    public static void main(String... args) throws Exception {
        //create default shared JavaCompiler - reused across multiple compilations
        JavaCompiler comp = ToolProvider.getSystemJavaCompiler();
        StandardJavaFileManager fm = comp.getStandardFileManager(null, null, null);

        for (CastInfo cInfo : allCastInfo()) {
            for (ExpressionKind ek : ExpressionKind.values()) {
                new IntersectionTargetTypeTest(cInfo, ek).run(comp, fm);
            }
        }
        System.out.println("Total check executed: " + checkCount);
    }

    static List<CastInfo> allCastInfo() {
        ListBuffer<CastInfo> buf = ListBuffer.lb();
        for (CastKind kind : CastKind.values()) {
            for (TypeKind b1 : TypeKind.values()) {
                if (kind.nbounds == 1) {
                    buf.append(new CastInfo(kind, b1));
                    continue;
                } else {
                    for (TypeKind b2 : TypeKind.values()) {
                        if (kind.nbounds == 2) {
                            buf.append(new CastInfo(kind, b1, b2));
                            continue;
                        } else {
                            for (TypeKind b3 : TypeKind.values()) {
                                buf.append(new CastInfo(kind, b1, b2, b3));
                            }
                        }
                    }
                }
            }
        }
        return buf.toList();
    }

    CastInfo cInfo;
    ExpressionKind ek;
    JavaSource source;
    DiagnosticChecker diagChecker;

    IntersectionTargetTypeTest(CastInfo cInfo, ExpressionKind ek) {
        this.cInfo = cInfo;
        this.ek = ek;
        this.source = new JavaSource();
        this.diagChecker = new DiagnosticChecker();
    }

    class JavaSource extends SimpleJavaFileObject {

        String bodyTemplate = "class Test {\n" +
                              "   void m() { }\n" +
                              "   void test() {\n" +
                              "      Object o = #C#E;\n" +
                              "   } }";

        String source = "";

        public JavaSource() {
            super(URI.create("myfo:/Test.java"), JavaFileObject.Kind.SOURCE);
            for (TypeKind tk : TypeKind.values()) {
                source += tk.declStr;
            }
            source += bodyTemplate.replaceAll("#C", cInfo.getCast()).replaceAll("#E", ek.exprString);
        }

        @Override
        public CharSequence getCharContent(boolean ignoreEncodingErrors) {
            return source;
        }
    }

    void run(JavaCompiler tool, StandardJavaFileManager fm) throws Exception {
        JavacTask ct = (JavacTask)tool.getTask(null, fm, diagChecker,
                null, null, Arrays.asList(source));
        try {
            ct.analyze();
        } catch (Throwable ex) {
            throw new AssertionError("Error thrown when compiling the following code:\n" + source.getCharContent(true));
        }
        check();
    }

    void check() {
        checkCount++;

        boolean errorExpected = !cInfo.wellFormed();

        if (ek.isFunctional) {
            //first bound must be a SAM
            errorExpected |= cInfo.types[0].boundKind != BoundKind.SAM;
            if (cInfo.types.length > 1) {
                //additional bounds must be ZAMs
                for (int i = 1; i < cInfo.types.length; i++) {
                    errorExpected |= cInfo.types[i].boundKind != BoundKind.ZAM;
                }
            }
        }

        if (errorExpected != diagChecker.errorFound) {
            throw new Error("invalid diagnostics for source:\n" +
                source.getCharContent(true) +
                "\nFound error: " + diagChecker.errorFound +
                "\nExpected error: " + errorExpected);
        }
    }

    static class DiagnosticChecker implements javax.tools.DiagnosticListener<JavaFileObject> {

        boolean errorFound;

        public void report(Diagnostic<? extends JavaFileObject> diagnostic) {
            if (diagnostic.getKind() == Diagnostic.Kind.ERROR) {
                errorFound = true;
            }
        }
    }
}
