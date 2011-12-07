/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import java.util.List;

/**
 *
 * @author Long
 */
public class ListBag<E> {
    private List<E> items;
    private int count;

    public ListBag(){
        count = 0;
    }

    public ListBag(List<E> es){
        items = es;
        count = es.size();
    }

    public List<E> getItems(){
        return items;
    }

    public void setItems(List<E> its){
        items = its;
    }

    public int getCount() {
        return count;
    }

    public void setCount(int count) {
        this.count = count;
    }
    
}
