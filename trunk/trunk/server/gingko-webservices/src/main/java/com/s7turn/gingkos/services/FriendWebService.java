/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.search.community.Friend;
import javax.jws.WebMethod;
import javax.jws.WebService;

/**
 * 获得我的朋友及朋友相关操作。
 * @author Long
 */
@WebService
public interface FriendWebService {

    @WebMethod(operationName = "getFriends", exclude = false)
    ListBag<Friend> getFriends(String tokenId, int from, int end);

    @WebMethod(operationName = "getTotalFriends", exclude = false)
    int getTotalFriends(String tokenId);

    @WebMethod(operationName = "requestAsFriend", exclude = false)
    String requestAsFriend(String tokenId, Long mid);
}
