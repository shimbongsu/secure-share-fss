/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.community.struts;

import com.s7turn.common.struts.ActionBase;
import com.s7turn.search.community.Message;
import com.s7turn.search.community.MessageBox;
import com.s7turn.search.community.services.MessageService;
import com.s7turn.search.engine.member.User;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Long
 */
public class MessageAction extends ActionBase {
    
    private MessageService messageService;
    private Long messageId;
    private Long id;
    private Message message;
    private List<Message> messages;
    private Long[] friendId;
    private Long toUserId;
    private String messageType;
    private String toUsers;
    private String boxName;
    private String moveToBox;
    private List<MessageBox> boxes;

    public List<MessageBox> getBoxes() {
        return boxes;
    }

    public void setBoxes(List<MessageBox> boxes) {
        this.boxes = boxes;
    }

    

    public String getMoveToBox() {
        return moveToBox;
    }

    public void setMoveToBox(String moveToBox) {
        this.moveToBox = moveToBox;
    }

    
    public String getBoxName() {
        return boxName;
    }

    public void setBoxName(String boxName) {
        this.boxName = boxName;
    }

    
    public MessageAction(){
        this.setNavigator("account");
    }

    public String getMessageType() {
        return messageType;
    }

    public void setMessageType(String type) {
        this.messageType = type;
    }

    public String getToUsers() {
        return toUsers;
    }

    public void setToUsers(String toUsers) {
        this.toUsers = toUsers;
    }
    
    public Long getToUserId() {
        return toUserId;
    }

    public void setToUserId(Long fromId) {
        this.toUserId = fromId;
    }
    
    public Long[] getFriendId() {
        return friendId;
    }

    public void setFriendId(Long[] friendId) {
        this.friendId = friendId;
    }
    
    public List<Message> getMessages() {
        return messages;
    }

    public void setMessages(List<Message> messages) {
        this.messages = messages;
    }
    
    protected List<Long> getToUserList(){
        String s = this.getToUsers();
        String[] ids = s.split(";");
        List<Long> vvs = new ArrayList<Long>();
        for( int i = 0; i < ids.length; i ++ ){
            String item = ids[i];
            try{
                Long lId = Long.valueOf(item);
                vvs.add(lId);
            }catch(Exception ex){
                try{
                    User u = this.getUserService().findByCode(item);
                    vvs.add(u.getId());
                }catch(Exception emx){}
            }
        }
        return vvs;
    }

    

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public Message getMessage() {
        return message;
    }

    public void setMessage(Message message) {
        this.message = message;
    }

    public Long getMessageId() {
        return messageId;
    }

    public void setMessageId(Long messageId) {
        this.messageId = messageId;
    }

    public MessageService getMessageService() {
        return messageService;
    }

    public void setMessageService(MessageService messageService) {
        this.messageService = messageService;
    }

    
    
    public String list() throws Exception{
        setActionStatus(SUCCESS);
        List<Message> msgList = null;

        if( isEmpty( this.getBoxName() ) ) {
            this.setBoxName("inbox");
        }

        msgList = this.getMessageService().findByBox(this.getCurrentUser().getId(),
                this.getBoxName());

        this.setMessages(computer(msgList));
        
        return "success";
    }
    
    public String read() throws Exception{
        setActionStatus(SUCCESS);
        Message msg = getMessageService().findBy( this.getId() );
        this.setMessage(msg);
        return "success";
    }
        
    public String delete()  throws Exception{
        this.setActionStatus(SUCCESS);
        Message md = new Message();
        md.setId(this.getId());
        md.setToUser(this.getCurrentUser().getId());
        getMessageService().delete(md);
        return "success";
    }

    public String move() throws Exception{
        this.setActionStatus(SUCCESS);
        this.getMessageService().moveToBox(this.getMessage(), 
                this.getCurrentUser().getId(), getBoxName());
        return "success";
    }

    public String createBox() throws Exception {
        this.setActionStatus(SUCCESS);
        this.getMessageService().createBox(this.getBoxName(), 
                this.getCurrentUser().getId());
        return "success";
    }

    public String deleteBox() throws Exception {
        this.setActionStatus(SUCCESS);
        this.getMessageService().deleteBox(this.getBoxName(),
                this.getCurrentUser().getId(), getMoveToBox());
        return "success";
    }

    public String messageBoxes() throws Exception{
        this.setActionStatus(SUCCESS);
        this.setBoxes( this.getMessageService().getBoxes(getCurrentUser().getId()) );
        return SUCCESS;
    }
    
    public String saveAsDraft()  throws Exception{
        this.setActionStatus(SUCCESS);
        Message msg = getMessage();
        msg.setStatus( Message.STATUS_DRAFT );
        this.getMessageService().saveMessage(msg, this.getToUsers());
        return "success";
    }
    
    public String send() throws Exception{
        this.setActionStatus(SUCCESS);
        Message msg = getMessage();
        msg.setFromUser( this.getCurrentUser().getId() );
        msg.setStatus( Message.STATUS_RECEIVED );
        getMessageService().sendMessage(msg, getToUsers(), getToUserList());
        return "success";
    }
    
    public String mock() throws Exception {
        return "success";
    }
    
}
