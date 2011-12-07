/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.community.services.ibatis;

import com.s7turn.search.community.Group;
import com.s7turn.search.community.services.GroupService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.util.List;

/**
 *
 * @author Long
 */
public class GroupServiceBean extends CommonServiceBean<Group, Long> implements GroupService{
    public GroupServiceBean(){
        super( Group.class );
    }

    public List<Group> findByOwner(Long userId) throws ServiceException {
        return findByQuery( "Group.findByOwner", userId );
    }

    public List<Group> findGroupByMember(User u) throws ServiceException {
        return findByQuery("Group.findByUser", u);
    }
}
