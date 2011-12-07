/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.gingkos.services;

import com.s7turn.search.engine.ServiceException;
import javax.jws.WebMethod;
import javax.jws.WebService;
import javax.jws.WebParam;
import javax.jws.WebResult;

/**
 *
 * @author Long
 */
@WebService
public interface GingkoWebService
{
    /**
     * FileId is the identify of the file, the message pack will be show out the GingkoPermission struct.
     * if the user have not permission, the GingkoPermission should shown.
     * @param fileId
     * @param tokenId
     * @return
     */
    @WebMethod(operationName = "getGingkoPermission", exclude = false)
    @WebResult(name="gingkoPermission")
    GingkoPermission getGingkoPermission(@WebParam(name="fileId")String fileId,
                                            @WebParam(name="tokenId")String tokenId);

    @WebMethod(operationName = "createGingko", exclude = false)
    String saveGingko(@WebParam(name="tokenId")String tokenId,
                          @WebParam(name="companyId")String companyId,
                          @WebParam(name="fileId")String fileId,
                          @WebParam(name="publickKey")String cert,
                          @WebParam(name="privateKey")String prvCert,
                          @WebParam(name="fileName")String fileName,
                          @WebParam(name="description")String desc,
                          @WebParam(name="tags")String     tag,
                          @WebParam(name="version")String  version,
                          @WebParam(name="folderId")String   folderId,
                          @WebParam(name="length")Long     length,
                          @WebParam(name="contents")byte[] contents);

    @WebMethod(operationName = "createFolder", exclude = false)
    @WebResult(name="createResult")
    String createFolder(@WebParam(name="tokenId")String tokenId,
                          @WebParam(name="companyId")String companyId,
                          @WebParam(name="folderName")String folderName,
                          @WebParam(name="tag")String tag,
                          @WebParam(name="description")String desc,
                          @WebParam(name="folderParent")String folderParent );


    @WebMethod(operationName = "listFolder", exclude = false)
    @WebResult(partName="folderList")
    GingkoFolder[] listFolder(@WebParam(name="tokenId")String tokenId,
                                @WebParam(name="parentFolder")String parentFolder) throws ServiceException;
    
    @WebMethod(operationName = "listFiles", exclude = false)
    @WebResult(partName="fileList")
    GingkoFile[] listFiles(@WebParam(name="tokenId")String          tokenId,
                             @WebParam(name="parentFolder")String    parentFolder,
                             @WebParam(name="findPattern")String     findPattern,
                             @WebParam(name="includeFolder")boolean  includeFolder,
                             @WebParam(name="start")int start, 
                             @WebParam(name="count")int count) throws ServiceException;

    @WebMethod(operationName = "delete", exclude = false)
    String delete(@WebParam(name="tokenId")String tokenId,
                   @WebParam(name="contentId")String contentId);
    
}
