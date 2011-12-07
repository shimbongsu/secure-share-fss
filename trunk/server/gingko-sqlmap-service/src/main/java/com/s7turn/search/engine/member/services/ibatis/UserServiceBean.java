/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.member.services.ibatis;

import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.member.services.UserService;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;

/**
 *
 * @author Long
 */
public class UserServiceBean extends CommonServiceBean<User, Long> implements UserService{

    public UserServiceBean(){
        super(User.class);
    }
    
    public void generateRetrivePassword() {
        
        throw new UnsupportedOperationException("Not supported yet.");
    }

}
