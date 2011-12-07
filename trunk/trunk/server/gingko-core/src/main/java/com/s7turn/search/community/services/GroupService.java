/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community.services;

import com.s7turn.search.community.Group;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;

/**
 *
 * @author Long
 */
public interface GroupService extends CommonService<Group, Long> {
    /**
     * List all group by the owner.
     * @param userId
     * @return
     * @throws com.s7turn.search.engine.ServiceException
     */
    List<Group> findByOwner(Long userId) throws ServiceException;

    List<Group> findGroupByMember( User u ) throws ServiceException;
    


}
