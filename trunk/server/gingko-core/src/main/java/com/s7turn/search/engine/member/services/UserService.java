/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.member.services;

import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.services.CommonService;

/**
 *
 * @author Long
 */
public interface UserService extends CommonService<User, Long>{
    void generateRetrivePassword();
}
