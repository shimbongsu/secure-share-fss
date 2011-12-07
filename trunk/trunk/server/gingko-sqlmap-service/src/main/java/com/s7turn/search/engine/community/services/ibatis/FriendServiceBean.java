/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.community.services.ibatis;

import com.s7turn.search.community.Friend;
import com.s7turn.search.community.services.FriendService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.sql.SQLException;
import java.util.List;

/**
 *
 * @author Long
 */
public class FriendServiceBean extends CommonServiceBean<Friend, Long> implements FriendService{
    public FriendServiceBean(){
        super( Friend.class );
    }

    public List<Friend> findByUser(Long userId) throws ServiceException {
        return findByQuery("Friend.findByUser", userId);
    }

    public List<Friend> findByUser(Long userId, String friendType) throws ServiceException {
        Friend frd = new Friend();
        frd.setUserId(userId);
        frd.setFriendType(friendType);
        return findByQuery("Friend.findByUserWithType", frd);
    }

    public List<Friend> findFriendTypeById(Long userId,Long anotherUserId) throws ServiceException {
        Friend frd = new Friend();
        User friendUser = new User(anotherUserId);
        frd.setUserId(userId);
        frd.setFriendUser(friendUser);
        return findByQuery("Friend.findFriendTypeById", frd);
    }


    public void acceptFriend(Friend fr) throws ServiceException {
        try{
            this.getSqlMapClient().update("Friend.acceptFriend", fr);
        }catch(Exception e){
            throw new ServiceException(e.getMessage(), e);
        }
    }

    public List<Friend> findByFriendName(Long userId, String name) throws ServiceException {
        Friend fr = new Friend();
        fr.setUserId(userId);
        User us = new User();
        us.setFullName("%" + name.trim() + "%" );
        fr.setFriendUser(us);
        try{
            return this.getSqlMapClient().queryForList("Friend.findByUserWithFriendName", fr);
        }catch(SQLException e){
            throw new ServiceException(e.getMessage(), e);
        }
    }
}
