/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.search.engine.member.User;
import javax.jws.WebMethod;
import javax.jws.WebService;

/**
 *
 * @author Long
 */
@WebService
public interface MemberWebService {
    @WebMethod(operationName = "getMembers", exclude = false)
    ListBag<User> getMembers(String tokenId, int from, int end );
    @WebMethod(operationName = "getTotalMembers", exclude = false)
    int getTotalMembers(String tokenId);
}
