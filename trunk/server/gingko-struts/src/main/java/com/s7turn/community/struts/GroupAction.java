/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.community.struts;

import com.s7turn.common.struts.ActionBase;
import com.s7turn.common.struts.Constants;
import com.s7turn.search.community.Group;
import com.s7turn.search.community.GroupMember;
import com.s7turn.search.community.GroupTeam;
import com.s7turn.search.community.services.GroupService;
import com.s7turn.search.community.services.GroupTeamService;
import com.s7turn.search.engine.member.User;
import java.util.List;

/**
 *
 * @author Long
 */
public class GroupAction extends ActionBase {
    
    private Group group;
    private GroupService groupService;
    private GroupTeamService teamService;
    private Long groupId;
    private List<GroupMember> members;
    private List<Group> groups;
    private List<GroupTeam> teams;
    private GroupTeam team;
    private String type;

    public GroupAction(){
        this.setNavigator(Constants.NAVIGATOR_MY_FRIEND);
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }


    public List<GroupTeam> getTeams() {
        return teams;
    }

    public GroupTeam getTeam() {
        return team;
    }

    public void setTeam(GroupTeam team) {
        this.team = team;
    }
    
    public void setTeams(List<GroupTeam> teams) {
        this.teams = teams;
    }

    public List<Group> getGroups() {
        return groups;
    }

    public void setGroups(List<Group> groups) {
        this.groups = groups;
    }
    
    
    public Group getGroup() {
        return group;
    }

    public void setGroup(Group group) {
        this.group = group;
    }

    public Long getGroupId() {
        return groupId;
    }

    public void setGroupId(Long groupId) {
        this.groupId = groupId;
    }

    public GroupTeamService getTeamService() {
        return teamService;
    }

    public void setTeamService(GroupTeamService teamService) {
        this.teamService = teamService;
    }

    public GroupService getGroupService() {
        return groupService;
    }

    public void setGroupService(GroupService groupService) {
        this.groupService = groupService;
    }

    public List<GroupMember> getMembers() {
        return members;
    }

    public void setMembers(List<GroupMember> members) {
        this.members = members;
    }
    
    public String create() throws Exception{
        Group g = this.getGroup();
        g.setUserId( this.getCurrentUser().getId() );
        getGroupService().insert(g);
        //getTeamService().addUserToTeam(this.getCurrentUser(), g, gm);
        return "success";
    }
    
    public String update() throws Exception{
        Group g = this.getGroup();

        Group fg = this.getGroupService().findBy(g.getId()); ///we should allow the master of this group to edit this group info.
        if( this.hasMasterPermission(fg ) ){
            if( fg.getUserId().equals(this.getCurrentUser().getId())){
                fg.setGroupType(g.getGroupType());
                fg.setActorId(g.getActorId());
                fg.setDescription(g.getDescription());
                fg.setName(g.getName());
                fg.setTags(g.getTags());
                getGroupService().update(fg);
                return "success";
            }
        }else{
            this.setActionStatus("No rights!");
        }
        return "failed";
    }
        
    public String list() throws Exception{
        this.setActionStatus(SUCCESS);
        if( this.getGroup() == null || (this.getGroup().getId() == null || getGroup().getId() == 0 ) ){
            setGroups( this.getGroupService().findGroupByMember(this.getCurrentUser()) );
        }else {
            if( getTeam() == null || getTeam().getId() == null || getTeam().getId() == 0 ){
                setTeams( this.getTeamService().findAllTeams(this.getGroup(), false) );
            }else{
                setTeams( getTeamService().findChildrenTeams(this.getGroup(), getTeam()));
            }
            setMembers( this.getTeamService().findTeamMembers(this.getGroup(), getTeam() ) );
        }
        return "success";
    }


    public String viewGroup() throws Exception{
        setActionStatus(SUCCESS);
        setGroup(this.getGroupService().findBy(this.getGroupId()));
        return  SUCCESS;
    }


    public String viewTeam() throws Exception{
        setActionStatus(SUCCESS);
        setTeam(this.getTeamService().findBy(this.getTeam().getId()));
        return  SUCCESS;
    }

