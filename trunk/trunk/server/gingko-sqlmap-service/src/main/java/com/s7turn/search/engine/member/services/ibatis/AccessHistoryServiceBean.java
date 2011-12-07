/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.member.services.ibatis;

import com.s7turn.search.engine.member.AccessHistory;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.services.AccessHistoryService;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.sql.Timestamp;
import java.util.Calendar;
import java.util.List;

/**
 *
 * @author Long
 */
public class AccessHistoryServiceBean extends CommonServiceBean<AccessHistory, Long> implements AccessHistoryService {

    public AccessHistoryServiceBean(){
        super(AccessHistory.class);
    }
    
    public List<AccessHistory> findByAccessBy(int cate, Long whereId) throws ServiceException {
        AccessHistory ah = new AccessHistory();
        ah.setAccessCategory(cate);
        ah.setAccessToId(whereId);
        return findByQuery("AccessHistory.findByAccessBy", ah);
    }

    public List<AccessHistory> findByAccessTo(int cate, Long whoId) throws ServiceException {
        AccessHistory ah = new AccessHistory();
        ah.setAccessCategory(cate);
        ah.setUserId(whoId);
        return findByQuery("AccessHistory.findByAccessTo", ah);
    }
    
    @Override
    public boolean smartSave(AccessHistory ah) throws ServiceException{
        List<AccessHistory> ahs = null;
        try{
            ah.setWhen( new Timestamp( Calendar.getInstance().getTimeInMillis() ) );
            ahs = findByQuery("AccessHistory.findByThisDayAccess", ah);
        }catch(ServiceException e){
            //insert this value
            ahs = null;
        }
        
        if( ahs != null && ahs.size() > 0 ){
            ah.setId(ahs.get(0).getId());
            update( ah );
        }else{
            insert( ah );
        }
        
        return true;
    }

}
