/*
 * Copyright (c) 1998, 2010, Oracle and/or its affiliates. All rights reserved.
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

package javax.security.auth;

import java.util.*;
import java.io.*;
import java.lang.reflect.*;
import java.text.MessageFormat;
import java.security.AccessController;
import java.security.AccessControlContext;
import java.security.DomainCombiner;
import java.security.Permission;
import java.security.PermissionCollection;
import java.security.Principal;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.ProtectionDomain;
import sun.security.util.ResourcesMgr;

/**
 * <p> A <code>Subject</code> represents a grouping of related information
 * for a single entity, such as a person.
 * Such information includes the Subject's identities as well as
 * its security-related attributes
 * (passwords and cryptographic keys, for example).
 *
 * <p> Subjects may potentially have multiple identities.
 * Each identity is represented as a <code>Principal</code>
 * within the <code>Subject</code>.  Principals simply bind names to a
 * <code>Subject</code>.  For example, a <code>Subject</code> that happens
 * to be a person, Alice, might have two Principals:
 * one which binds "Alice Bar", the name on her driver license,
 * to the <code>Subject</code>, and another which binds,
 * "999-99-9999", the number on her student identification card,
 * to the <code>Subject</code>.  Both Principals refer to the same
 * <code>Subject</code> even though each has a different name.
 *
 * <p> A <code>Subject</code> may also own security-related attributes,
 * which are referred to as credentials.
 * Sensitive credentials that require special protection, such as
 * private cryptographic keys, are stored within a private credential
 * <code>Set</code>.  Credentials intended to be shared, such as
 * public key certificates or Kerberos server tickets are stored
 * within a public credential <code>Set</code>.  Different permissions
 * are required to access and modify the different credential Sets.
 *
 * <p> To retrieve all the Principals associated with a <code>Subject</code>,
 * invoke the <code>getPrincipals</code> method.  To retrieve
 * all the public or private credentials belonging to a <code>Subject</code>,
 * invoke the <code>getPublicCredentials</code> method or
 * <code>getPrivateCredentials</code> method, respectively.
 * To modify the returned <code>Set</code> of Principals and credentials,
 * use the methods defined in the <code>Set</code> class.
 * For example:
 * <pre>
 *      Subject subject;
 *      Principal principal;
 *      Object credential;
 *
 *      // add a Principal and credential to the Subject
 *      subject.getPrincipals().add(principal);
 *      subject.getPublicCredentials().add(credential);
 * </pre>
 *
 * <p> This <code>Subject</code> class implements <code>Serializable</code>.
 * While the Principals associated with the <code>Subject</code> are serialized,
 * the credentials associated with the <code>Subject</code> are not.
 * Note that the <code>java.security.Principal</code> class
 * does not implement <code>Serializable</code>.  Therefore all concrete
 * <code>Principal</code> implementations associated with Subjects
 * must implement <code>Serializable</code>.
 *
 * @see java.security.Principal
 * @see java.security.DomainCombiner
 */
public final class Subject implements java.io.Serializable {

    private static final long serialVersionUID = -8308522755600156056L;

    /**
     * A <code>Set</code> that provides a view of all of this
     * Subject's Principals
     *
     * <p>
     *
     * @serial Each element in this set is a
     *          <code>java.security.Principal</code>.
     *          The set is a <code>Subject.SecureSet</code>.
     */
    Set<Principal> principals;

    /**
     * Sets that provide a view of all of this
     * Subject's Credentials
     */
    transient Set<Object> pubCredentials;
    transient Set<Object> privCredentials;

    /**
     * Whether this Subject is read-only
     *
     * @serial
     */
    private volatile boolean readOnly = false;

    private static final int PRINCIPAL_SET = 1;
    private static final int PUB_CREDENTIAL_SET = 2;
    private static final int PRIV_CREDENTIAL_SET = 3;

    private static final ProtectionDomain[] NULL_PD_ARRAY
        = new ProtectionDomain[0];

    /**
     * Create an instance of a <code>Subject</code>
     * with an empty <code>Set</code> of Principals and empty
     * Sets of public and private credentials.
     *
     * <p> The newly constructed Sets check whether this <code>Subject</code>
     * has been set read-only before permitting subsequent modifications.
     * The newly created Sets also prevent illegal modifications
     * by ensuring that callers have sufficient permissions.
     *
     * <p> To modify the Principals Set, the caller must have
     * <code>AuthPermission("modifyPrincipals")</code>.
     * To modify the public credential Set, the caller must have
     * <code>AuthPermission("modifyPublicCredentials")</code>.
     * To modify the private credential Set, the caller must have
     * <code>AuthPermission("modifyPrivateCredentials")</code>.
     */
    public Subject() {

        this.principals = Collections.synchronizedSet
                        (new SecureSet<Principal>(this, PRINCIPAL_SET));
        this.pubCredentials = Collections.synchronizedSet
                        (new SecureSet<Object>(this, PUB_CREDENTIAL_SET));
        this.privCredentials = Collections.synchronizedSet
                        (new SecureSet<Object>(this, PRIV_CREDENTIAL_SET));
    }

