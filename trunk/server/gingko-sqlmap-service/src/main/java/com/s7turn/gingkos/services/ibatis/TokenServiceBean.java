/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services.ibatis;

import com.s7turn.gingkos.GingkoToken;
import com.s7turn.gingkos.services.TokenService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.member.services.UserService;
import com.s7turn.search.engine.services.ibatis.CommonServiceBean;
import com.s7turn.search.engine.utils.DigestUtil;
import java.util.List;

/**
 *
 * @author Long
 */
public class TokenServiceBean  extends CommonServiceBean<GingkoToken, Long> implements TokenService{
    private UserService userService;

    public TokenServiceBean()
    {
        super(GingkoToken.class);
    }

    public UserService getUserService() {
        return userService;
    }

    public void setUserService(UserService userService) {
        this.userService = userService;
    }



    private GingkoToken createToken( Long userId ) throws ServiceException
    {
        try{
            getSqlMapClient().update("GingkoToken.deactiveToken", userId);
        }catch(Exception e){
            ///ignore this exception.
            ///if this SQL runs failed, may be there are two active thread for a single user.
            ///it's allowed.
        }
        
        GingkoToken gtk = new GingkoToken();
        gtk.setActive(true);
        gtk.setAddress("");
        gtk.setUserId(userId);
        gtk.setToken(DigestUtil.generateToken());
        this.insert(gtk);
        return gtk;
    }

    public GingkoToken findByUser(Long userId, boolean withCreate) throws ServiceException{
        List<GingkoToken> gtkList = null;
        if( withCreate )
        {
            return createToken(userId);
        }
        
        try{
            gtkList =  this.findByQuery("GingkoToken.findByUserId", userId);
        }catch(Exception e){}

        if( (gtkList == null || gtkList.size() == 0) )
        {
            ///Create a new Token
            return createToken(userId);
        }
        return gtkList.get(0);
    }

    public GingkoToken findByUser(String loginId, boolean withCreate) throws ServiceException{
        User u = this.getUserService().findByCode(loginId);
        return findByUser(u, withCreate);
    }


    public GingkoToken findByUser( User user, boolean withCreate) throws ServiceException{
        List<GingkoToken> gtkList = null;
        Long userId = 0L;
        

        if( user != null )
        {
            userId = user.getId();
        }

        if( withCreate )
        {
            return createToken(userId);
        }

        try{
            gtkList = this.findByQuery("GingkoToken.findByUserId", userId);
        }catch(Exception e){}

        if( (gtkList == null || gtkList.size() == 0) )
        {
            ///Create a new Token
            return createToken(userId);
        }
        return gtkList.get(0);
    }

    public GingkoToken findByToken(String token) throws ServiceException{
        List<GingkoToken> gtkList = null;
        try{
            gtkList = this.findByQuery("GingkoToken.findByToken", token);
        }catch(Exception e){}

        if( gtkList != null && gtkList.size() > 0 )
        {
            return gtkList.get(0);
        }
        return null;
    }
}
