/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services.impl;

import com.s7turn.gingkos.Gingko;
import com.s7turn.gingkos.GingkoContent;
import com.s7turn.gingkos.GingkoException;
import com.s7turn.gingkos.GingkoPerment;
import com.s7turn.gingkos.GingkoToken;
import com.s7turn.gingkos.services.GingkoComponentService;
import com.s7turn.gingkos.services.GingkoContentService;
import com.s7turn.gingkos.services.GingkoFile;
import com.s7turn.gingkos.services.GingkoFolder;
import com.s7turn.gingkos.services.GingkoPermission;
import com.s7turn.gingkos.services.GingkoService;
import com.s7turn.gingkos.services.GingkoWebService;
import com.s7turn.gingkos.services.TokenService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.member.User;
import java.util.List;
import java.util.UUID;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.jws.WebService;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 *
 * @author Long
 */

@WebService(endpointInterface = "com.s7turn.gingkos.services.GingkoWebService")
public class GingkoWebServiceImpl implements GingkoWebService{

    private final static Log logger = LogFactory.getLog(GingkoWebServiceImpl.class);
    private GingkoComponentService componentService;
    private GingkoService gingkoService;
    private GingkoContentService contentService;
    private TokenService tokenService;
    private String folderIcon;
    private String folderThumb;
    

    public String getFolderIcon() {
        return folderIcon;
    }

    public void setFolderIcon(String folderIcon) {
        this.folderIcon = folderIcon;
    }

    public String getFolderThumb() {
        return folderThumb;
    }

    public void setFolderThumb(String folderThumb) {
        this.folderThumb = folderThumb;
    }


    public GingkoService getGingkoService() {
        return gingkoService;
    }

    public void setGingkoService(GingkoService gingkoService) {
        this.gingkoService = gingkoService;
    }

    public TokenService getTokenService() {
        return tokenService;
    }

    public void setTokenService(TokenService tokenService) {
        this.tokenService = tokenService;
    }

    public GingkoContentService getContentService() {
        return contentService;
    }

    public void setContentService(GingkoContentService contentService) {
        this.contentService = contentService;
    }

    public GingkoComponentService getComponentService() {
        return componentService;
    }

    public void setComponentService(GingkoComponentService componentService) {
        this.componentService = componentService;
    }
    
    @Override
    public GingkoPermission getGingkoPermission(String fileId, String tokenId) {
        //gingkoService.findContentGingkosWithUser(gct);
        GingkoToken token = null;
        try{
            token = tokenService.findByToken(tokenId);
        }catch(Exception ex){}
        if( token != null )
        {
            User u = new User();
            u.setId( token.getUserId() );
            GingkoPermission gp = new GingkoPermission();
            try {
                GingkoPerment gpt = gingkoService.findPerment(fileId);
                if( gpt != null && gpt.isDeleted() == false )
                {
                    gp.setDeleted(false);
                    gp.setCertification(gpt.getPublicKey());
                    Gingko gko = gingkoService.findGingko( gpt.getGuid(), u);
                    if( gko != null )
                    {
                        gp.setCopyable( gko.isCopy() );
                        gp.setPrintable( gko.isPrint() );
                        gp.setReadable( gko.isRead() );
                        gp.setWritable( gko.isEdit() );
                    }else ///else the returned gingko is null, the user can not view this content
                    {
                        gp.setCopyable( false );
                        gp.setPrintable( false );
                        gp.setReadable( false );
                        gp.setWritable( false );
                        gp.setDeleted(true); ///if this file can not be found,
                                             ///del it in local file system.
                    }
                }else{
                    gp.setCopyable( false );
                    gp.setPrintable( false );
                    gp.setReadable( false );
                    gp.setWritable( false );
                    gp.setDeleted(true);
                }
                
                gp.setStatus( GingkoPermission.STATUS_SUCCESS );
            } catch (ServiceException ex) {
                gp.setStatus( GingkoPermission.STATUS_ERROR );
                gp.setCertification( ex.getMessage() );
            }
            
            return gp;
        }
        return GingkoPermission.DefaultPermssion;
    }


