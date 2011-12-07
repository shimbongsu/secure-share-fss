/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services.plugins;

import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoException;
import com.s7turn.search.engine.PhysicalFile;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.utils.DigestUtil;
import com.s7turn.search.engine.utils.ImageUtil;
import java.io.File;
import java.util.List;
import java.util.Map;

/**
 *
 * @author Long
 */
public class SetIconAndThumbImagePlugin extends AbstractGingkoFileTypePlugin {

    private int thumbWidth;
    private int thumbHeight;
    private int iconWidth;
    private int iconHeight;

    private Map<String, String> fileTypeIconMapper;
    private Map<String, String> fileTypeThumbMapper;
    private List<String> imageTypes;
    private String defaultIcon;
    private String defaultThumb;

    public String getDefaultIcon() {
        return defaultIcon;
    }

    public void setDefaultIcon(String defaultIcon) {
        this.defaultIcon = defaultIcon;
    }

    public String getDefaultThumb() {
        return defaultThumb;
    }

    public void setDefaultThumb(String defaultThumb) {
        this.defaultThumb = defaultThumb;
    }
    
    public Map<String, String> getFileTypeIconMapper() {
        return fileTypeIconMapper;
    }

    public void setFileTypeIconMapper(Map<String, String> fileTypeIconMapper) {
        this.fileTypeIconMapper = fileTypeIconMapper;
    }

    public Map<String, String> getFileTypeThumbMapper() {
        return fileTypeThumbMapper;
    }

    public void setFileTypeThumbMapper(Map<String, String> fileTypeThumbMapper) {
        this.fileTypeThumbMapper = fileTypeThumbMapper;
    }

    public List<String> getImageTypes() {
        return imageTypes;
    }

    public void setImageTypes(List<String> imageTypes) {
        this.imageTypes = imageTypes;
    }
    
    public int getIconHeight() {
        return iconHeight;
    }

    public void setIconHeight(int iconHeight) {
        this.iconHeight = iconHeight;
    }

    public int getIconWidth() {
        return iconWidth;
    }

    public void setIconWidth(int iconWidth) {
        this.iconWidth = iconWidth;
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

    private boolean isImage(String fileName){
        if( this.getImageTypes() != null ){
            return this.getImageTypes().contains(this.getFileExtension(fileName));
        }
        return false;
    }

    protected String convertThumb(PhysicalFile phf, User user, int width, int height) throws GingkoException {
        try{
            String pfthumbfile = ImageUtil.convert(phf.getOriginalFileName(), width, height );
            PhysicalFile pfthumb = new PhysicalFile();
            pfthumb.setDescription(pfthumbfile);
            pfthumb.setFileName(pfthumbfile.substring(pfthumbfile.lastIndexOf("/") + 1));
            pfthumb.setOriginalFileName(pfthumbfile);
            pfthumb.setFileSize( (new File( pfthumbfile )).length() );
            pfthumb.setMd5Code(DigestUtil.getFileMd5(pfthumbfile));
            pfthumb.setStatus(phf.getStatus());
            pfthumb.setUserId(phf.getUserId());
            pfthumb.setMimeType(phf.getMimeType());
            this.getPhysicalFileService().insert(pfthumb);
            return this.getPhysicalFileService().getAccessUrl(pfthumb);
        }catch(Exception ex){
            throw new GingkoException(ex.getMessage(), ex);
        }
    }


    public boolean watch(PhysicalFile phf, GingkoContent parent,
            GingkoContent current,
            User user) throws GingkoException {
        if( phf == null ){
            throw new GingkoException("The PhysicalFile which you uploaded should not be null value.");
        }
        if( parent == null ){
            throw new GingkoException("The content must be in a parent folder.");
        }

        if( current == null ){
            throw new GingkoException("The current GingkoContent should not be null.");
        }
        
        String fileExt = this.getFileExtension(phf.getOriginalFileName());
        if( isImage (fileExt) ){
            current.setIcon(convertThumb(phf, user, this.getIconWidth(),this.getIconHeight()));
            current.setThumb(convertThumb(phf, user, this.getThumbWidth(),this.getThumbHeight()));
        }else{
            String icon = getFileTypeIconMapper() == null ? getDefaultIcon () : this.getFileTypeIconMapper().get(fileExt);
            if( icon == null ){
                icon = this.getDefaultIcon();
            }
            current.setIcon( icon );
            String thumb = this.getFileTypeThumbMapper() == null ? getDefaultThumb() : this.getFileTypeThumbMapper().get(fileExt);
            if( thumb == null ){
                thumb = this.getDefaultThumb();
            }
            current.setThumb( thumb );
        }
        
        //if( getFileTypeMimeType() != null ){
        //    current.setMimeType( getFileTypeMimeType().get(getFileExtension(phf.getOriginalFileName())));
        //}else{
        current.setMimeType( phf.getMimeType() );
        //}
        //current.setThumb(thumb);
        return true;
    }

    @Override
    public boolean accept(PhysicalFile phf) {
        if( phf == null ){
            return false;
        }

        if( phf.getId() == null || phf.getId() == 0 ){
            return false;
        }
        return true;
    }

}
