/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services.ibatis;

import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.services.GingkoContentService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.util.List;

/**
 *
 * @author Long
 */
public class GingkoContentServiceBean  extends CommonServiceBean<GingkoContent, String> implements GingkoContentService{
    
    public GingkoContentServiceBean(){
        super( GingkoContent.class );
    }
    
    public List<GingkoContent> findContents(GingkoContent gct, int opts) throws ServiceException {
        gct.setRelatedId(opts); ///here i just use the relatedId to do this options.
        return this.findByQuery("GingkoContent.findByContents", gct);
    }

    public List<GingkoContent> findContents(GingkoContent gct, int opts, int start, int count) throws ServiceException {
        gct.setRelatedId(opts); ///here i just use the relatedId to do this options.
        return this.findByQuery("GingkoContent.findByContents", gct, start, count );
    }

}
