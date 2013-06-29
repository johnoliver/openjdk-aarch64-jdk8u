/*
 * reserved comment block
 * DO NOT REMOVE OR ALTER!
 */
/*
 * Copyright  1999-2004 The Apache Software Foundation.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
package com.sun.org.apache.xml.internal.security.utils;


import java.io.IOException;
import java.io.OutputStream;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.sun.org.apache.xml.internal.security.c14n.CanonicalizationException;
import com.sun.org.apache.xml.internal.security.c14n.Canonicalizer;
import com.sun.org.apache.xml.internal.security.c14n.InvalidCanonicalizerException;
import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;



/**
 * DOM and XML accessibility and comfort functions.
 *
 * @author Christian Geuer-Pollmann
 */
public class XMLUtils {

   private static boolean ignoreLineBreaks =
      AccessController.doPrivileged(new PrivilegedAction<Boolean>() {
         public Boolean run() {
            return Boolean.getBoolean
               ("com.sun.org.apache.xml.internal.security.ignoreLineBreaks");
         }
      });

    private static volatile String dsPrefix = "ds";
    private static volatile String xencPrefix = "xenc";

   private static final java.util.logging.Logger log =
       java.util.logging.Logger.getLogger(XMLUtils.class.getName());

   /**
    * Constructor XMLUtils
    *
    */
   private XMLUtils() {

      // we don't allow instantiation
   }

    /**
     * Set the prefix for the digital signature namespace
     * @param prefix the new prefix for the digital signature namespace
     */
    public static void setDsPrefix(String prefix) {
        dsPrefix = prefix;
    }

    /**
     * Set the prefix for the encryption namespace
     * @param prefix the new prefix for the encryption namespace
     */
    public static void setXencPrefix(String prefix) {
        xencPrefix = prefix;
    }

   public static Element getNextElement(Node el) {
           while ((el!=null) && (el.getNodeType()!=Node.ELEMENT_NODE)) {
                   el=el.getNextSibling();
           }
           return (Element)el;

   }

   /**
    * @param rootNode
    * @param result
    * @param exclude
    * @param com wheather comments or not
    */
   public static void getSet(Node rootNode,Set<Node> result,Node exclude ,boolean com) {
          if ((exclude!=null) && isDescendantOrSelf(exclude,rootNode)){
                return;
      }
      getSetRec(rootNode,result,exclude,com);
   }

   @SuppressWarnings("fallthrough")
   static final void getSetRec(final Node rootNode,final Set<Node> result,
        final Node exclude ,final boolean com) {
           //Set result = new HashSet();
       if (rootNode==exclude) {
          return;
       }
           switch (rootNode.getNodeType()) {
                case Node.ELEMENT_NODE:
                                result.add(rootNode);
                        Element el=(Element)rootNode;
                if (el.hasAttributes()) {
                                NamedNodeMap nl = ((Element)rootNode).getAttributes();
                                for (int i=0;i<nl.getLength();i++) {
                                        result.add(nl.item(i));
                                }
                }
                //no return keep working - ignore fallthrough warning
                case Node.DOCUMENT_NODE:
                                for (Node r=rootNode.getFirstChild();r!=null;r=r.getNextSibling()){
                                        if (r.getNodeType()==Node.TEXT_NODE) {
                                                result.add(r);
                                                while ((r!=null) && (r.getNodeType()==Node.TEXT_NODE)) {
                                                        r=r.getNextSibling();
                                                }
                                                if (r==null)
                                                        return;
                                        }
                                        getSetRec(r,result,exclude,com);
                                }
                                return;
                        case Node.COMMENT_NODE:
                                if (com) {
                                        result.add(rootNode);
                                }
                            return;
                        case Node.DOCUMENT_TYPE_NODE:
                                return;
                        default:
                                result.add(rootNode);
           }
           return;
   }


