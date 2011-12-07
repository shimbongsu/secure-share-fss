/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community.services;

import com.s7turn.search.community.Comment;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;

/**
 *
 * @author Long
 */
public interface CommentService extends CommonService<Comment, Long> {
    
    /**
     * Get all comments by type
     * @param forType
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<Comment> findByForUser( Long uerId ) throws ServiceException;
    
    /**
     * Get all comments by type of id.
     * @param commentForId
     * @param forType
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<Comment> findByForItem( Long commentForId, String forType ) throws ServiceException;
}
