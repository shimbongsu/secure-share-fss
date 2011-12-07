/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.member.services.ibatis;

import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.member.services.UserService;
import org.springframework.dao.DataAccessException;
import org.springframework.security.GrantedAuthority;
import org.springframework.security.userdetails.UserDetails;
import org.springframework.security.userdetails.UserDetailsService;
import org.springframework.security.userdetails.UsernameNotFoundException;

/**
 *
 * @author Long
 */
public class CustomUserDetailsService implements UserDetailsService{
    private UserService userService;

    public UserService getUserService() {
        return userService;
    }

    public void setUserService(UserService userService) {
        this.userService = userService;
    }
    
    public UserDetails loadUserByUsername(String loginId) throws UsernameNotFoundException, DataAccessException {
        User user;
        try {
            user = getUserService().findByCode(loginId);
        } catch (ServiceException ex) {
            throw new DataAccessException(ex.getMessage(), ex) {};
        }
        if( user != null ){
            return new UserDetailWrapper( user );
        }
        throw new UsernameNotFoundException(loginId + " not found.");
    }

}
