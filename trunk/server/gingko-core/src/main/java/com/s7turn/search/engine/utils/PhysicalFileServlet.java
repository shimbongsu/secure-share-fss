/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.utils;

import com.s7turn.search.engine.PhysicalFile;
import com.s7turn.search.engine.ServiceException;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 *
 * @author Long
 */
public class PhysicalFileServlet extends HttpServlet {
   
    /** 
    * Processes requests for both HTTP <code>GET</code> and <code>POST</code> methods.
    * @param request servlet request
    * @param response servlet response
    */
    protected void processRequest(HttpServletRequest request, HttpServletResponse response)
    throws ServletException, IOException {
        OutputStream out = new BufferedOutputStream( response.getOutputStream() );
        InputStream bufis = null;
        try {
            String uri = request.getRequestURI();
            String[] uriparams = uri.split("/");
            Long physicalId = Long.parseLong(uriparams[ uriparams.length - 1]);
            PhysicalFile pf = null;
            String serviceFactoryClazz = getServletContext().getInitParameter("serviceFactory");
            ServiceFactoryBuilder.setServiceFactoryClass(serviceFactoryClazz);
            pf = ServiceFactoryBuilder.getInstance(getServletContext()).getPhysicalFileService().findBy(physicalId);
            response.setContentType( pf.getMimeType() );
            File storeFile = new File( pf.getOriginalFileName() ); 
            bufis = new BufferedInputStream( new FileInputStream( storeFile ) );
            byte[] buff = new byte[10240];
            int num = 0;
            while( (num = bufis.read(buff)) > 0 ){
                out.write(buff, 0, num);
            }
        } catch(ServiceException se){
            throw new ServletException(se.getMessage(), se);
        }finally {
            if( out != null ){
                out.close();
            }
            if( bufis != null ){
                try{ bufis.close(); }catch(Exception e){}
            }
        }
    } 

    // <editor-fold defaultstate="collapsed" desc="HttpServlet 方法。单击左侧的 + 号以编辑代码。">
    /** 
    * Handles the HTTP <code>GET</code> method.
    * @param request servlet request
    * @param response servlet response
    */
    protected void doGet(HttpServletRequest request, HttpServletResponse response)
    throws ServletException, IOException {
        processRequest(request, response);
    } 

    /** 
    * Handles the HTTP <code>POST</code> method.
    * @param request servlet request
    * @param response servlet response
    */
    protected void doPost(HttpServletRequest request, HttpServletResponse response)
    throws ServletException, IOException {
        processRequest(request, response);
    }

    /** 
    * Returns a short description of the servlet.
    */
    public String getServletInfo() {
        return "Physical File Viewer";
    }// </editor-fold>

}

