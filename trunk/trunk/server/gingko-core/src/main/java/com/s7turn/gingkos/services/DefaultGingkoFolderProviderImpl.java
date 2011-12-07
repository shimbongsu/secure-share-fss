/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.Gingko;
import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoException;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.User;
import java.util.ArrayList;
import java.util.List;
import org.apache.log4j.Logger;

/**
 *
 * @author Long
 */
public class DefaultGingkoFolderProviderImpl implements GingkoContentProvider{

    private GingkoContentService contentService;
    private GingkoProvider gingkoProvider;
    private static Logger logger = Logger.getLogger(DefaultGingkoFolderProviderImpl.class);
    private GingkoContent rootFolder;
    
    public void setRootFolder(GingkoContent rootFolder) {
        this.rootFolder = rootFolder;
    }

    public GingkoProvider getGingkoProvider() {
        return gingkoProvider;
    }

    public void setGingkoProvider(GingkoProvider gingkoProvider) {
        this.gingkoProvider = gingkoProvider;
    }
    
    public void setContentService(GingkoContentService contentService) {
        this.contentService = contentService;
    }
    
    public GingkoContentService getContentService() {
        return this.contentService; 
    }

    public List<GingkoContent> fetch(GingkoContent folder)  throws GingkoException{
        if( !folder.isFolder() ){
            throw new GingkoException( folder.getName() + " is not a folder.");
        }

        try{
            return this.getContentService().findContents(folder, 0);
        }catch(ServiceException se){
            throw new GingkoException(se.getMessage(), se);
        }
    }

    public Object narrow(GingkoContent gct) {
        return null;
    }

    public List<Gingko> getGingkos(GingkoContent gct) throws GingkoException{
        try{
            return this.getGingkoProvider().getContentGingkos(gct);
        }catch(Exception ex ){
            return new ArrayList<Gingko>(0);
        }
    }

    /**
     * this method should override again. 
     * @param gct
     * @param user
     * @return
     */
    public Gingko getGingko(GingkoContent gct, User user) {
        Gingko gk = this.getGingkoProvider().getDefaultGingko();
        if( gk == null ){
            gk = new Gingko();
            gk.setList(true);
        }
        gk.setContent(gct);
        gk.setUser(user);
        try{
            List<Gingko> gingkos = this.getGingkoProvider().getContentGingkos(gct);
            for( Gingko ggk : gingkos ){
                gk.setCopy(ggk.isCopy() ? true : gk.isCopy() );
                gk.setEdit(ggk.isEdit() ? true : gk.isEdit() );
                gk.setList(ggk.isList() ? true : gk.isList() );
                gk.setPrint(ggk.isPrint() ? true : gk.isPrint() );
                gk.setRead(ggk.isRead() ? true : gk.isRead() );
            }
        }catch(Exception ex){
            
        }
       return gk;
    }

    public GingkoContent getRootFolder(User user) {
        GingkoContent gc = null;
        try{
            gc = (GingkoContent) this.rootFolder.clone();
        }catch(Exception ex){
            gc = new GingkoContent();
        }
        gc.setOwner(user);
        gc.setFolder(true);
        gc.setName( user.getFullName() );
        return gc;
    }

    public List<GingkoContent> fetchFolders(GingkoContent folder)  throws GingkoException{
        if( !folder.isFolder() ){
            throw new GingkoException( folder.getName() + " is not a folder.");
        }
        try{
            return this.getContentService().findContents(folder, 1);
        }catch(ServiceException se){
            logger.debug(se.getMessage(), se);
            throw new GingkoException(se.getMessage(), se);
        }
    }

    public List<GingkoContent> fetchContents(GingkoContent folder)  throws GingkoException{
        if( !folder.isFolder() ){
            throw new GingkoException( folder.getName() + " is not a folder.");
        }

        try{
            return this.getContentService().findContents(folder, 2);
        }catch(ServiceException se){
            logger.debug(se.getMessage(), se);
            throw new GingkoException(se.getMessage(), se);
        }
    }

    public void setGingkos(GingkoContent gct, List<Gingko> gkos) throws GingkoException{
        for( Gingko gk : gkos ){
            try{
                this.getGingkoProvider().addGingko(gct, gk);
            }catch(Exception ge){
                logger.debug(ge.getMessage(), ge);
            }
        }
    }

    public void removeGingko(GingkoContent gct, Gingko gko) throws GingkoException{
        try{
            this.getGingkoProvider().removeGingko(gct, gko);
        }catch(Exception ge){
            logger.debug(ge.getMessage(), ge);
        }
    }

    public void addGingko(GingkoContent gct, Gingko gkos) throws GingkoException{
        this.getGingkoProvider().addGingko(gct, gkos);
    }

}
