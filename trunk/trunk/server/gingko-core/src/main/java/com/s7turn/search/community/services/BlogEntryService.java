/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.community.services;

import com.s7turn.search.community.BlogEntry;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.services.CommonService;
import java.util.List;

/**
 *
 * @author Long
 */
public interface BlogEntryService extends CommonService<BlogEntry, Long> {
    List<BlogEntry> findByBlog( Long blogId ) throws ServiceException;
    List<BlogEntry> findByBlog( Long blogId, Long userId ) throws ServiceException;
}