    /**
     * Create an instance of a <code>Subject</code> with
     * Principals and credentials.
     *
     * <p> The Principals and credentials from the specified Sets
     * are copied into newly constructed Sets.
     * These newly created Sets check whether this <code>Subject</code>
     * has been set read-only before permitting subsequent modifications.
     * The newly created Sets also prevent illegal modifications
     * by ensuring that callers have sufficient permissions.
     *
     * <p> To modify the Principals Set, the caller must have
     * <code>AuthPermission("modifyPrincipals")</code>.
     * To modify the public credential Set, the caller must have
     * <code>AuthPermission("modifyPublicCredentials")</code>.
     * To modify the private credential Set, the caller must have
     * <code>AuthPermission("modifyPrivateCredentials")</code>.
     * <p>
     *
     * @param readOnly true if the <code>Subject</code> is to be read-only,
     *          and false otherwise. <p>
     *
     * @param principals the <code>Set</code> of Principals
     *          to be associated with this <code>Subject</code>. <p>
     *
     * @param pubCredentials the <code>Set</code> of public credentials
     *          to be associated with this <code>Subject</code>. <p>
     *
     * @param privCredentials the <code>Set</code> of private credentials
     *          to be associated with this <code>Subject</code>.
     *
     * @exception NullPointerException if the specified
     *          <code>principals</code>, <code>pubCredentials</code>,
     *          or <code>privCredentials</code> are <code>null</code>.
     */
    public Subject(boolean readOnly, Set<? extends Principal> principals,
                   Set<?> pubCredentials, Set<?> privCredentials)
    {

        if (principals == null ||
            pubCredentials == null ||
            privCredentials == null)
            throw new NullPointerException
                (ResourcesMgr.getString("invalid.null.input.s."));

        this.principals = Collections.synchronizedSet(new SecureSet<Principal>
                                (this, PRINCIPAL_SET, principals));
        this.pubCredentials = Collections.synchronizedSet(new SecureSet<Object>
                                (this, PUB_CREDENTIAL_SET, pubCredentials));
        this.privCredentials = Collections.synchronizedSet(new SecureSet<Object>
                                (this, PRIV_CREDENTIAL_SET, privCredentials));
        this.readOnly = readOnly;
    }

    /**
     * Set this <code>Subject</code> to be read-only.
     *
     * <p> Modifications (additions and removals) to this Subject's
     * <code>Principal</code> <code>Set</code> and
     * credential Sets will be disallowed.
     * The <code>destroy</code> operation on this Subject's credentials will
     * still be permitted.
     *
     * <p> Subsequent attempts to modify the Subject's <code>Principal</code>
     * and credential Sets will result in an
     * <code>IllegalStateException</code> being thrown.
     * Also, once a <code>Subject</code> is read-only,
     * it can not be reset to being writable again.
     *
     * <p>
     *
     * @exception SecurityException if the caller does not have permission
     *          to set this <code>Subject</code> to be read-only.
     */
    public void setReadOnly() {
        java.lang.SecurityManager sm = System.getSecurityManager();
        if (sm != null) {
            sm.checkPermission(AuthPermissionHolder.SET_READ_ONLY_PERMISSION);
        }

        this.readOnly = true;
    }

    /**
     * Query whether this <code>Subject</code> is read-only.
     *
     * <p>
     *
     * @return true if this <code>Subject</code> is read-only, false otherwise.
     */
    public boolean isReadOnly() {
        return this.readOnly;
    }

    /**
     * Get the <code>Subject</code> associated with the provided
     * <code>AccessControlContext</code>.
     *
     * <p> The <code>AccessControlContext</code> may contain many
     * Subjects (from nested <code>doAs</code> calls).
     * In this situation, the most recent <code>Subject</code> associated
     * with the <code>AccessControlContext</code> is returned.
     *
     * <p>
     *
     * @param  acc the <code>AccessControlContext</code> from which to retrieve
     *          the <code>Subject</code>.
     *
     * @return  the <code>Subject</code> associated with the provided
     *          <code>AccessControlContext</code>, or <code>null</code>
     *          if no <code>Subject</code> is associated
     *          with the provided <code>AccessControlContext</code>.
     *
     * @exception SecurityException if the caller does not have permission
     *          to get the <code>Subject</code>. <p>
     *
     * @exception NullPointerException if the provided
     *          <code>AccessControlContext</code> is <code>null</code>.
     */
    public static Subject getSubject(final AccessControlContext acc) {

        java.lang.SecurityManager sm = System.getSecurityManager();
        if (sm != null) {
            sm.checkPermission(AuthPermissionHolder.GET_SUBJECT_PERMISSION);
        }

        if (acc == null) {
            throw new NullPointerException(ResourcesMgr.getString
                ("invalid.null.AccessControlContext.provided"));
        }

        // return the Subject from the DomainCombiner of the provided context
        return AccessController.doPrivileged
            (new java.security.PrivilegedAction<Subject>() {
            public Subject run() {
                DomainCombiner dc = acc.getDomainCombiner();
                if (!(dc instanceof SubjectDomainCombiner))
                    return null;
                SubjectDomainCombiner sdc = (SubjectDomainCombiner)dc;
                return sdc.getSubject();
            }
        });
    }

