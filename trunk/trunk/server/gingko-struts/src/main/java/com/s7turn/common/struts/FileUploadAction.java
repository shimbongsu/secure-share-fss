/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.common.struts;

import com.s7turn.search.engine.PhysicalFile;
import com.s7turn.search.engine.services.PhysicalFileService;
import com.s7turn.search.engine.utils.DigestUtil;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.OutputStream;
import java.security.MessageDigest;
import  java.sql.Timestamp;
import java.util.Calendar;
import java.util.Date;

/**
 *
 * @author Long
 */
public class FileUploadAction extends ActionBase {
    private static final long serialVersionUID = 572146812454l ;
    private static final int BUFFER_SIZE = 16 * 1024;
    private File[] uploadFile;
    private String[] contentType;
    private String[] fileName;
    private PhysicalFile[] uploadedFiles;
    private String caption;
    private String storePath;
    private String module;
    private String type;
    private String fileType;
    private String fileTypeDescription;
    private String postParams;

    public String getPostParams() {
        return postParams;
    }

    public void setPostParams(String postParams) {
        this.postParams = postParams;
    }

    public String getType() {
        return type;
    }

    public String getFileType() {
        return fileType;
    }

    public void setFileType(String fileType) {
        this.fileType = fileType;
    }

    public String getFileTypeDescription() {
        return fileTypeDescription;
    }

    public void setFileTypeDescription(String fileTypeDescription) {
        this.fileTypeDescription = fileTypeDescription;
    }

    public void setType(String type) {
        this.type = type;
    }
    
    public String getModule() {
        return module;
    }

    public void setModule(String module) {
        this.module = module;
    }
    
    public String getStorePath() {
        return storePath;
    }

    public void setStorePath(String storePath) {
        this.storePath = storePath;
    }
    
    public void setUploadFileContentType(String[] contentType) {
        this.contentType = contentType;
    }
    
    public void setUploadFileFileName(String[] fileName) {
        this.fileName = fileName;
    }
       
    public void setUploadFile(File[] myFile) {
        this.uploadFile = myFile;
    }
   
    public PhysicalFile[] getImageFiles() {
        return uploadedFiles;
    }
   
    public String getCaption() {
        return caption;
    }

    public void setCaption(String caption) {
        this.caption = caption;
    }
   
    private static String copy(File src, File dst) {
        try {
           InputStream in = null ;
           OutputStream out = null ;
           MessageDigest md5 = MessageDigest.getInstance("md5");
           try {
               in = new BufferedInputStream( new FileInputStream(src), BUFFER_SIZE);
               out = new BufferedOutputStream( new FileOutputStream(dst), BUFFER_SIZE);
                byte [] buffer = new byte [BUFFER_SIZE];
                int num = 0;
                while ( (num = in.read(buffer)) > 0 ) {
                   md5.update(buffer, 0, num );
                   out.write(buffer);
                }
                return DigestUtil.toHexString(md5.digest());
           } finally {
                if ( null != in) {
                   in.close();
               }
                if ( null != out) {
                   out.close();
               }
           }
       } catch (Exception e) {
           e.printStackTrace();
       }
        return null;
   }

    public PhysicalFile[] getUploadedFiles() {
        return uploadedFiles;
    }

    public void setUploadedFiles(PhysicalFile[] uploadedFiles) {
        this.uploadedFiles = uploadedFiles;
    }
   
    private static String getExtention(String fileName) {
        String fn = fileName.substring(fileName.lastIndexOf("/") + 1);
        int pos = fn.indexOf( "." );
        return fn.substring(pos);
   }
    
    public String uploadFile() throws Exception{
        if( uploadFile == null ){
            addActionError("error.fileSizeExceeds");
            return "error";
        }
        
        uploadedFiles = new PhysicalFile[uploadFile.length];
        for( int kk = 0; kk < this.uploadFile.length; kk ++ ){
               String uploadedFilename = new Date().getTime() + ""; // + getExtention(fileName);
               Timestamp tst = new Timestamp( Calendar.getInstance().getTimeInMillis() );
               String path = String.format(getStorePath(), tst, this.getModule(), this.getType(), this.getType());
               File pathFile = new File( path );
               try{
                   pathFile.mkdirs();
               }catch(Exception e){}
               int i = 0;
               String ext = getExtention(fileName[kk]);
               String namedUploadedFile = uploadedFilename + "X" + i + ext;
               File imageFile = new File( path + "/" + namedUploadedFile );
               while(imageFile.exists()){
                   i ++;
                   namedUploadedFile = uploadedFilename + "X" + i + ext;
                   imageFile = new File( path + "/" + namedUploadedFile );
               }
               String md5Code = copy( uploadFile[kk], imageFile );
               if( md5Code != null ){
                   PhysicalFile phyfile = new PhysicalFile();
                   phyfile.setFileName(namedUploadedFile);
                   phyfile.setMimeType(this.contentType[kk]);
                   phyfile.setFileSize(uploadFile[kk].length());
                   phyfile.setStatus(0L);
                   phyfile.setOriginalFileName(path + "/" + namedUploadedFile);
                   phyfile.setMd5Code(md5Code);
                   if( getPhysicalFileService() != null ){
                       getPhysicalFileService().insert(phyfile);
                   }
                   this.uploadedFiles[kk] = phyfile;
               }
           }
       return "success";
   } 
}
