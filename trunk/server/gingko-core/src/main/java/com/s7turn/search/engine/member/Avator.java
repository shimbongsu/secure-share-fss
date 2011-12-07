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
@Table( name = "member_avator")
@Entity
@NamedQueries({
    @NamedQuery(name="User.findByCode", query="SELECT g FROM Avator g WHERE g.avatorUrl = :code"),
    @NamedQuery(name="User.findByPrimaryKey", query="SELECT g FROM Avator g WHERE g.id = :id")
})
public class Avator implements Serializable{
    public final static int AVATOR_USER=1;
    public final static int AVATOR_GROUP=2;
    public final static int AVATOR_WEBSHOP=3;
    public final static int AVATOR_GALLERY=4;
    public final static int AVATOR_BLOG=5;
    
    private Long id;
    private Long avatorForId;
    private int avatorType;
    private String avatorUrl;
    private Timestamp lastUpdatedTime;

    @Column( name="avator_forid" )
    public Long getAvatorForId() {
        return avatorForId;
    }

    public void setAvatorForId(Long avatorForId) {
        this.avatorForId = avatorForId;
    }

    @Column( name="avator_type" )
    public int getAvatorType() {
        return avatorType;
    }

    public void setAvatorType(int avatorType) {
        this.avatorType = avatorType;
    }

    @Column( name="avator_url" )
    public String getAvatorUrl() {
        return avatorUrl;
    }

    public void setAvatorUrl(String avatorUrl) {
        this.avatorUrl = avatorUrl;
    }

    @Id
    @GeneratedValue( strategy=GenerationType.SEQUENCE)
    @Column( name="avator_id" )
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    @Column( name="avator_lastupdated" )
    public Timestamp getLastUpdatedTime() {
        return lastUpdatedTime;
    }

    public void setLastUpdatedTime(Timestamp lastUpdatedTime) {
        this.lastUpdatedTime = lastUpdatedTime;
    }
    
}