   /**
    * Outputs a DOM tree to an {@link OutputStream}.
    *
    * @param contextNode root node of the DOM tree
    * @param os the {@link OutputStream}
    */
   public static void outputDOM(Node contextNode, OutputStream os) {
      XMLUtils.outputDOM(contextNode, os, false);
   }

   /**
    * Outputs a DOM tree to an {@link OutputStream}. <I>If an Exception is
    * thrown during execution, it's StackTrace is output to System.out, but the
    * Exception is not re-thrown.</I>
    *
    * @param contextNode root node of the DOM tree
    * @param os the {@link OutputStream}
    * @param addPreamble
    */
   public static void outputDOM(Node contextNode, OutputStream os,
                                boolean addPreamble) {

      try {
         if (addPreamble) {
            os.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n".getBytes());
         }

         os.write(
            Canonicalizer.getInstance(
               Canonicalizer.ALGO_ID_C14N_WITH_COMMENTS).canonicalizeSubtree(
               contextNode));
      } catch (IOException ex) {}
      catch (InvalidCanonicalizerException ex) {
         ex.printStackTrace();
      } catch (CanonicalizationException ex) {
         ex.printStackTrace();
      }
   }

   /**
    * Serializes the <CODE>contextNode</CODE> into the OutputStream, <I>but
    * supresses all Exceptions</I>.
    * <BR />
    * NOTE: <I>This should only be used for debugging purposes,
    * NOT in a production environment; this method ignores all exceptions,
    * so you won't notice if something goes wrong. If you're asking what is to
    * be used in a production environment, simply use the code inside the
    * <code>try{}</code> statement, but handle the Exceptions appropriately.</I>
    *
    * @param contextNode
    * @param os
    */
   public static void outputDOMc14nWithComments(Node contextNode,
           OutputStream os) {

      try {
         os.write(
            Canonicalizer.getInstance(
               Canonicalizer.ALGO_ID_C14N_WITH_COMMENTS).canonicalizeSubtree(
               contextNode));
      } catch (IOException ex) {

         // throw new RuntimeException(ex.getMessage());
      } catch (InvalidCanonicalizerException ex) {

         // throw new RuntimeException(ex.getMessage());
      } catch (CanonicalizationException ex) {

         // throw new RuntimeException(ex.getMessage());
      }
   }


   /**
    * Method getFullTextChildrenFromElement
    *
    * @param element
    * @return the string of chi;ds
    */
   public static String getFullTextChildrenFromElement(Element element) {

      StringBuffer sb = new StringBuffer();
      NodeList children = element.getChildNodes();
      int iMax = children.getLength();

      for (int i = 0; i < iMax; i++) {
         Node curr = children.item(i);

         if (curr.getNodeType() == Node.TEXT_NODE) {
            sb.append(((Text) curr).getData());
         }
      }

      return sb.toString();
   }

   static Map<String, String> namePrefixes=new HashMap<String, String>();

   /**
    * Creates an Element in the XML Signature specification namespace.
    *
    * @param doc the factory Document
    * @param elementName the local name of the Element
    * @return the Element
    */
   public static Element createElementInSignatureSpace(Document doc,
           String elementName) {

      if (doc == null) {
         throw new RuntimeException("Document is null");
      }

      if ((dsPrefix == null) || (dsPrefix.length() == 0)) {
         return doc.createElementNS(Constants.SignatureSpecNS, elementName);
      }
      String namePrefix= namePrefixes.get(elementName);
      if (namePrefix==null) {
          StringBuffer tag=new StringBuffer(dsPrefix);
          tag.append(':');
          tag.append(elementName);
          namePrefix=tag.toString();
          namePrefixes.put(elementName,namePrefix);
      }
      return doc.createElementNS(Constants.SignatureSpecNS, namePrefix);
   }

