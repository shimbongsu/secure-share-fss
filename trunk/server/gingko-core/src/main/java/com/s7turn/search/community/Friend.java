/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community;

import com.s7turn.search.engine.member.User;
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
@Table(name="community_friend")
@NamedQueries({
    @NamedQuery(name="Friend.findByCode", query="SELECT g FROM Friend g WHERE g.userId = :code"),
    @NamedQuery(name="Friend.findByPrimaryKey", query="SELECT g FROM Friend g WHERE g.id = :id")
})
public class Friend  implements Serializable{
    private Long id;
    private Long userId;
    private User friendUser;
    private String friendType;
    private Timestamp lastUpdatedTime;

    @Column( name = "friend_anotheruser")
    public User getFriendUser() {
        return friendUser;
    }

    public void setFriendUser(User frUser) {
        this.friendUser = frUser;
    }

    @Column( name = "friend_friendtype")
    public String getFriendType() {
        return friendType;
    }

    public void setFriendType(String friendType) {
        this.friendType = friendType;
    }

    @Id
    @GeneratedValue( strategy=GenerationType.SEQUENCE)
    @Column( name = "friend_id")
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    @Column( name = "friend_lastupdated")
    public Timestamp getLastUpdatedTime() {
        return lastUpdatedTime;
    }

    public void setLastUpdatedTime(Timestamp lastUpdatedTime) {
        this.lastUpdatedTime = lastUpdatedTime;
    }

    @Column( name = "friend_userid")
    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }
    
    
}
