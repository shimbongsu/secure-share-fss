/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.gingkos.GingkoContent;

/**
 *
 * @author Long
 */
public class GingkoFolder extends GingkoFile {

    public GingkoFolder()
    {
    }

    public GingkoFolder(GingkoContent ct)
    {
        super(ct);
    }
    
    @Override
    public boolean isFolder()
    {
        return true;
    }

    public static GingkoFolder fromGingkoContent(GingkoContent ct)
    {
        if( ct.isFolder() )
        {
            return new GingkoFolder( ct );
        }
        return null;
    }
}
