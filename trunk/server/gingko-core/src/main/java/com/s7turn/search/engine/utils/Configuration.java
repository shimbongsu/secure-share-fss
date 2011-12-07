/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.utils;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

/**
 *
 * @author Long
 */
public class Configuration implements Serializable
{
    private static Configuration instance = new Configuration();
    private Map<String, String> configs = new HashMap<String, String>();
    private String indexLocation;

    public String getIndexLocation() {
        return getConfig().get("index.location");
    }

    public void setIndexLocation(String indexLocation) {
        getConfig().put("index.location", indexLocation);
    }
     
    private Configuration(){
        
    }
    
    public static Configuration getInstance(){
        return instance;
    }
    
    
    public void setConfig(String name, String value){
        configs.put(name, value);
    }
    
    public Map<String, String> getConfig(){
        return configs;
    }
    
}