    /**
     * Perform work as a particular <code>Subject</code>.
     *
     * <p> This method first retrieves the current Thread's
     * <code>AccessControlContext</code> via
     * <code>AccessController.getContext</code>,
     * and then instantiates a new <code>AccessControlContext</code>
     * using the retrieved context along with a new
     * <code>SubjectDomainCombiner</code> (constructed using
     * the provided <code>Subject</code>).
     * Finally, this method invokes <code>AccessController.doPrivileged</code>,
     * passing it the provided <code>PrivilegedAction</code>,
     * as well as the newly constructed <code>AccessControlContext</code>.
     *
     * <p>
     *
     * @param subject the <code>Subject</code> that the specified
     *                  <code>action</code> will run as.  This parameter
     *                  may be <code>null</code>. <p>
     *
     * @param action the code to be run as the specified
     *                  <code>Subject</code>. <p>
     *
     * @return the value returned by the PrivilegedAction's
     *                  <code>run</code> method.
     *
     * @exception NullPointerException if the <code>PrivilegedAction</code>
     *                  is <code>null</code>. <p>
     *
     * @exception SecurityException if the caller does not have permission
     *                  to invoke this method.
     */
    public static <T> T doAs(final Subject subject,
                        final java.security.PrivilegedAction<T> action) {

        java.lang.SecurityManager sm = System.getSecurityManager();
        if (sm != null) {
            sm.checkPermission(AuthPermissionHolder.DO_AS_PERMISSION);
        }
        if (action == null)
            throw new NullPointerException
                (ResourcesMgr.getString("invalid.null.action.provided"));

        // set up the new Subject-based AccessControlContext
        // for doPrivileged
        final AccessControlContext currentAcc = AccessController.getContext();

        // call doPrivileged and push this new context on the stack
        return java.security.AccessController.doPrivileged
                                        (action,
                                        createContext(subject, currentAcc));
    }

    /**
     * Perform work as a particular <code>Subject</code>.
     *
     * <p> This method first retrieves the current Thread's
     * <code>AccessControlContext</code> via
     * <code>AccessController.getContext</code>,
     * and then instantiates a new <code>AccessControlContext</code>
     * using the retrieved context along with a new
     * <code>SubjectDomainCombiner</code> (constructed using
     * the provided <code>Subject</code>).
     * Finally, this method invokes <code>AccessController.doPrivileged</code>,
     * passing it the provided <code>PrivilegedExceptionAction</code>,
     * as well as the newly constructed <code>AccessControlContext</code>.
     *
     * <p>
     *
     * @param subject the <code>Subject</code> that the specified
     *                  <code>action</code> will run as.  This parameter
     *                  may be <code>null</code>. <p>
     *
     * @param action the code to be run as the specified
     *                  <code>Subject</code>. <p>
     *
     * @return the value returned by the
     *                  PrivilegedExceptionAction's <code>run</code> method.
     *
     * @exception PrivilegedActionException if the
     *                  <code>PrivilegedExceptionAction.run</code>
     *                  method throws a checked exception. <p>
     *
     * @exception NullPointerException if the specified
     *                  <code>PrivilegedExceptionAction</code> is
     *                  <code>null</code>. <p>
     *
     * @exception SecurityException if the caller does not have permission
     *                  to invoke this method.
     */
    public static <T> T doAs(final Subject subject,
                        final java.security.PrivilegedExceptionAction<T> action)
                        throws java.security.PrivilegedActionException {

        java.lang.SecurityManager sm = System.getSecurityManager();
        if (sm != null) {
            sm.checkPermission(AuthPermissionHolder.DO_AS_PERMISSION);
        }

        if (action == null)
            throw new NullPointerException
                (ResourcesMgr.getString("invalid.null.action.provided"));

        // set up the new Subject-based AccessControlContext for doPrivileged
        final AccessControlContext currentAcc = AccessController.getContext();

        // call doPrivileged and push this new context on the stack
        return java.security.AccessController.doPrivileged
                                        (action,
                                        createContext(subject, currentAcc));
    }

    /**
     * Perform privileged work as a particular <code>Subject</code>.
     *
     * <p> This method behaves exactly as <code>Subject.doAs</code>,
     * except that instead of retrieving the current Thread's
     * <code>AccessControlContext</code>, it uses the provided
     * <code>AccessControlContext</code>.  If the provided
     * <code>AccessControlContext</code> is <code>null</code>,
     * this method instantiates a new <code>AccessControlContext</code>
     * with an empty collection of ProtectionDomains.
     *
     * <p>
     *
     * @param subject the <code>Subject</code> that the specified
     *                  <code>action</code> will run as.  This parameter
     *                  may be <code>null</code>. <p>
     *
     * @param action the code to be run as the specified
     *                  <code>Subject</code>. <p>
     *
     * @param acc the <code>AccessControlContext</code> to be tied to the
     *                  specified <i>subject</i> and <i>action</i>. <p>
     *
     * @return the value returned by the PrivilegedAction's
     *                  <code>run</code> method.
     *
     * @exception NullPointerException if the <code>PrivilegedAction</code>
     *                  is <code>null</code>. <p>
     *
     * @exception SecurityException if the caller does not have permission
     *                  to invoke this method.
     */
    public static <T> T doAsPrivileged(final Subject subject,
                        final java.security.PrivilegedAction<T> action,
                        final java.security.AccessControlContext acc) {

        java.lang.SecurityManager sm = System.getSecurityManager();
        if (sm != null) {
            sm.checkPermission(AuthPermissionHolder.DO_AS_PRIVILEGED_PERMISSION);
        }

        if (action == null)
            throw new NullPointerException
                (ResourcesMgr.getString("invalid.null.action.provided"));

        // set up the new Subject-based AccessControlContext
        // for doPrivileged
        final AccessControlContext callerAcc =
                (acc == null ?
                new AccessControlContext(NULL_PD_ARRAY) :
                acc);

        // call doPrivileged and push this new context on the stack
        return java.security.AccessController.doPrivileged
                                        (action,
                                        createContext(subject, callerAcc));
    }

