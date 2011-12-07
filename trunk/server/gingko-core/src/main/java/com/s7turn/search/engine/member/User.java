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
@Table( name = "member_user")
@Entity
@NamedQueries({
    @NamedQuery(name="User.findByCode", query="SELECT g FROM User g WHERE g.loginId = :code"),
    @NamedQuery(name="User.findByPrimaryKey", query="SELECT g FROM User g WHERE g.id = :id")
})
public class User implements Serializable{
    public final static int PUBLIC = 0; ///EVERY ONE CAN ADD THIS AS FRIEND
    public final static int REQUEST = 1;///EVERY ONE ADD THIS AS FRIEND SHOULD SEND A MESSAGE TO BE AUTHORIZED BY THIS USER.
    public final static int PRIVATE = 2;///EVERY ON CAN NOT ADD THIS AS FRIEND

    private Long id;
    private String loginId;
    private String password;
    private String screenName;
    private String fullName;
    private String email;
    private boolean retreiveFlag;
    private boolean disabled;
    private boolean locked;
    private boolean expired;
    private String rsa;
    private String retreivePassword;
    private Timestamp createDate;
    private Timestamp lastUpdatedTime;
    private int security;

    public int getSecurity() {
        return security;
    }

    public void setSecurity(int security) {
        this.security = security;
    }


    public User(){
    }

    public User(Long uid){
        this.setId(uid);
    }
    
    @Column(name="user_disabled")
    public boolean isDisabled() {
        return disabled;
    }

    public void setDisabled(boolean disabled) {
        this.disabled = disabled;
    }

    @Column(name="user_expired")
    public boolean isExpired() {
        return expired;
    }

    public void setExpired(boolean expired) {
        this.expired = expired;
    }

    @Column(name="user_locked")
    public boolean isLocked() {
        return locked;
    }

    public void setLocked(boolean locked) {
        this.locked = locked;
    }

    
    @Column(name="create_date")
    public Timestamp getCreateDate() {
        return createDate;
    }

    public void setCreateDate(Timestamp createDate) {
        this.createDate = createDate;
    }

    @Column(name="last_updated")
    public Timestamp getLastUpdatedTime() {
        return lastUpdatedTime;
    }

    public void setLastUpdatedTime(Timestamp lastUpdatedTime) {
        this.lastUpdatedTime = lastUpdatedTime;
    }

    @Column(name="user_email")
    public String getEmail() {
        return email;
    }

    public void setEmail(String email) {
        this.email = email;
    }

    @Column(name="full_name")
    public String getFullName() {
        return fullName;
    }

    public void setFullName(String fullName) {
        this.fullName = fullName;
    }
    
    @Id
    @GeneratedValue(strategy = GenerationType.SEQUENCE)
    @Column(name="user_id")
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    @Column(name="login_id")
    public String getLoginId() {
        return loginId;
    }

    public void setLoginId(String loginId) {
        this.loginId = loginId;
    }

    @Column(name="user_passwd")
    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    @Column(name="forget_flag")
    public boolean isRetreiveFlag() {
        return retreiveFlag;
    }

    public void setRetreiveFlag(boolean retreiveFlag) {
        this.retreiveFlag = retreiveFlag;
    }

    @Column(name="ret_password")
    public String getRetreivePassword() {
        return retreivePassword;
    }

    public void setRetreivePassword(String retreivePassword) {
        this.retreivePassword = retreivePassword;
    }

    @Column(name="passwd_rsa")
    public String getRsa() {
        return rsa;
    }

    public void setRsa(String rsa) {
        this.rsa = rsa;
    }

    @Column(name="user_screenname")
    public String getScreenName() {
        return screenName;
    }

    public void setScreenName(String screenName) {
        this.screenName = screenName;
    }

    @Override
    public int hashCode(){
        if( this.getId() != null ){
            return this.getId().hashCode();
        }
        return super.hashCode();
    }

    @Override
    public boolean equals(Object o){
        if( o instanceof User ){
            return this.hashCode() == o.hashCode();
        }
        return false;
    }
    
    
}