    @Override
    public String saveGingko(String tokenId,
                                 String companyId,
                                 String fileId,
                                 String cert,
                                 String prvCert,
                                 String fileName,
                                 String desc,
                                 String tag,
                                 String version,
                                 String folderId,
                                 Long   length,
                                 byte[] contents)
    {
        //create the fileId with
        try{
            GingkoToken gtoken = tokenService.findByToken(tokenId);
            if( gtoken != null )
            {
                boolean insert = true;
                GingkoContent gct = new GingkoContent();
                
                if( fileId == null || fileId.trim().length() == 0 )
                {
                    gct.setGuid(generateUuid());
                }else
                {
                    gct.setGuid(fileId);
                    gct = getComponentService().lookup(gct).getContentService().findByCode(fileId);
                    if( gct != null )
                    {
                        insert = false;
                    }else{
                        gct = new GingkoContent();
                        gct.setGuid(fileId);
                    }
                }
                gct.setContents(contents);
                gct.setFolder(false);
                gct.setTags(tag);
                gct.setCompanyId(companyId);
                gct.setDescription(desc);
                gct.setVersion(version);
                gct.setName(fileName);
                User u = new User();
                u.setId( gtoken.getUserId() );
                gct.setOwner( u );
                gct.setLength(length);
                GingkoContent parent = contentService.findBy(folderId);
                
                if( !u.getId().equals( parent.getOwner().getId() ) )
                {
                    throw new GingkoException("No permission to create a new content in this folder.");
                }
                
                gct.setParent( parent );
                //contentService.insert(gct);
                if( !insert )
                {
                    getComponentService().lookup(gct).getContentService().update(gct);
                    return GingkoPermission.STATUS_SUCCESS;
                }
                
                ////Insert operation
                getComponentService().lookup(gct).getContentService().insert(gct);
                GingkoPerment gpt = new GingkoPerment();
                gpt.setGuid(gct.getGuid());
                gpt.setPublicKey(cert);
                gpt.setPrivateKey(prvCert);
                gingkoService.insertPerment(gpt);
                Gingko gko = new Gingko();
                gko.setUser(u);
                gko.setCopy(true);
                gko.setEdit(true);
                gko.setDefaultGingko(true);
                gko.setContent(gct);
                gko.setRead(true);
                gko.setPrint(true);
                gko.setGroup(null);
                gko.setList(true);
                gko.setPermanent(true);
                gingkoService.insert(gko);
                return GingkoPermission.STATUS_SUCCESS;
            }
            return GingkoPermission.STATUS_FAILED + "-- You given a error token id. May be you have not logged in.";
        }catch(Exception se)
        {
            logger.warn(se.getMessage());
            return GingkoPermission.STATUS_ERROR + "---" + se.getMessage();
        }
    }

    @Override
    public String createFolder(String tokenId, String companyId,
            String folderName, String tag, String desc, String folderParent) {
        GingkoContent gctx = new GingkoContent();
        gctx.setGuid( generateUuid() );
        gctx.setFolder(true);
        gctx.setName(folderName);
        gctx.setDescription(desc);        
        gctx.setTags(tag);
        gctx.setIcon(this.getFolderIcon());
        gctx.setThumb(this.getFolderThumb());

        try{
            GingkoToken token = getTokenService().findByToken(tokenId);
            User user = new User();
            user.setId(token.getUserId());

            if( folderParent != null ){
                //GingkoContent parent = new GingkoContent();
                GingkoContent parent = getComponentService().lookup(gctx).getContentService().findBy(folderParent);
                if( parent != null )
                {
                    if( parent.getOwner().getId().equals( token.getUserId() ) )
                    {
                        gctx.setParent(parent);
                    }else{
                        throw new GingkoException("You have not permission to add a GingkoContent in this folder.");
                    }
                }else
                {
                    throw new GingkoException("The parent folder should be specifield.");
                }
            }

            gctx.setOwner(user);

            gctx.setCompanyId(companyId);
            getComponentService().lookup(gctx).getContentService().insert(gctx);
            return GingkoPermission.STATUS_SUCCESS;
        }catch(Exception ex){
            logger.warn(ex.getMessage());
            return GingkoPermission.STATUS_FAILED + "---" + ex.getMessage();
        }
    }

