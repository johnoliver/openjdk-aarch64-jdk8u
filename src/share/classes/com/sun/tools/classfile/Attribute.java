/*
 * Copyright (c) 2007, 2012, Oracle and/or its affiliates. All rights reserved.
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

package com.sun.tools.classfile;

import java.io.IOException;
import java.lang.reflect.Constructor;
import java.util.HashMap;
import java.util.Map;

/**
 *  <p><b>This is NOT part of any supported API.
 *  If you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */

public abstract class Attribute {
    public static final String AnnotationDefault        = "AnnotationDefault";
    public static final String BootstrapMethods         = "BootstrapMethods";
    public static final String CharacterRangeTable      = "CharacterRangeTable";
    public static final String Code                     = "Code";
    public static final String ConstantValue            = "ConstantValue";
    public static final String CompilationID            = "CompilationID";
    public static final String Deprecated               = "Deprecated";
    public static final String EnclosingMethod          = "EnclosingMethod";
    public static final String Exceptions               = "Exceptions";
    public static final String InnerClasses             = "InnerClasses";
    public static final String LineNumberTable          = "LineNumberTable";
    public static final String LocalVariableTable       = "LocalVariableTable";
    public static final String LocalVariableTypeTable   = "LocalVariableTypeTable";
    public static final String MethodParameters         = "MethodParameters";
    public static final String RuntimeVisibleAnnotations = "RuntimeVisibleAnnotations";
    public static final String RuntimeInvisibleAnnotations = "RuntimeInvisibleAnnotations";
    public static final String RuntimeVisibleParameterAnnotations = "RuntimeVisibleParameterAnnotations";
    public static final String RuntimeInvisibleParameterAnnotations = "RuntimeInvisibleParameterAnnotations";
    public static final String Signature                = "Signature";
    public static final String SourceDebugExtension     = "SourceDebugExtension";
    public static final String SourceFile               = "SourceFile";
    public static final String SourceID                 = "SourceID";
    public static final String StackMap                 = "StackMap";
    public static final String StackMapTable            = "StackMapTable";
    public static final String Synthetic                = "Synthetic";

    public static class Factory {
        public Factory() {
            // defer init of standardAttributeClasses until after options set up
        }

        public void setCompat(boolean compat) {
            this.compat = compat;
        }

        public Attribute createAttribute(ClassReader cr, int name_index, byte[] data)
                throws IOException {
            if (standardAttributes == null)
                init();

            ConstantPool cp = cr.getConstantPool();
            try {
                String name = cp.getUTF8Value(name_index);
                Class<? extends Attribute> attrClass = standardAttributes.get(name);
                if (attrClass != null) {
                    try {
                        Class<?>[] constrArgTypes = {ClassReader.class, int.class, int.class};
                        Constructor<? extends Attribute> constr = attrClass.getDeclaredConstructor(constrArgTypes);
                        return constr.newInstance(new Object[] { cr, name_index, data.length });
                    } catch (Throwable t) {
                        // fall through and use DefaultAttribute
                        // t.printStackTrace();
                    }
                }
            } catch (ConstantPoolException e) {
                // fall through and use DefaultAttribute
            }
            return new DefaultAttribute(cr, name_index, data);
        }

