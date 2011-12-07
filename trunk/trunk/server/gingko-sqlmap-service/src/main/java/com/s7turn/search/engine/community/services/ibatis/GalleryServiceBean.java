/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.community.services.ibatis;

import com.s7turn.search.community.Gallery;
import com.s7turn.search.community.services.GalleryService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.util.List;

/**
 *
 * @author Long
 */
public class GalleryServiceBean extends CommonServiceBean<Gallery, Long> implements GalleryService{
    public GalleryServiceBean(){
        super( Gallery.class );
    }

    public List<Gallery> findByOwner(Long ownerId, int ownerType) throws ServiceException {
        Gallery g = new Gallery();
        g.setGalleryType(ownerType);
        g.setCreateBy(ownerId);
        return findByQuery("Gallery.findByOwnerWithType",g);
    }

    public List<Gallery> findByOwnerType(int ownerType) throws ServiceException {
        return findByQuery("Gallery.findByOwnerType", ownerType);
    }
}
