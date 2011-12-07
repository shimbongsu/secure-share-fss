/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.GingkoToken;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.services.CommonService;

/**
 *
 * @author Long
 */
public interface TokenService extends CommonService<GingkoToken, Long>{
    GingkoToken findByUser(Long userId, boolean withCreate) throws ServiceException;
    GingkoToken findByUser(String loginId, boolean withCreate) throws ServiceException;
    GingkoToken findByToken(String token) throws ServiceException;
    GingkoToken findByUser( User user, boolean widthCreate) throws ServiceException;
}
