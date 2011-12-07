/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.s7turn.community.struts;

import com.s7turn.common.struts.ActionBase;
import com.s7turn.search.community.Blog;
import com.s7turn.search.community.BlogEntry;
import com.s7turn.search.community.services.BlogEntryService;
import com.s7turn.search.community.services.BlogService;
import com.s7turn.search.engine.ServiceException;
import com.s7turn.search.engine.WebappException;
import java.util.List;
import org.htmlparser.Parser;
import org.htmlparser.Tag;
import org.htmlparser.Text;
import org.htmlparser.visitors.NodeVisitor;

/**
 *
 * @author Long
 */
public class BlogAction extends ActionBase {    
    private Long userId;
    private Long blogId;
    private Long entryId;
    private Blog blog;
    private BlogEntry blogEntry;
    private BlogService blogService;
    private BlogEntryService blogEntryService;
    private List<Blog> blogs;
    private List<BlogEntry> blogEntryList;
    private boolean hasBlogs;
    private Long groupId;
    
    public BlogAction(){
        this.setNavigator("account");
    }
    
    public Blog getBlog() {
        return blog;
    }

    public void setBlog(Blog blog) {
        this.blog = blog;
    }

    public Long getBlogId() {
        return blogId;
    }

    public void setBlogId(Long blogId) {
        this.blogId = blogId;
    }

    public Long getUserId() {
        return userId;
    }

    public void setUserId(Long userId) {
        this.userId = userId;
    }

    public Long getGroupId() {
        return groupId;
    }

    public void setGroupId(Long groupId) {
        this.groupId = groupId;
    }

    public Long getEntryId() {
        return entryId;
    }

    public void setEntryId(Long entryId) {
        this.entryId = entryId;
    }

    
    public BlogEntry getBlogEntry() {
        return blogEntry;
    }

    public void setBlogEntry(BlogEntry blogEntry) {
        this.blogEntry = blogEntry;
    }

    public BlogEntryService getBlogEntryService() {
        return blogEntryService;
    }

    public void setBlogEntryService(BlogEntryService blogEntryService) {
        this.blogEntryService = blogEntryService;
    }

    public BlogService getBlogService() {
        return blogService;
    }

    public void setBlogService(BlogService blogService) {
        this.blogService = blogService;
    }

    public List<BlogEntry> getBlogEntryList() {
        return blogEntryList;
    }

    public void setBlogEntryList(List<BlogEntry> blogEntryList) {
        this.blogEntryList = blogEntryList;
    }

    public List<Blog> getBlogs() {
        return blogs;
    }

    public void setBlogs(List<Blog> blogs) {
        this.blogs = blogs;
    }

    public boolean isHasBlogs() {
        return hasBlogs;
    }

    public void setHasBlogs(boolean hasBlogs) {
        this.hasBlogs = hasBlogs;
    }
    
    public List<BlogEntry> findEntryList( Long bId ) throws Exception{
        return this.getBlogEntryService().findByBlog(bId);
    }
    
    public String list() throws Exception{
        try{
            Blog blg = getBlogService().findBy(getBlogId());
            List<BlogEntry> entries = getBlogEntryService().findByBlog( getBlogId() );
            setBlog( blg );
            setBlogEntryList(computer(entries));
        }catch(ServiceException se){
            throw new Exception( se.getMessage(), se );
        }
        return "success";
    }
    
    public String viewBlog() throws Exception{
        Long bid = this.getBlogId();
        Blog bl = this.getBlogService().findBy(bid);
        this.setBlog(bl);
        return "success";
    }
    
    public String myblog() throws Exception{
        Long gid = getGroupId();
        
        Long uid = this.getCurrentUser().getId();
        try{
            
            List<Blog> bs = null;
            if( gid == null || gid == 0 ){
                bs = this.getBlogService().findByUser(uid);
            }else{
                bs =  this.getBlogService().findByGroup(gid);
            }
            
            if( bs != null && bs.size() > 0 )
            {
                this.setHasBlogs(true);
                this.setBlogs(computer(bs));
            }
        }catch(Exception e){
        }
        return "success";
    }
    
    public String saveBlog() throws Exception{
        Blog b = getBlog();
        if( b.getBlogType() == Blog.USER_BLOG ){
            b.setOwner(this.getCurrentUser().getId());
        }else if( b.getBlogType() == Blog.GROUP_BLOG ){
            b.setOwner(this.getGroupId());
        }
        
        this.getBlogService().smartSave(getBlog());
        return "success";
    }
    
    private String toShortDesc( String longText ){
        Parser p = Parser.createParser(longText, "UTF-8");
        ShortBriefingVisitor sbv = new ShortBriefingVisitor();
        try{
            p.visitAllNodesWith(sbv);
        }catch(Exception e){}
        return sbv.getStringBuffer().toString();
    }
    
    public String saveEntry() throws Exception{
        BlogEntry be = this.getBlogEntry();
        if( be != null ){
            if( be.getId() == null || be.getId() == 0 ){
                be.setUserId(this.getCurrentUser().getId());
                be.setShortDescription(toShortDesc( be.getContent() ) );
                getBlogEntryService().insert(be);
            }else{
                BlogEntry mbe = getBlogEntryService().findBy(be.getId());
                if( mbe.getUserId() == this.getCurrentUser().getId() ){
                    be.setUserId( mbe.getUserId() );
                    be.setShortDescription(toShortDesc( be.getContent() ));
                    getBlogEntryService().update(be);
                }else{
                    throw new WebappException(500, "No rights");
                }
            }
        }
        return "success";
    }
    
    @Override
    public String execute() throws Exception{
        Long beid = this.getEntryId();
        if( beid == null || beid == 0){
            Long bid = this.getBlogId();
            if( bid != null && bid != 0){
                setBlog(this.getBlogService().findBy(bid));
                BlogEntry be = new BlogEntry();
                be.setBlogId( bid );
                if( getCurrentUser() != null ){
                    be.setUserId( this.getCurrentUser().getId() );
                }
                setBlogEntry(be);
            }else{
                Blog blg = new Blog();
                setBlog(blg);
            }
        }else{
            setBlogEntry( getBlogEntryService().findBy(beid) );
            setBlog( getBlogService().findBy(getBlogEntry().getBlogId()) );
        }
        return "success";
    }
    
    public String mock() throws Exception {
        return "success";
    }
    
    static class ShortBriefingVisitor extends NodeVisitor{
        StringBuffer buffer = new StringBuffer();
        boolean shouldChildren = true;
        public ShortBriefingVisitor(){}
        public void visitTag( Tag tag ){
            return;
        }
        
        public boolean shouldRecurseChildren(){
            return shouldChildren;
        }
        
        public boolean shouldRecurseSelf(){
            return shouldChildren;
        }
        
        public void visitStringNode(Text text){
            if( buffer.length() < 400 ){
                if( text.getText() != null ){
                    buffer.append(text.getText().trim());
                }
            }else{
                shouldChildren = false;
            }
        }
        
        public StringBuffer getStringBuffer(){
            if( buffer.length() > 400 ){
                return buffer.delete(400, buffer.length());
            }
            return buffer;
        }
        
    }
    
}
