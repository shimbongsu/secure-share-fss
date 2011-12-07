/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community;

import java.io.Serializable;
import java.sql.Timestamp;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.NamedQueries;
import javax.persistence.NamedQuery;
import javax.persistence.Table;

/**
 *
 * @author Long
 */
@Entity
@Table(name="comments")
@NamedQueries({
    @NamedQuery(name="Comment.findByCode", query="SELECT g FROM Comment g WHERE g.subject = :code"),
    @NamedQuery(name="Comment.findByPrimaryKey", query="SELECT g FROM Comment g WHERE g.id = :id")
})
public class Comment  implements Serializable{
    private Long id;
    private Long userId;
    private String commentForType;
    private Long commentForId;
    private String subject;
    private String content;
    private Long replyId;
    private Timestamp lastUpdatedTime;

    @Column(name="comment_forid")
    public Long getCommentForId() {
        return commentForId;
    }

    public void setCommentForId(Long commentForId) {
        this.commentForId = commentForId;
    }

    @Column(name="comment_fortype")
    public String getCommentForType() {
        return commentForType;
    }

    public void setCommentForType(String commentForType) {
        this.commentForType = commentForType;
    }

    @Column(name="comment_content")
    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }

    @Id
    @GeneratedValue( strategy=GenerationType.SEQUENCE)
    @Column(name="comment_id")
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    @Column(name="comment_lastupdatedtime")
    public Timestamp getLastUpdatedTime() {
        return lastUpdatedTime;
    }

    public void setLastUpdatedTime(Timestamp lastUpdatedTime) {
        this.lastUpdatedTime = lastUpdatedTime;
    }

    @Column(name="comment_replyid")
    public Long getReplyId() {
        return replyId;
    }

    public void setReplyId(Long replyId) {
        this.replyId = replyId;
    }

    @Column(name="comment_subject")
    public String getSubject() {
        return subject;
    }

    public void setSubject(String subject) {
        this.subject = subject;
    }

    @Column(name="comment_userid")
    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }
    
}
