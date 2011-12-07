var DivTips = {	
	initialize:function(){
		
	},
	
	loadUrl:function(sURL){
		var myAjax = new Ajax.Request(
			sURL,
			{
				method: 'get',
				parameters: null,
				onComplete: this.showResponse
			});
	},
	
	showResponse:function(request)
	{
		Tip(request.responseText,SHADOW, true, FADEIN, 300, FADEOUT, 300, STICKY, 1, CLOSEBTN, true, CLICKCLOSE, true, TITLE, 'Sub Category', PADDING, 9);
	},
	
	hidden:function(){
		UnTip();
	}
	
};

DivTips.initialize();
