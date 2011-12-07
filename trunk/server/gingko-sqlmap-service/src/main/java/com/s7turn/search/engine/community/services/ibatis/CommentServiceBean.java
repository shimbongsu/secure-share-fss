/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.community.services.ibatis;

import com.s7turn.search.community.Comment;
import com.s7turn.search.community.services.CommentService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.util.List;

/**
 *
 * @author Long
 */
public class CommentServiceBean extends CommonServiceBean<Comment, Long> implements CommentService{
    public CommentServiceBean(){
        super( Comment.class );
    }

    public List<Comment> findByForUser(Long userId) throws ServiceException {
        return findByQuery( "Comment.findByUser", userId );
    }

    public List<Comment> findByForItem(Long commentForId, String forType) throws ServiceException {
        Comment comment = new Comment();
        comment.setCommentForId(commentForId);
        comment.setCommentForType(forType);
        return findByQuery("Comment.findByForType", comment);
    }
}
