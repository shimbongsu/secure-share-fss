/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoException;
import com.s7turn.search.engine.PhysicalFile;
import com.s7turn.search.engine.member.User;

/**
 *
 * @author Long
 */
public interface GingkoFileTypePlugin {
    /**
     * this method check the PhysicalFile which is upload to server wheather it
     * will be accept to process by this plugin.
     * @param phf
     * @return
     */
    boolean accept( PhysicalFile phf );


    /**
     * Convert the uploaded file as a GingkoContent
     * @param phf
     * @param parent
     * @param current
     * @param user
     * @return
     * @throws com.s7turn.gingkos.GingkoException
     */
    boolean watch( PhysicalFile phf, GingkoContent parent,
                    GingkoContent current,
                    User user ) throws GingkoException;


}
