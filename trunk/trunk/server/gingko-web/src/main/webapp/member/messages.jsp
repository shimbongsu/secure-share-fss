<%@ include file="/common/taglibs.jsp"%>
<%response.setContentType("application/x-json");%>
{
	mainPanel:{
		id:'window-01',
		layout: 'border',
		region: 'center',
		margins: '0 0 0 0',
		items:[new JPoet.ServiceGrid({
			id:'service-grid-message',
			region: 'center',
			margins: '0 0 0 0',
			split:true,
			store: new Ext.data.JsonStore({
					root: 'messages',
        	idProperty: 'id',
        	remoteSort: false,
        	autoLoad: true,
        	fields: ['id','messageId','content', 'fromUser', 'toUser', 'lastUpdatedTime', 'status', 'subject', 'sendTime'],
        	url: '${ctx}/messages/jlist'
        }),
      listeners:{render: function(p){ p.getSelectionModel().on('rowselect', function(sm, row, rec){ 
      											if( rec ){
      												var msgReadPnl = Ext.getCmp('msg-read-panel');
      												if( msgReadPnl ){
      													msgReadPnl.readMsg( rec.data.messageId );
      												}
      											}
      										});}},
			cm: new Ext.grid.ColumnModel([new Ext.grid.RowNumberer(), 
						{header:'', sortable: true, dataIndex:'status', width: 28, fixed:true,menuDisabled:true, renderer: renderStatus},
						{header:'From', sortable: true, dataIndex:'fromUser', width: 100, renderer: renderAvator},
						{header:'Subject', sortable: true, dataIndex:'subject', width: 300},
						{header:'Send Time', sortable: true, dataIndex:'sendTime', width: 160}
			 		]),
			tbar: [ {text: 'Read', width: 100, handler: function(vobj, env){
			 								var gm = Ext.getCmp('service-grid-message'); 
			 								var s = gm.getSelections();
			 								var sls = s.length ? s[0].data : {};
			 								var data = { id: sls['id'], text: sls['subject'], url:'${ctx}/messages/jread?id=' + sls['messageId'],
			 									  root: 'message',
			 									  mappings: [{cid: 'msg-read-subject', field:'subject'},
			 									  					 {cid: 'msg-read-fromUser', field:'fromUser'},
			 									  					 {cid: 'msg-read-content', field:'content'}],
			 									  config:{
			 									  	id:'service-msg-read-panel-' + sls['id'],
			 									  	xtype:  'panel',
			 									  	layout: 'border',
			 									  	region: 'center',
			 									  	items: [{
																					xtype: 'panel',
																					region: 'center',
																					layout: 'border',
																					split:true,
																					height: 300,
																					border:false,
																					shadow:true,
																					margins: '0 0 0 0',
																					items:[{
																						xtype: 'panel',
																						region: 'north',
																						layout: 'border',
																						border:false,
																						margins: '0 0 0 0',
																						height: 40,
																						items:[{
																						xtype: 'panel',
																						region: 'north',
																						layout: 'column',
																						border:false,
																						margins: '2 2 0 0',
																						height: 20,
																						items:[{
																							xtype: 'label',
																							region: 'west',
																							width: 100,
																							text:'Subject:'
																						},{xtype: 'label', id:'msg-read-subject', region: 'east', text: 'Testing'}]
																					},{
																						xtype: 'panel',
																						region: 'center',
																						border:false,
																						margins: '2 2 0 0',
																						height: 20,
																						layout: 'column',
																						items:[{
																							xtype: 'label',
																							region: 'west',
																							border:false,
																							width: 100,
																							text:'From:'
																						},{xtype: 'label', id:'msg-read-fromUser', region: 'east', text: 'Long Zou'}]
																					}]}
																					,{ xtype: 'panel',  region:'center', layout:'border', margins: '5 5 5 5', 
																						items:[{xtype: 'label', id:'msg-read-content', region:'center', html:''}]}]
																				}]
			 									  }
			 								};
			 								
			 								var jsl = new JPoet.ServiceLaunch(data);
			 								jsl.launch(Ext.getCmp('main-tabs'));
							}, scope: this }, 
			 				'-', {text: 'New',width: 100, handler:function(vobj, vs){ 
			 								var rec = {data:{id:0}};
			 								var repTitle = 'New message';
			 								this.launchListeners.writeMsg.createDelegate(this, [rec, repTitle, '']).call();  
			 							}, scope: this}, 
			 				'-', {text: 'Reply', width: 100, handler: function( vobj, env ){ 
			 								var gm = Ext.getCmp('service-grid-message'); 
			 								var s = gm.getSelections();
			 								var rec = s.length ? s[0] : {data:{id:0}};
			 								var repTitle = rec.data.subject ? 'RE:' + rec.data.subject : 'New message';
			 								this.launchListeners.writeMsg.createDelegate(this, [rec, repTitle, 'RE:']).call();  
			 							}, scope: this}, 
			 				'-', {text: 'Forward', width: 100, handler: function( vobj, env ){ 
			 								var gm = Ext.getCmp('service-grid-message'); 
			 								var s = gm.getSelections();
			 								var rec = s.length ? s[0] : {data:{id:0}};
			 								var repTitle = rec.data.subject ? 'Fw:' + rec.data.subject : 'New message';
			 								this.launchListeners.writeMsg.createDelegate(this, [rec, repTitle, 'Fw:']).call();  
			 							}, scope: this}, 
              '-', {text: 'Delete', width: 100, handler: function( vobj, env ){ 
			 								var gm = Ext.getCmp('service-grid-message'); 
			 								var s = gm.getSelections();
			 								var recs = [];
			 								for( var i = 0; i < s.length; i ++ ){
			 									var r = s[i];
			 									if( r ){
			 										recs[recs.length] = r.data;
			 									}
			 								}
			 								var repTitle = 'Do you want to delete these messages?';
			 								this.launchListeners.deleteMsg.createDelegate(this, [recs, repTitle]).call();  
			 							}, scope: this}]
		}), {xtype:'readMsgPanel', id:'msg-read-panel', split:true, region:'south'}]
	},
	listeners: {
		 writeMsg:function(rec, text, subPrefix){
					 var data = { 
					       id:'msg-write-1', text: 'New Message', 
								 url:'${ctx}/messages/jread?id=' + ( rec ? rec.data.id : 0 ),
								 root: 'message',
								 config: {xtype:'panel', layout:'border', region:'center', items:[ {xtype:'writeMessagePanel', region:'center', title:'Write Message'} ]},
								 baseParams: rec ? rec.data : {},
								 mappings:[{cid:'msg-w-subject', field:'subject', prefix: subPrefix},
								 					 {cid:'msg-w-toUser', field:'fromUser'},
								 					 {cid:'msg-w-content', field:'content', prefix: '<div class="msg-body">', suffix: '</div>'}]
					 };
					if( typeof data.config != 'undefined'){
						if( typeof data.config.title != 'undefined' ){
							if( text ){
								data.config.title = text;
							}
						}
						(new JPoet.ServiceLaunch( data )).launch(Ext.getCmp('main-tabs')); 
					}
		},
		deleteMsg:function( recs, text ){
			Ext.MessageBox.confirm( 'Delete?', text, function(btn, txt ){ if( btn =='yes' ){
			  var msgIds='';
			  for( var i = 0; i < recs.length; i ++ ){
			  	msgIds = msgIds +',' + recs[i].id;
			  }
				Ext.Ajax.request({url:'${ctx}/message/jdelete?id=' + msgIds, 
					success: function(resp, opt){
						alert(resp.responseText);
					}, 
					failure: function(resp, opt){alert(resp.responseText);}}); 
			} } );
		}
		}
}
