/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.services;

import com.s7turn.search.engine.ServiceException;
import java.util.List;
import java.util.Map;

/**
 * The common service for update
 * @author Long
 */
public interface CommonService<E, PK> 
{
    
    //get the product by key
    E findBy(PK pk) throws ServiceException;
    
    E findByCode( String code ) throws ServiceException;
    
    //load compare list of special product
    //List<E> getCompareList(String code);
    
    //update the record
    void update(E e) throws ServiceException;
    
    //insert a new record.
    void insert(E e) throws ServiceException;
    
    //delete the record
    void delete( E e ) throws ServiceException;
    
    //Event handler
    void addServiceListener( ServiceListener<E> sl );
    
    void removeServiceListener( ServiceListener<E> sl );
    //the event trigger     
    //list all item
    List<E> findAll() throws ServiceException;

    List<E> findByQuery(String query, Object params, int start, int count) throws ServiceException;
    
    List<E> findByQuery(String query, Object params) throws ServiceException;
    //list by sepcial query
    List<E> findByQuery(String query, Map<String, Object> params) throws ServiceException;

    List<E> findByQuery(String query, Map<String, Object> params, int start, int count) throws ServiceException;
    //save the entity with the sub entities;
    boolean smartSave( E e ) throws ServiceException;
}
