/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine;

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
@Table(name="common_phyfiles")
@NamedQueries({
    @NamedQuery(name="PhysicalFile.findByCode", query="SELECT g FROM PhysicalFile g WHERE g.fileName = :code"),
    @NamedQuery(name="PhysicalFile.findByPrimaryKey", query="SELECT g FROM PhysicalFile g WHERE g.id = :id")
})
public class PhysicalFile implements Serializable {
    private Long id;
    private String fileName;
    private String mimeType;
    private String originalFileName;
    private String md5Code;
    private Long fileSize;
    private Timestamp lastUpdatedTime;
    private Long status;
    private String description;
    private Long userId;

    @Column( name="pf_desc" )
    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    @Column( name="pf_owneruser" )
    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }

    
    @Column( name="pf_filename" )
    public String getFileName() {
        return fileName;
    }

    public void setFileName(String fileName) {
        this.fileName = fileName;
    }

    @Column( name="pf_filesize" )
    public Long getFileSize() {
        return fileSize;
    }

    public void setFileSize(Long fileSize) {
        this.fileSize = fileSize;
    }

    @Id
    @GeneratedValue( strategy = GenerationType.SEQUENCE)
    @Column( name="pf_id" )
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    @Column( name="pf_lastupdatedtime" )
    public Timestamp getLastUpdatedTime() {
        return lastUpdatedTime;
    }

    public void setLastUpdatedTime(Timestamp lastUpdatedTime) {
        this.lastUpdatedTime = lastUpdatedTime;
    }

    @Column( name="pf_mimetype" )
    public String getMimeType() {
        return mimeType;
    }

    public void setMimeType(String mimeType) {
        this.mimeType = mimeType;
    }

    @Column( name="pf_org_filename" )
    public String getOriginalFileName() {
        return originalFileName;
    }

    public void setOriginalFileName(String originalFileName) {
        this.originalFileName = originalFileName;
    }

    @Column( name="pf_status" ) ///if 1, exist and  built the relationship to other object, else, this file can be deleted.
    public Long getStatus() {
        return status;
    }

    public void setStatus(Long status) {
        this.status = status;
    }

    @Column( name="pf_md5" )
    public String getMd5Code() {
        return md5Code;
    }

    public void setMd5Code(String md5Code) {
        this.md5Code = md5Code;
    }
    
    
}
