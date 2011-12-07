/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.member.services;

import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.UserDetail;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;
import java.util.Map;

/**
 *
 * @author Long
 */
public interface UserDetailService extends CommonService<UserDetail, Long>{
    List<UserDetail> findUsers( String key, Map<String, Object> options ) throws ServiceException;
    List<UserDetail> findUsers( String key ) throws ServiceException;
}