    /**
     * Perform privileged work as a particular <code>Subject</code>.
     *
     * <p> This method behaves exactly as <code>Subject.doAs</code>,
     * except that instead of retrieving the current Thread's
     * <code>AccessControlContext</code>, it uses the provided
     * <code>AccessControlContext</code>.  If the provided
     * <code>AccessControlContext</code> is <code>null</code>,
     * this method instantiates a new <code>AccessControlContext</code>
     * with an empty collection of ProtectionDomains.
     *
     * <p>
     *
     * @param subject the <code>Subject</code> that the specified
     *                  <code>action</code> will run as.  This parameter
     *                  may be <code>null</code>. <p>
     *
     * @param action the code to be run as the specified
     *                  <code>Subject</code>. <p>
     *
     * @param acc the <code>AccessControlContext</code> to be tied to the
     *                  specified <i>subject</i> and <i>action</i>. <p>
     *
     * @return the value returned by the
     *                  PrivilegedExceptionAction's <code>run</code> method.
     *
     * @exception PrivilegedActionException if the
     *                  <code>PrivilegedExceptionAction.run</code>
     *                  method throws a checked exception. <p>
     *
     * @exception NullPointerException if the specified
     *                  <code>PrivilegedExceptionAction</code> is
     *                  <code>null</code>. <p>
     *
     * @exception SecurityException if the caller does not have permission
     *                  to invoke this method.
     */
    public static <T> T doAsPrivileged(final Subject subject,
                        final java.security.PrivilegedExceptionAction<T> action,
                        final java.security.AccessControlContext acc)
                        throws java.security.PrivilegedActionException {

        java.lang.SecurityManager sm = System.getSecurityManager();
        if (sm != null) {
            sm.checkPermission(AuthPermissionHolder.DO_AS_PRIVILEGED_PERMISSION);
        }

        if (action == null)
            throw new NullPointerException
                (ResourcesMgr.getString("invalid.null.action.provided"));

        // set up the new Subject-based AccessControlContext for doPrivileged
        final AccessControlContext callerAcc =
                (acc == null ?
                new AccessControlContext(NULL_PD_ARRAY) :
                acc);

        // call doPrivileged and push this new context on the stack
        return java.security.AccessController.doPrivileged
                                        (action,
                                        createContext(subject, callerAcc));
    }

    private static AccessControlContext createContext(final Subject subject,
                                        final AccessControlContext acc) {


        return java.security.AccessController.doPrivileged
            (new java.security.PrivilegedAction<AccessControlContext>() {
            public AccessControlContext run() {
                if (subject == null)
                    return new AccessControlContext(acc, null);
                else
                    return new AccessControlContext
                                        (acc,
                                        new SubjectDomainCombiner(subject));
            }
        });
    }

    /**
     * Return the <code>Set</code> of Principals associated with this
     * <code>Subject</code>.  Each <code>Principal</code> represents
     * an identity for this <code>Subject</code>.
     *
     * <p> The returned <code>Set</code> is backed by this Subject's
     * internal <code>Principal</code> <code>Set</code>.  Any modification
     * to the returned <code>Set</code> affects the internal
     * <code>Principal</code> <code>Set</code> as well.
     *
     * <p>
     *
     * @return  The <code>Set</code> of Principals associated with this
     *          <code>Subject</code>.
     */
    public Set<Principal> getPrincipals() {

        // always return an empty Set instead of null
        // so LoginModules can add to the Set if necessary
        return principals;
    }

    /**
     * Return a <code>Set</code> of Principals associated with this
     * <code>Subject</code> that are instances or subclasses of the specified
     * <code>Class</code>.
     *
     * <p> The returned <code>Set</code> is not backed by this Subject's
     * internal <code>Principal</code> <code>Set</code>.  A new
     * <code>Set</code> is created and returned for each method invocation.
     * Modifications to the returned <code>Set</code>
     * will not affect the internal <code>Principal</code> <code>Set</code>.
     *
     * <p>
     *
     * @param c the returned <code>Set</code> of Principals will all be
     *          instances of this class.
     *
     * @return a <code>Set</code> of Principals that are instances of the
     *          specified <code>Class</code>.
     *
     * @exception NullPointerException if the specified <code>Class</code>
     *                  is <code>null</code>.
     */
    public <T extends Principal> Set<T> getPrincipals(Class<T> c) {

        if (c == null)
            throw new NullPointerException
                (ResourcesMgr.getString("invalid.null.Class.provided"));

        // always return an empty Set instead of null
        // so LoginModules can add to the Set if necessary
        return new ClassSet<T>(PRINCIPAL_SET, c);
    }

    /**
     * Return the <code>Set</code> of public credentials held by this
     * <code>Subject</code>.
     *
     * <p> The returned <code>Set</code> is backed by this Subject's
     * internal public Credential <code>Set</code>.  Any modification
     * to the returned <code>Set</code> affects the internal public
     * Credential <code>Set</code> as well.
     *
     * <p>
     *
     * @return  A <code>Set</code> of public credentials held by this
     *          <code>Subject</code>.
     */
    public Set<Object> getPublicCredentials() {

        // always return an empty Set instead of null
        // so LoginModules can add to the Set if necessary
        return pubCredentials;
    }

