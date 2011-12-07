/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community;

import java.io.Serializable;
import java.sql.Timestamp;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.NamedQueries;
import javax.persistence.NamedQuery;
import javax.persistence.Table;

/**
 *
 * @author Long
 */
@Entity
@Table(name="community_messages")
@NamedQueries({
    @NamedQuery(name="Message.findByCode", query="SELECT g FROM Message g WHERE g.subject = :code"),
    @NamedQuery(name="Message.findByPrimaryKey", query="SELECT g FROM Message g WHERE g.id = :id")
})
public class Message  implements Serializable{
    public final static int STATUS_READED=2;
    public final static int STATUS_SENT=3;
    public final static int STATUS_RECEIVED=1;
    public final static int STATUS_REPLIED=4;
    public final static int STATUS_FORWARD=5;
    public final static int STATUS_DRAFT=0;
    public final static int STATUS_DELETED=-1;
    private Long id;
    private String subject;
    private String content;
    private Long fromUser;
    private Long toUser;
    private Long previousId; //related message id.
    private Timestamp sendTime;
    private Timestamp readTime;
    private Timestamp lastUpdatedTime;
    private String boxName;
    private String type;
    private String sendToList;
    private Long messageId;
    private int status; //-1:draft status, 0:received, 1: sent already, 2: reply already, 3: forward

    @Column(name="msg_lastupdated")
    public Timestamp getLastUpdatedTime() {
        return lastUpdatedTime;
    }

    public void setLastUpdatedTime(Timestamp tms) {
        this.lastUpdatedTime = tms;
    }

    public String getSendToList() {
        return sendToList;
    }

    public void setSendToList(String sendToList) {
        this.sendToList = sendToList;
    }

    public String getBoxName() {
        return boxName;
    }

    public void setBoxName(String boxName) {
        this.boxName = boxName;
    }


    @Column(name="message_id")
    public Long getMessageId() {
        return messageId;
    }

    public void setMessageId(Long messageId) {
        this.messageId = messageId;
    }
    
    @Column(name="msg_type")
    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }
    
    @Column(name="msg_content")
    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }

    @Column(name="msg_fromuser")
    public Long getFromUser() {
        return fromUser;
    }

    public void setFromUser(Long fromUser) {
        this.fromUser = fromUser;
    }

    @Column(name="msg_id")
    @Id
    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    @Column(name="msg_relateid")
    public Long getPreviousId() {
        return previousId;
    }

    public void setPreviousId(Long previousId) {
        this.previousId = previousId;
    }

    @Column(name="msg_readtime")
    public Timestamp getReadTime() {
        return readTime;
    }

    public void setReadTime(Timestamp readTime) {
        this.readTime = readTime;
    }

    @Column(name="msg_sendtime")
    public Timestamp getSendTime() {
        return sendTime;
    }

    public void setSendTime(Timestamp sendTime) {
        this.sendTime = sendTime;
    }

    @Column(name="msg_status")
    public int getStatus() {
        return status;
    }

    public void setStatus(int status) {
        this.status = status;
    }

    @Column(name="msg_subject")
    public String getSubject() {
        return subject;
    }

    public void setSubject(String subject) {
        this.subject = subject;
    }

    @Column(name="msg_touser")
    public Long getToUser() {
        return toUser;
    }

    public void setToUser(Long toUser) {
        this.toUser = toUser;
    }
    
}
