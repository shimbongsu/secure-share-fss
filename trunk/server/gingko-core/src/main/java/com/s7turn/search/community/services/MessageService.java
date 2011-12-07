/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community.services;

import com.s7turn.search.community.Message;
import com.s7turn.search.community.MessageBox;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;

/**
 *
 * @author Long
 */
public interface MessageService extends CommonService<Message, Long> {
    
    int sendMessage( Message msg, String sendToList, List<Long> sendList ) throws ServiceException;

    int saveMessage(Message msg, String sendToList) throws ServiceException;
    
    Message readMessage( Long id )  throws ServiceException;
    
    void updateStatus( Message msg ) throws ServiceException;
    
    //List<Message> findByDraftMessages(Long userId) throws ServiceException;
    
    //List<Message> findByAllMessages(Long userId) throws ServiceException;
    
    //List<Message> findBySendMessages(Long userId) throws ServiceException;
    
    //List<Message> findByRecivedMessages(Long userId) throws ServiceException;
    
    //List<Message> findByRecivedMessagesWithType(Long userId, String type) throws ServiceException;

    Long createBox(String boxName, Long userId) throws ServiceException;
    void deleteBox(String boxName, Long userId, String moveToBox) throws ServiceException;
    Long getBoxId(String boxName, Long userId) throws ServiceException;
    List<MessageBox> getBoxes( Long userId) throws ServiceException;
    List<Message> findByBox(Long userId, String boxName) throws ServiceException;
    void moveToBox(Message msg, Long userId, String boxName) throws ServiceException;
}
