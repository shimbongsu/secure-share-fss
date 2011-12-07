/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine;

/**
 *
 * @author Long
 */
public class ServiceException extends Exception{
    public ServiceException(){
        super();
    }
    
    public ServiceException(String msg){
        super(msg);
    }
    
    public ServiceException(String msg, Exception e){
        super(msg, e);
    }
    
}
