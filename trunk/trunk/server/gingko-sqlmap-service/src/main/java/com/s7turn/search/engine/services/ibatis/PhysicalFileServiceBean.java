/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.services.ibatis;

import com.s7turn.search.engine.PhysicalFile;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.PhysicalFileService;
import java.lang.reflect.Method;
import java.util.List;
import javax.el.BeanELResolver;
import javax.el.ELContext;
import javax.el.ELResolver;
import javax.el.FunctionMapper;
import javax.el.ValueExpression;
import javax.el.VariableMapper;

/**
 *
 * @author Long
 */
public class PhysicalFileServiceBean  extends CommonServiceBean<PhysicalFile, Long> implements PhysicalFileService{

    private String accessUrlPattern;
    
    public PhysicalFileServiceBean(){
        super(PhysicalFile.class);
    }

    public String getAccessUrlPattern() {
        return accessUrlPattern;
    }

    public void setAccessUrlPattern(String accessUrlPattern) {
        this.accessUrlPattern = accessUrlPattern;
    }
    
    public String getAccessUrl(PhysicalFile pf) throws ServiceException {        
        return String.format(accessUrlPattern, pf.getId(), pf.getFileName(), pf.getMd5Code(), pf.getOriginalFileName(), pf.getUserId());
    }
    
    public List<PhysicalFile> findByUserId( Long userId ) throws ServiceException{
        return this.findByQuery("PhysicalFile.findByUserId", userId);
    }

    public List<PhysicalFile> findByUserIdAndStatus( Long userId, Long status ) throws ServiceException{
        PhysicalFile pf = new PhysicalFile();
        pf.setUserId(userId);
        pf.setStatus(status);
        return this.findByQuery("PhysicalFile.findByUserAndStatus", pf);
    }

}