        protected void init() {
            standardAttributes = new HashMap<String,Class<? extends Attribute>>();
            standardAttributes.put(AnnotationDefault, AnnotationDefault_attribute.class);
            standardAttributes.put(BootstrapMethods, BootstrapMethods_attribute.class);
            standardAttributes.put(CharacterRangeTable, CharacterRangeTable_attribute.class);
            standardAttributes.put(Code,              Code_attribute.class);
            standardAttributes.put(ConstantValue,     ConstantValue_attribute.class);
            standardAttributes.put(Deprecated,        Deprecated_attribute.class);
            standardAttributes.put(EnclosingMethod,   EnclosingMethod_attribute.class);
            standardAttributes.put(Exceptions,        Exceptions_attribute.class);
            standardAttributes.put(InnerClasses,      InnerClasses_attribute.class);
            standardAttributes.put(LineNumberTable,   LineNumberTable_attribute.class);
            standardAttributes.put(LocalVariableTable, LocalVariableTable_attribute.class);
            standardAttributes.put(LocalVariableTypeTable, LocalVariableTypeTable_attribute.class);

            if (!compat) { // old javap does not recognize recent attributes
                standardAttributes.put(MethodParameters, MethodParameters_attribute.class);
                standardAttributes.put(CompilationID, CompilationID_attribute.class);
                standardAttributes.put(RuntimeInvisibleAnnotations, RuntimeInvisibleAnnotations_attribute.class);
                standardAttributes.put(RuntimeInvisibleParameterAnnotations, RuntimeInvisibleParameterAnnotations_attribute.class);
                standardAttributes.put(RuntimeVisibleAnnotations, RuntimeVisibleAnnotations_attribute.class);
                standardAttributes.put(RuntimeVisibleParameterAnnotations, RuntimeVisibleParameterAnnotations_attribute.class);
                standardAttributes.put(Signature,     Signature_attribute.class);
                standardAttributes.put(SourceID, SourceID_attribute.class);
            }

            standardAttributes.put(SourceDebugExtension, SourceDebugExtension_attribute.class);
            standardAttributes.put(SourceFile,        SourceFile_attribute.class);
            standardAttributes.put(StackMap,          StackMap_attribute.class);
            standardAttributes.put(StackMapTable,     StackMapTable_attribute.class);
            standardAttributes.put(Synthetic,         Synthetic_attribute.class);
        }

        private Map<String,Class<? extends Attribute>> standardAttributes;
        private boolean compat; // don't support recent attrs in compatibility mode
    }

    public static Attribute read(ClassReader cr) throws IOException {
        return cr.readAttribute();
    }

    protected Attribute(int name_index, int length) {
        attribute_name_index = name_index;
        attribute_length = length;
    }

    public String getName(ConstantPool constant_pool) throws ConstantPoolException {
        return constant_pool.getUTF8Value(attribute_name_index);
    }

    public abstract <R,D> R accept(Attribute.Visitor<R,D> visitor, D data);

    public int byteLength() {
        return 6 + attribute_length;
    }

    public final int attribute_name_index;
    public final int attribute_length;


    public interface Visitor<R,P> {
        R visitBootstrapMethods(BootstrapMethods_attribute attr, P p);
        R visitDefault(DefaultAttribute attr, P p);
        R visitAnnotationDefault(AnnotationDefault_attribute attr, P p);
        R visitCharacterRangeTable(CharacterRangeTable_attribute attr, P p);
        R visitCode(Code_attribute attr, P p);
        R visitCompilationID(CompilationID_attribute attr, P p);
        R visitConstantValue(ConstantValue_attribute attr, P p);
        R visitDeprecated(Deprecated_attribute attr, P p);
        R visitEnclosingMethod(EnclosingMethod_attribute attr, P p);
        R visitExceptions(Exceptions_attribute attr, P p);
        R visitInnerClasses(InnerClasses_attribute attr, P p);
        R visitLineNumberTable(LineNumberTable_attribute attr, P p);
        R visitLocalVariableTable(LocalVariableTable_attribute attr, P p);
        R visitLocalVariableTypeTable(LocalVariableTypeTable_attribute attr, P p);
        R visitMethodParameters(MethodParameters_attribute attr, P p);
        R visitRuntimeVisibleAnnotations(RuntimeVisibleAnnotations_attribute attr, P p);
        R visitRuntimeInvisibleAnnotations(RuntimeInvisibleAnnotations_attribute attr, P p);
        R visitRuntimeVisibleParameterAnnotations(RuntimeVisibleParameterAnnotations_attribute attr, P p);
        R visitRuntimeInvisibleParameterAnnotations(RuntimeInvisibleParameterAnnotations_attribute attr, P p);
        R visitSignature(Signature_attribute attr, P p);
        R visitSourceDebugExtension(SourceDebugExtension_attribute attr, P p);
        R visitSourceFile(SourceFile_attribute attr, P p);
        R visitSourceID(SourceID_attribute attr, P p);
        R visitStackMap(StackMap_attribute attr, P p);
        R visitStackMapTable(StackMapTable_attribute attr, P p);
        R visitSynthetic(Synthetic_attribute attr, P p);
    }
}
