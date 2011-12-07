/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.community.struts;

import com.opensymphony.xwork2.Action;
import com.s7turn.common.struts.ActionBase;
import com.s7turn.common.struts.FileUploadAction;
import com.s7turn.search.community.Gallery;
import com.s7turn.search.community.Photo;
import com.s7turn.search.community.services.GalleryService;
import com.s7turn.search.community.services.PhotoService;
import com.s7turn.search.engine.PhysicalFile;
import com.s7turn.search.engine.utils.DigestUtil;
import com.s7turn.search.engine.utils.ImageUtil;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URLDecoder;
import java.util.List;

/**
 *
 * @author Long
 */
public class GalleryAction extends FileUploadAction {
    private int galleryType;
    private Long galleryId;
    private Gallery gallery;
    private Long photoId;
    private Photo photo;
    private List<Photo> photos;
    private PhotoService photoService;
    private GalleryService galleryService;
    private List<Gallery> galleryList;
    private Long[] arrayPhotoId;

    private int thumbWidth;
    private int thumbHeight;
    private int smallWidth;
    private int smallHeight;
    private float scale;
    private int left;
    private int top;
    private int width;
    private int height;

    public Long[] getArrayId() {
        return arrayPhotoId;
    }

    public void setArrayId(Long[] arrayPhotoId) {
        this.arrayPhotoId = arrayPhotoId;
    }
    
    public int getGalleryType() {
        return galleryType;
    }

    public void setGalleryType(int galleryType) {
        this.galleryType = galleryType;
    }
    
    public GalleryAction(){
        this.setNavigator("account");
    }

    public int getSmallHeight() {
        return smallHeight;
    }

    public void setSmallHeight(int smallHeight) {
        this.smallHeight = smallHeight;
    }

    public int getSmallWidth() {
        return smallWidth;
    }

    public void setSmallWidth(int smallWidth) {
        this.smallWidth = smallWidth;
    }

    public int getThumbHeight() {
        return thumbHeight;
    }

    public void setThumbHeight(int thumbHeight) {
        this.thumbHeight = thumbHeight;
    }

    public int getThumbWidth() {
        return thumbWidth;
    }

    public void setThumbWidth(int thumbWidth) {
        this.thumbWidth = thumbWidth;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public int getLeft() {
        return left;
    }

    public void setLeft(int left) {
        this.left = left;
    }

    public float getScale() {
        return scale;
    }

    public void setScale(float scale) {
        this.scale = scale;
    }

    public int getTop() {
        return top;
    }

    public void setTop(int top) {
        this.top = top;
    }

    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }
    
    

    public List<Gallery> getGalleryList() {
        return galleryList;
    }

    public void setGalleryList(List<Gallery> galleryList) {
        this.galleryList = galleryList;
    }

    public Gallery getGallery() {
        return gallery;
    }

    public void setGallery(Gallery gallery) {
        this.gallery = gallery;
    }

    public Long getGalleryId() {
        return galleryId;
    }

    public void setGalleryId(Long galleryId) {
        this.galleryId = galleryId;
    }

    public List<Photo> getPhotos() {
        return photos;
    }

    public void setPhotos(List<Photo> photos) {
        this.photos = photos;
    }
    
    public GalleryService getGalleryService() {
        return galleryService;
    }

    public void setGalleryService(GalleryService galleryService) {
        this.galleryService = galleryService;
    }

    public Photo getPhoto() {
        return photo;
    }

    public void setPhoto(Photo photo) {
        this.photo = photo;
    }

    public Long getPhotoId() {
        return photoId;
    }

    public void setPhotoId(Long photoId) {
        this.photoId = photoId;
    }

    public PhotoService getPhotoService() {
        return photoService;
    }

    public void setPhotoService(PhotoService photoService) {
        this.photoService = photoService;
    }
    
    @Override
    public InputStream getInputStream() throws IOException{
        return new FileInputStream( this.getPhoto().getFileUrl() );
    }
    
    public String create() throws Exception{
        Gallery g = this.getGallery();
        g.setCreateBy( this.getCurrentUser().getId() );
        getGalleryService().insert(g);
        this.setActionStatus(SUCCESS);
        return ActionBase.SUCCESS;
    }
    
    public String update() throws Exception{
        Gallery g = this.getGallery();
        if( g.getCreateBy() == this.getCurrentUser().getId() ){
            getGalleryService().update(g);
            this.setActionStatus(SUCCESS);
            return ActionBase.SUCCESS;
        }
        return "failed";
    }
    
    public String view() throws Exception{
        Long id = this.getGalleryId();
        if( id != null && id > 0 ){
            this.setGallery(this.getGalleryService().findBy(id));
            this.setPhotos( this.getPhotoService().findByGallery(id));
        }else{
            Gallery g = new Gallery();
            if( getGalleryType() > 0 ){
                g.setGalleryType(this.getGalleryType());
            }else{
                g.setGalleryType(1);
            }
            this.setGallery(g);
        }
        
        this.setPostParams("galleryId : \"" + this.getGalleryId() + "\"" );
        this.setActionStatus(SUCCESS);
        return ActionBase.SUCCESS;
    }
   
    public String list() throws Exception{
        Long uid = this.getUserId();
        if( uid == null ){
            uid = this.getCurrentUser().getId();
        }
        this.setGalleryList(computer(getGalleryService().findByOwner(uid, 1)));
        this.setActionStatus(SUCCESS);
        return ActionBase.SUCCESS;
    }
    
