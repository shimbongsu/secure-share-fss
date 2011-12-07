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
public interface SmartEntityPersistence {
    void persist(Object eobj) throws ServiceException;
}
