<%@ include file="/common/taglibs.jsp"%>
<%response.setContentType("application/x-json");%>
{
	window:{
		id:'msg-write-form',
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
      											//var dstore = g.getStore();
      											if( rec ){
      												//var rec = dstore.getAt(row);
      												var msgFrom = Ext.getCmp('msg-read-panel-fromUser');
      												var msgContent = Ext.getCmp('msg-read-panel-content');
      												var msgSubject = Ext.getCmp('msg-read-panel-subject');
      												msgFrom.setText( rec.data.fromUser );
      												msgSubject.setText( rec.data.subject );
      												if( msgContent.items ){ 
	      												var cmpInContent = msgContent.getComponent(0);
	      												if( cmpInContent ){
	      													//msgContent.remove( cmpInContent );
	      													cmpInContent.setText(rec.data.content,false);
	      												}
      												}
      												//msgContent.add( {xtype: 'label', html: rec.data.content, region: 'center'} );
      												//alert( rec.data.content );
      											}
      										});}},
			cm: new Ext.grid.ColumnModel([new Ext.grid.RowNumberer(), 
						{header:'', sortable: true, dataIndex:'status', width: 28, fixed:true,menuDisabled:true, renderer: renderStatus},
						{header:'From', sortable: true, dataIndex:'fromUser', width: 100, renderer: renderAvator},
						{header:'Subject', sortable: true, dataIndex:'subject', width: 300},
						{header:'Send Time', sortable: true, dataIndex:'sendTime', width: 160}
			 		]),
			tbar: [ {text: 'Read', width: 100, handler: function(vobj, env){ alert( this.getXType() ); }, scope: Ext.getCmp('service-grid-message') }, 
			 				'-', {text: 'New',width: 100, handler:function(vobj, vs){ 
			 								var gm = Ext.getCmp('service-grid-message'); 
			 								var s = gm.getSelections();
			 								var sls = s.length ? s[0].data : {};
			 								var urlData = {};
			 								urlData.text = sls['subject'];
			 								urlData.url = "${ctx}/messages/read?id=" + sls['messageId'];
			 								urlData.id = sls['id'];
			 								var thisSpl = new JPoet.ServiceLaunch(urlData);
			 								thisSpl.launch( Ext.getCmp('main-tabs') );
			 							}}, 
			 				'-', {text: 'Reply', width: 100}, 
			 				'-', {text: 'Forward', width: 100}, 
              '-', {text: 'Delete', width: 100}]
		}), {
			xtype: 'panel',
			region: 'south',
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
				},{xtype: 'label', id:'msg-read-panel-subject', region: 'east', text: 'Testing'}]
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
				},{xtype: 'label', id:'msg-read-panel-fromUser', region: 'east', text: 'Long Zou'}]
			}]}
			,{ xtype: 'panel', id:'msg-read-panel-content', region:'center', layout:'border', margins: '5 5 5 5', 
				items:[{xtype: 'label', region:'center', html:''}]}]
		}]
	}
}
