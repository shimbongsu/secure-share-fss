/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.community.services.ibatis;


import com.s7turn.search.community.Message;
import com.s7turn.search.community.MessageBox;
import com.s7turn.search.community.services.MessageService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.sql.SQLException;
import java.util.List;

/**
 *
 * @author Long
 */
public class MessageServiceBean extends CommonServiceBean<Message, Long> implements MessageService{
    public MessageServiceBean(){
        super( Message.class );
    }

    public int sendMessage(Message msg, String sendToList,  List<Long> sendList) throws ServiceException {
        this.saveMessage(msg, sendToList);
        
        msg.setMessageId( msg.getId() );
        
        try{
            this.moveToBox(msg, msg.getFromUser(), "Send");
            getSqlMapClient().update("Message.sendMessageUpdateDraft", msg);

            for( Long toUserId : sendList ){
                msg.setToUser(toUserId);
                msg.setBoxName("Inbox");
                this.getSqlMapClient().insert("Message.sendMessage", msg);
            }
        }catch(SQLException se){
            throw new ServiceException(se.getMessage(), se);
        }
        return 0;
    }

    public void updateStatus(Message msg) throws ServiceException {
        try{
            getSqlMapClient().update("Message.updateMessageReadState", msg);
        }catch(SQLException e){
            throw new ServiceException(e.getMessage(), e);
        }
    }

    
    
    public List<Message> findByDraftMessages(Long userId) throws ServiceException{
        return findByQuery( "Message.findByDraftMessages", userId);
    }

    public List<Message> findByAllMessages(Long userId) throws ServiceException{
        return findByQuery( "Message.findByAllMessages", userId);
    }

    public List<Message> findBySendMessages(Long userId) throws ServiceException{
        return findByQuery( "Message.findBySendMessages", userId);
    }

    public List<Message> findByRecivedMessages(Long userId) throws ServiceException{
        return findByQuery( "Message.findByRecivedMessages", userId);
    }
    
    public List<Message> findByRecivedMessagesWithType(Long userId, String type) throws ServiceException{
        Message m = new Message();
        m.setToUser(userId);
        m.setType(type);
        return findByQuery( "Message.findByRecivedMessagesWithType", m);
    }

    public Message readMessage(Long id) throws ServiceException {
        Message message = null;
        try{
            message = (Message) this.getSqlMapClient().queryForObject("Message.findByMessageBoxId", id);
            if( message.getStatus() == 0 ){
                message.setStatus(Message.STATUS_READED);
                this.updateStatus(message);
            }
        }catch(SQLException e){
            throw new ServiceException(e.getMessage(), e);
        }
        
        return message;
    }

    public Long createBox(String boxName, Long userId) throws ServiceException {
        MessageBox mbox = new MessageBox();
        mbox.setBoxName(boxName);
        mbox.setUserId(userId);
        try{
            this.getSqlMapClient().insert("Message.createBox", mbox);
            return mbox.getId();
        }catch(SQLException e){
            throw new ServiceException(e.getMessage(), e);
        }
    }

    public Long getBoxId(String boxName, Long userId) throws ServiceException {
        MessageBox mbox = new MessageBox();
        mbox.setBoxName(boxName);
        mbox.setUserId(userId);     
        try{
            MessageBox box = (MessageBox)this.getSqlMapClient().queryForObject("Message.selectBox", mbox);
            return box.getId();
        }catch(SQLException e){
            throw new ServiceException(e.getMessage(), e);
        }
    }

    public List<MessageBox> getBoxes(Long userId) throws ServiceException {
        MessageBox mbox = new MessageBox();
        mbox.setUserId(userId);
        try{
            return this.getSqlMapClient().queryForList("Message.selectBoxes", mbox);
        }catch(SQLException e){
            throw new ServiceException(e.getMessage(), e);
        }
    }

    public List<Message> findByBox(Long userId, String boxName) throws ServiceException {
        MessageBox mbox = new MessageBox();
        mbox.setBoxName(boxName);
        mbox.setUserId(userId);
        try{
            return this.getSqlMapClient().queryForList("Message.findByBox", mbox);
        }catch(SQLException e){
            throw new ServiceException(e.getMessage(), e);
        }
    }

    public void moveToBox(Message msg, Long userId, String boxName) throws ServiceException {
        MessageBox mbox = new MessageBox();
        mbox.setBoxName(boxName);
        mbox.setId(msg.getMessageId());
        mbox.setUserId(userId);
        try{
            this.getSqlMapClient().update("Message.moveMessageToBox", mbox);
        }catch(SQLException e){
            throw new ServiceException(e.getMessage(), e);
        }
        
    }

    public void deleteBox(String boxName, Long userId, String moveToBox) throws ServiceException {
        MessageBox box = new MessageBox();
        box.setBoxName(moveToBox);
        box.setOldBoxName(boxName);
        box.setUserId(userId);
        try{
            if( moveToBox == null ){
                //delete the messages in box
                //delete the messagebox,
                this.getSqlMapClient().delete("Message.deleteMessageInBox", box);
                this.getSqlMapClient().delete("Message.deleteBox", box);
            }else{
                //update the message to boxName,
                //delete the messagebox name
                this.getSqlMapClient().delete("Message.updateMessageInBox", box);
                this.getSqlMapClient().delete("Message.deleteBox", box);
            }
        }catch(SQLException e){
            throw new ServiceException(e.getMessage(), e);
        }
    }

    public int saveMessage(Message msg, String sendToList) throws ServiceException {
        msg.setSendToList(sendToList);
        if( msg.getId() == null || msg.getId() == 0 ){
            this.insert(msg);
            msg.setMessageId(msg.getId());
            msg.setBoxName("Draft");
            msg.setToUser(msg.getFromUser());
            try {
                this.getSqlMapClient().insert("Message.sendMessage", msg);
            }catch(SQLException ex){
                throw new ServiceException(ex.getMessage(), ex);
            }
        }else{
            this.update(msg);
        }
        return 0;
    }
    
}
