<%@ include file="/common/taglibs.jsp"%>
<%response.setContentType("application/x-json");%>
{
	mainPanel:{
		id:'gallery-s-p',
		title: 'Gallery',
		header: false,
		xtype: 'panel',
		layout: 'border',
		region: 'center',
		margins: '0 0 0 0',
		autoScroll:true,
		height: 300,
		items:[  new JPoet.DataViewPanel({
		  xtype:'dataviewpanel',
		  id: 'gallery-dvp',
		  region:'center',
		  dataView: {itemSelector: 'div.thumb-wrap',
		  					 style:'overflow:auto',
		  					 multiSelect: true,
		  					 id:'gallery-dvp-dataview',
		  					 emptyText: 'No images to display',
		  					 cls:'thumb-view'
		  },
		  layout:'fit',
		  store: new Ext.data.JsonStore({  
	  						url: '${ctx}/gallery/jview',
    						root: 'photos',
    						autoLoad: false,
    						fields: [ 'createDate', 'description', 'fileId', 'fileSize', 'fileType',
													'fileUrl', 'galleryId', 'id', 'name', 'smallFileId', 'smallFileUrl',
													'tags', 'thumbFileId', 'thumbFileUrl', 'userId']}),
		  tbar: [ {text:'Create', width:100, handler: function(v){ 
		  							var data = {
		  							  text: 'Create Gallery',
		  							  id: 'gallery-info',
		  							  modal: true,
		  								config: this.launchWindows['galleryPanel'],
		  								mappings: []
		  							};
		  				 			(new JPoet.ServiceLaunch(data)).launchWin(Ext.getBody()); }, 
		  				 		scope: this}, 
		  				'-', 'My Gallery', {
		  					id:'combo-mygalleries',
		  					xtype:'combo',
		  					width:280,
			    			displayField: 'name', 
			    			valueField: 'id', 
			    			mode: 'remote',
						    typeAhead: true,
					      forceSelection: true,
						    triggerAction: 'all',
						    emptyText:'My Galleries...',
						    selectOnFocus:true,
						    listeners: { render:function(v){ 
						    								v.store= new Ext.data.JsonStore({  
																  url: '${ctx}/gallery/jmygalleries',
															    root: 'galleryList',
															    autoLoad: true,
															    fields: ['id', 'description', 'createBy','galleryType', 'lastUpdatedTime','name', 'security', 'tags']}),
															    v.on('change', function(cmp, nv, ov){
															  		 var galleryDvp = this.findById('gallery-dvp');
															  		 if( galleryDvp ){
															  			 if( galleryDvp.store ){
															  			  if( galleryDvp.store.baseParams ){
															  			  	galleryDvp.store.baseParams['galleryId'] = nv;
															  			  }else{
															  			  	galleryDvp.store.baseParams = { "galleryId":nv };
															  			  }
															  			 	galleryDvp.store.load();
															  			 }
															  		 }
															  	}, 
															  	this );
    						 							}, scope:this }
						   },
						  {text:'Detail', width:100, handler: function(v){ 
		  							var data = {
		  							  text: 'Gallery Detail',
		  							  id: 'gallery-info',
		  							  modal: true,
		  							  url: '${ctx}/gallery/jview?id=' + Ext.getCmp('combo-mygalleries').getValue(),
		  								config: this.launchWindows['galleryPanel'],
		  								mappings: []
		  							};
		  				 			(new JPoet.ServiceLaunch(data)).launchWin(Ext.getBody()); }, 
		  				 		scope: this}, 
		  				'-',{text:'Upload',width: 100, handler:function(v){
		  						var gallery = Ext.getCmp('combo-mygalleries');
		  						var galleryId = gallery.getValue();
		  						var galleryName = gallery.lastSelectionText;
		  							if( galleryId == ''){
		  								galleryId = 0;
		  							}
		  							var dataConfig = {
		  							  text: 'Upload Photos for ' + galleryName,
		  							  id: 'g-info-' + galleryId,
		  							  modal: true,
		  							  formId: 'photo-w-form',
		  								config: this.launchWindows['photoPanel'],
		  								initParams: {'galleryId':galleryId},
		  								mappings: [{cid:'entry-w-galleryId', field:'galleryId'}]
		  							};
		  							
		  							var lap = new JPoet.ServiceLaunch(dataConfig);
		  				 			lap.launchWin(Ext.getCmp('main-tabs'));
		  				 			var swfp = lap.findById('g-swf-uplader');
		  				 			if( swfp ){
		  				 				swfp.on('uploadQueueCompleted', function(){
		  				 						var p = Ext.getCmp('gallery-dvp');
		  				 						if( p ){
		  				 						  var pds = p.getStore();
		  				 						  if( pds ){
		  				 						  	pds.load();
		  				 						  }
		  				 						}
		  				 				});
		  				 			}
		  				 			delete data;
		  				 			
		  				}, scope:this},
		  				{text:'Edit Photo',width:100, handler:function(v){}, scope:this}, 
		  				{text:'Delete',width:100, handler:function(v){
		  				  
		  					var dvp = Ext.getCmp('gallery-dvp-dataview');
		  					var ids = '0';
		  					if( dvp ){
		  						var selectedRecs = dvp.getSelectedRecords();
		  						for( var i = 0; i < selectedRecs.length; i ++ ){
		  							var rec = selectedRecs[i];
		  							ids += ',' + rec.data.id;
		  							//ids = rec.data.id;
		  						}
		  						
		  					}
								Ext.Ajax.request({
								   url: '${ctx}/gallery/jdelete?arrayId=' + ids,
								   success: function(resp){
								      var ret;
								      try{
								        ret = eval( '(' + resp.responseText + ')' );
								      }catch(ex){}
								      if( typeof ret != 'undefined' ){
								        if( typeof ret.actionStatus != 'undefined' ){
								          if( ret.actionStatus == 'success' ){
														var p = Ext.getCmp('gallery-dvp');
								 						if( p ){
								 						  var pds = p.getStore();
								 						  if( pds ){
								 						  	pds.load();
								 						  }
								 						}
							 						}
							 					}
						 					}
								   },
								   failure: function(resp){ alert('Delete photo failure!');}
								});
		  				}, scope:this}],
		 	tpl:new Ext.XTemplate(
            '<tpl for=".">',
            '<div class="thumb-wrap" id="{id}">',
            '<div class="thumb"><img src="${ctx}/{thumbFileUrl}" class="thumb-img"></div>',
            '<span>{name}</span></div>',
            '</tpl>'
    	)	
		})]
	},
	galleryPanel: {
			id:'edit-blog-info', 
			xtype: 'panel',
			region: 'center',
			border: false,
			layout: 'border',
			buttons: [ {text:'Save', handler: function(){ Ext.getCmp('gallery-w-form').getForm().submit(); }}],
			items:[{xtype: 'form',
						  layout:'border',
						  id:'gallery-w-form',
							baseParams:{"gallery.galleryType":1},
							method: 'post',
							url:'${ctx}/gallery/createGallery',
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
								items:[{anchor: '98%',fieldLabel: 'Name', name: 'gallery.name', id:'gallery-w-name', xtype:'textfield'},
											 {anchor: '98%',fieldLabel: 'Category', name: 'gallery.category', id:'gallery-w-category', xtype:'combo'},
											 {anchor: '98%',fieldLabel: 'Tags', name: 'gallery.tags', id:'gallery-w-tags', xtype:'textfield'}											 
											]
						  }, 
						  { xtype: 'panel', region:'center', border: false, layout:'border', items:[
						  			{xtype:'label',border: false,  text:'Briefing(Please describes your gallery)', region:'north'}, 
				 							{xtype:'panel', border: false, layout:'fit', region:'center', items:[
				 							{xtype:'htmleditor', name:'gallery.description', id:'g-desc', region:'center'}]
				 						}]
						  }]
						}]
		},
	photoPanel: {
			xtype: 'panel',
			region: 'center',
			border: false,
			layout: 'border',
			bbar: [{
						xtype: 'statusbar',
            id: 'form-statusbar',
            defaultText: 'Ready',
            plugins: new JPoet.ValidationStatus({formId:'photo-w-form'})
        }],
			items:[ {xtype:'form', layout:'border', id:'photo-w-form', region:'center', items:[
													{
															xtype: 'panel',
															region: 'north',
															border:false,
															layout:'form',
															height: 50,
															margins: '5 5 5 5',
															items:[{anchor: '99%',fieldLabel: 'Name', name: 'photo.name', id:'entry-w-name', xtype:'textfield', allowBlank:false},
																		 {anchor: '99%',fieldLabel: 'Tags', name: 'photo.tags', id:'entry-w-tags', xtype:'textfield', allowBlank:false},
																		 {anchor: '99%',fieldLabel: 'galleryId', name: 'galleryId', id:'entry-w-galleryId', xtype:'hidden'}
																		]
													},			
							{
							xtype: 'tabpanel',
							region: 'center',
              plain:false,
              activeTab:0,
              margins:'0 5 5 0',
              resizeTabs:true,
            	tabWidth:150,							
							items:[{
				 								id:'g-swf-uplader',
				 								title: 'Upload Photos',
				 								xtype:'swfuploadpanel', frame:true, region:'south', height: 300, splite:true,
				 								border: false, layout:'border',	
				 								upload_url:'${ctx}/gallery/uploadPhoto;jsessionid=${sessionId}',
				 								post_params:{},
				 								file_post_name:'uploadFile',
				 								attach_form:'photo-w-form'
			        		},{ xtype:'panel',
			        		    title: 'Describe photos',
			        		    layout:'border',
			        				items:[
														{xtype: 'panel',
												  	layout:'border',
														baseParams:{"blogEntry.status":0, "blogEntry.blogId": 0, "blogEntry.userId":0},
														method: 'post',
														url:'${ctx}/blogs/saveEntry',
												  	border: false,
												  	frame:true,
												  	region:'center',
												  	items:[
													  {xtype: 'panel', region:'center', border: false, layout:'border', items:[
													  			{xtype:'label',border: false,  text:'Photos description', region:'north'}, 
											 							{xtype:'panel', border: false, layout:'fit', region:'center', items:[
											 							{xtype:'htmleditor', name:'photo.description', id:'content', region:'center'}]
											 						}]
													  }
													]}			        				
			        				]			        		
			        		}]
			  }]}]
		},

	stores:[ new Ext.data.JsonStore({  
	  url: '${ctx}/blogs/jlist',
    root: 'blogs',
    autoLoad: true,
    fields: ['id', 'status', 'createTime','subject', 'lastUpdatedTime','content', 'blogId', 'tags']})
     ]
}
