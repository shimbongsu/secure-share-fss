/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community.services;

import com.s7turn.search.community.Photo;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;

/**
 *
 * @author Long
 */
public interface PhotoService extends CommonService<Photo, Long>{
    /**
     * List out all photos for a gallery
     * @param galleryId
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<Photo> findByGallery(Long galleryId) throws ServiceException; 
}
