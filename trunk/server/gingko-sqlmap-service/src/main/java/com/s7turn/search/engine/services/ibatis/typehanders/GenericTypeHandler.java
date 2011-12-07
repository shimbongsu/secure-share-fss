/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.services.ibatis.typehanders;

import com.ibatis.sqlmap.engine.type.TypeHandler;
import java.sql.CallableStatement;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Iterator;
import java.util.Map;

/**
 *
 * @author Long
 */
public abstract class GenericTypeHandler<E> implements TypeHandler{

    private String idProperty;
    private Class instClass;
    
    public GenericTypeHandler( Class clz, String idp ){
        idProperty = idp;
        instClass = clz;
    }
    
    public abstract Map<String, String> getFieldMapper();
    
    public void setParameter(PreparedStatement ps, int index, Object value, String name) throws SQLException {
        if( value.getClass().equals(instClass) ){
            ps.setObject(index, TypeHandlerUtils.getObject(value, idProperty));
        }
    }

    public Object getResult(ResultSet result, String fname) throws SQLException {
        Object res = TypeHandlerUtils.newInstance(instClass);
        TypeHandlerUtils.populate(result, res, fname, idProperty);
        Iterator<Map.Entry<String, String>> its = this.getFieldMapper().entrySet().iterator();
        while( its.hasNext() ){
            Map.Entry<String, String> valuePair =  its.next();
            TypeHandlerUtils.populate(result, res, valuePair.getValue(), valuePair.getKey());
        }
        return res;        
    }

    public Object getResult(ResultSet result, int index) throws SQLException {
        Object idp = result.getObject(index);
        Object res = TypeHandlerUtils.newInstance(instClass);
        TypeHandlerUtils.setProperty(res, idProperty, idp);
        Iterator<Map.Entry<String, String>> its = this.getFieldMapper().entrySet().iterator();
        while( its.hasNext() ){
            Map.Entry<String, String> valuePair =  its.next();
            TypeHandlerUtils.populate(result, res, valuePair.getValue(), valuePair.getKey());
        }
        return res;        
    }

    public Object getResult(CallableStatement result, int index) throws SQLException {
        Object idp = result.getObject(index);
        Object res = TypeHandlerUtils.newInstance(instClass);
        TypeHandlerUtils.setProperty(res, idProperty, idp);
        Iterator<Map.Entry<String, String>> its = this.getFieldMapper().entrySet().iterator();
        while( its.hasNext() ){
            Map.Entry<String, String> valuePair =  its.next();
            TypeHandlerUtils.populate(result, res, valuePair.getValue(), valuePair.getKey());
        }
        return res;
    }

    public Object valueOf(String value) {        
        E g = (E)TypeHandlerUtils.newInstance(instClass);
        TypeHandlerUtils.setProperty(g, idProperty, value);
        return g;
    }

    public boolean equals(Object value, String code) {
        if( value.getClass().equals(instClass) ){
            if( code == null )
                return false;
            return code.equals( TypeHandlerUtils.getString(value, idProperty) );
        }
        return false;
    }

}
