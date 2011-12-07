/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community.services;

import com.s7turn.search.community.Gallery;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;

/**
 *
 * @author Long
 */
public interface GalleryService extends CommonService<Gallery, Long>{
    
    /**
     * List all Galleries for this onwer.
     * @param ownerId
     * @param ownerType
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<Gallery> findByOwner( Long ownerId, int ownerType ) throws ServiceException;
    
    /**
     * List all Galleries for this type
     * @param ownerType
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<Gallery> findByOwnerType( int ownerType ) throws ServiceException;
}