    /**
     * Returns true if the element is in XML Signature namespace and the local
     * name equals the supplied one.
     *
     * @param element
     * @param localName
     * @return true if the element is in XML Signature namespace and the local name equals the supplied one
     */
    public static boolean elementIsInSignatureSpace(Element element, String localName) {
        if (element == null) {
            return false;
        }

        return Constants.SignatureSpecNS.equals(element.getNamespaceURI())
            && element.getLocalName().equals(localName);
    }

    /**
     * Returns true if the element is in XML Encryption namespace and the local
     * name equals the supplied one.
     *
     * @param element
     * @param localName
     * @return true if the element is in XML Encryption namespace and the local name equals the supplied one
     */
    public static boolean elementIsInEncryptionSpace(Element element, String localName) {
        if (element == null) {
            return false;
        }
        return EncryptionConstants.EncryptionSpecNS.equals(element.getNamespaceURI())
            && element.getLocalName().equals(localName);
    }

   /**
    * This method returns the owner document of a particular node.
    * This method is necessary because it <I>always</I> returns a
    * {@link Document}. {@link Node#getOwnerDocument} returns <CODE>null</CODE>
    * if the {@link Node} is a {@link Document}.
    *
    * @param node
    * @return the owner document of the node
    */
   public static Document getOwnerDocument(Node node) {

      if (node.getNodeType() == Node.DOCUMENT_NODE) {
         return (Document) node;
      }
         try {
            return node.getOwnerDocument();
         } catch (NullPointerException npe) {
            throw new NullPointerException(I18n.translate("endorsed.jdk1.4.0")
                                           + " Original message was \""
                                           + npe.getMessage() + "\"");
         }

   }

    /**
     * This method returns the first non-null owner document of the Node's in this Set.
     * This method is necessary because it <I>always</I> returns a
     * {@link Document}. {@link Node#getOwnerDocument} returns <CODE>null</CODE>
     * if the {@link Node} is a {@link Document}.
     *
     * @param xpathNodeSet
     * @return the owner document
     */
    public static Document getOwnerDocument(Set<Node> xpathNodeSet) {
       NullPointerException npe = null;
       for (Node node : xpathNodeSet) {
           int nodeType =node.getNodeType();
           if (nodeType == Node.DOCUMENT_NODE) {
              return (Document) node;
           }
              try {
                 if (nodeType==Node.ATTRIBUTE_NODE) {
                    return ((Attr)node).getOwnerElement().getOwnerDocument();
                 }
                 return node.getOwnerDocument();
              } catch (NullPointerException e) {
                  npe = e;
              }

       }
       throw new NullPointerException(I18n.translate("endorsed.jdk1.4.0")
                                       + " Original message was \""
                                       + (npe == null ? "" : npe.getMessage()) + "\"");
    }

    /**
     * Method createDSctx
     *
     * @param doc
     * @param prefix
     * @param namespace
     * @return the element.
     */
    public static Element createDSctx(Document doc, String prefix,
                                      String namespace) {

       if ((prefix == null) || (prefix.trim().length() == 0)) {
          throw new IllegalArgumentException("You must supply a prefix");
       }

       Element ctx = doc.createElementNS(null, "namespaceContext");

       ctx.setAttributeNS(Constants.NamespaceSpecNS, "xmlns:" + prefix.trim(),
                          namespace);

       return ctx;
    }

   /**
    * Method addReturnToElement
    *
    * @param e
    */
   public static void addReturnToElement(Element e) {

      if (!ignoreLineBreaks) {
         Document doc = e.getOwnerDocument();
         e.appendChild(doc.createTextNode("\n"));
      }
   }

   public static void addReturnToElement(Document doc, HelperNodeList nl) {
      if (!ignoreLineBreaks) {
         nl.appendChild(doc.createTextNode("\n"));
      }
   }

   public static void addReturnBeforeChild(Element e, Node child) {
      if (!ignoreLineBreaks) {
         Document doc = e.getOwnerDocument();
         e.insertBefore(doc.createTextNode("\n"), child);
      }
   }

