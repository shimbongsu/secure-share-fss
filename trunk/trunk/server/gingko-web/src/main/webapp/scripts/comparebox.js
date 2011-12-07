function getCookie( name )
{     
	var start = document.cookie.indexOf( name + "=" );
	var len = start + name.length + 1;
	if ( ( !start ) && ( name != document.cookie.substring( 0, name.length ) ) ) 
	{         
		return null;
	}     
	if ( start == -1 ) return null;
	var end = document.cookie.indexOf( ';', len );
	if ( end == -1 ) end = document.cookie.length;
	return unescape( document.cookie.substring( len, end ) ); 
}  

function setCookie( name, value, expires, path, domain, secure )
{
	var today = new Date();
	today.setTime(today.getTime());
	if( expires ){
		expires = expires * 1000 * 60 * 60 * 24;
		}
	var expires_date = new Date( today.getTime() + (expires) );
	document.cookie = name+'='+escape( value ) + ( ( expires ) ? ';expires='+expires_date.toGMTString() : '' ) + //expires.toGMTString()
	         ( ( path ) ? ';path=' + path : '' ) +
	         ( ( domain ) ? ';domain=' + domain : '' ) +
	         ( ( secure ) ? ';secure' : '' ); 
}

function deleteCookie( name, path, domain ) 
{     
	if(getCookie(name)) 
		document.cookie = name + '=' + ( ( path ) ? ';path=' + path : '') + ( ( domain ) ? ';domain=' + domain : '' ) + ';expires=Thu, 01-Jan-1970 00:00:01 GMT'; 
}
	
	
var Comparer = new Object();
Comparer={
	
	getProducts:function(){
		var cps = getCookie( 'compareProducts' );
		return cps;
	},
	
	spliteProducts:function(cps){
		//format is: xxx:xxx:yyyyy;xxx:xxx:yyyy;
		var prds = new Array();
		var pp = 0;
		if( cps == null )
			return prds;
		while(1){
			var first = cps.indexOf(":");
			if( first < 0 )
				break;
			var firstStr = cps.substring(0,first);
			cps=cps.substring(first+1);
			var second = cps.indexOf(":");
			if( second < 0 )
				break;
			var secondStr = cps.substring(0,second);
			cps=cps.substring(second+1);
			var third = cps.indexOf(";");
			if( third < 0 )
				break;

			var thirdStr = cps.substring(0, third );
			cps=cps.substring(third + 1);
			prds[pp]=new Array();
			prds[pp][0]=firstStr;
			prds[pp][1]=secondStr;
			prds[pp][2]=thirdStr;
			pp ++;
		}
		return prds;
	},
	
	writeCookie:function(prds){
		var cstr="";
		for( var i = 0; i < prds.length; i ++ ){
			if(prds[i] != null ){
				cstr=prds[i][0]+":"+prds[i][1]+":"+prds[i][2]+";" + cstr;
			}
		}
		if( cstr != "" ){
			setCookie('compareProducts', cstr);
		}else{
			deleteCookie('compareProducts');
		}
	},

	showList:function(prods, divName){
		var TheDiv=document.getElementById(divName);
		var nodata = 1;
		if( TheDiv != null ){
			var html="<ul>";
			for(var i=0;i<prods.length; i ++){
				if( prods[i] != null ){
					html = html + "<li class='compareitem' title='" + prods[i][2]+ "'><div class='compareitem_left'>" + prods[i][2] + "</div><a class='compareitem_right' href='javascript:Comparer.remove(" + prods[i][0]+ ");'><img src='./images/remove.gif' border='0' width='24' height='24'/></a></li>"; 
					nodata = 0;
				}
			}
			html = html + "</ul>";
			TheDiv.innerHTML=html;
			TheDiv.style.display="inline";
			var CpDiv = document.getElementById('ComparePanel');
			if( CpDiv != null ){
				if( nodata == 1 ){
					CpDiv.style.display="none";
				}else{
					CpDiv.style.display="inline";
				}
			}
		}
	},
	
	addProduct:function(prodId, categoryId, prodName){
		var exists = 0;
		var prods = this.spliteProducts(this.getProducts());
		for( var i = 0; i < prods.length; i ++ ){
			if( prods[i] != null ){
				if( prodId == prods[i][0] ){
					exists = 1;
					break;
				}
				if( categoryId != prods[i][1] ){
					exists = 2;
					break;
				}
			}
		}
		
		if( exists == 0 ){
			prods[prods.length] = new Array(prodId, categoryId, prodName );
			this.writeCookie(prods);
			this.showList(prods, 'CompareProduct');
		}else if( exists == 1 ){
			alert(prodName + "已经存在于比较列表了。");
		}else if( exists == 2 ){
			alert(prodName + "与比较列表里的产品不是同一类产品，不能比较。");
		}
	},
	
	remove:function( prodId ){
		var prods = this.spliteProducts(this.getProducts());
		for( var i = 0; i < prods.length; i ++ ){
			if( prodId == prods[i][0] ){
				for( var j = i; j < prods.length - 1; j ++ ){
					prods[j] = prods[j+1];
				}
				prods[prods.length-1]=null;
				break;
			}
		}
		this.writeCookie(prods);
		this.showList( prods, 'CompareProduct');
	},
	
	clear:function(){
		var prods = new Array();
		prods[0] = null;
		this.writeCookie(prods);
		this.showList( prods, 'CompareProduct');
	},
	
	autoLoad:function(){
		var prods = this.spliteProducts(this.getProducts());
		this.showList( prods, 'CompareProduct');
	}
};


