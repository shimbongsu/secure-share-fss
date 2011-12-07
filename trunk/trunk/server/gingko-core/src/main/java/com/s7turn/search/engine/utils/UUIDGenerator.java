/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.utils;

import java.util.UUID;

/**
 *
 * @author Long
 */
public abstract class UUIDGenerator {
    private UUIDGenerator(){}
    public static String getNewUuid(){
        return UUID.randomUUID().toString();
    }
}
