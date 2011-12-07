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
@Table(name="community_photos")
@NamedQueries({
    @NamedQuery(name="Photo.findByCode", query="SELECT g FROM Photo g WHERE g.name = :code"),
    @NamedQuery(name="Photo.findByPrimaryKey", query="SELECT g FROM Photo g WHERE g.id = :id")
})
public class Photo  implements Serializable{
    private Long id;
    private String name;
    private String description;
    private Long userId;
    private Long smallFileId;
    private Long thumbFileId;
    private Long fileId;
    private String smallFileUrl;
    private String thumbFileUrl;
    private String fileUrl;
    private String fileType;
    private String fileSize;
    private Timestamp createDate;
    private String tags;
    private Long galleryId;

    @Column(name="photo_gallery")
    public Long getGalleryId() {
        return galleryId;
    }

    public void setGalleryId(Long galleryId) {
        this.galleryId = galleryId;
    }

    @Column(name="photo_fileid")
    public Long getFileId() {
        return fileId;
    }

    public void setFileId(Long fileId) {
        this.fileId = fileId;
    }

    @Column(name="photo_smallfileid")
    public Long getSmallFileId() {
        return smallFileId;
    }

    public void setSmallFileId(Long smallFileId) {
        this.smallFileId = smallFileId;
    }

    @Column(name="photo_smallfileurl")
    public String getSmallFileUrl() {
        return smallFileUrl;
    }

    public void setSmallFileUrl(String smallFileUrl) {
        this.smallFileUrl = smallFileUrl;
    }

    @Column(name="photo_thumbfileid")
    public Long getThumbFileId() {
        return thumbFileId;
    }

    public void setThumbFileId(Long thumbFileId) {
        this.thumbFileId = thumbFileId;
    }

    @Column(name="photo_thumbfileurl")
    public String getThumbFileUrl() {
        return thumbFileUrl;
    }

    public void setThumbFileUrl(String thumbFileUrl) {
        this.thumbFileUrl = thumbFileUrl;
    }
    
    @Column(name="community_tags")
    public String getTags() {
        return tags;
    }

    public void setTags(String tags) {
        this.tags = tags;
    }
    
    @Column(name="photo_createdate")
    public Timestamp getCreateDate() {
        return createDate;
    }

    public void setCreateDate(Timestamp createDate) {
        this.createDate = createDate;
    }

    @Column(name="photo_desc")
    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    @Column(name="photo_filesize") //a description for file size type like: 50x50
    public String getFileSize() {
        return fileSize;
    }

    public void setFileSize(String fileSize) {
        this.fileSize = fileSize;
    }

    @Column(name="photo_filetype")
    public String getFileType() {
        return fileType;
    }

    public void setFileType(String fileType) {
        this.fileType = fileType;
    }

    @Column(name="photo_fileurl")
    public String getFileUrl() {
        return fileUrl;
    }

    public void setFileUrl(String fileUrl) {
        this.fileUrl = fileUrl;
    }

    @Id
    @GeneratedValue(strategy=GenerationType.SEQUENCE)
    @Column(name="photo_id")
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    @Column(name="photo_name")
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    @Column(name="photo_owneruser")
    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }

    
}
