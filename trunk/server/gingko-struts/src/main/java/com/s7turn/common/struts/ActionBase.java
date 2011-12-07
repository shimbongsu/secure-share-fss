/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.common.struts;

import com.opensymphony.xwork2.ActionSupport;
import com.opensymphony.xwork2.Preparable;
import com.s7turn.search.community.Comment;
import com.s7turn.search.community.services.CommentService;
import com.s7turn.search.engine.PhysicalFile;
import com.s7turn.search.engine.member.AccessHistory;
import com.s7turn.search.engine.member.User;
import com.s7turn.search.engine.member.UserWrapper;
import com.s7turn.search.engine.member.services.AccessHistoryService;
import com.s7turn.search.engine.member.services.UserService;
import com.s7turn.search.engine.services.PhysicalFileService;
import com.s7turn.search.engine.services.utils.Page;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import org.apache.commons.beanutils.PropertyUtils;
import org.apache.log4j.Logger;
import org.apache.struts2.interceptor.ServletRequestAware;
import org.apache.struts2.interceptor.ServletResponseAware;
import org.springframework.security.Authentication;
import org.springframework.security.context.SecurityContextHolder;
import org.springframework.security.userdetails.UserDetails;

/**
 *
 * @author Long
 */
public class ActionBase extends ActionSupport implements ServletRequestAware, ServletResponseAware, Preparable{
    protected static Logger logger = Logger.getLogger(ActionBase.class);
    private HttpServletRequest request;
    private HttpServletResponse response;
    private String firstUrl;
    private String lastUrl;
    private String previousUrl;
    private String nextUrl;
    private String currentUrl;
    private List<Page> pagings;
    private int totals;
    private int totalPages;
    private int max;
    private int startat;
    private int totalCount;
    private int limit;
    private String greybox;
    private String navigator;    
    private String returnUrl;
    private String actionStatus = "success";
    private String fileId;
    private PhysicalFile physicalFile; 
    private CommentService commentService;
    private AccessHistoryService accessHistoryService;
    private User user;
    private Long userId;
    private UserService userService;

    public String getActionStatus() {
        return actionStatus;
    }

    public void setActionStatus(String actionStatus) {
        this.actionStatus = actionStatus;
    }

    public String getGreybox() {
        return greybox;
    }

    public void setGreybox(String greybox) {
        this.greybox = greybox;
    }



    protected Map buildMap( List<?> org,String key ) throws Exception{
       Map map = new HashMap();
      for(Object obj : org){
          map.put( PropertyUtils.getSimpleProperty(obj,key),obj);
      }      
      return map;   
    }

     /**
     * Save the message in the session, appending if messages already exist
     * @param msg the message to put in the session
     */
    @SuppressWarnings("unchecked")
    protected void saveMessage(String msg) {
        List messages = (List) getServletRequest().getSession().getAttribute("messages");
        if (messages == null) {
            messages = new ArrayList();
        }
        messages.add(msg);
        getServletRequest().getSession().setAttribute("messages", messages);
    }

     /**
     * Save the message in the session, appending if messages already exist
     * @param msg the message to put in the session
     */
    @SuppressWarnings("unchecked")
    protected void saveError(String error) {
        List errors = (List) getServletRequest().getSession().getAttribute("errors");
        if (errors == null) {
            errors = new ArrayList();
        }
        errors.add(error);
        getServletRequest().getSession().setAttribute("errors", errors);
    }


    protected List computer(List org){
        ///at least ten page.
        if( org == null ){
            setTotalCount(0);
            return new ArrayList();
        }
        
        int sum = org.size();
        if( sum < this.getLimit() * 10 ){
            setTotalCount( this.startat + sum );
        }else{
            setTotalCount( this.startat + sum + getLimit() );
        }
        if( this.getLimit() > 0 ){
            return org.subList(0, org.size() > this.getLimit() ? this.getLimit() : org.size() );
        }else{
            return org;
        }
        
    }

    public int getLimit() {
        return limit;
    }

    public void setLimit(int limit) {
        this.limit = limit;
    }

    public int getTotalCount() {
        return totalCount;
    }

    public void setTotalCount(int totalCount) {
        this.totalCount = totalCount;
    }
    
    
    public User getUser() {
        return user;
    }

