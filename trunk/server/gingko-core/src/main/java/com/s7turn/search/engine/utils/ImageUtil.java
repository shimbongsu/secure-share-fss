/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.search.engine.utils;

import java.awt.Color;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import javax.imageio.ImageIO;

/**
 *
 * @author Long
 */
public class ImageUtil {
    private ImageUtil(){}
    
    public static void convert(String file, float scale, int left, int top,  
            int viewWidth, int viewHeight, int cropWidth, int cropHeight,
            String extType, OutputStream output) throws IOException{
            convert(new File(file), scale, left, top, viewWidth, 
                    viewHeight, cropWidth, cropHeight, extType, output);
    }
    
    public static String convert(String file, float scale, int left, int top,  
            int viewWidth, int viewHeight, int cropWidth, int cropHeight) throws IOException{
        FileOutputStream fos = null;
        String fileXsizePath = file.substring(0, file.lastIndexOf("/"));
        String fileXsize = file.substring(file.lastIndexOf("/") + 1);
        String ext = fileXsize.substring(fileXsize.lastIndexOf(".") + 1 );
        String fileNamePart = fileXsize.substring(0, fileXsize.lastIndexOf("."));
        fileXsize = fileNamePart + "_" + cropWidth + "_" + cropHeight + "." + ext;
        File fileOutput = new File( fileXsizePath + "/" + fileXsize );
        fos = new FileOutputStream(fileOutput);
        try{
            convert(new File(file), scale, left, top, viewWidth,
                    viewHeight, cropWidth, cropHeight, ext.toLowerCase(), fos);
            return fileXsizePath + "/" + fileXsize;
        }finally{
            if( fos != null ){ try{ fos.close(); } catch(Exception e){}}
        }
    }
    
    
    public static void convert(File file, float scale, int left, int top,  
            int viewWidth, int viewHeight, int cropWidth, int cropHeight,
            String extType, OutputStream output) throws IOException{
        //String mimeType = getImageMimeType();
        Image image = ImageIO.read( file );
        convert(image, scale, left, top, viewWidth, viewHeight, 
                cropWidth, cropHeight, extType, output);
    }

    public static String convert( String file, int viewWidth, int viewHeight ) throws IOException{
        FileOutputStream fos = null;
        String fileXsizePath = file.substring(0, file.lastIndexOf("/"));
        String fileXsize = file.substring(file.lastIndexOf("/") + 1);
        String ext = fileXsize.substring(fileXsize.lastIndexOf(".") + 1 );
        String fileNamePart = fileXsize.substring(0, fileXsize.lastIndexOf("."));
        fileXsize = fileNamePart + "_" + viewWidth + "_" + viewHeight + "." + ext;
        File fileOutput = new File( fileXsizePath + "/" + fileXsize );
        fos = new FileOutputStream(fileOutput);
        try{
            convert(new File(file), viewWidth, viewHeight, ext.toLowerCase(), fos);
            return fileXsizePath + "/" + fileXsize;
        }finally{
            if( fos != null ){ try{ fos.close(); } catch(Exception e){}}
        }
    }
    
    public static void convert(File file, 
            int viewWidth, int viewHeight, 
            String extType, OutputStream output) throws IOException{
        //String mimeType = getImageMimeType();
        Image image = ImageIO.read( file );
        int width = image.getWidth( null );
        int height = image.getHeight( null );
        float wsc = 1.0f; 
        if( width > viewWidth ){
            wsc = ( (float) viewWidth ) /  ((float)width);
        }
        
        float hsc = 1.0f;
        if( height > viewHeight ){
            hsc = ( (float) viewHeight ) / ((float)height);
        }
        
        float scale = wsc < hsc ? wsc : hsc;
        int left = (int) ( width * scale );
        int top = -(int) ( height * scale );
        convert(image, scale, (viewWidth - left)/2, (top - viewHeight)/2, viewWidth, viewHeight, 
                viewWidth, viewHeight, extType, output);
    }
    
    public static void convert(Image img, float scale, int left, int top,  
            int viewWidth, int viewHeight, int cropWidth, int cropHeight,
            String extType, OutputStream output) throws IOException{
        //String mimeType = getImageMimeType();
        Image image = img;
        int realHeight = image.getHeight(null);
        int realWidth = image.getWidth(null);
        int iWidth = (int) (realWidth * scale);
        int iHeight = (int) (realHeight * scale);
        BufferedImage tag = new BufferedImage(iWidth, iHeight,BufferedImage.TYPE_INT_RGB);
        tag.getGraphics().drawImage(image, 0, 0, iWidth, iHeight, null);
        int h = -((viewHeight-cropHeight)/2 + cropHeight);
        int w = (viewWidth-cropWidth)/2;
        int iTop = h - top;
        int iLeft = w - left;
        BufferedImage leaveTag = new BufferedImage(cropWidth, cropHeight,BufferedImage.TYPE_INT_RGB);
        leaveTag.getGraphics().setColor(Color.white);
        leaveTag.getGraphics().fillRect(0, 0, cropWidth, cropHeight);
        //leaveTag.getGraphics().drawImage(tag, iLeft, iTop, this.getCropSize(), this.getCropSize(), null);
        leaveTag.getGraphics().drawImage(tag, 0, 0, cropWidth, cropHeight, 
                iLeft, iTop, iLeft + cropWidth, iTop + cropHeight, Color.red, null);
        
        ImageIO.write(leaveTag, extType, output);
    }    
    
    public static void  main(String[] args) throws Exception{
        File imgfile = new File("D:/results.jpg");
        File outfile = new File("D:/results_out.jpg");
        FileOutputStream fos = new FileOutputStream(outfile);
        convert(imgfile, 240, 240, "jpg", fos );
        fos.close();
    }
}
