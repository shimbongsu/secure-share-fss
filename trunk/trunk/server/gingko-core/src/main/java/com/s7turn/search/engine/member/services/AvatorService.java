/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.member.services;

import com.s7turn.search.engine.member.Avator;
import com.s7turn.search.engine.services.CommonService;

/**
 *
 * @author Long
 */
public interface AvatorService extends CommonService<Avator, Long>{
    Avator findByTypeWithId( int type, Long id );
}
