/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community;

import java.io.Serializable;

/**
 *
 * @author Long
 */
public class MessageBox implements Serializable{
    private Long id;
    private String boxName;
    private String oldBoxName;
    private Long userId;

    public String getBoxName() {
        return boxName;
    }

    public void setBoxName(String boxName) {
        this.boxName = boxName;
    }

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getOldBoxName() {
        return oldBoxName;
    }

    public void setOldBoxName(String oldBoxName) {
        this.oldBoxName = oldBoxName;
    }

    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }
    
}
