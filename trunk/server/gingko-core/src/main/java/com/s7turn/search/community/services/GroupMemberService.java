/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community.services;

import com.s7turn.search.community.GroupMember;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;

/**
 *
 * @author Long
 */
public interface GroupMemberService extends CommonService<GroupMember, Long> {
    /**
     * find all members for a group
     * @param groupId
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<GroupMember> findByGroup( Long groupId ) throws ServiceException;
    
    /**
     * find all members who has the role for a group.
     * @param groupId
     * @param role
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<GroupMember> findByGroupWithRole( Long groupId, String role ) throws ServiceException;
}
