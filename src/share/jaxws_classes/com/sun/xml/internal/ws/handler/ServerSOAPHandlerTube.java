/*
 * Copyright (c) 1997, 2010, Oracle and/or its affiliates. All rights reserved.
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

package com.sun.xml.internal.ws.handler;

import com.sun.xml.internal.ws.api.WSBinding;
import com.sun.xml.internal.ws.api.message.Packet;
import com.sun.xml.internal.ws.api.message.AttachmentSet;
import com.sun.xml.internal.ws.api.message.Attachment;
import com.sun.xml.internal.ws.api.pipe.TubeCloner;
import com.sun.xml.internal.ws.api.pipe.Tube;
import com.sun.xml.internal.ws.api.pipe.helper.AbstractFilterTubeImpl;
import com.sun.xml.internal.ws.api.model.wsdl.WSDLPort;
import com.sun.xml.internal.ws.client.HandlerConfiguration;
import com.sun.xml.internal.ws.binding.BindingImpl;
import com.sun.xml.internal.ws.message.DataHandlerAttachment;

import javax.xml.ws.handler.soap.SOAPHandler;
import javax.xml.ws.handler.MessageContext;
import javax.xml.ws.handler.Handler;
import javax.xml.ws.WebServiceException;
import javax.activation.DataHandler;
import java.util.*;

/**
 *
 * @author WS Development Team
 */
public class ServerSOAPHandlerTube extends HandlerTube {

    private Set<String> roles;

    /**
     * Creates a new instance of SOAPHandlerTube
     */
    public ServerSOAPHandlerTube(WSBinding binding, WSDLPort port, Tube next) {
        super(next, port, binding);
        if (binding.getSOAPVersion() != null) {
            // SOAPHandlerTube should n't be used for bindings other than SOAP.
            // TODO: throw Exception
        }
        setUpHandlersOnce();
    }

    // Handle to LogicalHandlerTube means its used on SERVER-SIDE

    /**
     * This constructor is used on client-side where, LogicalHandlerTube is created
     * first and then a SOAPHandlerTube is created with a handler to that
     * LogicalHandlerTube.
     * With this handle, SOAPHandlerTube can call LogicalHandlerTube.closeHandlers()
     */
    public ServerSOAPHandlerTube(WSBinding binding, Tube next, HandlerTube cousinTube) {
        super(next, cousinTube, binding);
        setUpHandlersOnce();
    }

    /**
     * Copy constructor for {@link com.sun.xml.internal.ws.api.pipe.Tube#copy(com.sun.xml.internal.ws.api.pipe.TubeCloner)}.
     */
    private ServerSOAPHandlerTube(ServerSOAPHandlerTube that, TubeCloner cloner) {
        super(that, cloner);
        this.handlers = that.handlers;
        this.roles = that.roles;
    }


    public AbstractFilterTubeImpl copy(TubeCloner cloner) {
        return new ServerSOAPHandlerTube(this, cloner);
    }

    private void setUpHandlersOnce() {
        handlers = new ArrayList<Handler>();
        HandlerConfiguration handlerConfig = ((BindingImpl) getBinding()).getHandlerConfig();
        List<SOAPHandler> soapSnapShot= handlerConfig.getSoapHandlers();
        if (!soapSnapShot.isEmpty()) {
            handlers.addAll(soapSnapShot);
            roles = new HashSet<String>();
            roles.addAll(handlerConfig.getRoles());
        }
    }

    protected void resetProcessor() {
        processor = null;
    }

    void setUpProcessor() {
        if(!handlers.isEmpty() && processor == null)
            processor = new SOAPHandlerProcessor(false, this, getBinding(), handlers);
    }
    MessageUpdatableContext getContext(Packet packet) {
        SOAPMessageContextImpl context = new SOAPMessageContextImpl(getBinding(), packet,roles);
        return context;
    }

    boolean callHandlersOnRequest(MessageUpdatableContext context, boolean isOneWay) {

        boolean handlerResult;
        try {
            //SERVER-SIDE
            handlerResult = processor.callHandlersRequest(HandlerProcessor.Direction.INBOUND, context, !isOneWay);

        } catch (RuntimeException re) {
            remedyActionTaken = true;
            throw re;

        }
        if (!handlerResult) {
            remedyActionTaken = true;
        }
        return handlerResult;
    }

    void callHandlersOnResponse(MessageUpdatableContext context, boolean handleFault) {

        //Lets copy all the MessageContext.OUTBOUND_ATTACHMENT_PROPERTY to the message
        Map<String, DataHandler> atts = (Map<String, DataHandler>) context.get(MessageContext.OUTBOUND_MESSAGE_ATTACHMENTS);
        AttachmentSet attSet = context.packet.getMessage().getAttachments();
        for(String cid : atts.keySet()){
            if (attSet.get(cid) == null) { // Otherwise we would be adding attachments twice
                Attachment att = new DataHandlerAttachment(cid, atts.get(cid));
                attSet.add(att);
            }
        }

        try {
            //SERVER-SIDE
            processor.callHandlersResponse(HandlerProcessor.Direction.OUTBOUND, context, handleFault);

        } catch (WebServiceException wse) {
            //no rewrapping
            throw wse;
        } catch (RuntimeException re) {
            throw re;

        }
    }

    void closeHandlers(MessageContext mc) {
        closeServersideHandlers(mc);

    }
}