    /**
     * Return the <code>Set</code> of private credentials held by this
     * <code>Subject</code>.
     *
     * <p> The returned <code>Set</code> is backed by this Subject's
     * internal private Credential <code>Set</code>.  Any modification
     * to the returned <code>Set</code> affects the internal private
     * Credential <code>Set</code> as well.
     *
     * <p> A caller requires permissions to access the Credentials
     * in the returned <code>Set</code>, or to modify the
     * <code>Set</code> itself.  A <code>SecurityException</code>
     * is thrown if the caller does not have the proper permissions.
     *
     * <p> While iterating through the <code>Set</code>,
     * a <code>SecurityException</code> is thrown
     * if the caller does not have permission to access a
     * particular Credential.  The <code>Iterator</code>
     * is nevertheless advanced to next element in the <code>Set</code>.
     *
     * <p>
     *
     * @return  A <code>Set</code> of private credentials held by this
     *          <code>Subject</code>.
     */
    public Set<Object> getPrivateCredentials() {

        // XXX
        // we do not need a security check for
        // AuthPermission(getPrivateCredentials)
        // because we already restrict access to private credentials
        // via the PrivateCredentialPermission.  all the extra AuthPermission
        // would do is protect the set operations themselves
        // (like size()), which don't seem security-sensitive.

        // always return an empty Set instead of null
        // so LoginModules can add to the Set if necessary
        return privCredentials;
    }

    /**
     * Return a <code>Set</code> of public credentials associated with this
     * <code>Subject</code> that are instances or subclasses of the specified
     * <code>Class</code>.
     *
     * <p> The returned <code>Set</code> is not backed by this Subject's
     * internal public Credential <code>Set</code>.  A new
     * <code>Set</code> is created and returned for each method invocation.
     * Modifications to the returned <code>Set</code>
     * will not affect the internal public Credential <code>Set</code>.
     *
     * <p>
     *
     * @param c the returned <code>Set</code> of public credentials will all be
     *          instances of this class.
     *
     * @return a <code>Set</code> of public credentials that are instances
     *          of the  specified <code>Class</code>.
     *
     * @exception NullPointerException if the specified <code>Class</code>
     *          is <code>null</code>.
     */
    public <T> Set<T> getPublicCredentials(Class<T> c) {

        if (c == null)
            throw new NullPointerException
                (ResourcesMgr.getString("invalid.null.Class.provided"));

        // always return an empty Set instead of null
        // so LoginModules can add to the Set if necessary
        return new ClassSet<T>(PUB_CREDENTIAL_SET, c);
    }

    /**
     * Return a <code>Set</code> of private credentials associated with this
     * <code>Subject</code> that are instances or subclasses of the specified
     * <code>Class</code>.
     *
     * <p> The caller must have permission to access all of the
     * requested Credentials, or a <code>SecurityException</code>
     * will be thrown.
     *
     * <p> The returned <code>Set</code> is not backed by this Subject's
     * internal private Credential <code>Set</code>.  A new
     * <code>Set</code> is created and returned for each method invocation.
     * Modifications to the returned <code>Set</code>
     * will not affect the internal private Credential <code>Set</code>.
     *
     * <p>
     *
     * @param c the returned <code>Set</code> of private credentials will all be
     *          instances of this class.
     *
     * @return a <code>Set</code> of private credentials that are instances
     *          of the  specified <code>Class</code>.
     *
     * @exception NullPointerException if the specified <code>Class</code>
     *          is <code>null</code>.
     */
    public <T> Set<T> getPrivateCredentials(Class<T> c) {

        // XXX
        // we do not need a security check for
        // AuthPermission(getPrivateCredentials)
        // because we already restrict access to private credentials
        // via the PrivateCredentialPermission.  all the extra AuthPermission
        // would do is protect the set operations themselves
        // (like size()), which don't seem security-sensitive.

        if (c == null)
            throw new NullPointerException
                (ResourcesMgr.getString("invalid.null.Class.provided"));

        // always return an empty Set instead of null
        // so LoginModules can add to the Set if necessary
        return new ClassSet<T>(PRIV_CREDENTIAL_SET, c);
    }

    /**
     * Compares the specified Object with this <code>Subject</code>
     * for equality.  Returns true if the given object is also a Subject
     * and the two <code>Subject</code> instances are equivalent.
     * More formally, two <code>Subject</code> instances are
     * equal if their <code>Principal</code> and <code>Credential</code>
     * Sets are equal.
     *
     * <p>
     *
     * @param o Object to be compared for equality with this
     *          <code>Subject</code>.
     *
     * @return true if the specified Object is equal to this
     *          <code>Subject</code>.
     *
     * @exception SecurityException if the caller does not have permission
     *          to access the private credentials for this <code>Subject</code>,
     *          or if the caller does not have permission to access the
     *          private credentials for the provided <code>Subject</code>.
     */
    public boolean equals(Object o) {

        if (o == null)
            return false;

        if (this == o)
            return true;

        if (o instanceof Subject) {

            final Subject that = (Subject)o;

            // check the principal and credential sets
            Set<Principal> thatPrincipals;
            synchronized(that.principals) {
                // avoid deadlock from dual locks
                thatPrincipals = new HashSet<Principal>(that.principals);
            }
            if (!principals.equals(thatPrincipals)) {
                return false;
            }

            Set<Object> thatPubCredentials;
            synchronized(that.pubCredentials) {
                // avoid deadlock from dual locks
                thatPubCredentials = new HashSet<Object>(that.pubCredentials);
            }
            if (!pubCredentials.equals(thatPubCredentials)) {
                return false;
            }

            Set<Object> thatPrivCredentials;
            synchronized(that.privCredentials) {
                // avoid deadlock from dual locks
                thatPrivCredentials = new HashSet<Object>(that.privCredentials);
            }
            if (!privCredentials.equals(thatPrivCredentials)) {
                return false;
            }
            return true;
        }
        return false;
    }

