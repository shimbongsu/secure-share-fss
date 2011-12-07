/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.member.services.ibatis;

import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.Avator;
import com.s7turn.search.engine.member.services.AvatorService;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;

/**
 *
 * @author Long
 */
public class AvatorServiceBean  extends CommonServiceBean<Avator, Long> implements AvatorService{
    private Avator defaultAvator;
    
    public AvatorServiceBean(){
        super(Avator.class);
    }

    public Avator getDefaultAvator() {
        return defaultAvator;
    }

    public void setDefaultAvator(Avator defaultAvator) {
        this.defaultAvator = defaultAvator;
    }
    
    @Override
    public boolean smartSave( Avator avator ) throws ServiceException{
        Avator avt = findByTypeWithId( avator.getAvatorType(), avator.getAvatorForId());
        if( avt.getId() == null || avt.getId() == 0 ){
            insert( avator );
        }else{
            avator.setId( avt.getId() );
            update( avator );
        }
        return  true;
    }
    
    public Avator findByTypeWithId(int type, Long id) {
        Avator ava = new Avator();
        ava.setAvatorType(type);
        ava.setAvatorForId(id);
        Avator avator = null;
        try{
            avator = (Avator)getSqlMapClient().queryForObject("Avator.findByTypeWithId", ava);
        }catch(Exception e){            
        }
        if( avator == null ){
            avator = getDefaultAvator();
        }
        return avator;
    }
}
