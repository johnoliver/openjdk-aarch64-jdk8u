/*
 * Copyright 2000-2008 Sun Microsystems, Inc.  All Rights Reserved.
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
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 */


package com.sun.java.swing.action;

import javax.swing.KeyStroke;

// Referenced classes of package com.sun.java.swing.action:
//            DelegateAction, ActionManager

public class SaveAction extends DelegateAction
{

    public SaveAction()
    {
        this("general/Save16.gif");
    }

    public SaveAction(String iconPath)
    {
        super("Save", ActionManager.getIcon(iconPath));
        putValue("ActionCommandKey", "save-command");
        putValue("ShortDescription", "Commit changes to a permanent storage area");
        putValue("LongDescription", "Commit changes to a permanent storage area");
        putValue("MnemonicKey", VALUE_MNEMONIC);
        putValue("AcceleratorKey", VALUE_ACCELERATOR);
    }

    public static final String VALUE_COMMAND = "save-command";
    public static final String VALUE_NAME = "Save";
    public static final String VALUE_SMALL_ICON = "general/Save16.gif";
    public static final String VALUE_LARGE_ICON = "general/Save24.gif";
    public static final Integer VALUE_MNEMONIC = new Integer(83);
    public static final KeyStroke VALUE_ACCELERATOR = KeyStroke.getKeyStroke(83, 2);
    public static final String VALUE_SHORT_DESCRIPTION = "Commit changes to a permanent storage area";
    public static final String VALUE_LONG_DESCRIPTION = "Commit changes to a permanent storage area";

}