    /**
     * Return the String representation of this <code>Subject</code>.
     *
     * <p>
     *
     * @return the String representation of this <code>Subject</code>.
     */
    public String toString() {
        return toString(true);
    }

    /**
     * package private convenience method to print out the Subject
     * without firing off a security check when trying to access
     * the Private Credentials
     */
    String toString(boolean includePrivateCredentials) {

        String s = ResourcesMgr.getString("Subject.");
        String suffix = "";

        synchronized(principals) {
            Iterator<Principal> pI = principals.iterator();
            while (pI.hasNext()) {
                Principal p = pI.next();
                suffix = suffix + ResourcesMgr.getString(".Principal.") +
                        p.toString() + ResourcesMgr.getString("NEWLINE");
            }
        }

        synchronized(pubCredentials) {
            Iterator<Object> pI = pubCredentials.iterator();
            while (pI.hasNext()) {
                Object o = pI.next();
                suffix = suffix +
                        ResourcesMgr.getString(".Public.Credential.") +
                        o.toString() + ResourcesMgr.getString("NEWLINE");
            }
        }

        if (includePrivateCredentials) {
            synchronized(privCredentials) {
                Iterator<Object> pI = privCredentials.iterator();
                while (pI.hasNext()) {
                    try {
                        Object o = pI.next();
                        suffix += ResourcesMgr.getString
                                        (".Private.Credential.") +
                                        o.toString() +
                                        ResourcesMgr.getString("NEWLINE");
                    } catch (SecurityException se) {
                        suffix += ResourcesMgr.getString
                                (".Private.Credential.inaccessible.");
                        break;
                    }
                }
            }
        }
        return s + suffix;
    }

    /**
     * Returns a hashcode for this <code>Subject</code>.
     *
     * <p>
     *
     * @return a hashcode for this <code>Subject</code>.
     *
     * @exception SecurityException if the caller does not have permission
     *          to access this Subject's private credentials.
     */
    public int hashCode() {

        /**
         * The hashcode is derived exclusive or-ing the
         * hashcodes of this Subject's Principals and credentials.
         *
         * If a particular credential was destroyed
         * (<code>credential.hashCode()</code> throws an
         * <code>IllegalStateException</code>),
         * the hashcode for that credential is derived via:
         * <code>credential.getClass().toString().hashCode()</code>.
         */

        int hashCode = 0;

        synchronized(principals) {
            Iterator<Principal> pIterator = principals.iterator();
            while (pIterator.hasNext()) {
                Principal p = pIterator.next();
                hashCode ^= p.hashCode();
            }
        }

        synchronized(pubCredentials) {
            Iterator<Object> pubCIterator = pubCredentials.iterator();
            while (pubCIterator.hasNext()) {
                hashCode ^= getCredHashCode(pubCIterator.next());
            }
        }
        return hashCode;
    }

    /**
     * get a credential's hashcode
     */
    private int getCredHashCode(Object o) {
        try {
            return o.hashCode();
        } catch (IllegalStateException ise) {
            return o.getClass().toString().hashCode();
        }
    }

    /**
     * Writes this object out to a stream (i.e., serializes it).
     */
    private void writeObject(java.io.ObjectOutputStream oos)
                throws java.io.IOException {
        synchronized(principals) {
            oos.defaultWriteObject();
        }
    }

    /**
     * Reads this object from a stream (i.e., deserializes it)
     */
    private void readObject(java.io.ObjectInputStream s)
                throws java.io.IOException, ClassNotFoundException {

        s.defaultReadObject();

        // The Credential <code>Set</code> is not serialized, but we do not
        // want the default deserialization routine to set it to null.
        this.pubCredentials = Collections.synchronizedSet
                        (new SecureSet<Object>(this, PUB_CREDENTIAL_SET));
        this.privCredentials = Collections.synchronizedSet
                        (new SecureSet<Object>(this, PRIV_CREDENTIAL_SET));
    }

