/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.community.services.ibatis;

import com.s7turn.search.community.Group;
import com.s7turn.search.community.GroupMember;
import com.s7turn.search.community.GroupTeam;
import com.s7turn.search.community.services.GroupTeamService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.sql.SQLException;
import java.util.List;

/**
 *
 * @author Long
 */
public class GroupTeamServiceBean extends CommonServiceBean<GroupTeam, Long> implements GroupTeamService{

    public GroupTeamServiceBean(){
        super(GroupTeam.class);
    }

    public List<GroupTeam> findAllTeams(Group g, boolean includeChildren) throws ServiceException {
        try{
            if( includeChildren ){
                return this.getSqlMapClient().queryForList("GroupTeam.findAllTeams", g);
            }else{
                return this.getSqlMapClient().queryForList("GroupTeam.findAllRootTeams", g);
            }
        }catch(SQLException se){
            throw new ServiceException(se.getMessage(), se);
        }
    }

    public List<GroupTeam> findChildrenTeams(Group g, GroupTeam gt) throws ServiceException {
        try{
            gt.setGroup(g);
            return this.getSqlMapClient().queryForList("GroupTeam.findChildrenTeams", gt);
        }catch(SQLException se){
            throw new ServiceException(se.getMessage(), se);
        }
    }

    public List<GroupMember> findTeamMembers(Group g, GroupTeam gt) throws ServiceException {
        GroupMember gm = new GroupMember();
        gm.setGroup(g);
        
        try{
            if( gt == null ){
                return this.getSqlMapClient().queryForList("GroupTeam.findMemberInGroup", gm);
            }else{
                gm.setTeam(gt);
                return this.getSqlMapClient().queryForList("GroupTeam.findTeamMembers", gm);
            }
        }catch(SQLException se){
            throw new ServiceException(se.getMessage(), se);
        }
    }

    public void addUserToTeam(User u, Group g, GroupTeam gt) throws ServiceException {
        GroupMember gm = new GroupMember();
        gm.setGroup(g);
        gm.setTeam(gt);
        gm.setUser(u);
        try{
            this.getSqlMapClient().insert("GroupTeam.insertTeamMember", gm);
        }catch(SQLException se){
            throw new ServiceException(se.getMessage(), se);
        }
    }

    public void removeUserFromTeam(User u, Group g, GroupTeam gt) throws ServiceException {
        GroupMember gm = new GroupMember();
        gm.setGroup(g);
        gm.setTeam(gt);
        gm.setUser(u);
        try{
            this.getSqlMapClient().delete("GroupTeam.deleteTeamMember", gm);
        }catch(SQLException se){
            throw new ServiceException(se.getMessage(), se);
        }
    }

    /**
     * if the user is in the group/groupTeam, return true, else return false.
     * @param u
     * @param g
     * @param gt, GroupTeam, nullable, if this value is null, then search the user in this group.
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    public boolean isUserInGroup(User u, Group g, GroupTeam gt) throws ServiceException {
        GroupMember gm = new GroupMember();
        gm.setGroup(g);
        gm.setTeam(gt);
        gm.setUser(u);
        try{
            List lst = this.getSqlMapClient().queryForList("GroupTeam.findMemberInTeam", gm);
            if( lst != null ){
                return lst.size() > 0;
            }
        }catch(SQLException se){
            throw new ServiceException(se.getMessage(), se);
        }
        return false;
    }

    public GroupTeam findTeam(Group g, String teanName) throws ServiceException {
        try{
            GroupTeam gt = new GroupTeam();
            gt.setGroup(g);
            gt.setName(teanName);
            return (GroupTeam) this.getSqlMapClient().queryForObject("GroupTeam.findByTeamName", gt);
        }catch(SQLException se){
            throw new ServiceException(se.getMessage(), se);
        }
    }

    public List<GroupTeam> findTeamByName(Group g, String teanName) throws ServiceException {
        try{
            GroupTeam gt = new GroupTeam();
            gt.setGroup(g);
            gt.setName("%" + teanName + "%");
            return this.getSqlMapClient().queryForList("GroupTeam.findByTeamNameAny", gt);
        }catch(SQLException se){
            throw new ServiceException(se.getMessage(), se);
        }
    }

    public List<GroupTeam> findGroupTeamByMember(User u, Group g) throws ServiceException {
        GroupMember gm = new GroupMember();
        gm.setGroup(g);
        gm.setUser(u);
        return this.findByQuery("GroupTeam.findByMemberInTeam", gm);
    }

}
