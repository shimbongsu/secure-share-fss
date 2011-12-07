/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.community.struts;

import com.s7turn.common.struts.ActionBase;
import com.s7turn.search.community.Comment;
import com.s7turn.search.community.services.CommentService;

/**
 *
 * @author Long
 */
public class CommentAction extends ActionBase {
    
    private Long userId;
    private Long commentId;
    private Comment comment;
    private CommentService commentService;
    
    public CommentAction(){
        this.setNavigator("account");
    }

    
    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }

    public Comment getComment() {
        return comment;
    }

    public void setComment(Comment comment) {
        this.comment = comment;
    }

    public Long getCommentId() {
        return commentId;
    }

    public void setCommentId(Long commentId) {
        this.commentId = commentId;
    }

    public CommentService getCommentService() {
        return commentService;
    }

    public void setCommentService(CommentService commentService) {
        this.commentService = commentService;
    }
    
    /**
     * Save the comments
     * @return
     * @throws java.lang.Exception
     */
    public String save() throws Exception{
        if( this.getComment() != null ){
            if( this.getComment().getId() == null || this.getComment().getId() == 0 ){
                this.getCommentService().insert(getComment());
            }else{
                this.getCommentService().update(getComment());
            }
        }
        return "success";
    }
    
    public String mock() throws Exception {
        return "success";
    }
    
}
