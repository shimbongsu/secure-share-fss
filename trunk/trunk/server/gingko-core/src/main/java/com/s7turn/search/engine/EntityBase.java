/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine;

import java.io.Serializable;
import java.sql.Timestamp;

/**
 *
 * @author Long
 */
public interface EntityBase<PK extends Serializable> {
    PK getId();
    void setId(PK id);
    Timestamp getLastUpdatedTime();
    void setLastUpdatedTime(Timestamp ts);
}
