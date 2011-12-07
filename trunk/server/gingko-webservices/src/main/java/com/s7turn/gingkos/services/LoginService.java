/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.GingkoToken;
import javax.jws.WebMethod;
import javax.jws.WebParam;
import javax.jws.WebService;

/**
 *
 * @author Long
 */
@WebService
public interface LoginService {
    
    @WebMethod(operationName = "GingkoLogin", exclude = false)
    GingkoToken GingkoLogin(
                @WebParam(name="userid")String userName,
                @WebParam(name="password")String password);

    @WebMethod(operationName = "GingkoRegister", exclude = false)
    GingkoToken GingkoRegister( 
                @WebParam(name="email")String email,
                @WebParam(name="password")String password,
                @WebParam(name="retryPassword")String retryPassword,
                @WebParam(name="name")String name,
                @WebParam(name="idcard")String idcard,
                @WebParam(name="certType")String certType,
                @WebParam(name="tokenId")String tokenId
                );
}