    public void setUser(User user) {
        this.user = user;
    }

    
    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }

    public UserService getUserService() {
        return userService;
    }

    public void setUserService(UserService userService) {
        this.userService = userService;
    }

    private PhysicalFileService physicalFileService;
    
    public PhysicalFileService getPhysicalFileService() {
        return physicalFileService;
    }

    public void setPhysicalFileService(PhysicalFileService physicalFileService) {
        this.physicalFileService = physicalFileService;
    }

    public AccessHistoryService getAccessHistoryService() {
        return accessHistoryService;
    }

    public void setAccessHistoryService(AccessHistoryService accessHistoryService) {
        this.accessHistoryService = accessHistoryService;
    }
    
    @Override
    public void setServletRequest(HttpServletRequest reqs) {
        request = reqs;
    }

    @Override
    public void setServletResponse(HttpServletResponse resp) {
        response = resp;
        //response.setCharacterEncoding("utf-8");
        //response.setContentType("text/html; charset=utf-8");
    }

    public HttpServletRequest getServletRequest() {
        return request;
    }

    public HttpServletResponse getServletResponse() {
        return response;
    }

    public CommentService getCommentService() {
        return commentService;
    }

    public void setCommentService(CommentService commentService) {
        this.commentService = commentService;
    }
    
    public String getNavigator() {
        return navigator;
    }

    public void setNavigator(String navigator) {
        this.navigator = navigator;
    }
    
    @Override
    public void prepare() throws Exception {
        
    }

    public String getReturnUrl() {
        if( returnUrl == null ){
            returnUrl = this.getServletRequest().getRequestURI() + "?" + getServletRequest().getQueryString();
            if( getServletRequest().getContextPath().length() > 1){
                returnUrl = returnUrl.substring(getServletRequest().getContextPath().length());
            }
        }
        return returnUrl;
    }

    public void setReturnUrl(String returnUrl) {
        this.returnUrl = returnUrl;
    }
    
    
    
    public String getFirstUrl() {
        return firstUrl;
    }

    public void setFirstUrl(String firstUrl) {
        this.firstUrl = firstUrl;
    }

    public String getLastUrl() {
        return lastUrl;
    }

    public void setLastUrl(String lastUrl) {
        this.lastUrl = lastUrl;
    }

    public String getNextUrl() {
        return nextUrl;
    }

    public void setNextUrl(String nextUrl) {
        this.nextUrl = nextUrl;
    }

    public String getPreviousUrl() {
        return previousUrl;
    }

    public void setPreviousUrl(String previousUrl) {
        this.previousUrl = previousUrl;
    }

    public String getCurrentUrl() {
        return currentUrl;
    }

    public void setCurrentUrl(String currentUrl) {
        this.currentUrl = currentUrl;
    }
    
    public int getListPaging(){
        return 15;
    }
    
    public List<Page> getPagings(){
        return pagings;
    }
    
    public int getTotals(){
        return totals;
    }

    public void setTotals(int totals) {
        this.totals = totals;
    }

    public int getTotalPages() {
        return totalPages;
    }

    public void setTotalPages(int totalPages) {
        this.totalPages = totalPages;
    }
    
    public int getMax() {
        return max;
    }

    public void setMax(int max) {
        this.max = max;
    }

    public int getStartat() {
        return startat;
    }

    public void setStartat(int startat) {
        this.startat = startat;
    }
    
    
    public int getMaxResults(){
        return this.getMax() == 0 ? 15 : getMax();
    }
    
    public int getStartAt(){
        return this.getStartat();
    }
    
    public int getNextStartAt(){
        return getStartAt() * getMaxResults() + getMaxResults();
    }
    
    public int getPrevStartAt(){
        int startAt = getStartAt() * getMaxResults() - getMaxResults();
        if( startAt < 0 )
            return 0;
        else
            return startAt;
    }
               
    protected void calcPagings(){ 
        int tt = this.getTotals();
        int maxPages = this.getMaxResults();
        int pages = (int) Math.round(((float)tt / (float)maxPages) + 0.49999 ); // + 0.5);
        int curPage = this.getStartAt(); // / max;
        firstUrl = getPageUrl(0, maxPages );
        lastUrl = getPageUrl( pages - 1 < 0 ? 0 : pages - 1, maxPages );
        totalPages = pages;
        if( curPage - 1 > 0 ){
            previousUrl = getPageUrl( curPage - 1, maxPages );
        }
        
        currentUrl = getPageUrl( curPage, maxPages );
        
        if( curPage + 1 < pages ){
            nextUrl = getPageUrl( curPage + 1, maxPages );
        }
        
        int startPage = 0;
        
        startPage = curPage - (this.getListPaging()/ 2);
        
        if( startPage > 0 && (startPage + getListPaging()) > pages ){
            startPage = pages - getListPaging();
        }

        if( startPage < 0 ){
            startPage = 0;
        }
        
        List<Page> listPages = new ArrayList<Page>();
        if( startPage > 0 ){
            Page f1 = new Page();
            f1.setName("...");
            f1.setUrl(null);
            f1.setActual(false);
            f1.setCurrent(false);
            listPages.add(f1);
        }
        
        for( int i = startPage; i < pages; i ++ ){
            if( i > startPage + getListPaging() ){
                break;
            }
            
            Page pi = new Page();
            pi.setActual(true);
            pi.setCurrent(i == curPage);
            pi.setUrl(getPageUrl(i, maxPages));
            pi.setName("" + (i + 1));
            listPages.add(pi);
        }

        if( startPage + getListPaging() < pages ){
            Page f1 = new Page();
            f1.setName("...");
            f1.setUrl(null);
            f1.setActual(false);
            f1.setCurrent(false);
            listPages.add(f1);
        }
        
        pagings = listPages;
    }
    
    protected String getPageUrl( int startAt, int max ){
        return "";
    }

    public String getFileId() {
        return fileId;
    }

    public void setFileId(String fileId) {
        this.fileId = fileId;
    }
    

    public PhysicalFile getPhysicalFile() {
        return physicalFile;
    }

    protected void setPhysicalFile(PhysicalFile physicalFile) {
        this.physicalFile = physicalFile;
    }
    
    
    
    public InputStream getInputStream() throws IOException{
        return new FileInputStream( this.getPhysicalFile().getOriginalFileName() );
    }
    
    public String viewPhysicalFile() throws Exception{
        /***
        String uri = this.getServletRequest().getRequestURI();
        String[] uris = uri.split("/");
        
        if( uris != null && uris.length > 0 ){
            String fId = uris[ uris.length - 1 ];
            Long phyId = Long.parseLong(fId);
            this.setPhysicalFile( this.getPhysicalFileService().findBy(phyId) );
            return "success";
        }
         **/ 
        Long phyId = Long.parseLong(fileId);
        this.setPhysicalFile( this.getPhysicalFileService().findBy(phyId) );
        return "success";
    }
    
    public User getCurrentUser(){
        Authentication auth = SecurityContextHolder.getContext().getAuthentication();
        if( auth != null ){
            Object principal = auth.getPrincipal();
            if( principal instanceof UserWrapper ){
                UserWrapper uw = (UserWrapper)principal;
                return uw.getUser();
            } else if( principal instanceof UserDetails ){
                User u = new User();
                u.setLoginId(((UserDetails)principal).getUsername());
                u.setExpired(false);
                u.setDisabled(false);
                u.setLocked(false);
                return u;
            }
        }
        return null;
    }
    
    protected void accessLog(Long id, int category, String description) throws Exception{
        if( getAccessHistoryService() != null ){
            AccessHistory ah = new AccessHistory();
            ah.setAccessCategory(category);
            ah.setAccessToId(id);
            ah.setDescription( description );
            User curUser = this.getCurrentUser();
            if( curUser != null ){
                ah.setUserId(curUser.getId());
            }else{
                ah.setUserId(0L); //anomouse
            }
            ///remote address
            if( getServletRequest() != null ){
                ah.setRemoteAddress(  getServletRequest().getRemoteAddr() + "/" + getServletRequest().getRemoteHost());
            }
            this.getAccessHistoryService().smartSave(ah);
        }
    }
    
    public List<Comment> findComments( Long forId, String type ) throws Exception{
        if( getCommentService() != null ){
            return getCommentService().findByForItem(forId, type);
        }
        return new ArrayList<Comment>();
    }
    
    public String getCookieName(){
        return "jsessionid";
    }
    
    public String getSessionId(){
        
        Cookie[] cookies = getServletRequest().getCookies();
        String cn = getCookieName();
        for( Cookie cok : cookies ){
            if( cn.equalsIgnoreCase(cok.getName()) ){
                return cok.getValue();
            }
        }
        return "";
    }
    
    public boolean isPermit(Long resourceId, Long owner, int permitType) throws Exception{
        
        if( permitType == 0 ){
            return true;
        }
        
        if( this.getCurrentUser() == null ){
            return false;
        }
        
        if( this.getCurrentUser().getId() == owner ){
            return true;
        }
        ////below are the special logic for member or others
        return false;
    }

    protected boolean isEmpty( String s ){
        return s == null || s.trim().length() == 0;
    }
}
