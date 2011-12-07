/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.struts;

import com.s7turn.common.struts.ActionBase;
import com.s7turn.gingkos.Gingko;
import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.services.GingkoComponentService;
import com.s7turn.search.engine.PhysicalFile;
import com.s7turn.search.engine.member.User;
import java.util.List;

/**
 *
 * @author Long
 */
public class GingkoAction extends ActionBase {
    private GingkoComponentService componentService;
    private List<GingkoContent> folders;
    private List<GingkoContent> contents;
    private String id;
    private String folderIcon;
    private String folderThumb;
    
    private GingkoContent content;
    
    private String name;
    private String description;
    private String parentId;
    private String tags;
    private String type;
    private Gingko gingko;
    private List<Gingko> gingkos;

    public String getFolderIcon() {
        return folderIcon;
    }

    public void setFolderIcon(String folderIcon) {
        this.folderIcon = folderIcon;
    }

    public String getFolderThumb() {
        return folderThumb;
    }

    public void setFolderThumb(String folderThumb) {
        this.folderThumb = folderThumb;
    }

    public GingkoContent getContent() {
        return content;
    }

    public void setContent(GingkoContent content) {
        this.content = content;
    }

    public Gingko getGingko() {
        return gingko;
    }

    public void setGingko(Gingko gingko) {
        this.gingko = gingko;
    }

    public List<Gingko> getGingkos() {
        return gingkos;
    }

    public void setGingkos(List<Gingko> gingkos) {
        this.gingkos = gingkos;
    }

    
    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    
    public List<GingkoContent> getContents() {
        return contents;
    }

    public void setContents(List<GingkoContent> contents) {
        this.contents = contents;
    }


    public String getTags() {
        return tags;
    }

    public void setTags(String tags) {
        this.tags = tags;
    }



    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
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

    public void setName(String name) {
        this.name = name;
    }

    public String getParentId() {
        return parentId;
    }

    public void setParentId(String parentId) {
        this.parentId = parentId;
    }
    
    public GingkoComponentService getComponentService() {
        return componentService;
    }

    public void setComponentService(GingkoComponentService componentService) {
        this.componentService = componentService;
    }
    
    public List<GingkoContent> getFolders(){
        return folders;
    }
    
    public void setFolders( List<GingkoContent> rts ){
        folders = rts;
    }
        
    public String fetch() throws Exception {
        User user = this.getCurrentUser();
        GingkoContent gct = new GingkoContent();
        if( this.getId() == null ){
            this.setFolders(computer( this.getComponentService().getRootFolders(user)));
        }else{
            gct.setGuid(this.getId());
            gct.setFolder(true);
            gct.setOwner(this.getCurrentUser());
            this.setFolders(computer( this.getComponentService().getContents(gct, user, true) ));
        }
        return ActionBase.SUCCESS;
    }

    /**
     * list all contents includes folder and contents
     * @return
     * @throws java.lang.Exception
     */
    public String list() throws Exception {
        User user = this.getCurrentUser();
        GingkoContent gct = new GingkoContent();
        if( this.getId() == null ){
            this.setContents(computer( this.getComponentService().getRootFolders(user)));
        }else{
            gct.setGuid(this.getId());
            gct.setFolder(true);
            gct.setOwner(this.getCurrentUser());
            this.setContents(computer( this.getComponentService().getContents(gct, user, false) ));
        }
        return ActionBase.SUCCESS;
    }

    public String delete() throws Exception{
        GingkoContent gct = new GingkoContent();
        gct.setGuid(this.getId());
        gct.setOwner(this.getCurrentUser()); ///only the onwer
        gct.setType(this.getType());
        gct.setFolder(true);
        List<GingkoContent> ctxes = getComponentService().getContents(gct, this.getCurrentUser(), false);
        if( ctxes != null && ctxes.size() > 0 ){
            this.setActionStatus("Failed. There are some contents under this folder.");
        }else{
            getComponentService().lookup(gct).getContentService().delete(gct);
            this.setActionStatus(ActionBase.SUCCESS);
        }
        return ActionBase.SUCCESS;
    }