   /**
    * Method convertNodelistToSet
    *
    * @param xpathNodeSet
    * @return the set with the nodelist
    */
   public static Set<Node> convertNodelistToSet(NodeList xpathNodeSet) {

      if (xpathNodeSet == null) {
         return new HashSet<Node>();
      }

      int length = xpathNodeSet.getLength();
      Set<Node> set = new HashSet<Node>(length);

      for (int i = 0; i < length; i++) {
         set.add(xpathNodeSet.item(i));
      }

      return set;
   }


   /**
    * This method spreads all namespace attributes in a DOM document to their
    * children. This is needed because the XML Signature XPath transform
    * must evaluate the XPath against all nodes in the input, even against
    * XPath namespace nodes. Through a bug in XalanJ2, the namespace nodes are
    * not fully visible in the Xalan XPath model, so we have to do this by
    * hand in DOM spaces so that the nodes become visible in XPath space.
    *
    * @param doc
    * @see <A HREF="http://nagoya.apache.org/bugzilla/show_bug.cgi?id=2650">Namespace axis resolution is not XPath compliant </A>
    */
   public static void circumventBug2650(Document doc) {

      Element documentElement = doc.getDocumentElement();

      // if the document element has no xmlns definition, we add xmlns=""
      Attr xmlnsAttr =
         documentElement.getAttributeNodeNS(Constants.NamespaceSpecNS, "xmlns");

      if (xmlnsAttr == null) {
         documentElement.setAttributeNS(Constants.NamespaceSpecNS, "xmlns", "");
      }

      XMLUtils.circumventBug2650internal(doc);
   }

   /**
    * This is the work horse for {@link #circumventBug2650}.
    *
    * @param node
    * @see <A HREF="http://nagoya.apache.org/bugzilla/show_bug.cgi?id=2650">Namespace axis resolution is not XPath compliant </A>
    */
   @SuppressWarnings("fallthrough")
   private static void circumventBug2650internal(Node node) {
           Node parent=null;
           Node sibling=null;
           final String namespaceNs=Constants.NamespaceSpecNS;
           do {
         switch (node.getNodeType()) {
         case Node.ELEMENT_NODE :
                 Element element = (Element) node;
             if (!element.hasChildNodes())
                 break;
             if (element.hasAttributes()) {
             NamedNodeMap attributes = element.getAttributes();
             int attributesLength = attributes.getLength();

             for (Node child = element.getFirstChild(); child!=null;
                child=child.getNextSibling()) {

                if (child.getNodeType() != Node.ELEMENT_NODE) {
                        continue;
                }
                Element childElement = (Element) child;

                for (int i = 0; i < attributesLength; i++) {
                        Attr currentAttr = (Attr) attributes.item(i);
                        if (namespaceNs!=currentAttr.getNamespaceURI())
                                continue;
                        if (childElement.hasAttributeNS(namespaceNs,
                                                        currentAttr.getLocalName())) {
                                        continue;
                        }
                        childElement.setAttributeNS(namespaceNs,
                                                currentAttr.getName(),
                                                currentAttr.getNodeValue());


                }
             }
             }
         case Node.ENTITY_REFERENCE_NODE :
         case Node.DOCUMENT_NODE :
                 parent=node;
                 sibling=node.getFirstChild();
             break;
         }
         while ((sibling==null) && (parent!=null)) {
                         sibling=parent.getNextSibling();
                         parent=parent.getParentNode();
                 };
       if (sibling==null) {
                         return;
                 }

         node=sibling;
         sibling=node.getNextSibling();
           } while (true);
   }

    /**
     * @param sibling
     * @param nodeName
     * @param number
     * @return nodes with the constrain
     */
    public static Element selectDsNode(Node sibling, String nodeName, int number) {
        while (sibling != null) {
            if (Constants.SignatureSpecNS.equals(sibling.getNamespaceURI())
                && sibling.getLocalName().equals(nodeName)) {
                if (number == 0){
                    return (Element)sibling;
                }
                number--;
            }
            sibling = sibling.getNextSibling();
        }
        return null;
    }

