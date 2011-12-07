/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.member.services;

import com.s7turn.search.engine.member.AccessHistory;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;

/**
 *
 * @author Long
 */
public interface AccessHistoryService  extends CommonService<AccessHistory, Long>{
    List<AccessHistory> findByAccessBy(int cate, Long whereId) throws ServiceException;
    List<AccessHistory> findByAccessTo(int cate, Long whoId) throws ServiceException;
}
