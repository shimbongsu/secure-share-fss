/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services.plugins;

import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoException;
import com.s7turn.gingkos.services.GingkoContentService;
import com.s7turn.gingkos.services.GingkoFileTypePlugin;
import com.s7turn.gingkos.services.GingkoService;
import com.s7turn.search.engine.PhysicalFile;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.services.PhysicalFileService;

/**
 *
 * @author Long
 */
public abstract class AbstractGingkoFileTypePlugin implements GingkoFileTypePlugin{
    private String acceptedFiletype;
    private PhysicalFileService physicalFileService;
    private GingkoContentService gingkoContentService;
    private GingkoService gingkoService;

    public GingkoContentService getGingkoContentService() {
        return gingkoContentService;
    }

    public void setGingkoContentService(GingkoContentService gingkoContentService) {
        this.gingkoContentService = gingkoContentService;
    }

    public GingkoService getGingkoService() {
        return gingkoService;
    }

    public void setGingkoService(GingkoService gingkoService) {
        this.gingkoService = gingkoService;
    }

    public PhysicalFileService getPhysicalFileService() {
        return physicalFileService;
    }

    public void setPhysicalFileService(PhysicalFileService physicalFileService) {
        this.physicalFileService = physicalFileService;
    }
    
    public String getAcceptedFiletype() {
        return acceptedFiletype;
    }

    public void setAcceptedFiletype(String acceptedFiletype) {
        this.acceptedFiletype = acceptedFiletype;
    }

    protected String getFileExtension( String fileName ){
        if( fileName != null ){
            int last = fileName.lastIndexOf(".");
            if( last >= 0 ){
                return fileName.substring(last).toLowerCase();
            }
        }
        return "";
    }


    public boolean accept(PhysicalFile phf) {
        if( phf == null ){
            return false;
        }

        if( phf.getId() == null || phf.getId() == 0 ){
            return false;
        }

        String fileName = phf.getFileName();

        if( fileName.trim().toLowerCase().endsWith(this.acceptedFiletype.trim().toLowerCase()) ){
            return true;
        }
        
        return false;
    }

}
