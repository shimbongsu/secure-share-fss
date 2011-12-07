/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.GingkoContent;
import javax.jws.WebMethod;
import javax.jws.WebParam;
import javax.jws.WebResult;
import javax.jws.WebService;

/**
 *
 * @author Long
 */
@WebService
public interface DigitalWebService
{
    @WebMethod(operationName = "findContents", exclude = false)
    @WebResult(name="contentList")
    ListBag<GingkoContent> findContents(
            @WebParam(name="searchCondition")String condition,
            @WebParam(name="from")int from,
            @WebParam(name="end")int end);

    @WebMethod(operationName = "justPing", exclude = false)
    @WebResult(name="pingResult")
    String justPing(@WebParam(name="tokenId")String tokenId);
}
