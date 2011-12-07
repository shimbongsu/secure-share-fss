/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.Gingko;
import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoException;
import com.s7turn.search.engine.member.User;
import java.util.List;

/**
 *
 * @author Long
 */
public interface GingkoContentProvider {
    
    /**
     * Get the basic GingkoContentService
     * @return
     */
    GingkoContentService getContentService();
    
    /**
     * fetch all contents in a folder. 
     * @param folder
     * @return
     */
    List<GingkoContent> fetch( GingkoContent folder ) throws GingkoException;
    List<GingkoContent> fetchFolders( GingkoContent folder ) throws GingkoException;
    List<GingkoContent> fetchContents( GingkoContent folder ) throws GingkoException;
    /**
     * narrow the GingkoContent to your pojo. the suggest code:
     * <code>
     * if( gct.extendObject == null ){
     *   //your code to get this content.
     *   ....
     *   gct.extendObject = extobj;
     * }
     * return gct.extendObject;
     * </code>
     * @param gct
     * @return
     */
    Object narrow( GingkoContent gct ) throws GingkoException;
    
    /**
     * get all gingko of this content
     * @param gct
     * @return
     */
    List<Gingko> getGingkos( GingkoContent gct ) throws GingkoException;
    void setGingkos( GingkoContent gct, List<Gingko> gkos ) throws GingkoException;
    void addGingko( GingkoContent gct, Gingko gkos ) throws GingkoException;
    void removeGingko( GingkoContent gct, Gingko gko ) throws GingkoException;
    /**
     * get a user of this gingko.. which should be voted process
     * @param gct
     * @param user
     * @return
     */
    Gingko getGingko( GingkoContent gct, User user );
    
    /**
     * get the root folder of this type of content. 
     * the root should base on the User.
     * @param user
     * @return
     */
    GingkoContent getRootFolder( User user );
    
}