    /**
     * @param sibling
     * @param nodeName
     * @param number
     * @return nodes with the constrain
     */
    public static Element selectXencNode(Node sibling, String nodeName, int number) {
        while (sibling != null) {
            if (EncryptionConstants.EncryptionSpecNS.equals(sibling.getNamespaceURI())
                && sibling.getLocalName().equals(nodeName)) {
                if (number == 0){
                    return (Element)sibling;
                }
                number--;
            }
            sibling = sibling.getNextSibling();
        }
        return null;
    }

   /**
    * @param sibling
    * @param nodeName
    * @param number
    * @return nodes with the constrain
    */
   public static Text selectDsNodeText(Node sibling, String nodeName, int number) {
            Node n=selectDsNode(sibling,nodeName,number);
        if (n==null) {
                return null;
        }
        n=n.getFirstChild();
        while (n!=null && n.getNodeType()!=Node.TEXT_NODE) {
                n=n.getNextSibling();
        }
        return (Text)n;
   }

   /**
    * @param sibling
    * @param uri
    * @param nodeName
    * @param number
    * @return nodes with the constrain
    */
   public static Text selectNodeText(Node sibling, String uri, String nodeName, int number) {
        Node n=selectNode(sibling,uri,nodeName,number);
    if (n==null) {
        return null;
    }
    n=n.getFirstChild();
    while (n!=null && n.getNodeType()!=Node.TEXT_NODE) {
        n=n.getNextSibling();
    }
    return (Text)n;
   }

    /**
     * @param sibling
     * @param uri
     * @param nodeName
     * @param number
     * @return nodes with the constrain
     */
    public static Element selectNode(Node sibling, String uri, String nodeName, int number) {
        while (sibling != null) {
            if (sibling.getNamespaceURI() != null && sibling.getNamespaceURI().equals(uri)
                && sibling.getLocalName().equals(nodeName)) {
                if (number == 0){
                    return (Element)sibling;
                }
                number--;
            }
            sibling = sibling.getNextSibling();
        }
        return null;
    }

    /**
     * @param sibling
     * @param nodeName
     * @return nodes with the constrain
     */
    public static Element[] selectDsNodes(Node sibling, String nodeName) {
        return selectNodes(sibling,Constants.SignatureSpecNS, nodeName);
    }

    /**
     * @param sibling
     * @param uri
     * @param nodeName
     * @return nodes with the constrain
     */
     public static Element[] selectNodes(Node sibling, String uri, String nodeName) {
        List<Element> list = new ArrayList<Element>();
        while (sibling != null) {
            if (sibling.getNamespaceURI() != null && sibling.getNamespaceURI().equals(uri)
                && sibling.getLocalName().equals(nodeName)) {
                list.add((Element)sibling);
            }
            sibling = sibling.getNextSibling();
        }
        return list.toArray(new Element[list.size()]);
    }

   /**
    * @param signatureElement
    * @param inputSet
    * @return nodes with the constrain
    */
    public static Set<Node> excludeNodeFromSet(Node signatureElement, Set<Node> inputSet) {
          Set<Node> resultSet = new HashSet<Node>();
          Iterator<Node> iterator = inputSet.iterator();

          while (iterator.hasNext()) {
            Node inputNode = iterator.next();

            if (!XMLUtils
                    .isDescendantOrSelf(signatureElement, inputNode)) {
               resultSet.add(inputNode);
            }
         }
         return resultSet;
     }

   /**
    * Returns true if the descendantOrSelf is on the descendant-or-self axis
    * of the context node.
    *
    * @param ctx
    * @param descendantOrSelf
    * @return true if the node is descendant
    */
   static public boolean isDescendantOrSelf(Node ctx, Node descendantOrSelf) {

      if (ctx == descendantOrSelf) {
         return true;
      }

      Node parent = descendantOrSelf;

      while (true) {
         if (parent == null) {
            return false;
         }

         if (parent == ctx) {
            return true;
         }

         if (parent.getNodeType() == Node.ATTRIBUTE_NODE) {
            parent = ((Attr) parent).getOwnerElement();
         } else {
            parent = parent.getParentNode();
         }
      }
   }

