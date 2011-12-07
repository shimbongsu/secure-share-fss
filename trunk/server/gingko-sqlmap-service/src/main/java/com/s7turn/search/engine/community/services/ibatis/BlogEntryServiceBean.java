/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.community.services.ibatis;

import com.s7turn.search.community.BlogEntry;
import com.s7turn.search.community.services.BlogEntryService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.util.List;

/**
 *
 * @author Long
 */
public class BlogEntryServiceBean extends CommonServiceBean<BlogEntry, Long> implements BlogEntryService{
    public BlogEntryServiceBean(){
        super( BlogEntry.class );
    }

    public List<BlogEntry> findByBlog(Long blogId) throws ServiceException {
        return findByQuery( "BlogEntry.findByBlog", blogId);
    }

    public List<BlogEntry> findByBlog(Long blogId, Long userId) throws ServiceException {
        BlogEntry be = new BlogEntry();
        be.setBlogId(blogId);
        be.setUserId(userId);
        return findByQuery( "BlogEntry.findByBlogAndUser", be );
    }
    
    @Override
    public boolean smartSave( BlogEntry be ) throws ServiceException{
        if( be.getId() == null || be.getId() == 0 ){
            insert( be );
        }else{
            update( be );
        }
        return  true;
    }
}
