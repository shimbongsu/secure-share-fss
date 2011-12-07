/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.utils;

import com.s7turn.search.engine.ServiceException;
import javax.servlet.ServletContext;

/**
 *
 * @author Long
 */
public abstract class ServiceFactoryBuilder 
{
    public final static String SERVICE_FACTORY = "s7turn.searchengin.services.factory";
    private static ServiceFactory factory = null;
    private static String lockObject = "lock";
    private static String factoryClass;
    
    public static ServiceFactory getInstance() throws ServiceException
    {
        if( factory == null )
        {
            try{
                factory = create();
            }catch(Exception e){
                throw new ServiceException(e.getMessage(), e);
            }
        }
        return factory;
    }

    public static ServiceFactory getInstance(ServletContext sctx) throws ServiceException
    {
        if( factory == null )
        {
            try{
                factory = create(sctx);
            }catch(Exception e){
                throw new ServiceException(e.getMessage(), e);
            }
        }
        return factory;
    }
    
    public static ServiceFactory getFactory() {
        return factory;
    }

    public static void setFactory(ServiceFactory fact) {
        factory = fact;
    }
    
    public static void setServiceFactoryClass(String clzName){
        factoryClass = clzName;
    }
    
    public static String getServiceFactoryClass(){
        if( factoryClass == null ){
            factoryClass = System.getProperty(SERVICE_FACTORY);
        }
        
        return factoryClass;
    }
    
    
    private static ServiceFactory create() throws Exception{
        String serviceFactoryKey = getServiceFactoryClass();
        Class clzz = Class.forName(serviceFactoryKey);
        Object obj = clzz.newInstance();
        return (ServiceFactory) obj;
    }

    private static ServiceFactory create(ServletContext sctx) throws Exception{
        String serviceFactoryKey = getServiceFactoryClass();
        Class clzz = Class.forName(serviceFactoryKey);
        Object obj = clzz.getConstructor( new Class[]{ ServletContext.class } ).newInstance( sctx );
        return (ServiceFactory) obj;
    }
    
}
