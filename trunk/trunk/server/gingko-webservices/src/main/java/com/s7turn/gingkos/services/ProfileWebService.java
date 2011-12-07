/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.search.engine.member.UserDetail;
import javax.jws.WebMethod;
import javax.jws.WebService;

/**
 *
 * @author Long
 */
@WebService
public interface ProfileWebService {

    @WebMethod(operationName = "updateProfile", exclude = false)
    String updateProfile(String tokenId, UserDetail ud);

    @WebMethod(operationName = "getMyProfile", exclude = false)
    UserDetail getMyProfile(String tokenId);

    @WebMethod(operationName = "getProfile", exclude = false)
    UserDetail getProfile(String tokenId, Long userId);
}