    public String fetchTeams() throws Exception{
        this.setActionStatus(SUCCESS);

        if( getTeam() == null ){
            setTeams( this.getTeamService().findAllTeams(this.getGroup(), false) );
        }else{
            setTeams( getTeamService().findChildrenTeams(this.getGroup(), getTeam()));
        }
        
        return "success";
    }

    ///the createTeam and updateTeam should be in the Masters Team in this group. if the user is
    ///the owner of the group, she/he is already in this team.

    public String createTeam() throws Exception{
        this.setActionStatus(SUCCESS);
        if( hasMasterPermission(this.getTeam().getGroup()) ){
            if( this.getTeam() != null){
                this.getTeamService().insert(this.getTeam());
            }
        }else{
            this.setActionStatus("No rights!");
        }
        return SUCCESS;
    }
    
    public String updateTeam() throws Exception{
        this.setActionStatus(SUCCESS);
        if( hasMasterPermission(this.getTeam().getGroup()) ){
            if( this.getTeam() != null){
                this.getTeamService().update(this.getTeam());
            }
        }else{
            this.setActionStatus("No rights!");
        }
        return SUCCESS;
    }

    public String delete() throws Exception{
        this.setActionStatus(SUCCESS);
        Group g = this.getGroupService().findBy(this.getGroup().getId());
        if( "group".equals(this.getType()) ){
             if( g.getUserId().equals( this.getCurrentUser().getId() ) ){
                        this.getGroupService().delete(this.getGroup());
             }else{
                 this.setActionStatus("No rights!");
             }
        }else{
            if( hasMasterPermission(g)){
                if( "member".equals(this.getType()) ){
                    User u = new User(this.getUserId());
                    this.getTeamService().removeUserFromTeam(u, getGroup(), getTeam());
                }else if( "team".equals(this.getType())) {
                    GroupTeam t = this.getTeam();
                    t.setGroup(this.getGroup());
                    List<GroupTeam> gts = this.getTeamService().findChildrenTeams(this.getGroup(), t);
                    if( gts != null && gts.size() > 0 ){
                        setActionStatus("This group can not be deleted.");
                    }else{
                        this.getTeamService().delete(this.getTeam());
                    }
                }else if( "group".equals(this.getType()) ){
                    ///the group's owner should be the myself
                }
            }else{
                this.setActionStatus("No rights!");
            }
        }
        return SUCCESS;
    }

    ////request join group should join the group team.
    public String requestJoinGroup() throws Exception{
        User u = this.getUserService().findBy(this.getUserId());
        this.setUser(u);
        if( u.getSecurity() == User.PUBLIC ){
            if( hasMasterPermission(this.getGroup()) ){
                this.getTeamService().addUserToTeam(u, this.getGroup(), getTeam());
            }else{ /// user is member of this
                if( this.getTeamService().isUserInGroup(this.getCurrentUser(), this.getGroup(), null) ){
                    setActionStatus("request");
                }else{
                    setActionStatus("reject");
                }
            }
        }else if( u.getSecurity() == User.REQUEST){
            setActionStatus("request");
        }else{
            setActionStatus("reject");
        }
        return SUCCESS;
    }

    public String accept() throws Exception{
        this.setActionStatus(SUCCESS);
        if( hasMasterPermission( getGroup() ) ){
            User u = this.getUserService().findBy(this.getUserId());
            this.setUser(u);
            this.getTeamService().addUserToTeam(u, this.getGroup(), null);
        }
        return SUCCESS;
    }
    
    private boolean hasMasterPermission(Group gid) throws Exception{
        Group g = gid;
        if( gid.getUserId() == null || gid.getUserId() == 0 ){
            g = this.getGroupService().findBy(gid.getId());
        }

        if( g.getUserId().equals(this.getCurrentUser().getId())){
            return true;
        }
        GroupTeam t = this.getTeamService().findTeam(g, "Masters");
        if( t != null ){
            return this.getTeamService().isUserInGroup(this.getCurrentUser(), g, t);
        }
        return false;
    }
}
