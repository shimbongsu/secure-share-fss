/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.services;

import com.s7turn.search.engine.PhysicalFile;
import com.s7turn.search.engine.ServiceException;
import java.util.List;

/**
 *
 * @author Long
 */
public interface PhysicalFileService  extends CommonService<PhysicalFile, Long>{
    ///return the access url by application special config
    public String getAccessUrl(PhysicalFile pf) throws ServiceException;
    
    List<PhysicalFile> findByUserId( Long userId ) throws ServiceException;

    List<PhysicalFile> findByUserIdAndStatus( Long userId, Long status ) throws ServiceException;

    
}
