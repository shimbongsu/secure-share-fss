/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos;

/**
 *
 * @author Long
 */
public class GingkoPerment {
    private Long gcid;
    private String guid;
    private String publicKey;
    private String privateKey;
    private boolean deleted;

    public Long getGcid() {
        return gcid;
    }

    public void setGcid(Long gcid) {
        this.gcid = gcid;
    }

    public String getGuid() {
        return guid;
    }

    public void setGuid(String guid) {
        this.guid = guid;
    }

    public String getPrivateKey() {
        return privateKey;
    }

    public void setPrivateKey(String privateKey) {
        this.privateKey = privateKey;
    }

    public String getPublicKey() {
        return publicKey;
    }

    public void setPublicKey(String publicKey) {
        this.publicKey = publicKey;
    }

    public boolean isDeleted() {
        return deleted;
    }

    public void setDeleted(boolean deleted) {
        this.deleted = deleted;
    }


    
}
