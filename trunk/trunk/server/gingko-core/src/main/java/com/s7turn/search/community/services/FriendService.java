/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community.services;

import com.s7turn.search.community.Friend;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;

/**
 *
 * @author Long
 */
public interface FriendService extends CommonService<Friend, Long> {
    /**
     * find friend by user and return all friend for this user.
     * @param userId
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<Friend> findByUser( Long userId )  throws ServiceException;

    /**
     *
     * @param userId
     * @param anotherUserId
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    public List<Friend> findFriendTypeById(Long userId,Long anotherUserId) throws ServiceException;
    
    /**
     * find friend by user and return the friends by this type
     * @param userId
     * @param friendType
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<Friend> findByUser( Long userId, String friendType )  throws ServiceException;

    /**
     * the user accept the user as his friends so that
     * @param fr
     * @throws com.s7turn.search.engine.ServiceException
     */
    void acceptFriend( Friend fr ) throws ServiceException;

    /**
     * search the friend in my friend list by the friend's name.
     * @param userId
     * @param name
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<Friend> findByFriendName( Long userId, String name ) throws ServiceException;
}
