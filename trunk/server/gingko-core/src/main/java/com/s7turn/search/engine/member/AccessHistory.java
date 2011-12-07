/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.member;

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
@Table(name="access_history")
@NamedQueries({
    //@NamedQuery(name="AccessHistory.findByCode", query="SELECT g FROM AccessHistory g WHERE g.name = :code"),
    @NamedQuery(name="AccessHistory.findByPrimaryKey", query="SELECT g FROM AccessHistory g WHERE g.id = :id")
})
public class AccessHistory implements Serializable{
    public final static int CATEGORY_USERPROFILE = 1;
    public final static int CATEGORY_PHOTO = 2;
    public final static int CATEGORY_GROUP = 3;
    public final static int CATEGORY_WEBSHOP = 4;
    public final static int CATEGORY_BLOG = 5;
   
    private Long id;
    private Long userId;
    private Long accessToId;
    private Timestamp when;
    private int accessCategory;
    private String description;
    private String remoteAddress;

    
    @Id
    @GeneratedValue( strategy=GenerationType.SEQUENCE)
    @Column(name="history_id")
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    @Column(name="access_desc")
    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    @Column(name="access_address")
    public String getRemoteAddress() {
        return remoteAddress;
    }

    public void setRemoteAddress(String remoteAddress) {
        this.remoteAddress = remoteAddress;
    }

    
    
    @Column(name="where_type")
    public int getAccessCategory() {
        return accessCategory;
    }

    public void setAccessCategory(int accessCategory) {
        this.accessCategory = accessCategory;
    }

    @Column(name="where_id")
    public Long getAccessToId() {
        return accessToId;
    }

    public void setAccessToId(Long accessToId) {
        this.accessToId = accessToId;
    }

    @Column(name="who_id")
    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }

    @Column(name="when_access")
    public Timestamp getWhen() {
        return when;
    }

    public void setWhen(Timestamp when) {
        this.when = when;
    }
    
}
