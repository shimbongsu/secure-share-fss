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
@Table(name="community_gallery")
@NamedQueries({
    @NamedQuery(name="Gallery.findByCode", query="SELECT g FROM Gallery g WHERE g.name = :code"),
    @NamedQuery(name="Gallery.findByPrimaryKey", query="SELECT g FROM Gallery g WHERE g.id = :id")
})
public class Gallery  implements Serializable{
    public static final int GALLERY_PUBLIC=0;
    public static final int GALLERY_PRIVATE=1;
    public static final int GALLERY_MEMBER=2;
    public static final int GALLERY_FRIENDS=3;
    public static final int GALLERY_REQUEST_PASSWORD=4;
    
    private Long id;
    private String name;
    private String description;
    private Long actorId; ///related to photo id
    private Long createBy; //
    private int galleryType; //1. User's photo, 2. User's Webshop and Product photo, 3. Group photos
    private Timestamp lastUpdatedTime;
    private String tags;
    private int security;

    @Column(name="gallery_security")
    public int getSecurity() {
        return security;
    }

    public void setSecurity(int security) {
        this.security = security;
    }

    
    @Column(name="community_tags")
    public String getTags() {
        return tags;
    }

    public void setTags(String tags) {
        this.tags = tags;
    }

    @Column(name="gallery_actorid")
    public Long getActorId() {
        return actorId;
    }

    public void setActorId(Long actorId) {
        this.actorId = actorId;
    }

    @Column(name="gallery_owner")
    public Long getCreateBy() {
        return createBy;
    }

    public void setCreateBy(Long createBy) {
        this.createBy = createBy;
    }

    @Column(name="gallery_desc")
    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    @Column(name="gallery_type")
    public int getGalleryType() {
        return galleryType;
    }

    public void setGalleryType(int galleryType) {
        this.galleryType = galleryType;
    }

    @Id
    @GeneratedValue( strategy= GenerationType.SEQUENCE)
    @Column(name="gallery_id")
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    @Column(name="gallery_lastupdated")
    public Timestamp getLastUpdatedTime() {
        return lastUpdatedTime;
    }

    public void setLastUpdatedTime(Timestamp lastUpdatedTime) {
        this.lastUpdatedTime = lastUpdatedTime;
    }

    @Column(name="gallery_name")
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

}
