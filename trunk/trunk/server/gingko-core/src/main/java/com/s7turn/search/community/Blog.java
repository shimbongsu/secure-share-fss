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
 * Blog
 * @author Long
 */
@Entity
@Table(name="community_blogs")
@NamedQueries({
    @NamedQuery(name="Blog.findByCode", query="SELECT g FROM Blog g WHERE g.name = :code"),
    @NamedQuery(name="Blog.findByPrimaryKey", query="SELECT g FROM Blog g WHERE g.id = :id")
})
public class Blog  implements Serializable{
    public final static int GROUP_BLOG = 1;
    public final static int USER_BLOG = 0;
    private Long id;
    private String name;
    private String description;
    private String status;
    private Long owner;   ///user id or group id
    private int blogType; ///user's or group's 
    private Timestamp lastUpdatedTime;
    private Timestamp createTime;
    private String tags;

    @Column(name="community_tags")
    public String getTags() {
        return tags;
    }

    public void setTags(String tags) {
        this.tags = tags;
    }

    @Column(name="blog_createtime")
    public Timestamp getCreateTime() {
        return createTime;
    }

    public void setCreateTime(Timestamp createTime) {
        this.createTime = createTime;
    }
    
    
    @Column(name="blog_lastupdated")
    public Timestamp getLastUpdatedTime() {
        return lastUpdatedTime;
    }

    public void setLastUpdatedTime(Timestamp lastUpdatedTime) {
        this.lastUpdatedTime = lastUpdatedTime;
    }
    
    @Column(name="blog_blogtype")
    public int getBlogType() {
        return blogType;
    }

    public void setBlogType(int blogType) {
        this.blogType = blogType;
    }

    @Column(name="blog_desc")
    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    @Id
    @GeneratedValue(strategy=GenerationType.SEQUENCE)
    @Column(name="blog_createtime")
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    @Column(name="blog_name")
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    @Column(name="blog_ownerid")
    public Long getOwner() {
        return owner;
    }

    public void setOwner(Long owner) {
        this.owner = owner;
    }

    @Column(name="blog_status")
    public String getStatus() {
        return status;
    }

    public void setStatus(String status) {
        this.status = status;
    }
    
}
