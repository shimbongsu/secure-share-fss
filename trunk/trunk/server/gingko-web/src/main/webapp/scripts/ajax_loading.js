var AjaxLoader = {	
	
	initialize:function(){
		
	},
	
	loadUrl:function(sURL, sId){
		this.responseId = sId;
		
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
		$(this.responseId).innerHTML = request.responseText;
		Tip(request.responseText,SHADOW, true, FADEIN, 300, FADEOUT, 300, STICKY, 1, CLOSEBTN, true, CLICKCLOSE, true, TITLE, 'Sub Category', PADDING, 9);
		$(this.responseId).style.display="block";
	},
	
	hidden:function(){
		UnTip();
	}
	
};

AjaxLoader.initialize();
