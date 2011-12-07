/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.Gingko;
import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoException;
import com.s7turn.search.engine.member.User;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;

/**
 *
 * @author Long
 */
public class DefaultGingkoServiceManager implements GingkoComponentService, Serializable{

    private static Logger logger = Logger.getLogger(DefaultGingkoServiceManager.class);
    
    private Map<String, GingkoContentProvider> services = new HashMap<String, GingkoContentProvider>();

    private GingkoContentProvider gingkoBaseProvider;
    private GingkoContentProvider gingkoFolderProvider;
    private GingkoSearchProvider searchProvider;
    private GingkoContentService contentService;

    public GingkoSearchProvider getSearchProvider() {
        return searchProvider;
    }

    public void setSearchProvider(GingkoSearchProvider searchProvider) {
        this.searchProvider = searchProvider;
    }
    
    public GingkoContentService getContentService() {
        return contentService;
    }

    public void setContentService(GingkoContentService contentService) {
        this.contentService = contentService;
    }
    
    public GingkoContentProvider getGingkoBaseProvider() {
        return gingkoBaseProvider;
    }

    public void setGingkoBaseProvider(GingkoContentProvider gingkoBaseProvider) {
        this.gingkoBaseProvider = gingkoBaseProvider;
    }

    public GingkoContentProvider getGingkoFolderProvider() {
        return gingkoFolderProvider;
    }

    public void setGingkoFolderProvider(GingkoContentProvider gingkoFolderProvider) {
        this.gingkoFolderProvider = gingkoFolderProvider;
    }
    
    public void setServices(Map<String, GingkoContentProvider> s){
        services = s;
    }
    
    
    public void registerGingko(String gkName, String className) {
        throw new UnsupportedOperationException("Not supported yet.");
    }

    public GingkoContentProvider lookup(GingkoContent gct) throws GingkoException {
        GingkoContentProvider gcp = services.get(gct.getType());
        if( gcp == null ){
            return gct.isFolder() ? this.gingkoFolderProvider : this.gingkoBaseProvider;
        }else{
            return gcp;
        }
    }

    public Object getExtendObject(GingkoContent gct) {
        GingkoContentProvider gcp = services.get(gct.getType());
        if( gcp != null ){
            try {
                return gcp.narrow(gct);
            } catch (Exception ex) {
                logger.debug(ex.getMessage(), ex);
            }
        }
        return null;
    }

    public List<GingkoContent> getRootFolders(User user) throws GingkoException {
        List<GingkoContent> gingkos = new ArrayList<GingkoContent>(); 
        GingkoContent gc = new GingkoContent();
        gc.setOwner(user);
        gc.setFolder(true);
        gc.setParent(gc); ///fetch root folder.
        gingkos.addAll( this.getGingkoFolderProvider().fetch(gc) );
        for( GingkoContentProvider gcp : services.values() ){
            GingkoContent gct = gcp.getRootFolder(user);
            if( gct != null ){
                gingkos.add(gct);
            }
        }
        return gingkos;
    }

    public List<GingkoContent> getContents(GingkoContent gct, User user, boolean folderOnly) throws GingkoException {
        if( !gct.isFolder() ){
            throw new GingkoException(gct.getName() + " is not a folder.");
        }
        
        GingkoContentProvider gcp = this.lookup(gct);
        Gingko gko = gcp.getGingko(gct, user);
        if( gko.isList() ){
            if( folderOnly == true ){
                return gcp.fetchFolders(gct);
            }
            return gcp.fetch(gct);
        }
        throw new GingkoException(gct.getName() + " is not allowed your to access.");
    }

    public List<GingkoContent> searchContents(GingkoContent gct, String keywords, long start, long limit)  throws GingkoException {
        return searchProvider.search(gct, keywords, start, limit);
    }
    
}
