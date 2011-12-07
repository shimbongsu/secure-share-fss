/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos;

/**
 *
 * @author Long
 */
public class GingkoException extends Exception{
    public GingkoException(){
        super();
    }
    
    public GingkoException(String msg){
        super(msg);
    }
    
    public GingkoException(String msg, Throwable e){
        super( msg, e );
    }
}
