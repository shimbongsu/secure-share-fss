/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.member.services.ibatis;

import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.member.UserWrapper;
import org.springframework.security.GrantedAuthority;
import org.springframework.security.userdetails.UserDetails;

/**
 *
 * @author Long
 */
public class UserDetailWrapper implements UserDetails, UserWrapper{

    private User user;
    
    public UserDetailWrapper(User user){
        this.user = user;
    }

    public User getUser() {
        return user;
    }

    public void setUser(User user) {
        this.user = user;
    }
    
    
    
    public Long getId(){
        return user.getId();
    }
    
    public GrantedAuthority[] getAuthorities() {
        return new GrantedAuthority[]{
          new GrantedAuthority(){

            public String getAuthority() {
                return "ROLE_USER";
            }

            public int compareTo(Object o) {
                if( o != null && o instanceof String ){
                    return "ROLE_USER".compareTo((String)o);
                }
                return -1;
            }
          }  
        };
    }
    
    public String getFullName(){
        return user.getFullName();
    }
    
    public String getScreenName(){
        return user.getScreenName();
    }
    
    public String getEmail(){
        return user.getEmail();
    }

    public String getPassword() {
        return user.getPassword();
    }

    public String getUsername() {
        return user.getLoginId();
    }

    public boolean isAccountNonExpired() {
        return !user.isExpired();
    }

    public boolean isAccountNonLocked() {
        return !user.isLocked();
    }

    public boolean isCredentialsNonExpired() {
        return !user.isExpired();
    }

    public boolean isEnabled() {
        return !user.isDisabled();
    }

}