    public String uploadPhoto() throws Exception{
        String actionReturn = uploadFile();
        if( actionReturn.equals("success" ) ){
            PhysicalFile[] phyfiles = this.getUploadedFiles();
            for( PhysicalFile pf : phyfiles ){
                String pfthumbfile = ImageUtil.convert(pf.getOriginalFileName(), getThumbWidth(), getThumbHeight() );
                PhysicalFile pfthumb = new PhysicalFile();
                pfthumb.setDescription(pfthumbfile);
                pfthumb.setFileName(pfthumbfile.substring(pfthumbfile.lastIndexOf("/") + 1));
                pfthumb.setOriginalFileName(pfthumbfile);
                pfthumb.setFileSize( (new File( pfthumbfile )).length() );
                pfthumb.setMd5Code(DigestUtil.getFileMd5(pfthumbfile));
                pfthumb.setStatus(pf.getStatus());
                pfthumb.setUserId(pf.getUserId());
                pfthumb.setMimeType(pf.getMimeType());                
                this.getPhysicalFileService().insert(pfthumb);
                
                String pfsmallFile = ImageUtil.convert(pf.getOriginalFileName(), getSmallWidth(), getSmallHeight() );
                PhysicalFile pfsmall = new PhysicalFile();
                pfsmall.setDescription(pf.getDescription());
                pfsmall.setFileName(pfsmallFile.substring(pfsmallFile.lastIndexOf("/") + 1));
                pfsmall.setOriginalFileName(pfsmallFile);
                pfsmall.setFileSize( (new File( pfsmallFile )).length() );
                pfsmall.setMd5Code(DigestUtil.getFileMd5(pfsmallFile));
                pfsmall.setStatus(pf.getStatus());
                pfsmall.setUserId(pf.getUserId());
                pfsmall.setMimeType(pf.getMimeType());        
                this.getPhysicalFileService().insert(pfsmall);
                
                Photo ph = new Photo();
                ph.setUserId(this.getCurrentUser().getId());
                ph.setGalleryId(this.getGalleryId());
                try{
                    ph.setDescription( URLDecoder.decode( this.getPhoto().getDescription(), "utf-8" ));
                }catch(Exception e){
                    logger.info( e.getMessage(), e );
                }
                ph.setFileSize( pf.getFileSize() + "" );
                ph.setFileType( pf.getMimeType() );
                ph.setFileId(pf.getId());
                ph.setFileUrl( getPhysicalFileService().getAccessUrl(pf));
                ph.setThumbFileId(pfthumb.getId());
                ph.setThumbFileUrl( getPhysicalFileService().getAccessUrl(pfthumb) );
                ph.setSmallFileId(pfsmall.getId());
                ph.setSmallFileUrl( getPhysicalFileService().getAccessUrl(pfsmall) );

                if( this.getPhoto().getName() == null || "".equals( this.getPhoto().getName().trim() ) ){
                    ph.setName( pf.getFileName() );
                }else{
                    try{
                        ph.setName( URLDecoder.decode( this.getPhoto().getName(),"utf-8" ));
                    }catch(Exception e){
                        logger.info( e.getMessage(), e );
                    }

                }
                try{
                    ph.setTags( URLDecoder.decode(this.getPhoto().getTags(), "utf-8"));
                }catch(Exception e){
                    logger.info( e.getMessage(), e );
                }
                getPhotoService().insert(ph);
            }
        }
        this.setActionStatus(SUCCESS);
        return actionReturn;
    }
    
    public String jsonMock() throws Exception{
        return Action.SUCCESS;
    }
    
    public String photosList() throws Exception{
        Long gid = this.getGalleryId();
        this.setGallery(this.getGalleryService().findBy(gid));
        this.setPhotos( computer(this.getPhotoService().findByGallery(gid)) );
        this.setActionStatus(SUCCESS);
        return Action.SUCCESS;
    }
    
    public String viewPhoto() throws Exception{
        Photo ph = this.getPhotoService().findBy(getPhotoId());
        this.setPhoto(ph);
        this.setActionStatus(SUCCESS);
        return Action.SUCCESS;
    }
    
    public String photoCropSave() throws Exception{
        Photo ph = this.getPhotoService().findBy(getPhotoId());
        if( ph != null ){
            PhysicalFile pf = getPhysicalFileService().findBy(ph.getFileId());
            String orf = pf.getOriginalFileName();
            if( "thumb".equals(this.getType()) ){
                ImageUtil.convert( orf, getScale(), getLeft(), getTop(), getWidth(), getHeight(),
                    this.getThumbWidth(), this.getThumbHeight());                
            }else if( "small".equals(this.getType()) ){
                ImageUtil.convert( orf, getScale(), getLeft(), getTop(), getWidth(), getHeight(),
                    this.getSmallWidth(), this.getSmallHeight());
            }
        }
        this.setActionStatus(SUCCESS);
        return Action.SUCCESS;
    }
    
    public String mock() throws Exception {
        return "success";
    }

    public String delete() throws Exception {
        Long phId = this.getPhotoId();
        Photo ph = new Photo();
        if( phId != null ){
            ph.setId(phId);
            this.getPhotoService().delete(ph);
        }else if( this.getArrayId() != null ){
            Long[] arraypids =  this.getArrayId();
            for( Long aid : arraypids ){
                ph.setId( aid );
                this.getPhotoService().delete(ph);
            }
        }
        this.setActionStatus(SUCCESS);
        return Action.SUCCESS;
    }
}
