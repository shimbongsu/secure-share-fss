/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.Gingko;
import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoException;
import java.util.List;

/**
 *
 * @author Long
 */
public interface GingkoProvider {
    
    /**
     * Get this default Gingko of this module.
     * @return
     */
    Gingko getDefaultGingko();
    
    /**
     * fetch all Gingko of this content
     * @param gct
     * @return
     */
    List<Gingko> getContentGingkos( GingkoContent gct ) throws GingkoException; 
    
    /**
     * add a Gingko set to GingkoContent
     * @param gct
     * @param gko
     * @throws com.s7turn.gingkos.GingkoException
     */
    void addGingko( GingkoContent gct, Gingko gko ) throws GingkoException;

    /**
     * remove a Gingko set for a GingkoContent
     * @param gct
     * @param gko
     * @throws com.s7turn.gingkos.GingkoException
     */
    void removeGingko( GingkoContent gct, Gingko gko ) throws GingkoException;
}
