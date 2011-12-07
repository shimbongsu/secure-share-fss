/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.services.ibatis;

import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.CommonService;
import com.s7turn.search.engine.services.ServiceListener;
import com.s7turn.search.engine.services.SmartEntityPersistence;
import java.io.Serializable;
import java.sql.SQLException;
import java.util.List;
import java.util.Map;
import java.util.Vector;
import org.apache.commons.beanutils.PropertyUtils;
import org.springframework.orm.ibatis.support.SqlMapClientDaoSupport;

/**
 *
 * @author Long
 */
public class CommonServiceBean<E, PK extends Serializable> extends SqlMapClientDaoSupport implements CommonService<E, PK> {
    private SmartEntityPersistence smartPersistence;
    private Class clazz;
    private Vector<ServiceListener<E>> listeners = new Vector<ServiceListener<E>>();
        
    public CommonServiceBean(Class clz){
        clazz = clz;
    }

    public SmartEntityPersistence getSmartPersistence() {
        return smartPersistence;
    }

    public void setSmartPersistence(SmartEntityPersistence smartPersistence) {
        this.smartPersistence = smartPersistence;
    }
    
    
    @Override
    public E findBy(PK pk) throws ServiceException{
        
        try{
            String query = clazz.getSimpleName() + ".findByPrimaryKey";
            return (E) getSqlMapClient().queryForObject(query, pk);
        }catch(Exception ex){
            throw new ServiceException(ex.getMessage(), ex);
        }
    }

    @Override
    public void update(E e) throws ServiceException{
        try{
            if( this.fireOnUpdateEvent(e, true) ){
                this.getSqlMapClient().update(clazz.getSimpleName() + ".update" + clazz.getSimpleName(), e);
                if(this.fireOnUpdateEvent(e, false)){
                    ////I am thinking on this creation.
                }
            }
        }catch(Exception ex){
            throw new ServiceException(ex.getMessage(), ex);
        }
        
    }

    @Override
    public void insert(E e) throws ServiceException{
        try{
            if( this.fireOnCreateEvent(e, true) ){
                this.getSqlMapClient().insert(clazz.getSimpleName() + ".insert" + clazz.getSimpleName(), e);
                this.fireOnCreateEvent(e, false);
            }
        }catch(Exception ex){
            throw new ServiceException(ex.getMessage(), ex);
        }
    }

    @Override
    public void delete(E e) throws ServiceException{
        try{
            if( this.fireOnDeleteEvent(e, true) ){
                this.getSqlMapClient().delete(clazz.getSimpleName() + ".delete" + clazz.getSimpleName(), e);
                this.fireOnDeleteEvent(e, false);
            }
        }catch(Exception ex){
            throw new ServiceException(ex.getMessage(), ex);
        }
    }

    public List<E> findAll() throws ServiceException{
        try{
            return this.getSqlMapClient().queryForList( clazz.getSimpleName() + ".findAll" );
        }catch(Exception e){
            throw new ServiceException(e.getMessage(), e);
        }
    }

    public List<E> findByQuery(String query, Map<String, Object> params) throws ServiceException{
        try{        
            if( params == null ){
                return getSqlMapClient().queryForList(query);
            }

            if( params.size() == 0 ){
                return getSqlMapClient().queryForList(query);
            }

            return getSqlMapClient().queryForList(query, params); 
        }catch(Exception ex){
            throw new ServiceException( ex.getMessage(), ex);
        }
    }

    public List<E> findByQuery(String query, Object params) throws ServiceException{
        try{        
            if( params == null ){
                return getSqlMapClient().queryForList(query);
            }
            return getSqlMapClient().queryForList(query, params); 
        }catch(Exception ex){
            throw new ServiceException( ex.getMessage(), ex);
        }
    }

    public List<E> findByQuery(String query, Object params, int start, int count) throws ServiceException{
        try{
            if( params == null ){
                return getSqlMapClient().queryForList(query, start, count );
            }
            return getSqlMapClient().queryForList(query, params, start, count);
        }catch(Exception ex){
            throw new ServiceException( ex.getMessage(), ex);
        }
    }
    
    public List<E> findByQuery(String query) throws ServiceException{
        return findByQuery( query, null );
    }
    
    public boolean smartSave(E e) throws ServiceException{
        if( smartPersistence != null ){
            smartPersistence.persist(e);
        }else{
            ////execute the standard smartSave process.
            if( e != null ){
                try{
                   Object idVal = PropertyUtils.getProperty(e, "id");
                   if( idVal == null || (idVal instanceof Long && (Long)idVal == 0 ) ){
                       this.insert(e);
                   }else{
                       this.update(e);
                   }
                }catch(Exception ex){
                    throw new ServiceException( "Can not persist this object because there is not 'id' defined. ", ex);
                }
            }
        }
        return true;
    }

    public E findByCode(String code) throws ServiceException {
        try{
        String query = clazz.getSimpleName() + ".findByCode";
        List<E> list = (List<E>)getSqlMapClient().queryForList(query, code);
        if( list != null && list.size() > 0 ){
            return list.get(0);
        }
        }catch(SQLException se){
            throw new ServiceException(se.getMessage(), se);
        }
        return null;
        //return (E)getHibernateTemplate().findByNamedQuery(query, new Object[]{ code });
        //return (E)getHibernateTemplate().get(clazz.getSimpleName(), code);
    }

    public Vector<ServiceListener<E>> getListeners() {
        return listeners;
    }

    public void setListeners(Vector<ServiceListener<E>> listeners) {
        for( ServiceListener<E> sle : listeners ){
            this.addServiceListener(sle);
        }
    }
    
    public void addServiceListener(ServiceListener<E> sl) {
        if( !this.listeners.contains(sl) ){
            listeners.add(sl);
        }
    }

    public void removeServiceListener(ServiceListener<E> sl) {
        if( !this.listeners.contains(sl)){
            listeners.remove(sl);
        }
    }
    
    protected boolean fireOnCreateEvent( E e, boolean bf ) throws ServiceException{
        for( ServiceListener<E> sl : listeners ){
            if( !sl.onCreate(e, bf) ){
                return false;
            }
        }
        return true;
    } 
    protected boolean fireOnUpdateEvent( E e, boolean bf ) throws ServiceException{
        for( ServiceListener<E> sl : listeners ){
            if( !sl.onUpdate(e, bf) ){
                return false;
            }
        }
        return true;
    } 
    protected boolean fireOnDeleteEvent( E e, boolean bf ) throws ServiceException{
        for( ServiceListener<E> sl : listeners ){
            if( !sl.onCreate(e, bf) ){
                return false;
            }
        }
        return true;
    }

    public List<E> findByQuery(String query, Map<String, Object> params, int start, int count) throws ServiceException {
        try{
            if( params == null ){
                return getSqlMapClient().queryForList(query, start, count);
                //return getSqlMapClient().queryForList(query);
            }

            if( params.size() == 0 ){
                return getSqlMapClient().queryForList(query, start, count);
                //return getSqlMapClient().queryForList(query);
            }

            return getSqlMapClient().queryForList(query, params, start, count);
        }catch(Exception ex){
            throw new ServiceException( ex.getMessage(), ex);
        }
    }

}
