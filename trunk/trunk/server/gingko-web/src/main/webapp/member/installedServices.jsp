<%@ include file="/common/taglibs.jsp"%>
<%response.setContentType("application/x-json");%>
[{
    "id":"10",
    text:'Community',
    expanded: true,
    children:[{
    		"id":"101",
        text:'Blogs',
        url: '${ctx}/member/blogs.jsp',
        leaf:true
    },{
    	"id":"102",
        text:'Gallery',
        url: '${ctx}/member/gallery.jsp',
        leaf:true
    },{
    	"id":"friends-root",
        serviceName:'friends',
        text:'Friends',
        leaf:true
    }]
},{
    	"id":"group-root",
        text:'Group',
        serviceName:'groups',
        url: '${ctx}/member/groups.jsp',
        leaf:false
 },{
        "id":"messagebox-root",
        text:'Messages',
        serviceName:'messagebox',
        url: '${ctx}/member/messages.jsp',
        leaf:false
},{
    text:'File Share Services',
    id:'fileshare-root',
    serviceName:'fileShare',
    leaf:false
}]