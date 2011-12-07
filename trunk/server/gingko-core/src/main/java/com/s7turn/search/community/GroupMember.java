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
@Table(name="community_groupmember")
@NamedQueries({
    @NamedQuery(name="GroupMember.findByCode", query="SELECT g FROM GroupMember g WHERE g.userId = :code"),
    @NamedQuery(name="GroupMember.findByPrimaryKey", query="SELECT g FROM GroupMember g WHERE g.id = :id")
})
public class GroupMember  implements Serializable{
    private Long id;
    private Group group;
    private User user;
    private GroupTeam team;
    private String role; ///owner, master, member
    private Timestamp joinTime;
    private Timestamp lastUpdatedTime;


    @Id
    @GeneratedValue(strategy=GenerationType.SEQUENCE)
    @Column(name="gmem_id")
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    @Column(name="gmem_jointime")
    public Timestamp getJoinTime() {
        return joinTime;
    }

    public void setJoinTime(Timestamp joinTime) {
        this.joinTime = joinTime;
    }

    @Column(name="gmem_role")
    public String getRole() {
        return role;
    }

    public void setRole(String role) {
        this.role = role;
    }

    public Group getGroup() {
        return group;
    }

    public void setGroup(Group group) {
        this.group = group;
    }

    public Timestamp getLastUpdatedTime() {
        return lastUpdatedTime;
    }

    public void setLastUpdatedTime(Timestamp lastUpdatedTime) {
        this.lastUpdatedTime = lastUpdatedTime;
    }

    public GroupTeam getTeam() {
        return team;
    }

    public void setTeam(GroupTeam team) {
        this.team = team;
    }

    public User getUser() {
        return user;
    }

    public void setUser(User user) {
        this.user = user;
    }


}
