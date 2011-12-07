/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos;

import com.s7turn.search.engine.member.User;
import java.io.Serializable;
import java.sql.Timestamp;

/**
 *
 * @author Long
 */
public class GingkoContent implements Serializable, Cloneable
{
    private String guid;                    /// the guid of this content
    private String name;                    /// the name of content,
    private String type;                    /// the type of content, the type will be used for fetch out the system contents 
    private boolean folder;                 /// is this a folder?
    private String mimeType;                /// browser base mimetype
    private long length;                    /// content length /// folder's length should the number of records
    private String description;             /// briefing of this content. 400
    private String tags;                    /// the tags of this content.
    private byte[] contents;                /// the contents. if the content is stored out of this table, the content should include the path/table name / provider name.
    private String storage;                 /// storage of system.
    private String icon;                    /// url of icon
    private String thumb;                   /// url of thumb
    private String url;                     /// url of this content which will be used for read the detail.
    private User owner;                     /// the content's owner
    private Timestamp createTime;           /// the create time. only generated on create time.
    private Timestamp lastUpdated;          /// the Latest updated time
    private String version;                 /// the version of content
    private boolean deleted;                /// is the content deleted
    private long relatedId;                 /// if ths content related to another table. this field should be maintained by provide.
    private Object extendObject;            /// the pojo was extend from contents. //don't store only for read
    private GingkoContent parent;           /// the parent folder of this content.
    private String companyId;

    public String getGuid() {
        return guid;
    }

    public void setGuid(String guid) {
        this.guid = guid;
    }

    
    public String getCompanyId() {
        return companyId;
    }

    public void setCompanyId(String encoding) {
        this.companyId = encoding;
    }

    public GingkoContent getParent() {
        return parent;
    }

    public void setParent(GingkoContent parent) {
        this.parent = parent;
    }

    public byte[] getContents() {
        return contents;
    }

    public void setContents(byte[] contents) {
        this.contents = contents;
    }

    public Timestamp getCreateTime() {
        return createTime;
    }

    public void setCreateTime(Timestamp createTime) {
        this.createTime = createTime;
    }

    public boolean isDeleted() {
        return deleted;
    }

    public void setDeleted(boolean deleted) {
        this.deleted = deleted;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public Object getExtendObject() {
        return extendObject;
    }

    public void setExtendObject(Object extendObject) {
        this.extendObject = extendObject;
    }

    public boolean isFolder() {
        return folder;
    }

    public void setFolder(boolean folder) {
        this.folder = folder;
    }

    public String getIcon() {
        return icon;
    }

    public void setIcon(String icon) {
        this.icon = icon;
    }

    public Timestamp getLastUpdated() {
        return lastUpdated;
    }

    public void setLastUpdated(Timestamp lastUpdated) {
        this.lastUpdated = lastUpdated;
    }

    public long getLength() {
        return length;
    }

    public void setLength(long length) {
        this.length = length;
    }

    public String getMimeType() {
        return mimeType;
    }

    public void setMimeType(String mimeType) {
        this.mimeType = mimeType;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public User getOwner() {
        return owner;
    }

    public void setOwner(User owner) {
        this.owner = owner;
    }

    public long getRelatedId() {
        return relatedId;
    }

    public void setRelatedId(long relatedId) {
        this.relatedId = relatedId;
    }

    public String getStorage() {
        return storage;
    }

    public void setStorage(String storage) {
        this.storage = storage;
    }

    public String getTags() {
        return tags;
    }

    public void setTags(String tags) {
        this.tags = tags;
    }

    public String getThumb() {
        return thumb;
    }

    public void setThumb(String thumb) {
        this.thumb = thumb;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;
    }

    public String getVersion() {
        return version;
    }

    public void setVersion(String version) {
        this.version = version;
    }
    
    @Override
    public Object clone() throws CloneNotSupportedException{
        return super.clone(); 
    }
    
}
