/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services.impl;

import com.s7turn.gingkos.GingkoToken;
import com.s7turn.gingkos.services.*;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.member.services.UserService;
import javax.annotation.Resource;
import javax.jws.WebService;
import javax.xml.ws.WebServiceContext;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 *
 * @author Long
 */
@WebService(endpointInterface = "com.s7turn.gingkos.services.LoginService")
public class LoginServiceImpl implements LoginService
{

    private final static Log logger = LogFactory.getLog(GingkoWebServiceImpl.class);
    //private static Logger logger = Logger.getLogger("LoginService");
    @Resource
    WebServiceContext wsContext;
    
    private TokenService tokenService;
    private UserService  userService;
    public TokenService getTokenService() {
        return tokenService;
    }

    public void setTokenService(TokenService tokenService) {
        this.tokenService = tokenService;
    }

    public UserService getUserService() {
        return userService;
    }

    public void setUserService(UserService userService) {
        this.userService = userService;
    }


    
    @Override
    public GingkoToken GingkoLogin(String userName, String password)
    {
        logger.debug( "WebServiceContext is " + (wsContext == null ? "null" : " not null.") );
        GingkoToken gtk = new GingkoToken();
        try{
            User user = userService.findByCode(userName);
            
            if( user != null )
            {
                if( user.getPassword() == null )
                {
                    gtk.setActive(false);
                    gtk.setToken("User have not set password already. can not logged in.");
                    gtk.setUserId(user.getId());
                    return gtk;
                }
                
                if( user.getPassword().equals(password) )
                {
                    ///generate a new token.
                    gtk = tokenService.findByUser(user, true);
                }
            }
        }catch(ServiceException se)
        {
            gtk.setActive(false);
            gtk.setAddress("");
            gtk.setToken(se.getMessage());
        }
        return gtk;
    }

    @Override
    public GingkoToken GingkoRegister(String email,
            String password,
            String retryPassword,
            String name,
            String idcard,
            String certType,
            String tokenId) {
        GingkoToken gTk = new GingkoToken();
        User us = new User();
        us.setEmail(email);
        us.setDisabled(false);
        us.setFullName(name);
        us.setPassword(password);
        us.setLoginId(email);
        us.setScreenName(name);
        us.setLocked(false);
        try{
            getUserService().insert(us);
            gTk.setActive(false);
            gTk.setUserId(us.getId());
            gTk.setToken("");
        }catch(ServiceException se){
            gTk.setToken( GingkoPermission.STATUS_ERROR );
        }
        return gTk;
    }
}