    @Override
    public GingkoFolder[] listFolder(String tokenId, String parentFolder)  throws ServiceException
    {
        GingkoContent gctx = new GingkoContent();
        if( parentFolder != null && parentFolder.trim().length() > 0 )
        {
            GingkoContent gctxParent = new GingkoContent();
            gctxParent.setGuid(parentFolder);
            gctx.setParent(gctxParent);
        }
        try{
            GingkoToken token = getTokenService().findByToken(tokenId);
            User user = new User();
            user.setId(token.getUserId());
            gctx.setOwner(user);
            List<GingkoContent> gcts = getComponentService().lookup(gctx).getContentService().findContents(gctx, 1);
            GingkoFolder[] folders = new GingkoFolder[gcts.size()];
            int i = 0;
            for( GingkoContent gct : gcts )
            {
                folders[i++] = (GingkoFolder) GingkoFolder.fromGingkoContent(gct);
            }
            return folders;
        }catch(Exception ex){
            logger.warn(ex.getMessage());
            throw new ServiceException(ex.getMessage(), ex);
        }
    }

    @Override
    public GingkoFile[] listFiles(String tokenId, String parentFolder, String findPattern, boolean includeFolder, int start, int count)  throws ServiceException
    {
        GingkoContent gctx = new GingkoContent();
        if( parentFolder != null && parentFolder.trim().length() > 0 )
        {
            GingkoContent gctxParent = new GingkoContent();
            gctxParent.setGuid(parentFolder);
            gctx.setParent(gctxParent);
        }
        try{
            GingkoToken token = getTokenService().findByToken(tokenId);
            User user = new User();
            user.setId(token.getUserId());
            gctx.setOwner(user);
            gctx.setName(findPattern);
            List<GingkoContent> gcts = getComponentService().lookup(gctx).getContentService().findContents(gctx, includeFolder ? 0 : 2, start, count );
            GingkoFile[] files = new GingkoFile[gcts.size()];
            int i = 0;
            for( GingkoContent gct : gcts )
            {
                if( gct.isFolder() )
                {
                    files[i++] = (GingkoFolder) GingkoFolder.fromGingkoContent(gct);
                }else{
                    files[i++] =  GingkoFile.fromGingkoContent(gct);
                }
            }
            return files;
        }catch(Exception ex){
            logger.warn(ex.getMessage());
            throw new ServiceException(ex.getMessage(), ex);
        }
    }

    private String generateUuid()
    {
        UUID uid = UUID.randomUUID();
        return uid.toString().replaceAll("-", "");
    }

    @Override
    public String delete(String tokenId, String contentId) {
        GingkoContent gctx = new GingkoContent();
        gctx.setGuid( contentId );
        try {
            GingkoToken token = tokenService.findByToken(tokenId);

            if( token == null ){
                return GingkoPermission.STATUS_ERROR;
            }
            
            GingkoContent gct = this.getComponentService().lookup(gctx).getContentService().findByCode(contentId);
            if( !gct.isFolder() )
            {
                ///should be the owner.
                if( token.getUserId().equals( gct.getOwner().getId() ) ){
                    this.getComponentService().lookup(gctx).getContentService().delete(gct);
                }else{
                    throw new GingkoException("No permission for deleting this content.");
                }
            }else{
                GingkoContent query = new GingkoContent();
                query.setParent(gctx);
                User own = new User();
                own.setId(token.getUserId());
                query.setOwner(own);
                List<GingkoContent> gctList = this.getComponentService().lookup(gctx).getContentService().findContents(query, 0);

                if( gctList != null && gctList.size() > 0 ){
                    throw new GingkoException("This folder can not be deleted because there are many file or sub-folder under this folder.");
                }
                
                this.getComponentService().lookup(gctx).getContentService().delete(gct);
            }
        } catch (Exception ex) {
            Logger.getLogger(GingkoWebServiceImpl.class.getName()).log(Level.SEVERE, null, ex);
        }
        return GingkoPermission.STATUS_SUCCESS;
    }
}
