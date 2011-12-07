/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community.services;

import com.s7turn.search.community.Blog;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;

/**
 *
 * @author Long
 */
public interface BlogService extends CommonService<Blog, Long>{
    /**
     * find the Blogs by the User's and type should be user.
     * @param userId
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<Blog> findByUser( Long userId ) throws ServiceException; 
    
    /**
     * find the Blogs by the Group and type should be group
     * @param groupId
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<Blog> findByGroup( Long groupId ) throws ServiceException;
}
