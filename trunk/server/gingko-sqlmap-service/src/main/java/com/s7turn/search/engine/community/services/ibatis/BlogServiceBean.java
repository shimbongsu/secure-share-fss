/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.community.services.ibatis;

import com.s7turn.search.community.Blog;
import com.s7turn.search.community.services.BlogService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.util.List;

/**
 *
 * @author Long
 */
public class BlogServiceBean extends CommonServiceBean<Blog, Long> implements BlogService{
    public BlogServiceBean(){
        super( Blog.class );
    }

    public List<Blog> findByUser(Long userId) throws ServiceException {
        return findByQuery("Blog.findByOwnerForUser", userId);
    }

    public List<Blog> findByGroup(Long groupId) throws ServiceException {
        return findByQuery("Blog.findByOwnerForGroup", groupId);
    }
    
    @Override
    public boolean smartSave( Blog blog ) throws ServiceException{
        Blog b = null;
        
        try{
            b = findByCode(blog.getName());
        }catch(Exception e){
            b = null;
        }
        
        if( blog.getId() == null || blog.getId() == 0 ){
            if( b == null ){
                insert( blog );
            }else{
                throw new ServiceException("Duplication Blog Name: " + blog.getName() );
            }
        }else{
            update( blog );
        }
        return true;
     }

}
