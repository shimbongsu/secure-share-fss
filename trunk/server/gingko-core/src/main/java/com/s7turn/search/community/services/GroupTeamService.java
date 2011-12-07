/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community.services;

import com.s7turn.search.community.Group;
import com.s7turn.search.community.GroupMember;
import com.s7turn.search.community.GroupTeam;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;

/**
 *
 * @author Long
 */
public interface GroupTeamService extends CommonService<GroupTeam, Long> {
    List<GroupTeam> findAllTeams(Group g, boolean includeChildren) throws ServiceException;
    List<GroupTeam> findChildrenTeams(Group g, GroupTeam gt) throws ServiceException;
    GroupTeam findTeam(Group g, String teanName) throws ServiceException;
    List<GroupMember> findTeamMembers(Group g, GroupTeam gt) throws ServiceException;
    List<GroupTeam> findTeamByName(Group g, String teanName) throws ServiceException;
    List<GroupTeam> findGroupTeamByMember( User u, Group g )  throws ServiceException;
    void addUserToTeam(User u, Group g, GroupTeam gt ) throws ServiceException;
    void removeUserFromTeam(User u, Group g, GroupTeam gt ) throws ServiceException;
    boolean isUserInGroup(User u, Group g, GroupTeam gt) throws ServiceException;
}
