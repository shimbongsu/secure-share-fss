/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.Gingko;
import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoPerment;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;

/**
 *
 * @author Long
 */
public interface GingkoService extends CommonService<Gingko, Long>{
    List<Gingko> findContentGingkos( GingkoContent gct ) throws ServiceException;
    List<Gingko> findContentGingkosWithUser(GingkoContent gct) throws ServiceException;
    Gingko findGingko( String guid, User user ) throws ServiceException;
    GingkoPerment findPerment(String guid) throws ServiceException;
    void deletePerment(String guid) throws ServiceException;
    void insertPerment(GingkoPerment gpt) throws ServiceException;
}
