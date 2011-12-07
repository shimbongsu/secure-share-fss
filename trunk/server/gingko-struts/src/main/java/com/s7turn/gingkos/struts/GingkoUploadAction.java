/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.struts;

import com.s7turn.common.struts.ActionBase;
import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.services.GingkoComponentService;
import com.s7turn.gingkos.services.GingkoFileTypePlugin;
import com.s7turn.gingkos.services.GingkoMimeTypeMapper;
import com.s7turn.search.engine.PhysicalFile;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.utils.DigestUtil;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.MessageDigest;
import java.sql.Timestamp;
import java.util.Calendar;
import java.util.Date;
import java.util.List;

/**
 *
 * @author Long
 */
public class GingkoUploadAction extends ActionBase{
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
    private String gingkoId;
    private Long gingkoUserId;
    private String description;
    private String tags;
    private String name;
    private GingkoMimeTypeMapper mapper;


    private GingkoContent currentContent;
    private List<GingkoFileTypePlugin> plugins;
    private GingkoComponentService gingkoComponentService;

    public GingkoMimeTypeMapper getMapper() {
        return mapper;
    }

    public void setMapper(GingkoMimeTypeMapper mapper) {
        this.mapper = mapper;
    }

    public GingkoComponentService getGingkoComponentService() {
        return gingkoComponentService;
    }

    public void setGingkoComponentService(GingkoComponentService gingkoComponentService) {
        this.gingkoComponentService = gingkoComponentService;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public String getTags() {
        return tags;
    }

    public void setTags(String tags) {
        this.tags = tags;
    }

    public Long getGingkoUserId() {
        return gingkoUserId;
    }

    public void setGingkoUserId(Long gingkoUserId) {
        this.gingkoUserId = gingkoUserId;
    }

    
    public GingkoContent getFolder() {
        return currentContent;
    }

    public void setFolder(GingkoContent currentContent) {
        this.currentContent = currentContent;
    }

    
    public String getGingkoId() {
        return gingkoId;
    }

    public void setGingkoId(String gingkoId) {
        this.gingkoId = gingkoId;
    }
    
    public List<GingkoFileTypePlugin> getPlugins() {
        return plugins;
    }

    public void setPlugins(List<GingkoFileTypePlugin> plugins) {
        this.plugins = plugins;
    }
    
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
            setActionStatus("error.fileSizeExceeds");
            return "error";
        }

        GingkoContent gctxFolder = this.getFolder();
        if( gctxFolder == null || (this.getGingkoId() != null ) ){
            gctxFolder = new GingkoContent();
            gctxFolder.setGuid( this.getGingkoId() );
            User o = new User();
            o.setId(this.getGingkoUserId());
            gctxFolder.setOwner(o);
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
                   if( getMapper() != null ){
                        phyfile.setMimeType(this.getMapper().getMimeType(ext));
                   }
                   phyfile.setOriginalFileName(path + "/" + namedUploadedFile);
                   phyfile.setMd5Code(md5Code);
                   if( getPhysicalFileService() != null ){
                       getPhysicalFileService().insert(phyfile);
                       List<GingkoFileTypePlugin> plugs = this.getPlugins();
                       GingkoContent curr = new GingkoContent();
                       curr.setParent(gctxFolder);
                       curr.setName(fileName[kk]);
                       curr.setDescription(this.getDescription());
                       curr.setLength(phyfile.getFileSize());
                       curr.setFolder(false);
                       curr.setRelatedId(phyfile.getId());
                       curr.setType("physicalFile");
                       curr.setTags(this.getTags());
                       curr.setVersion("0.1.0.0");
                       curr.setOwner(this.getCurrentUser());
                       curr.setStorage("extenal");
                       curr.setUrl( getPhysicalFileService().getAccessUrl(phyfile) );
                       for( GingkoFileTypePlugin gftsp : plugs ){
                           if( gftsp != null ){
                               if( gftsp.accept(phyfile) ){
                                   gftsp.watch( phyfile, gctxFolder, curr,
                                           getCurrentUser());
                               }
                           }
                       }
                       getGingkoComponentService().lookup(curr).getContentService().insert(curr);
                   }
                   this.uploadedFiles[kk] = phyfile;
               }
           }
       setActionStatus(ActionBase.SUCCESS);
       return "success";
   } 
}
