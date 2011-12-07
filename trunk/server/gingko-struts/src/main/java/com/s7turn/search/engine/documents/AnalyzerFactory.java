/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.documents;

import org.apache.lucene.analysis.Analyzer;

/**
 *
 * @author Long
 */
public interface AnalyzerFactory {
    Analyzer createAnalyzer();
}
