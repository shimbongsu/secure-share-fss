/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.services.utils;

/**
 *
 * @author Long
 */
public class Page {
        private String name;
        private boolean actual;
        private String url;
        private boolean current;

    public boolean isCurrent() {
        return current;
    }

    public void setCurrent(boolean current) {
        this.current = current;
    }

        
        public boolean isActual() {
            return actual;
        }

        public void setActual(boolean actual) {
            this.actual = actual;
        }

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        public String getUrl() {
            return url;
        }

        public void setUrl(String url) {
            this.url = url;
        }
}
