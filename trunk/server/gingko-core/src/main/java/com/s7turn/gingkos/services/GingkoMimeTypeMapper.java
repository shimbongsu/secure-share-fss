/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import java.util.Map;

/**
 *
 * @author Long
 */
public class GingkoMimeTypeMapper {
    private Map<String, String> mimetypeMappings;
    private String defaultMimeType;

    public String getDefaultMimeType() {
        return defaultMimeType;
    }

    public void setDefaultMimeType(String defaultMimeType) {
        this.defaultMimeType = defaultMimeType;
    }

    public Map<String, String> getMimetypeMappings() {
        return mimetypeMappings;
    }

    public void setMimetypeMappings(Map<String, String> mimetypeMappings) {
        this.mimetypeMappings = mimetypeMappings;
    }
    
    public String getMimeType(String ext){
        String fp = this.mimetypeMappings.get(ext);
        return fp == null ? this.getDefaultMimeType() : fp;
    }
}
