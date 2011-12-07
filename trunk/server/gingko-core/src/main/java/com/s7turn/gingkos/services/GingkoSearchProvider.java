/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoException;
import java.util.List;

/**
 *
 * @author Long
 */
public interface GingkoSearchProvider
{
    /**
     * Search the Contents from Contents
     * @param gct
     * @param keywords
     * @param start
     * @param limit
     * @return
     * @throws com.s7turn.gingkos.GingkoException
     */
    List<GingkoContent> search( GingkoContent gct, String keywords, long start, long limit  ) throws GingkoException;
    
    /**
     * Indexing the contents, if the idexing is based on database, i think we not need to implements these methods.
     * @param gct
     * @throws com.s7turn.gingkos.GingkoException
     */
    void indexContent( GingkoContent gct ) throws GingkoException;
    
    void indexContentList( List<GingkoContent> gct ) throws GingkoException;
}
