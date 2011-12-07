/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import javax.jws.WebResult;

/**
 * 
 * @author Long
 */

public class GingkoPermission {
    public static GingkoPermission DefaultPermssion = new GingkoPermission(false);
    public final static String STATUS_SUCCESS = "SUCCESSED";
    public final static String STATUS_FAILED = "FAILED";
    public final static String STATUS_ERROR = "ERROR";
    private String status;
    private String certification;
    private boolean readable;           //Read this file
    private boolean printable;          //Print this file
    private boolean copyable;           //Copy this file to another.
    private boolean writable;           //Update this file.
    private boolean deleted;            //This file was deleted.
    private boolean owner;
    private boolean holder;

    private GingkoPermission(boolean df)
    {
        readable = false;
        printable = false;
        copyable = false;
        writable = false;
        owner = false;
        holder = false;
        deleted = df;
        certification = "";
    }

    public GingkoPermission()
    {
        this.certification = "";
        this.holder = false;
        this.owner = false;
        this.writable = false;
        this.readable = false;
        this.printable = false;
        this.copyable = false; //Copy/Paste
        this.deleted = false;
        status = "SUCCESSED";
    }

    public GingkoPermission(String pms,boolean o, boolean h,  boolean w, boolean r, boolean del, boolean ca, boolean pa)
    {
        this.certification = pms;
        this.owner = o;
        this.holder = h;
        this.writable = w;
        this.readable = r;
        this.printable = pa;
        this.copyable = ca; //Copy/Paste
        this.deleted = del;
        status = "SUCCESSED";
    }

    public String getStatus() {
        return status;
    }

    public void setStatus(String status) {
        this.status = status;
    }

    public boolean isDeleted() {
        return deleted;
    }

    public void setDeleted(boolean deleted) {
        this.deleted = deleted;
    }

    public String getCertification() {
        return certification;
    }

    public void setCertification(String certification) {
        this.certification = certification;
    }

    public boolean isCopyable() {
        return copyable;
    }

    public void setCopyable(boolean copyable) {
        this.copyable = copyable;
    }

    public boolean isPrintable() {
        return printable;
    }

    public void setPrintable(boolean printable) {
        this.printable = printable;
    }

    public boolean isReadable() {
        return readable;
    }

    public void setReadable(boolean readable) {
        this.readable = readable;
    }

    public boolean isWritable() {
        return writable;
    }

    public void setWritable(boolean writable) {
        this.writable = writable;
    }

    public boolean isHolder() {
        return holder;
    }

    public void setHolder(boolean holder) {
        this.holder = holder;
    }

    public boolean isOwner() {
        return owner;
    }

    public void setOwner(boolean owner) {
        this.owner = owner;
    }


}
