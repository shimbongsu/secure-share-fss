/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services.ibatis;

import com.s7turn.gingkos.Gingko;
import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoPerment;
import com.s7turn.gingkos.services.GingkoService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.sql.Timestamp;
import java.util.Calendar;
import java.util.List;

/**
 *
 * @author Long
 */
public class GingkoServiceBean extends CommonServiceBean<Gingko, Long> implements GingkoService{

    public GingkoServiceBean(){
        super( Gingko.class );
    }
    
    public List<Gingko> findContentGingkos(GingkoContent gct) throws ServiceException {
        return this.findByQuery("Gingko.findContentCingkos", gct);
    }

    public List<Gingko> findContentGingkosWithUser(GingkoContent gct) throws ServiceException {
        if( gct.getOwner() == null){
            throw new java.lang.IllegalArgumentException("The GingkoContent which you given is not include the owner property. You must set it you want to query.");
        }
        return this.findByQuery("Gingko.findContentCingkosWithUser", gct);
    }

    public GingkoPerment findPerment(String guid) throws ServiceException{
        try{
            GingkoPerment gp = (GingkoPerment)this.getSqlMapClient().queryForObject("Gingko.findGingkoPermentsByUuid", guid);
            return gp;
        }catch(Exception ex){
            throw new ServiceException(ex.getMessage(), ex);
        }
    }

    public void deletePerment(String guid) throws ServiceException{
        try{
            this.getSqlMapClient().update("Gingko.deleteGingkoPerments", guid);
        }catch(Exception ex){
            throw new ServiceException(ex.getMessage(), ex);
        }
    }

    public void insertPerment(GingkoPerment gpt) throws ServiceException{
        try{
            this.getSqlMapClient().insert("Gingko.insertGingkoPerments", gpt);
        }catch(Exception ex){
            throw new ServiceException(ex.getMessage(), ex);
        }
    }
    
    public Gingko findGingko(String guid, User user) throws ServiceException {
        ///two step.
        ///the first is to check the gingko by user directly.
        ///if not found, then find the groups assign to be accessed to this file,
        ///if the user is in this group, then show out the biggest permission.

        //throw new UnsupportedOperationException("Not supported yet.");
        GingkoContent gct = new GingkoContent();
        //gct.setGcid(gcid);
        gct.setGuid(guid);
        gct.setOwner(user);

        List<Gingko> gs = findContentGingkosWithUser(gct);
        Gingko gko = new Gingko();
        Calendar calendar = Calendar.getInstance();
        gko.setBeginTime(new Timestamp(calendar.getTimeInMillis()));
        gko.setEndTime(new Timestamp(calendar.getTimeInMillis()));
        if( gs != null ){
            for( Gingko g : gs )
            {
                if( g.getBeginTime() != null )
                {
                    if( g.getBeginTime().before(gko.getBeginTime()) )
                    {
                        gko.setBeginTime(g.getBeginTime());
                    }
                }
                
                if( g.getEndTime() != null )
                {
                    if( g.getEndTime().after(gko.getEndTime())){
                        gko.setEndTime(g.getEndTime());
                    }
                }
                if( g.isRead() ){
                    gko.setRead(true);
                }
                if( g.isCopy()){
                    gko.setCopy(true);
                }
                if( g.isPrint() ){
                    gko.setPrint( true );
                }
                if( g.isEdit() ){
                    gko.setEdit(true);
                }
                if( g.isPermanent()){
                    gko.setPermanent(true);
                }
            }
        }
        
        return gko;
    }

}
