<%@ include file="/common/taglibs.jsp"%>
<%response.setContentType("application/x-json");%>
{
	mainPanel:{
		id:'friends-s-p',
		title: 'My Friends',
		header: false,
		xtype: 'panel',
		layout: 'border',
		region: 'center',
		margins: '0 0 0 0',
		autoScroll:true,
		height: 300,
		items:[new JPoet.DataViewPanel({
		  xtype:'dataviewpanel',
		  id: 'friends-dvp',
		  region:'center',
		  layout:'fit',
		  dataView: {itemSelector: 'div.thumb-wrap',
		  					 style:'overflow:auto',
		  					 multiSelect: true,
		  					 id:'friends-dvp-dataview',
		  					 emptyText: 'No friends to display',
		  					 cls:'thumb-view'
		  },		  
		  store: new Ext.data.JsonStore({
				        url: '${ctx}/friends/jfindMember',
								root:'userDetails',
				        fields: [{name:'id'}, { name:'fullName', mapping:'user.fullName'}, 
				         {name:'screenName', mapping:'user.screenName'},
				         {name:'id', mapping:'user.id'}, 
				         {name:'lastUpdatedTime', mapping:'user.lastUpdatedTime'},{name:'address'}, 
				         {name:'loginId', mapping:'user.loginId'}, 
				         {name:'createDate', mapping:'user.createDate'}],
				        listeners:{ 
				        			load: function(s, r,o){
				        				for( var i = 0; i < r.length; i ++ ){
											    new Ext.ToolTip({
											        target: 'f-m-' + r[i].data.id,
											        width: 200,
											        html:r[i].data.screenName
											        });
											   }
				        			}
				        }
				        }),
		  tbar: [ 'Search', {text:'Search', xtype:'searchfield', id:'friend-searchfield', width: 200,listeners:{ render: function(v){
							  			var dvp = Ext.getCmp('friends-dvp');
								  		if( dvp ){
								  			v.setStore( dvp.getStore() ); 
								  		}
							  		}
		  							,scope:this}}, 
		  									{text:'Add Friend', width:100, handler: function(v){ 
		  							var data = {
		  							  text: 'Friend Detail',
		  							  id: 'blog-info',
		  							  modal: true,
		  							  url: '${ctx}/blogs/view?blogId=0',
		  								config: this.launchWindows['entryPanel'],
		  								mappings: []
		  							};
		  				 			(new JPoet.ServiceLaunch(data)).launchWin(Ext.getBody()); }, 
		  				 		scope: this}, 
		  				'-',
		  				{text:'Delete',width:100, handler:function(v){
		  					Ext.Ajax.request( {
		  						url: '${ctx}/friends/jfindMember?userSearch=long',
		  						success: function(rsp){
		  							alert( rsp.responseText );
		  							var ud = eval('(' + rsp.responseText + ')');
		  							alert( ud.userDetails[0] );
		  							alert( ud.userDetails[0]['user']['screenName'] );
		  							alert( ud.userDetails[0]['user.screenName'] );
		  							//['user.screenName'] );
		  						}
		  					} );
		  				}, scope:this}],
		 	tpl:new Ext.XTemplate(
            '<tpl for=".">',
            '<div class="thumb-wrap" id="f-m-{id}">',
            '<div class="thumb"><img src="${ctx}/member/viewAvator?avatorType=1&avatorId={id}" class="thumb-img"></div>',
            '<span>{fullName}</span></div>',
            '</tpl>')	
		})]
	},
	blogPanel: {
			id:'add-info', 
			xtype: 'panel',
			region: 'center',
			border: false,
			layout: 'border',
			buttons: [ {text:'Save', handler: function(){ Ext.getCmp('blog-w-form').getForm().submit(); }}],
			items:[{xtype: 'form',
						  layout:'border',
						  id:'blog-w-form',
							baseParams:{"blog.blogType":1},
							method: 'post',
							url:'${ctx}/blogs/saveBlog',
						  border: false,
						  frame:true,
						  region:'center',
						  items:[
						  {
								xtype: 'panel',
								region: 'north',
								border:false,
								layout:'form',
								height: 80,
								margins: '5 5 5 5',
								items:[{anchor: '98%',fieldLabel: 'Name', name: 'blog.name', id:'blog-w-name', xtype:'textfield'},
											 {anchor: '98%',fieldLabel: 'Category', name: 'blog.category', id:'blog-w-category', xtype:'combo'},
											 {anchor: '98%',fieldLabel: 'Tags', name: 'blog.tags', id:'blog-w-tags', xtype:'textfield'}											 
											]
						  }, 
						  { xtype: 'panel', region:'center', border: false, layout:'border', items:[
						  			{xtype:'label',border: false,  text:'Briefing(Please describes your blog)', region:'north'}, 
				 							{xtype:'panel', border: false, layout:'fit', region:'center', items:[
				 							{xtype:'htmleditor', name:'blog.description', id:'bio2', region:'center'}]
				 						}]
						  }]
						}]
		},
	entryPanel: {
			id:'jpoet-tree-entry', 
			xtype: 'panel',
			region: 'center',
			border: false,
			layout: 'border',
			items:[ {xtype: 'jpoettree',
						  id:'entry-w-form',
						  border: false,
						  frame:true,
						  region:'center'}
						 ]
		},

	stores:[ new Ext.data.JsonStore({  
	  url: '${ctx}/blogs/jlist',
    root: 'blogs',
    autoLoad: true,
    fields: ['id', 'status', 'createTime','subject', 'lastUpdatedTime','content', 'blogId', 'tags']})
     ]
}
