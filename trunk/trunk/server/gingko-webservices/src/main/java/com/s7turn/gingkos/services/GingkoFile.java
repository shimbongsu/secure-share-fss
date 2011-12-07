/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.GingkoContent;
import java.util.Calendar;
import java.util.Date;

/**
 *
 * @author Long
 */
public class GingkoFile {

    private String              name;
    private GingkoFile          parent;
    private Date                createDate;
    private Date                lastUpdate;
    private String              description;
    private String              fileType;
    private String              mimeType;
    private String              uuid;
    private GingkoPermission    permission;
    private String              thumbView;
    private String              iconView;
    
    public GingkoFile(){
        createDate = Calendar.getInstance().getTime();
        lastUpdate = Calendar.getInstance().getTime();
        permission = GingkoPermission.DefaultPermssion;
    }

    public GingkoFile(GingkoContent ct)
    {
        uuid = ct.getGuid();
        name = ct.getName();
        createDate = ct.getCreateTime();
        lastUpdate = ct.getLastUpdated();
        description = ct.getDescription();
        thumbView = ct.getThumb();
        iconView = ct.getIcon();
        fileType = ct.getType();
        mimeType = ct.getMimeType();
        permission = GingkoPermission.DefaultPermssion;
    }

    public boolean isFolder()
    {
        return false;
    }

    public String getIconView() {
        return iconView;
    }

    public void setIconView(String iconView) {
        this.iconView = iconView;
    }

    public String getThumbView() {
        return thumbView;
    }

    public void setThumbView(String thumbView) {
        this.thumbView = thumbView;
    }

    public String getGuid() {
        return uuid;
    }

    public void setGuid(String uuid) {
        this.uuid = uuid;
    }



    public Date getCreateDate() {
        return createDate;
    }

    public void setCreateDate(Date createDate) {
        this.createDate = createDate;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public String getName() {
        return name;
    }

    public void setName(String fileName) {
        this.name = fileName;
    }

    public String getType() {
        return fileType;
    }

    public void setType(String fileType) {
        this.fileType = fileType;
    }

    public String getMimeType() {
        return mimeType;
    }

    public void setMimeType(String mimeType) {
        this.mimeType = mimeType;
    }

    public GingkoFile getParent() {
        return parent;
    }

    public void setParent(GingkoFile parent) {
        this.parent = parent;
    }

    public GingkoPermission getPermission() {
        return permission;
    }

    public void setPermission(GingkoPermission permission) {
        this.permission = permission;
    }

    

    public static GingkoFile fromGingkoContent(GingkoContent ct)
    {
        if( ct.isFolder() )
        {
            return null;
        }else{
            return new GingkoFile(ct);
        }        
    }
}
