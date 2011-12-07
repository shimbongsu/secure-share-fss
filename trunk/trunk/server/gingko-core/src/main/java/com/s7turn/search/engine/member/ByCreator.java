/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.member;

/**
 *
 * @author Long
 */
public interface ByCreator {
    User getOwner();
    void setOwner( User user );
}
