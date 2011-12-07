/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.common.struts;

import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.PhysicalFileService;
import com.s7turn.search.engine.utils.ServiceFactory;
import javax.servlet.ServletContext;
import org.springframework.web.context.WebApplicationContext;
import org.springframework.web.context.support.WebApplicationContextUtils;

/**
 *
 * @author Long
 */
public class SpringServiceFactory implements ServiceFactory{
    private WebApplicationContext wactx = null;
    public SpringServiceFactory( ServletContext context ){
        wactx = WebApplicationContextUtils.getWebApplicationContext(context);
    }


    @Override
    public PhysicalFileService getPhysicalFileService() throws ServiceException {
        return (PhysicalFileService)wactx.getBean("physicalFileServiceBean");
    }

}
