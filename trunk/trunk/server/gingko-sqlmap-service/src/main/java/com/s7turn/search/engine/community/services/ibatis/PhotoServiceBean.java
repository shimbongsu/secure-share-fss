/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.community.services.ibatis;

import com.s7turn.search.community.Photo;
import com.s7turn.search.community.services.PhotoService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.util.List;

/**
 *
 * @author Long
 */
public class PhotoServiceBean extends CommonServiceBean<Photo, Long> implements PhotoService{
    public PhotoServiceBean(){
        super( Photo.class );
    }

    public List<Photo> findByGallery(Long galleryId) throws ServiceException {
        return findByQuery("Photo.findByGallery", galleryId);
    }
    
    public List<Photo> findByUser(Long userId) throws ServiceException {
        return findByQuery("Photo.findByUser", userId);
    }
}
