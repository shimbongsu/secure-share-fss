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
@Table(name="community_blogentry")
@NamedQueries({
    @NamedQuery(name="BlogEntry.findByCode", query="SELECT g FROM BlogEntry g WHERE g.subject = :code"),
    @NamedQuery(name="BlogEntry.findByPrimaryKey", query="SELECT g FROM BlogEntry g WHERE g.id = :id")
})
public class BlogEntry  implements Serializable{
    public static final int ENTRY_DRAFT = 0;
    public static final int ENTRY_PUBLISHED = 1;
    private Long id;
    private String subject;
    private String content;
    private String shortDescription;
    private Timestamp lastUpdatedTime;
    private Timestamp createTime;
    private Long userId;
    private Long blogId;
    private String tags;
    private short status;

    @Column(name="community_tags")
    public String getTags() {
        return tags;
    }

    public void setTags(String tags) {
        this.tags = tags;
    }

    @Column(name="bentry_shortdesc")
    public String getShortDescription() {
        return shortDescription;
    }

    public void setShortDescription(String shortDescription) {
        this.shortDescription = shortDescription;
    }

    @Column(name="bentry_status")
    public short getStatus() {
        return status;
    }

    public void setStatus(short status) {
        this.status = status;
    }
    
    


    @Column(name="bentry_blogid")
    public Long getBlogId() {
        return blogId;
    }

    public void setBlogId(Long blogId) {
        this.blogId = blogId;
    }

    @Column(name="bentry_content")
    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }

    @Column(name="bentry_createtime")
    public Timestamp getCreateTime() {
        return createTime;
    }

    public void setCreateTime(Timestamp createTime) {
        this.createTime = createTime;
    }

    @Id
    @GeneratedValue( strategy=GenerationType.SEQUENCE)
    @Column(name="bentry_id")
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    @Column(name="bentry_lastupdated")
    public Timestamp getLastUpdatedTime() {
        return lastUpdatedTime;
    }

    public void setLastUpdatedTime(Timestamp lastUpdatedTime) {
        this.lastUpdatedTime = lastUpdatedTime;
    }

    @Column(name="bentry_subject")
    public String getSubject() {
        return subject;
    }

    public void setSubject(String subject) {
        this.subject = subject;
    }

    @Column(name="bentry_userid")
    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }
    
}
