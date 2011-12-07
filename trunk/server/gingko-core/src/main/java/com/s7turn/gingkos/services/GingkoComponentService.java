/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoException;
import com.s7turn.search.engine.member.User;
import java.util.List;

/**
 *
 * @author Long
 */
public interface GingkoComponentService {
    /**
     * gkname is the type of content. by default, Gingko has GingkoFolder and GingkoBase provider
     * @param gkName
     * @param className
     */
    void registerGingko( String gkName, String className );
    
    GingkoContentProvider lookup(GingkoContent gct) throws GingkoException;
    
    Object getExtendObject( GingkoContent gct );
    
    List<GingkoContent> getRootFolders( User user ) throws GingkoException;
    
    List<GingkoContent> getContents( GingkoContent gct, User user, boolean folderOnly ) throws GingkoException;
    
    List<GingkoContent> searchContents( GingkoContent gct, String keywords, long start, long limit ) throws GingkoException ;
    
}
