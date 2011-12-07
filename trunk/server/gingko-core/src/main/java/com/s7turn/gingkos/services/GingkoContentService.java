/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.GingkoContent;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;

/**
 *
 * @author Long
 */
public interface GingkoContentService extends CommonService<GingkoContent, String>{
    /**
     * Search all contents by special.
     * If opts == 0, search all contents with folders, 
     * If opts == 1, search all folders,
     * If opts == 2, search all contents,
     * @param gct
     * @param opts
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<GingkoContent> findContents( GingkoContent gct, int opts ) throws ServiceException;
    
    List<GingkoContent> findContents( GingkoContent gct, int opts, int start, int count ) throws ServiceException;
}
