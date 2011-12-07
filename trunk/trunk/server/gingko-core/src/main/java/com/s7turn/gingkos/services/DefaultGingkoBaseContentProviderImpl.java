/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoException;
import java.util.List;

/**
 *
 * @author Long
 */
public class DefaultGingkoBaseContentProviderImpl extends DefaultGingkoFolderProviderImpl{

    @Override
    public List<GingkoContent> fetch(GingkoContent folder) throws GingkoException {
        return null;
    }

    @Override
    public List<GingkoContent> fetchFolders(GingkoContent folder) throws GingkoException {
        return null;
    }

    @Override
    public List<GingkoContent> fetchContents(GingkoContent folder) throws GingkoException {
        return null;
    }

}
