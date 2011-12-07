/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.member.services.ibatis;

import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.ByCreator;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.services.CommonService;
import com.s7turn.search.engine.services.ServiceListener;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

/**
 *
 * @author Long
 */
public class MemberServiceListener implements ServiceListener<User>{
    
    private Map<CommonService, ByCreator> installMappers = new HashMap<CommonService, ByCreator>();

    public Map<CommonService, ByCreator> getServiceMappers() {
        return installMappers;
    }

    public void setServiceMappers(Map<CommonService, ByCreator> installMappers) {
        this.installMappers = installMappers;
    }


    public boolean onCreate(User ob, boolean before) throws ServiceException {
        if( !before ){
            Iterator<Map.Entry<CommonService, ByCreator>> entries = installMappers.entrySet().iterator();
            while( entries.hasNext() ){
                Map.Entry<CommonService, ByCreator> entry = entries.next();
                ByCreator creator = entry.getValue();
                creator.setOwner(ob);
                entry.getKey().insert(creator);
            }
        }
        return true;
    }

    public boolean onUpdate(User ob, boolean before) throws ServiceException {
        return true;
    }

    public boolean onDelete(User ob, boolean before) throws ServiceException {
        if( !before ){
            Iterator<Map.Entry<CommonService, ByCreator>> entries = installMappers.entrySet().iterator();
            while( entries.hasNext() ){
                Map.Entry<CommonService, ByCreator> entry = entries.next();
                ByCreator creator = entry.getValue();
                creator.setOwner(ob);
                entry.getKey().delete(creator);
            }
        }
        return true;
    }

}
