/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services.plugins;

import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoException;
import com.s7turn.gingkos.services.GingkoFileTypePlugin;
import com.s7turn.search.engine.PhysicalFile;
import com.s7turn.search.engine.member.User;
import java.util.Map;

/**
 *
 * @author Long
 */
public class FileTypeMappedPlugin implements GingkoFileTypePlugin{
    private Map<String, GingkoFileTypePlugin> fileTypeMap;

    protected String getFileExtension( String fileName ){
        if( fileName != null ){
            int last = fileName.lastIndexOf(".");
            if( last >= 0 ){
                return fileName.substring(last).toLowerCase();
            }
        }
        return "";
    }
    public Map<String, GingkoFileTypePlugin> getFileTypeMap() {
        return fileTypeMap;
    }

    public void setFileTypeMap(Map<String, GingkoFileTypePlugin> fileTypeMap) {
        this.fileTypeMap = fileTypeMap;
    }

    @Override
    public boolean accept(PhysicalFile phf){
        if( phf == null ){
            return false;
        }

        if( phf.getId() == null || phf.getId() == 0 ){
            return false;
        }        
        return this.getFileTypeMap().containsKey(getFileExtension( phf.getOriginalFileName() ) );
    }

    public boolean watch(PhysicalFile phf, GingkoContent parent, GingkoContent current, User user) throws GingkoException {
        GingkoFileTypePlugin gftp = this.getFileTypeMap().get(getFileExtension( phf.getOriginalFileName() ) );
        return gftp.watch(phf, parent, current, user);
    }

}