    public static boolean ignoreLineBreaks() {
        return ignoreLineBreaks;
    }

    /**
     * This method is a tree-search to help prevent against wrapping attacks.
     * It checks that no two Elements have ID Attributes that match the "value"
     * argument, if this is the case then "false" is returned. Note that a
     * return value of "true" does not necessarily mean that a matching Element
     * has been found, just that no wrapping attack has been detected.
     */
    public static boolean protectAgainstWrappingAttack(Node startNode,
                                                       String value)
    {
        Node startParent = startNode.getParentNode();
        Node processedNode = null;
        Element foundElement = null;

        String id = value.trim();
        if (id.charAt(0) == '#') {
            id = id.substring(1);
        }

        while (startNode != null) {
            if (startNode.getNodeType() == Node.ELEMENT_NODE) {
                Element se = (Element) startNode;

                NamedNodeMap attributes = se.getAttributes();
                if (attributes != null) {
                    for (int i = 0; i < attributes.getLength(); i++) {
                        Attr attr = (Attr)attributes.item(i);
                        if (attr.isId() && id.equals(attr.getValue())) {
                            if (foundElement == null) {
                                // Continue searching to find duplicates
                                foundElement = attr.getOwnerElement();
                            } else {
                                log.log(java.util.logging.Level.FINE, "Multiple elements with the same 'Id' attribute value!");
                                return false;
                            }
                        }
                    }
                }
            }

            processedNode = startNode;
            startNode = startNode.getFirstChild();

            // no child, this node is done.
            if (startNode == null) {
                // close node processing, get sibling
                startNode = processedNode.getNextSibling();
            }

            // no more siblings, get parent, all children
            // of parent are processed.
            while (startNode == null) {
                processedNode = processedNode.getParentNode();
                if (processedNode == startParent) {
                    return true;
                }
                // close parent node processing (processed node now)
                startNode = processedNode.getNextSibling();
            }
        }
        return true;
    }

    /**
     * This method is a tree-search to help prevent against wrapping attacks.
     * It checks that no other Element than the given "knownElement" argument
     * has an ID attribute that matches the "value" argument, which is the ID
     * value of "knownElement". If this is the case then "false" is returned.
     */
    public static boolean protectAgainstWrappingAttack(Node startNode,
                                                       Element knownElement,
                                                       String value)
    {
        Node startParent = startNode.getParentNode();
        Node processedNode = null;

        String id = value.trim();
        if (id.charAt(0) == '#') {
            id = id.substring(1);
        }

        while (startNode != null) {
            if (startNode.getNodeType() == Node.ELEMENT_NODE) {
                Element se = (Element) startNode;

                NamedNodeMap attributes = se.getAttributes();
                if (attributes != null) {
                    for (int i = 0; i < attributes.getLength(); i++) {
                        Attr attr = (Attr)attributes.item(i);
                        if (attr.isId() && id.equals(attr.getValue())
                            && se != knownElement)
                        {
                            log.log(java.util.logging.Level.FINE, "Multiple elements with the same 'Id' attribute value!");
                            return false;
                        }
                    }
                }
            }

            processedNode = startNode;
            startNode = startNode.getFirstChild();

            // no child, this node is done.
            if (startNode == null) {
                // close node processing, get sibling
                startNode = processedNode.getNextSibling();
            }

            // no more siblings, get parent, all children
            // of parent are processed.
            while (startNode == null) {
                processedNode = processedNode.getParentNode();
                if (processedNode == startParent) {
                    return true;
                }
                // close parent node processing (processed node now)
                startNode = processedNode.getNextSibling();
            }
        }
        return true;
    }

}
