/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.Gingko;
import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoException;
import com.s7turn.search.engine.ServiceException;
import java.util.List;

/**
 *
 * @author Long
 */
public class DefaultGingkoProviderImpl implements GingkoProvider{

    private Gingko defaultGingko;
    private GingkoService gingkoService;

    public GingkoService getGingkoService() {
        return gingkoService;
    }

    public void setGingkoService(GingkoService gingkoService) {
        this.gingkoService = gingkoService;
    }
    
    public Gingko getDefaultGingko() {
        return this.defaultGingko;
    }

    public List<Gingko> getContentGingkos(GingkoContent gct) throws GingkoException {
        try{
            return this.getGingkoService().findContentGingkos(gct);
        }catch(ServiceException ex){
            throw new GingkoException(ex.getMessage(), ex);
        }
            
    }

    public void addGingko(GingkoContent gct, Gingko gko) throws GingkoException {
        try{
            gko.setContent(gct);
            if( gko.getGingkoId() != null && gko.getGingkoId() > 0 ){
                this.getGingkoService().update(gko);
            }else{
                this.getGingkoService().insert(gko);
            }
        }catch(ServiceException ex){
            throw new GingkoException(ex.getMessage(), ex);
        }
    }

    public void removeGingko(GingkoContent gct, Gingko gko) throws GingkoException {
        try{
            gko.setContent(gct);
            this.getGingkoService().delete(gko);
        }catch(ServiceException ex){
            throw new GingkoException(ex.getMessage(), ex);
        }
    }
}
