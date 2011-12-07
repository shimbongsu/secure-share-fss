/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.community.struts;

import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.member.services.UserService;
import com.s7turn.common.struts.ActionBase;
import com.s7turn.common.struts.Constants;
import com.s7turn.search.community.Friend;
import com.s7turn.search.community.services.FriendService;
import com.s7turn.search.engine.member.UserDetail;
import com.s7turn.search.engine.member.services.UserDetailService;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 *
 * @author Long
 */
public class FriendAction extends ActionBase {
    
    private User user;
    private Long userId;
    private UserService userService;
    private Friend friend;
    private Long friendId;
    private List<Friend> friends;
    private List<UserDetail> userDetails;
    private FriendService friendService;
    private UserDetailService userDetailService;
    private String userSearch;
    private String accept;
    private Long id;

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }


    public String getAccept() {
        return accept;
    }

    public void setAccept(String accept) {
        this.accept = accept;
    }


    public UserDetailService getUserDetailService() {
        return userDetailService;
    }

    public void setUserDetailService(UserDetailService userDetailService) {
        this.userDetailService = userDetailService;
    }
    
    public FriendAction(){
        this.setNavigator(Constants.NAVIGATOR_MY_FRIEND);
    }

    public String getUserSearch() {
        return userSearch;
    }

    public void setUserSearch(String userSearch) {
        this.userSearch = userSearch;
    }

    public Friend getFriend() {
        return friend;
    }

    public void setFriend(Friend friend) {
        this.friend = friend;
    }

    public Long getFriendId() {
        return friendId;
    }

    public void setFriendId(Long friendId) {
        this.friendId = friendId;
    }

    public FriendService getFriendService() {
        return friendService;
    }

    public void setFriendService(FriendService friendService) {
        this.friendService = friendService;
    }

    public List<Friend> getFriends() {
        return friends;
    }

    public List<UserDetail> getUserDetails() {
        return userDetails;
    }

    public void setUserDetails(List<UserDetail> userDetails) {
        this.userDetails = userDetails;
    }

    public void setFriends(List<Friend> friends) {
        this.friends = friends;
    }

    /**
     * Accept the request user as my friend.
     * because the user's setting special
     * @return
     * @throws java.lang.Exception
     */
    public String acceptFriend() throws Exception{
        this.setActionStatus(ActionBase.SUCCESS);
        User curUser = this.getCurrentUser();
        if( curUser != null ){
            if( getFriendId() != null  && getFriendId() > 0 ){
                if( this.getAccept() != null && this.getAccept().equals("accept")){
                    Friend fr = new Friend();
                    fr.setUserId(curUser.getId());
                    fr.setFriendUser(new User(getFriendId()));
                    fr.setFriendType("FRIEND");
                    getFriendService().insert(fr); //WILL SHOULD EXCEPTION FOR FRIEND EXISTS.
                    Friend rf = new Friend();
                    rf.setUserId( getFriendId() );
                    rf.setFriendUser( curUser );
                    rf.setFriendType("FRIEND");
                    getFriendService().acceptFriend(rf); //WILL SHOULD EXCEPTION FOR FRIEND EXISTS.
                }else{
                    Friend rf = new Friend();
                    rf.setUserId( getFriendId() );
                    rf.setFriendUser( curUser );
                    rf.setFriendType("Rejected");
                    getFriendService().acceptFriend(rf); //WILL SHOULD EXCEPTION FOR FRIEND EXISTS.
                    this.setFriend(rf);
                }
                return "success";
            }
        }
        return "failed";
    }

    public String requestFriend() throws Exception {
        this.setActionStatus(ActionBase.SUCCESS);
        User curUser = this.getCurrentUser();
        if( curUser != null ){
            if( getFriendId() != null  && getFriendId() > 0 ){
                Friend fr = new Friend();
                User frUser = getUserService().findBy(getFriendId());
                fr.setUserId(curUser.getId());
                fr.setFriendUser(frUser);
                fr.setFriendType("FRIEND");

                if( frUser.getSecurity() == User.PUBLIC ){
                    ///add this as friend directly.
                    getFriendService().insert(fr); //WILL SHOULD EXCEPTION FOR FRIEND EXISTS.
                     this.setActionStatus("success");
                }else if( frUser.getSecurity() == User.PRIVATE ){
                    ///reject this directly.
                    this.setActionStatus("reject");
                }else if( frUser.getSecurity() == User.REQUEST ){
                    ///return the friends and ask the friend to send a message directly.
                    fr.setFriendType("Wait for accepting.");
                    getFriendService().insert(fr); //WILL SHOULD EXCEPTION FOR FRIEND EXISTS.
                    this.setActionStatus("request");
                }
                this.setFriend(fr);
                return "success";
            }

        }
        return "failed";
    }
    
    public String list() throws Exception{
        this.setActionStatus(ActionBase.SUCCESS);
        if( this.getUserId() == null || this.getUserId() == 0){
            if( isEmpty(this.getUserSearch()) ){
                setFriends( this.getFriendService().findByUser(this.getCurrentUser().getId()) );
            }else{
                setFriends( this.getFriendService().findByFriendName(this.getCurrentUser().getId(), getUserSearch()) );
            }
        }else{
            if( isEmpty(this.getUserSearch()) ){
                setFriends( this.getFriendService().findByUser(this.getUserId()) );
            }else{
                setFriends( this.getFriendService().findByFriendName(this.getUserId(), getUserSearch()) );
            }
        }
        return "success";
    }

    public String tree() throws Exception{
       
        
       return "success";
    }
    
    public String delete() throws Exception {
        this.setActionStatus(ActionBase.SUCCESS);
        Friend fr = new Friend();
        fr.setUserId(this.getCurrentUser().getId());
        fr.setFriendUser(new User(this.getFriendId()));
        fr.setId(getId());
        this.getFriendService().delete(fr);
        return SUCCESS;
    }

    public String mock() throws Exception {
        return "success";
    }


    public String findUsers() throws Exception{
        String key = this.getUserSearch();
        this.setUserDetails( this.computer( this.getUserDetailService().findUsers(key) ) );
        this.setActionStatus(ActionBase.SUCCESS);
        return ActionBase.SUCCESS;
    }

    public String findFriends() throws Exception {
        String key = this.getUserSearch();
        Map friendmap = new HashMap();
        //  conds.put("UserDetail.id", this.getCurrentUser().getId());
        List<UserDetail> searchResutl = this.computer(this.getUserDetailService().findUsers(key));
       // if (searchResutl != null && searchResutl.size() > 0) {
            List<Friend> myfriends = this.getFriendService().findByUser(this.getCurrentUser().getId());
            friendmap.putAll(buildMap(myfriends, "friendUser.id"));
            for (UserDetail detail : searchResutl) {
                Friend myfriend = (Friend) friendmap.get(detail.getId());
                if (myfriend != null) {
                    detail.setFriendType(myfriend.getFriendType());
                }
            }

            this.setUserDetails(searchResutl);

//        }else{
//
//        }
        this.setActionStatus(ActionBase.SUCCESS);

        return ActionBase.SUCCESS;
    }
    
}
