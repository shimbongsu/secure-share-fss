/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.community.services.ibatis;

import com.s7turn.search.community.GroupMember;
import com.s7turn.search.community.services.GroupMemberService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.util.List;

/**
 *
 * @author Long
 */
public class GroupMemberServiceBean extends CommonServiceBean<GroupMember, Long> implements GroupMemberService{
    public GroupMemberServiceBean(){
        super( GroupMember.class );
    }

    public List<GroupMember> findByGroup(Long groupId) throws ServiceException {
        return this.findByQuery("GroupMember.findByGroup", groupId);
    }

    public List<GroupMember> findByGroupWithRole(Long groupId, String role) throws ServiceException {
        GroupMember gm = new GroupMember();
        //gm.setGroupId(groupId);
        gm.setRole(role);
        return this.findByQuery("GroupMember.findByGroupWithRole", gm);
    }
}
