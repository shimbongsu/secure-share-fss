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
@Table(name="community_groups")
@NamedQueries({
    @NamedQuery(name="Group.findByCode", query="SELECT g FROM Group g WHERE g.name = :code"),
    @NamedQuery(name="Group.findByPrimaryKey", query="SELECT g FROM Group g WHERE g.id = :id")
})
public class Group  implements Serializable{
    public static final int TYPE_PUBLIC=0; /// MEMBER JOIN THIS GROUP DIECTLY. 
    public static final int TYPE_PRIVATE=1;/// MEMBER CAN NOT REQUEST TO JOIN THIS GROUP
                                           /// MODERATOR SHOULD INVIVTE MEMBER TO JOIN.
    public static final int TYPE_REQUEST=2;/// MEMBER MUST REQUEST TO JOIN, AND MODERATOR CAN NOT INVITE USER.
    public static final int TYPE_REQUEST_INVITE=3;/// MEMBER MUST REQUEST TO JOIN, AND MODERATOR CAN NOT INVITE USER.
    private Long id;
    private String name;
    private String description;
    private Long userId;
    private Timestamp createDate;
    private Timestamp lastUpdatedTime;
    private String tags;
    private Long actorId;
    private int groupType; ///public or private or member only.

    @Column(name="group_type")
    public int getGroupType() {
        return groupType;
    }

    public void setGroupType(int type) {
        this.groupType = type;
    }
    
    @Column(name="group_actor")
    public Long getActorId(){
    	return actorId;
    }
    
    public void setActorId(Long acit){
    	actorId = acit;
    }

    @Column(name="community_tags")
    public String getTags() {
        return tags;
    }

    public void setTags(String tags) {
        this.tags = tags;
    }

    @Column( name = "group_createdate" )
    public Timestamp getCreateDate() {
        return createDate;
    }

    public void setCreateDate(Timestamp createDate) {
        this.createDate = createDate;
    }

    @Column( name = "group_desc" )
    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    @Id
    @GeneratedValue( strategy=GenerationType.SEQUENCE)
    @Column( name = "group_id" )
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    @Column( name = "group_lastupdated" )
    public Timestamp getLastUpdatedTime() {
        return lastUpdatedTime;
    }

    public void setLastUpdatedTime(Timestamp lastUpdatedTime) {
        this.lastUpdatedTime = lastUpdatedTime;
    }

    @Column( name = "group_name" )
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    @Column( name = "group_owneruser" )
    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }
    
}
