/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community;

import java.sql.Timestamp;

/**
 *
 * @author Long
 */
public class GroupTeam {
    private Long id;
    private Group group;
    private GroupTeam parentTeam;
    private String name;
    private Integer teamType;
    private String description;
    private Timestamp createDate;
    private Timestamp lastUpdatedTime;

    public Integer getTeamType() {
        return teamType;
    }

    public void setTeamType(Integer teamType) {
        this.teamType = teamType;
    }

    
    public Timestamp getCreateDate() {
        return createDate;
    }

    public void setCreateDate(Timestamp createDate) {
        this.createDate = createDate;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public Group getGroup() {
        return group;
    }

    public void setGroup(Group group) {
        this.group = group;
    }

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public Timestamp getLastUpdatedTime() {
        return lastUpdatedTime;
    }

    public void setLastUpdatedTime(Timestamp lastUpdatedTime) {
        this.lastUpdatedTime = lastUpdatedTime;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public GroupTeam getParentTeam() {
        return parentTeam;
    }

    public void setParentTeam(GroupTeam parentTeam) {
        this.parentTeam = parentTeam;
    }

}
