/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.services.utils;

import com.s7turn.search.engine.ServiceException;

import com.s7turn.search.engine.services.PhysicalFileService;
import com.s7turn.search.engine.utils.ServiceFactory;
import javax.naming.InitialContext;
import javax.naming.NamingException;

/**
 * By EJB3 technology
 * @author Long
 */
public class EJB3ServiceFactory implements ServiceFactory {

    public EJB3ServiceFactory(){
        
    }
    
    protected String narrawName( String name){
        return name;
    }
    
    private Object lookup( String name ) throws ServiceException{
        InitialContext ictx = null; 
        try{
            ictx = new InitialContext();
            Object obj = ictx.lookup( name );
            return obj;
        }catch(NamingException ne){
            throw new ServiceException(ne.getExplanation(), ne);
        }
    }

    public PhysicalFileService getPhysicalFileService() throws ServiceException {
        return (PhysicalFileService) lookup( PhysicalFileService.class.getName() );
    }
    
}
