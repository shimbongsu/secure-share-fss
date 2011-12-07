/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.member.services.ibatis;

import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.UserDetail;
import com.s7turn.search.engine.member.services.UserDetailService;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 *
 * @author Long
 */
public class UserDetailServiceBean  extends CommonServiceBean<UserDetail, Long> implements UserDetailService{
    public UserDetailServiceBean(){
        super(UserDetail.class);
    }
    
    @Override
    public boolean smartSave( UserDetail detail ) throws ServiceException{
        UserDetail ud = null;
        try{
            ud = this.findBy(detail.getId());
        }catch(Exception e){
            ud = null;
        }
        
        if( ud != null ){
            this.update(detail);
        }else{
            this.insert(detail);
        }
        
        return true;
    }

    public List<UserDetail> findUsers(String key, Map<String, Object> options) throws ServiceException {
        Map<String, Object> conds = new HashMap<String, Object>();
        if( options == null ){
            try {
                conds.put("keyword", "%" + key + "%");
                return this.getSqlMapClient().queryForList("UserDetail.findUsers", conds);
            } catch (SQLException ex) {
                throw new ServiceException(ex.getMessage(), ex);
            }
        }else{
            try {
                conds.putAll(options);
                conds.put("keyword", "%" + key + "%");
                return this.getSqlMapClient().queryForList("UserDetail.findUsers", conds);
            } catch (SQLException ex) {
                throw new ServiceException(ex.getMessage(), ex);
            }
        }
    }

    public List<UserDetail> findUsers(String key) throws ServiceException {
        return findUsers( key, null );
    }
}
