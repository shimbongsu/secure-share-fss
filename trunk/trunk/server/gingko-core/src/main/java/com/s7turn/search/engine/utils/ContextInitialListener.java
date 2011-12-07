/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.utils;

import java.lang.reflect.Method;
import java.util.Enumeration;
import javax.servlet.ServletContext;
import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

/**
 * Web application lifecycle listener.
 * @author Long
 */

public class ContextInitialListener implements ServletContextListener {
    private final static String INITIALIZE_CLASSES="s7turn.initialize.classes";
    private final static String INITIALIZE_LOGGERS="s7turn.log4j.properties";
    private static Logger logger = Logger.getLogger(ContextInitialListener.class);
    public void contextInitialized(ServletContextEvent event) {
        
        ServletContext ctx = event.getServletContext();
        String props = ctx.getInitParameter(INITIALIZE_LOGGERS);
        try{
            PropertyConfigurator.configure(ctx.getResource(props));
        }catch(Exception ex){}
        
        ServiceFactoryBuilder.setServiceFactoryClass( ctx.getInitParameter(ServiceFactoryBuilder.SERVICE_FACTORY) );
        Enumeration eums = ctx.getInitParameterNames();
        
        while( eums.hasMoreElements() )
        {
            String name = (String)eums.nextElement();
            String value = ctx.getInitParameter(name);
            Configuration.getInstance().setConfig(name, value);
        }
        
        String multiClass = ctx.getInitParameter(INITIALIZE_CLASSES);
        if( multiClass != null ){
            String[] clazz = multiClass.split(";");
            for( String clz : clazz ){
                doInitialize( clz, ctx );
            }
        }
    }

    public void contextDestroyed(ServletContextEvent event) {
        
    }

    private void doInitialize(String clz, ServletContext ctx) {
        if( clz != null ){
            try{
                Class clzz = Class.forName( clz.trim());
                Method method = clzz.getMethod("initialize", new Class[]{ ServletContext.class });
                if( method != null ){
                    method.invoke(null, new Object[]{ ctx }); //call static method
                }
            }catch(Exception e){
                logger.info(clz + " cannot be initialized. Because " + e.getMessage(), e);
            }
        }
    }
}