    /**
     * Prevent modifications unless caller has permission.
     *
     * @serial include
     */
    private static class SecureSet<E>
        extends AbstractSet<E>
        implements java.io.Serializable {

        private static final long serialVersionUID = 7911754171111800359L;

        /**
         * @serialField this$0 Subject The outer Subject instance.
         * @serialField elements LinkedList The elements in this set.
         */
        private static final ObjectStreamField[] serialPersistentFields = {
            new ObjectStreamField("this$0", Subject.class),
            new ObjectStreamField("elements", LinkedList.class),
            new ObjectStreamField("which", int.class)
        };

        Subject subject;
        LinkedList<E> elements;

        /**
         * @serial An integer identifying the type of objects contained
         *      in this set.  If <code>which == 1</code>,
         *      this is a Principal set and all the elements are
         *      of type <code>java.security.Principal</code>.
         *      If <code>which == 2</code>, this is a public credential
         *      set and all the elements are of type <code>Object</code>.
         *      If <code>which == 3</code>, this is a private credential
         *      set and all the elements are of type <code>Object</code>.
         */
        private int which;

        SecureSet(Subject subject, int which) {
            this.subject = subject;
            this.which = which;
            this.elements = new LinkedList<E>();
        }

        SecureSet(Subject subject, int which, Set<? extends E> set) {
            this.subject = subject;
            this.which = which;
            this.elements = new LinkedList<E>(set);
        }

        public int size() {
            return elements.size();
        }

        public Iterator<E> iterator() {
            final LinkedList<E> list = elements;
            return new Iterator<E>() {
                ListIterator<E> i = list.listIterator(0);

                public boolean hasNext() {return i.hasNext();}

                public E next() {
                    if (which != Subject.PRIV_CREDENTIAL_SET) {
                        return i.next();
                    }

                    SecurityManager sm = System.getSecurityManager();
                    if (sm != null) {
                        try {
                            sm.checkPermission(new PrivateCredentialPermission
                                (list.get(i.nextIndex()).getClass().getName(),
                                subject.getPrincipals()));
                        } catch (SecurityException se) {
                            i.next();
                            throw (se);
                        }
                    }
                    return i.next();
                }

                public void remove() {

                    if (subject.isReadOnly()) {
                        throw new IllegalStateException(ResourcesMgr.getString
                                ("Subject.is.read.only"));
                    }

                    java.lang.SecurityManager sm = System.getSecurityManager();
                    if (sm != null) {
                        switch (which) {
                        case Subject.PRINCIPAL_SET:
                            sm.checkPermission(AuthPermissionHolder.MODIFY_PRINCIPALS_PERMISSION);
                            break;
                        case Subject.PUB_CREDENTIAL_SET:
                            sm.checkPermission(AuthPermissionHolder.MODIFY_PUBLIC_CREDENTIALS_PERMISSION);
                            break;
                        default:
                            sm.checkPermission(AuthPermissionHolder.MODIFY_PRIVATE_CREDENTIALS_PERMISSION);
                            break;
                        }
                    }
                    i.remove();
                }
            };
        }

        public boolean add(E o) {

            if (subject.isReadOnly()) {
                throw new IllegalStateException
                        (ResourcesMgr.getString("Subject.is.read.only"));
            }

            java.lang.SecurityManager sm = System.getSecurityManager();
            if (sm != null) {
                switch (which) {
                case Subject.PRINCIPAL_SET:
                    sm.checkPermission(AuthPermissionHolder.MODIFY_PRINCIPALS_PERMISSION);
                    break;
                case Subject.PUB_CREDENTIAL_SET:
                    sm.checkPermission(AuthPermissionHolder.MODIFY_PUBLIC_CREDENTIALS_PERMISSION);
                    break;
                default:
                    sm.checkPermission(AuthPermissionHolder.MODIFY_PRIVATE_CREDENTIALS_PERMISSION);
                    break;
                }
            }

            switch (which) {
            case Subject.PRINCIPAL_SET:
                if (!(o instanceof Principal)) {
                    throw new SecurityException(ResourcesMgr.getString
                        ("attempting.to.add.an.object.which.is.not.an.instance.of.java.security.Principal.to.a.Subject.s.Principal.Set"));
                }
                break;
            default:
                // ok to add Objects of any kind to credential sets
                break;
            }

            // check for duplicates
            if (!elements.contains(o))
                return elements.add(o);
            else
                return false;
        }

        public boolean remove(Object o) {

            final Iterator<E> e = iterator();
            while (e.hasNext()) {
                E next;
                if (which != Subject.PRIV_CREDENTIAL_SET) {
                    next = e.next();
                } else {
                    next = java.security.AccessController.doPrivileged
                        (new java.security.PrivilegedAction<E>() {
                        public E run() {
                            return e.next();
                        }
                    });
                }

                if (next == null) {
                    if (o == null) {
                        e.remove();
                        return true;
                    }
                } else if (next.equals(o)) {
                    e.remove();
                    return true;
                }
            }
            return false;
        }

        public boolean contains(Object o) {
            final Iterator<E> e = iterator();
            while (e.hasNext()) {
                E next;
                if (which != Subject.PRIV_CREDENTIAL_SET) {
                    next = e.next();
                } else {

                    // For private credentials:
                    // If the caller does not have read permission for
                    // for o.getClass(), we throw a SecurityException.
                    // Otherwise we check the private cred set to see whether
                    // it contains the Object

                    SecurityManager sm = System.getSecurityManager();
                    if (sm != null) {
                        sm.checkPermission(new PrivateCredentialPermission
                                                (o.getClass().getName(),
                                                subject.getPrincipals()));
                    }
                    next = java.security.AccessController.doPrivileged
                        (new java.security.PrivilegedAction<E>() {
                        public E run() {
                            return e.next();
                        }
                    });
                }

                if (next == null) {
                    if (o == null) {
                        return true;
                    }
                } else if (next.equals(o)) {
                    return true;
                }
            }
            return false;
        }

        public boolean removeAll(Collection<?> c) {

            boolean modified = false;
            final Iterator<E> e = iterator();
            while (e.hasNext()) {
                E next;
                if (which != Subject.PRIV_CREDENTIAL_SET) {
                    next = e.next();
                } else {
                    next = java.security.AccessController.doPrivileged
                        (new java.security.PrivilegedAction<E>() {
                        public E run() {
                            return e.next();
                        }
                    });
                }

                Iterator<?> ce = c.iterator();
                while (ce.hasNext()) {
                    Object o = ce.next();
                    if (next == null) {
                        if (o == null) {
                            e.remove();
                            modified = true;
                            break;
                        }
                    } else if (next.equals(o)) {
                        e.remove();
                        modified = true;
                        break;
                    }
                }
            }
            return modified;
        }

        public boolean retainAll(Collection<?> c) {

            boolean modified = false;
            boolean retain = false;
            final Iterator<E> e = iterator();
            while (e.hasNext()) {
                retain = false;
                E next;
                if (which != Subject.PRIV_CREDENTIAL_SET) {
                    next = e.next();
                } else {
                    next = java.security.AccessController.doPrivileged
                        (new java.security.PrivilegedAction<E>() {
                        public E run() {
                            return e.next();
                        }
                    });
                }

                Iterator<?> ce = c.iterator();
                while (ce.hasNext()) {
                    Object o = ce.next();
                    if (next == null) {
                        if (o == null) {
                            retain = true;
                            break;
                        }
                    } else if (next.equals(o)) {
                        retain = true;
                        break;
                    }
                }

                if (!retain) {
                    e.remove();
                    retain = false;
                    modified = true;
                }
            }
            return modified;
        }

        public void clear() {
            final Iterator<E> e = iterator();
            while (e.hasNext()) {
                E next;
                if (which != Subject.PRIV_CREDENTIAL_SET) {
                    next = e.next();
                } else {
                    next = java.security.AccessController.doPrivileged
                        (new java.security.PrivilegedAction<E>() {
                        public E run() {
                            return e.next();
                        }
                    });
                }
                e.remove();
            }
        }

        /**
         * Writes this object out to a stream (i.e., serializes it).
         *
         * <p>
         *
         * @serialData If this is a private credential set,
         *      a security check is performed to ensure that
         *      the caller has permission to access each credential
         *      in the set.  If the security check passes,
         *      the set is serialized.
         */
        private void writeObject(java.io.ObjectOutputStream oos)
                throws java.io.IOException {

            if (which == Subject.PRIV_CREDENTIAL_SET) {
                // check permissions before serializing
                Iterator<E> i = iterator();
                while (i.hasNext()) {
                    i.next();
                }
            }
            ObjectOutputStream.PutField fields = oos.putFields();
            fields.put("this$0", subject);
            fields.put("elements", elements);
            fields.put("which", which);
            oos.writeFields();
        }

        private void readObject(ObjectInputStream ois)
            throws IOException, ClassNotFoundException
        {
            ObjectInputStream.GetField fields = ois.readFields();
            subject = (Subject) fields.get("this$0", null);
            elements = (LinkedList<E>) fields.get("elements", null);
            which = fields.get("which", 0);
        }
    }