    public String saveGingko() throws Exception{
        if( this.getGingko() != null && this.getId() != null ){
            GingkoContent gct = new GingkoContent();
            gct.setGuid(getId());
            gct.setOwner(this.getCurrentUser()); ///only the onwer
            gct.setType(this.getType());
            getComponentService().lookup(gct).addGingko(gct, this.getGingko());

            this.setActionStatus(ActionBase.SUCCESS);
            return ActionBase.SUCCESS;
        }else{
            this.setActionStatus(ActionBase.INPUT);
            return ActionBase.INPUT;
        }
    }

    public String acquireGingkos() throws Exception{
        GingkoContent gct = new GingkoContent();
        gct.setGuid(getId());
        gct.setOwner(this.getCurrentUser()); ///only the onwer
        gct.setType(this.getType());
        setGingkos( getComponentService().lookup(gct).getGingkos(gct) );
        this.setActionStatus(ActionBase.SUCCESS);
        return ActionBase.SUCCESS;
    }

    public String removeGingko() throws Exception{
        this.setActionStatus(ActionBase.SUCCESS);
        GingkoContent gctx = new GingkoContent();
        gctx.setGuid(getId());
        getComponentService().lookup(gctx).removeGingko(gctx, this.getGingko());
        return ActionBase.SUCCESS;
    }

    public String createFolder() throws Exception{
        GingkoContent gctx = new GingkoContent();
        gctx.setFolder(true);
        gctx.setName(this.getName());
        gctx.setDescription(this.getDescription());
        if( this.getParentId() != null ){
            GingkoContent parent = new GingkoContent();
            parent.setGuid(this.getParentId());
            gctx.setParent(parent);
        }
        gctx.setTags(this.getTags());
        gctx.setIcon(this.getFolderIcon());
        gctx.setThumb(this.getFolderThumb());
        gctx.setOwner(this.getCurrentUser());
        getComponentService().lookup(gctx).getContentService().insert(gctx);
        setActionStatus(ActionBase.SUCCESS);
        return ActionBase.SUCCESS;
    }

    public String updateFolder() throws Exception{
        GingkoContent gctx = new GingkoContent();
        gctx.setGuid(this.getId());
        gctx.setName(this.getName());
        gctx.setDescription(this.getDescription());
        gctx.setTags(this.getTags());
        setActionStatus(ActionBase.SUCCESS);
        GingkoContent retctx = getComponentService().lookup(gctx).getContentService().findBy(this.getId());
        if( retctx != null ){
            if( retctx.getOwner()  != null && retctx.getOwner().getId().equals(this.getCurrentUser().getId()) ){
                retctx.setDescription(this.getDescription());
                retctx.setTags(this.getTags());
                retctx.setName(this.getName());
                getComponentService().lookup(retctx).getContentService().update(retctx);
                setActionStatus(ActionBase.SUCCESS);
            }else{
                ///actually, we need to check the gingko, which is 'edit' privallige.
                setActionStatus("You have not rights to modify this folder.");
            }
        }
        return ActionBase.SUCCESS;
    }

    public String get() throws Exception{
        setActionStatus(ActionBase.SUCCESS);
        GingkoContent gctx = new GingkoContent();
        //gctx.setGcid(this.getId());
        GingkoContent retctx = getComponentService().lookup(gctx).getContentService().findBy(this.getId());
        this.setContent(retctx);
        return ActionBase.SUCCESS;
    } 

    public String download() throws Exception{
        setActionStatus(ActionBase.SUCCESS);
        GingkoContent gctx = new GingkoContent();
        gctx.setGuid(this.getId());
        GingkoContent retctx = getComponentService().lookup(gctx).getContentService().findBy(this.getId());
        if( retctx != null ){
            PhysicalFile phfile = this.getPhysicalFileService().findBy(retctx.getRelatedId());
            phfile.setMimeType(retctx.getMimeType());
            phfile.setFileName(retctx.getName());
            this.setPhysicalFile(phfile);
        }
        return ActionBase.SUCCESS;
    }

}
