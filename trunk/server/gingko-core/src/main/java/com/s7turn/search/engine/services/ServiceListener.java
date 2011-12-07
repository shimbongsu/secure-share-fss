/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.services;

import com.s7turn.search.engine.ServiceException;

/**
 *
 * @author Long
 */
public interface ServiceListener<E> {
    boolean onCreate( E ob, boolean before ) throws ServiceException;
    boolean onUpdate( E ob, boolean before ) throws ServiceException;
    boolean onDelete( E ob, boolean before ) throws ServiceException;
}