    /**
     * This class implements a <code>Set</code> which returns only
     * members that are an instance of a specified Class.
     */
    private class ClassSet<T> extends AbstractSet<T> {

        private int which;
        private Class<T> c;
        private Set<T> set;

        ClassSet(int which, Class<T> c) {
            this.which = which;
            this.c = c;
            set = new HashSet<T>();

            switch (which) {
            case Subject.PRINCIPAL_SET:
                synchronized(principals) { populateSet(); }
                break;
            case Subject.PUB_CREDENTIAL_SET:
                synchronized(pubCredentials) { populateSet(); }
                break;
            default:
                synchronized(privCredentials) { populateSet(); }
                break;
            }
        }

        private void populateSet() {
            final Iterator<?> iterator;
            switch(which) {
            case Subject.PRINCIPAL_SET:
                iterator = Subject.this.principals.iterator();
                break;
            case Subject.PUB_CREDENTIAL_SET:
                iterator = Subject.this.pubCredentials.iterator();
                break;
            default:
                iterator = Subject.this.privCredentials.iterator();
                break;
            }

            // Check whether the caller has permisson to get
            // credentials of Class c

            while (iterator.hasNext()) {
                Object next;
                if (which == Subject.PRIV_CREDENTIAL_SET) {
                    next = java.security.AccessController.doPrivileged
                        (new java.security.PrivilegedAction<Object>() {
                        public Object run() {
                            return iterator.next();
                        }
                    });
                } else {
                    next = iterator.next();
                }
                if (c.isAssignableFrom(next.getClass())) {
                    if (which != Subject.PRIV_CREDENTIAL_SET) {
                        set.add((T)next);
                    } else {
                        // Check permission for private creds
                        SecurityManager sm = System.getSecurityManager();
                        if (sm != null) {
                            sm.checkPermission(new PrivateCredentialPermission
                                                (next.getClass().getName(),
                                                Subject.this.getPrincipals()));
                        }
                        set.add((T)next);
                    }
                }
            }
        }

        public int size() {
            return set.size();
        }

        public Iterator<T> iterator() {
            return set.iterator();
        }

        public boolean add(T o) {

            if (!o.getClass().isAssignableFrom(c)) {
                MessageFormat form = new MessageFormat(ResourcesMgr.getString
                        ("attempting.to.add.an.object.which.is.not.an.instance.of.class"));
                Object[] source = {c.toString()};
                throw new SecurityException(form.format(source));
            }

            return set.add(o);
        }
    }

    static class AuthPermissionHolder {
        static final AuthPermission DO_AS_PERMISSION =
            new AuthPermission("doAs");

        static final AuthPermission DO_AS_PRIVILEGED_PERMISSION =
            new AuthPermission("doAsPrivileged");

        static final AuthPermission SET_READ_ONLY_PERMISSION =
            new AuthPermission("setReadOnly");

        static final AuthPermission GET_SUBJECT_PERMISSION =
            new AuthPermission("getSubject");

        static final AuthPermission MODIFY_PRINCIPALS_PERMISSION =
            new AuthPermission("modifyPrincipals");

        static final AuthPermission MODIFY_PUBLIC_CREDENTIALS_PERMISSION =
            new AuthPermission("modifyPublicCredentials");

        static final AuthPermission MODIFY_PRIVATE_CREDENTIALS_PERMISSION =
            new AuthPermission("modifyPrivateCredentials");
    }
}
