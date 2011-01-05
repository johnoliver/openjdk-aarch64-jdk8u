/*
 * Copyright (c) 2003, 2010, Oracle and/or its affiliates. All rights reserved.
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

package com.sun.tools.doclets.internal.toolkit.builders;

import java.util.*;
import com.sun.tools.doclets.internal.toolkit.util.*;
import com.sun.tools.doclets.internal.toolkit.*;
import com.sun.javadoc.*;

/**
 * Builds documentation for a enum constants.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 *
 * @author Jamie Ho
 * @author Bhavesh Patel (Modified)
 * @since 1.5
 */
public class EnumConstantBuilder extends AbstractMemberBuilder {

    /**
     * The class whose enum constants are being documented.
     */
    private ClassDoc classDoc;

    /**
     * The visible enum constantss for the given class.
     */
    private VisibleMemberMap visibleMemberMap;

    /**
     * The writer to output the enum constants documentation.
     */
    private EnumConstantWriter writer;

    /**
     * The list of enum constants being documented.
     */
    private List<ProgramElementDoc> enumConstants;

    /**
     * The index of the current enum constant that is being documented at this point
     * in time.
     */
    private int currentEnumConstantsIndex;

    /**
     * Construct a new EnumConstantsBuilder.
     *
     * @param configuration the current configuration of the
     *                      doclet.
     */
    private EnumConstantBuilder(Configuration configuration) {
        super(configuration);
    }

    /**
     * Construct a new EnumConstantsBuilder.
     *
     * @param configuration the current configuration of the doclet.
     * @param classDoc the class whoses members are being documented.
     * @param writer the doclet specific writer.
     */
    public static EnumConstantBuilder getInstance(
            Configuration configuration,
            ClassDoc classDoc,
            EnumConstantWriter writer) {
        EnumConstantBuilder builder = new EnumConstantBuilder(configuration);
        builder.classDoc = classDoc;
        builder.writer = writer;
        builder.visibleMemberMap =
                new VisibleMemberMap(
                classDoc,
                VisibleMemberMap.ENUM_CONSTANTS,
                configuration.nodeprecated);
        builder.enumConstants =
                new ArrayList<ProgramElementDoc>(builder.visibleMemberMap.getMembersFor(classDoc));
        if (configuration.getMemberComparator() != null) {
            Collections.sort(
                    builder.enumConstants,
                    configuration.getMemberComparator());
        }
        return builder;
    }

    /**
     * {@inheritDoc}
     */
    public String getName() {
        return "EnumConstantDetails";
    }

    /**
     * Returns a list of enum constants that will be documented for the given class.
     * This information can be used for doclet specific documentation
     * generation.
     *
     * @param classDoc the {@link ClassDoc} we want to check.
     * @return a list of enum constants that will be documented.
     */
    public List<ProgramElementDoc> members(ClassDoc classDoc) {
        return visibleMemberMap.getMembersFor(classDoc);
    }

    /**
     * Returns the visible member map for the enum constants of this class.
     *
     * @return the visible member map for the enum constants of this class.
     */
    public VisibleMemberMap getVisibleMemberMap() {
        return visibleMemberMap;
    }

    /**
     * summaryOrder.size()
     */
    public boolean hasMembersToDocument() {
        return enumConstants.size() > 0;
    }

    /**
     * Build the enum constant documentation.
     *
     * @param node the XML element that specifies which components to document
     * @param memberDetailsTree the content tree to which the documentation will be added
     */
    public void buildEnumConstant(XMLNode node, Content memberDetailsTree) {
        if (writer == null) {
            return;
        }
        int size = enumConstants.size();
        if (size > 0) {
            Content enumConstantsDetailsTree = writer.getEnumConstantsDetailsTreeHeader(
                    classDoc, memberDetailsTree);
            for (currentEnumConstantsIndex = 0; currentEnumConstantsIndex < size;
                    currentEnumConstantsIndex++) {
                Content enumConstantsTree = writer.getEnumConstantsTreeHeader(
                        (FieldDoc) enumConstants.get(currentEnumConstantsIndex),
                        enumConstantsDetailsTree);
                buildChildren(node, enumConstantsTree);
                enumConstantsDetailsTree.addContent(writer.getEnumConstants(
                        enumConstantsTree, (currentEnumConstantsIndex == size - 1)));
            }
            memberDetailsTree.addContent(
                    writer.getEnumConstantsDetails(enumConstantsDetailsTree));
        }
    }

    /**
     * Build the signature.
     *
     * @param node the XML element that specifies which components to document
     * @param enumConstantsTree the content tree to which the documentation will be added
     */
    public void buildSignature(XMLNode node, Content enumConstantsTree) {
        enumConstantsTree.addContent(writer.getSignature(
                (FieldDoc) enumConstants.get(currentEnumConstantsIndex)));
    }

    /**
     * Build the deprecation information.
     *
     * @param node the XML element that specifies which components to document
     * @param enumConstantsTree the content tree to which the documentation will be added
     */
    public void buildDeprecationInfo(XMLNode node, Content enumConstantsTree) {
        writer.addDeprecated(
                (FieldDoc) enumConstants.get(currentEnumConstantsIndex),
                enumConstantsTree);
    }

    /**
     * Build the comments for the enum constant.  Do nothing if
     * {@link Configuration#nocomment} is set to true.
     *
     * @param node the XML element that specifies which components to document
     * @param enumConstantsTree the content tree to which the documentation will be added
     */
    public void buildEnumConstantComments(XMLNode node, Content enumConstantsTree) {
        if (!configuration.nocomment) {
            writer.addComments(
                    (FieldDoc) enumConstants.get(currentEnumConstantsIndex),
                    enumConstantsTree);
        }
    }

    /**
     * Build the tag information.
     *
     * @param node the XML element that specifies which components to document
     * @param enumConstantsTree the content tree to which the documentation will be added
     */
    public void buildTagInfo(XMLNode node, Content enumConstantsTree) {
        writer.addTags(
                (FieldDoc) enumConstants.get(currentEnumConstantsIndex),
                enumConstantsTree);
    }

    /**
     * Return the enum constant writer for this builder.
     *
     * @return the enum constant writer for this builder.
     */
    public EnumConstantWriter getWriter() {
        return writer;
    }
}
