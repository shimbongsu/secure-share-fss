/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services.impl;

import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoToken;
import com.s7turn.gingkos.services.DigitalWebService;
import com.s7turn.gingkos.services.GingkoContentService;
import com.s7turn.gingkos.services.ListBag;
import com.s7turn.gingkos.services.TokenService;
import com.s7turn.search.engine.ServiceException;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.jws.WebService;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * This class provide the Web Service to be accessed by the application to search the digital contents.
 * the content will be list by from, end.
 * @author Long
 */
@WebService(endpointInterface = "com.s7turn.gingkos.services.DigitalWebService")
public class DigitalWebServiceImpl implements DigitalWebService {

    private final static Log logger = LogFactory.getLog(GingkoWebServiceImpl.class);
    private GingkoContentService  gingkoContextService;
    private TokenService tokenService;

    public GingkoContentService getGingkoContextService() {
        return gingkoContextService;
    }

    public void setGingkoContextService(GingkoContentService gingkoContextService) {
        this.gingkoContextService = gingkoContextService;
    }

    public TokenService getTokenService() {
        return tokenService;
    }

    public void setTokenService(TokenService tokenService) {
        this.tokenService = tokenService;
    }

    

    @Override
    public ListBag<GingkoContent> findContents(String condition, int from, int end) {
        GingkoContent gct = new GingkoContent();
        List<GingkoContent> gcts = new ArrayList<GingkoContent>();
        try {
            gcts = this.getGingkoContextService().findContents(parseCondition(condition), 1);
        } catch (ServiceException ex) {
            Logger.getLogger(DigitalWebServiceImpl.class.getName()).log(Level.SEVERE, null, ex);
        }
        ListBag<GingkoContent> lBag = new ListBag<GingkoContent>(gcts);
        return lBag;
    }

    private GingkoContent parseCondition(String condition){
        GingkoContent gctx = new GingkoContent();
        StringTokenizer stk = new StringTokenizer(condition, "::");
        while( stk.hasMoreTokens() ){
            String token = stk.nextToken();
            int first = token.indexOf("=");
            String name = token.substring(0, first);
            String value = token.substring(first);
            if( name.equalsIgnoreCase("name") ){
                gctx.setName(value);
            }else if( name.equalsIgnoreCase( "desc" ) ){
                gctx.setDescription(value);
            }else if( name.equalsIgnoreCase("tags")){
                gctx.setTags(value);
            }
        }
        return gctx;
    }

    @Override
    public String justPing(String tokenId) {
        GingkoToken token = null;
        try{
            token = this.getTokenService().findByToken(tokenId);
        }catch(Exception e){
            return "ERROR---" + e.getMessage();
        }
        if( token == null ){
            return "DISACTVATED";
        }
        if( token.isActive() ){
            try {
                this.getTokenService().update(token);
                return "ACTIVATED";
            } catch (ServiceException ex) {
                Logger.getLogger(DigitalWebServiceImpl.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
        return "DISACTVATED";
    }

}
