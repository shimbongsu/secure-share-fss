/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos;

import com.s7turn.search.community.Group;
import com.s7turn.search.engine.member.User;
import java.io.Serializable;
import java.sql.Timestamp;

/**
 *
 * @author Long
 */
public class Gingko implements Serializable {
    private Long gingkoId;
    private GingkoContent content;
    private User user;
    private Group group;
    private boolean defaultGingko;  ///is a defaultGingko, the user and group should be null, which is for all guys
    private boolean read;           ///can be read.
    private boolean edit;           ///can be modified
    private boolean copy;           ///can be copied to 
    private boolean print;          ///can be print into document.
    private boolean list;           ///can be list
    private Timestamp beginTime;
    private Timestamp endTime;
    private boolean permanent;


    public Long getGingkoId() {
        return gingkoId;
    }

    public void setGingkoId(Long gingkoId) {
        this.gingkoId = gingkoId;
    }
    
    public GingkoContent getContent() {
        return content;
    }

    public void setContent(GingkoContent content) {
        this.content = content;
    }

    public boolean isCopy() {
        return copy;
    }

    public void setCopy(boolean copy) {
        this.copy = copy;
    }

    public boolean isDefaultGingko() {
        return defaultGingko;
    }

    public void setDefaultGingko(boolean defaultGingko) {
        this.defaultGingko = defaultGingko;
    }

    public boolean isEdit() {
        return edit;
    }

    public void setEdit(boolean edit) {
        this.edit = edit;
    }

    public Group getGroup() {
        return group;
    }

    public void setGroup(Group group) {
        this.group = group;
    }

    public boolean isPrint() {
        return print;
    }

    public void setPrint(boolean print) {
        this.print = print;
    }

    public boolean isRead() {
        return read;
    }

    public void setRead(boolean read) {
        this.read = read;
    }

    public User getUser() {
        return user;
    }

    public void setUser(User user) {
        this.user = user;
    }

    public boolean isList() {
        return list;
    }

    public void setList(boolean list) {
        this.list = list;
    }

    public Timestamp getBeginTime() {
        return beginTime;
    }

    public void setBeginTime(Timestamp beginTime) {
        this.beginTime = beginTime;
    }

    public Timestamp getEndTime() {
        return endTime;
    }

    public void setEndTime(Timestamp endTime) {
        this.endTime = endTime;
    }

    public boolean isPermanent() {
        return permanent;
    }

    public void setPermanent(boolean permanent) {
        this.permanent = permanent;
    }
    
}
