/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.services.ibatis.typehanders;

import java.sql.CallableStatement;
import java.sql.ResultSet;
import org.apache.commons.beanutils.ConvertUtils;
import org.apache.commons.beanutils.PropertyUtils;
import org.apache.log4j.Logger;

/**
 *
 * @author Long
 */
public class TypeHandlerUtils {
    private static Logger logger = Logger.getLogger(TypeHandlerUtils.class);
    
    public static void populate( ResultSet rs, Object obj, String fieldName, String property ){
        try{
            Object value = rs.getObject(fieldName);
            PropertyUtils.setProperty(obj, property, value);
        }catch(Exception ex){
            //ex.printStackTrace();
            if( logger.isDebugEnabled() ){
                logger.debug( fieldName + ":" + property + " was raised an exception. ", ex );
            }
        }
    }

    public static void populate( CallableStatement rs, Object obj, String fieldName, String property ){
        try{
            Object value = rs.getObject(fieldName);
            PropertyUtils.setProperty(obj, property, value);
        }catch(Exception ex){
            //ex.printStackTrace();
            if( logger.isDebugEnabled() ){
                logger.debug( fieldName + ":" + property + " was raised an exception. ", ex );
            }
        }
    }
    
    public static Object newInstance( Class clzz ) {
        try{
            return clzz.newInstance();
        }catch(Exception e){}
        return null;
    }
    
    public static void setProperty( Object obj, String property, String value ){
        try{
            PropertyUtils.setProperty(obj, property, ConvertUtils.convert(value, PropertyUtils.getPropertyType(obj, property)));
        }catch(Exception ex){}
    } 
    
    
    
    public static void setProperty( Object obj, String property, Object value ){
        try{
            PropertyUtils.setProperty(obj, property, value);
        }catch(Exception ex){}
    }
    
    public static String getString( Object obj, String property ){
        Object prValue = null;
        try{
            prValue = PropertyUtils.getProperty(obj, property).toString();
        }catch(Exception e){}
        
        return prValue == null ? null : prValue.toString();
        
    }

    public static Object getObject( Object obj, String property ){
        Object prValue = null;
        try{
            prValue = PropertyUtils.getProperty(obj, property).toString();
        }catch(Exception e){}
        
        return prValue;
        
    }

}
