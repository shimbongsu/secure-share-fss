/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine;

/**
 *
 * @author Long
 */
public class WebappException extends Exception {

    private int errorCode;
    public WebappException(int code){
        super("Error with code " + code );
        errorCode = code;
    }
    
    public WebappException(int code, String msg){
        super( msg );
        errorCode = code;
    }
    
    public WebappException(int code, String msg, Exception e){
        super( msg, e );
        errorCode = code;
    }

    public int getErrorCode() {
        return errorCode;
    }

    public void setErrorCode(int errorCode) {
        this.errorCode = errorCode;
    }
    
    
    
}
