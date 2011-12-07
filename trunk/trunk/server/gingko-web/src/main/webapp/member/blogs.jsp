<%@ include file="/common/taglibs.jsp"%>
<%response.setContentType("application/x-json");%>
{
	mainPanel:{
		id:'blog-s-p',
		title: 'Blogs',
		header: false,
		xtype: 'panel',
		layout: 'border',
		region: 'center',
		margins: '0 0 0 0',
		autoScroll:true,
		height: 300,
		items:[  new JPoet.DataViewPanel({
		  xtype:'dataviewpanel',
		  id: 'blog-dvp',
		  region:'center',
		  layout:'fit',
		  store: new Ext.data.JsonStore({  
	  						url: '${ctx}/blogs/jlist',
    						root: 'blogEntryList',
    						autoLoad: true,
    						fields: ['id', 'status', 'createTime','subject', 'lastUpdatedTime','shortDescription', 'blogId', 'tags']}),
		  tbar: [ {text:'Create', width:100, handler: function(v){ 
		  							var data = {
		  							  text: 'Blog Detail',
		  							  id: 'blog-info',
		  							  modal: true,
		  							  url: '${ctx}/blogs/view?blogId=0',
		  								config: this.launchWindows['blogPanel'],
		  								mappings: []
		  							};
		  				 			(new JPoet.ServiceLaunch(data)).launchWin(Ext.getBody()); }, 
		  				 		scope: this}, 
		  				'-', 'My Blog', {
		  					id:'combo-myblogs',
		  					xtype:'combo',
		  					width:280,
			    			displayField: 'name', 
			    			valueField: 'id', 
			    			mode: 'remote',
						    typeAhead: true,
					      forceSelection: true,
						    triggerAction: 'all',
						    emptyText:'My blogs...',
						    selectOnFocus:true,
						    listeners: { render:function(v){ 
						    								v.store= new Ext.data.JsonStore({  
																  url: '${ctx}/blogs/jmyblogs',
															    root: 'blogs',
															    autoLoad: true,
															    fields: ['id', 'blogType', 'createTime','description', 'lastUpdatedTime','name', 'owner', 'tags']});
															  v.on('change', function(cmp, nv, ov){
															  		 var blogDvp = this.findById('blog-dvp');
															  		 if( blogDvp ){
															  			 if( blogDvp.store ){
															  			  if( blogDvp.store.baseParams ){
															  			  	blogDvp.store.baseParams['blogId'] = nv;
															  			  }else{
															  			  	blogDvp.store.baseParams = { "blogId":nv };
															  			  }
															  			 	blogDvp.store.load();
															  			 }
															  		 }
															  	}, 
															  	this );
    						 							}, scope:this }
						   },
						  {text:'Detail', width:100, handler: function(v){ 
		  							var data = {
		  							  text: 'Blog Detail',
		  							  id: 'blog-info',
		  							  modal: true,
		  							  url: '${ctx}/blogs/view?blogId=' + Ext.getCmp('combo-myblogs').getValue(),
		  								config: this.launchWindows['blogPanel'],
		  								mappings: []
		  							};
		  				 			(new JPoet.ServiceLaunch(data)).launchWin(Ext.getBody()); }, 
		  				 		scope: this}, 
		  				'-',{text:'Write',width: 100, handler:function(v){
		  						var blog = Ext.getCmp('combo-myblogs');
		  						var blogId = blog.getValue();
		  						var blogName = blog.lastSelectionText;
		  							if( blogId == ''){
		  								blogId = 0;
		  							}
		  							
		  							var data = {
		  							  text: 'Write Blog--' + blogName,
		  							  id: 'blog-info-' + blogId,
		  							  modal: true,
		  							  formId: 'entry-w-form',
		  								config: this.launchWindows['entryPanel'],
		  								initParams: {'blogEntry.blogId':blogId},
		  								mappings: []
		  							};
		  							
		  				 			(new JPoet.ServiceLaunch(data)).launch(Ext.getCmp('main-tabs'));
		  				}, scope:this},
		  				{text:'Edit Log',width:100, handler:function(v){}, scope:this}, 
		  				{text:'Delete',width:100, handler:function(v){}, scope:this}],
		 	tpl:new Ext.XTemplate(
        '<tpl for=".">',
        '<div class="detail-item" id="{id}">',
            '<h4><span>{lastUpdatedTime}<br/>by {author}</span>',
            '<a href="${ctx}/blogs/view?id={id}">{subject}</a></h4>',
            '<p>{shortDescription}</p>',
        '</div></tpl>'
    	)	
		})]
	},
	blogPanel: {
			id:'edit-blog-info', 
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
			id:'edit-blog-entry', 
			xtype: 'panel',
			region: 'center',
			border: false,
			layout: 'border',
			tbar: [ {text:'Publish', width: 200, handler: function(){ 
									var entryForm = Ext.getCmp('entry-w-form').getForm();
									if( typeof entryForm.baseParams != 'undefined' ){
										entryForm.baseParams['blogEntry.status']=1;
									}
									entryForm.submit(); } },
							{text:'Save As Draft',width: 200,handler: function(){ 
									var entryForm = Ext.getCmp('entry-w-form').getForm();
									if( typeof entryForm.baseParams != 'undefined' ){
										entryForm.baseParams['blogEntry.status']=0;
									}
									entryForm.submit(); } },
							{text:'Discard',width: 200}],
			items:[{xtype: 'form',
						  layout:'border',
						  id:'entry-w-form',
							baseParams:{"blogEntry.status":0, "blogEntry.blogId": 0, "blogEntry.userId":0},
							method: 'post',
							url:'${ctx}/blogs/saveEntry',
						  border: false,
						  frame:true,
						  region:'center',
						  items:[
						  {
								xtype: 'panel',
								region: 'north',
								border:false,
								layout:'form',
								height: 50,
								margins: '5 5 5 5',
								items:[{anchor: '99%',fieldLabel: 'Subject', name: 'blogEntry.subject', id:'entry-w-name', xtype:'textfield'},
											 {anchor: '99%',fieldLabel: 'Tags', name: 'blogEntry.tags', id:'entry-w-tags', xtype:'textfield'}											 
											]
						  }, 
						  { xtype: 'panel', region:'center', border: false, layout:'border', items:[
						  			{xtype:'label',border: false,  text:'Content', region:'north'}, 
				 							{xtype:'panel', border: false, layout:'fit', region:'center', items:[
				 							{xtype:'htmleditor', name:'blogEntry.content', id:'content', region:'center'}]
				 						}]
						  }]
						}]
		},

	stores:[ new Ext.data.JsonStore({  
	  url: '${ctx}/blogs/jlist',
    root: 'blogs',
    autoLoad: true,
    fields: ['id', 'status', 'createTime','subject', 'lastUpdatedTime','content', 'blogId', 'tags']})
     ]
}
