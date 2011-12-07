/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.utils;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.security.MessageDigest;
import java.util.UUID;

/**
 *
 * @author Long
 */
public final class DigestUtil {
    public static char[] HEX_CHAR = {'0', '1', '2', '3',  
                                    '4', '5', '6', '7',  
                                    '8', '9', 'a', 'b',  
                                    'c', 'd', 'e', 'f'};

    public static String getFileMd5( String fileName ) throws Exception{
        return getFileHash(fileName, "md5");
    }

    public static String getFileMd5( File file ) throws Exception{
        return getFileHash(file, "md5");
    }
 
    public static String getFileHash(String fileName, String hashType) throws  Exception {  
        return getFileHash( new File( fileName ), hashType );
    }

    public static String getFileHash(File file, String hashType) throws Exception {  
        InputStream fis = null;  
        try{
            fis = new FileInputStream(file);
            byte[] buffer = new byte[10240];
            MessageDigest md5 = MessageDigest.getInstance(hashType);
            int numRead = 0;  
            while ((numRead = fis.read(buffer)) > 0) {  
                md5.update(buffer, 0, numRead);  
            }  
            return toHexString(md5.digest());  
        }finally{
            if( fis != null ){
                try{ fis.close(); }catch(Exception e){}
            }
        }
    }

    public static String toHexString(byte[] b) {  
        StringBuilder sb = new StringBuilder(b.length * 2);  
        for (int i = 0; i < b.length; i++) {  
            sb.append(HEX_CHAR[(b[i] & 0xf0) >>> 4]);  
            sb.append(HEX_CHAR[b[i] & 0x0f]);  
        }  
        return sb.toString();  
    }
    
    public static String escape( String esc ){
        return JavaScriptUnEscapse.escape(esc);
    }
    
    public static String unescape( String us ){
        return JavaScriptUnEscapse.unescape(us);
    }

    public static String generateToken()
    {
        try{
            MessageDigest md5 = MessageDigest.getInstance("md5");
            UUID uid = UUID.randomUUID();

            return uid.toString();
        }catch(Exception e){
            return "";
        }
    }
}
