/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.utils;

import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.PhysicalFileService;

/**
 *
 * @author Long
 */
public interface ServiceFactory 
{
    PhysicalFileService getPhysicalFileService() throws ServiceException;
}
