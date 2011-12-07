<%@ page pageEncoding="utf-8"%>
<%@ include file="/common/taglibs.jsp"%> 
<script type='text/javascript' src='../scripts/common.js'></script> 
<script type='text/javascript' src='../dwr/engine.js'></script>
<script type='text/javascript' src='../dwr/util.js'></script>
<script type='text/javascript' src='../dwr/interface/MessageRemote.js'></script>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html xmlns="http://www.w3.org/1999/xhtml">
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<meta http-equiv="Refresh" content="120"> 
		<script language="javascript">
		var activityCount = 0;
		var taskCount = 0;
        var viewType = 0;
		function viewMsg(){
            if (viewType == 0){
                viewType = 1;                  
		        MessageRemote.hasTaskMsg(hasTaskMsgValue); 
            } else {        
                viewType = 0;           
		        MessageRemote.hasActivityChangeMsg(hasActivityChangeMsgValue); 
            }
		    var t=setTimeout("viewMsg()",10000);
		}
        var leftPosition = screen.availWidth-410 ;
        var topPosition = screen.availHeight-200;
		function hasActivityChangeMsgValue(data){
		    if (data > activityCount){                              
		        activityCount = data; 
		        window.showModelessDialog("../activityChangeMsg.html?method=view","activityLogWindow","center:no;dialogLeft:"+leftPosition+";dialogTop:"+topPosition+";scroll:0;status:0;help:0;resizable:0;dialogWidth:400px;dialogHeight:160px");
		    } 
		}
		function hasTaskMsgValue(data){
		    if (data > taskCount){                              
		        taskCount = data;
                window.showModelessDialog("../activityChangeMsg.html?method=viewTask","taskWindow","center:no;dialogLeft:"+leftPosition+";dialogTop:"+topPosition+";scroll:0;status:0;help:0;resizable:0;dialogWidth:400px;dialogHeight:160px");
		    } 
		}
		var t=setTimeout("viewMsg()",10000);
		//activityChangeMsgList();
		function srcMarquee(){
			this.ID = document.getElementById(arguments[0]);
			if(!this.ID){this.ID = -1;return;}
			this.Direction = this.Width = this.Height = this.DelayTime = this.WaitTime = this.Correct = this.CTL = this.StartID = this.Stop = this.MouseOver = 0;
			this.Step = 1;
			this.Timer = 30;
			this.DirectionArray = {"top":0 , "bottom":1 , "left":2 , "right":3};
			if(typeof arguments[1] == "number")this.Direction = arguments[1];
			if(typeof arguments[2] == "number")this.Step = arguments[2];
			if(typeof arguments[3] == "number")this.Width = arguments[3];
			if(typeof arguments[4] == "number")this.Height = arguments[4];
			if(typeof arguments[5] == "number")this.Timer = arguments[5];
			if(typeof arguments[6] == "number")this.DelayTime = arguments[6];
			if(typeof arguments[7] == "number")this.WaitTime = arguments[7];
			if(typeof arguments[8] == "number")this.ScrollStep = arguments[8]
			this.ID.style.overflow = this.ID.style.overflowX = this.ID.style.overflowY = "hidden";
			this.ID.noWrap = true;
			this.IsNotOpera = (navigator.userAgent.toLowerCase().indexOf("opera") == -1);
			if(arguments.length >= 7)this.Start();
		}
		srcMarquee.prototype.Start = function(){
			if(this.ID == -1)return;
			if(this.WaitTime < 800)this.WaitTime = 800;
			if(this.Timer < 20)this.Timer = 20;
			if(this.Width == 0)this.Width = parseInt(this.ID.style.width);
			if(this.Height == 0)this.Height = parseInt(this.ID.style.height);
			if(typeof this.Direction == "string")this.Direction = this.DirectionArray[this.Direction.toString().toLowerCase()];
			this.HalfWidth = Math.round(this.Width / 2);
			this.BakStep = this.Step;
			this.ID.style.width = this.Width;
			this.ID.style.height = this.Height;
			if(typeof this.ScrollStep != "number")this.ScrollStep = this.Direction > 1 ? this.Width : this.Height;
			var msobj = this;
			var timer = this.Timer;
			var delaytime = this.DelayTime;
			var waittime = this.WaitTime;
			msobj.StartID = function(){msobj.Scroll()}
			msobj.Continue = function(){
				if(msobj.MouseOver == 1){
				setTimeout(msobj.Continue,delaytime);
		     }
		     else{ clearInterval(msobj.TimerID);
				msobj.CTL = msobj.Stop = 0;
				msobj.TimerID = setInterval(msobj.StartID,timer);
		     }
		    }
			msobj.Pause = function(){
				msobj.Stop = 1;
				clearInterval(msobj.TimerID);
				setTimeout(msobj.Continue,delaytime);
		    }
			msobj.Begin = function(){
		   msobj.ClientScroll = msobj.Direction > 1 ? msobj.ID.scrollWidth : msobj.ID.scrollHeight;
		   if((msobj.Direction <= 1 && msobj.ClientScroll <msobj.Height) || (msobj.Direction > 1 && msobj.ClientScroll <msobj.Width))return;
		   msobj.ID.innerHTML += msobj.ID.innerHTML;
		   msobj.TimerID = setInterval(msobj.StartID,timer);
		   if(msobj.ScrollStep < 0)return;
		   msobj.ID.onmousemove = function(event){
		       if(msobj.ScrollStep == 0 && msobj.Direction > 1){
					var event = event || window.event;
					if(window.event){
						if(msobj.IsNotOpera){msobj.EventLeft = event.srcElement.id == msobj.ID.id ? event.offsetX - msobj.ID.scrollLeft : event.srcElement.offsetLeft - msobj.ID.scrollLeft + event.offsetX;}
						else{msobj.ScrollStep = null;return;}
					}
					else{msobj.EventLeft = event.layerX - msobj.ID.scrollLeft;}
					msobj.Direction = msobj.EventLeft > msobj.HalfWidth ? 3 : 2;
					msobj.AbsCenter = Math.abs(msobj.HalfWidth - msobj.EventLeft);
					msobj.Step = Math.round(msobj.AbsCenter * (msobj.BakStep*2) / msobj.HalfWidth);
					}
				}
				msobj.ID.onmouseover = function(){
					if(msobj.ScrollStep == 0)return;
					msobj.MouseOver = 1;
					clearInterval(msobj.TimerID);
				}
				msobj.ID.onmouseout = function(){
				if(msobj.ScrollStep == 0){
					if(msobj.Step == 0)msobj.Step = 1;
					return;
				}
				msobj.MouseOver = 0;
				if(msobj.Stop == 0){
					clearInterval(msobj.TimerID);
					msobj.TimerID = setInterval(msobj.StartID,timer);
				}}}
				setTimeout(msobj.Begin,waittime);
		}
		
		srcMarquee.prototype.Scroll = function(){
			switch(this.Direction){
			case 0:
				this.CTL += this.Step;
				if(this.CTL >= this.ScrollStep && this.DelayTime > 0){
					this.ID.scrollTop += this.ScrollStep + this.Step - this.CTL;
					this.Pause();
					return;
				}
				else{
					if(this.ID.scrollTop >= this.ClientScroll){this.ID.scrollTop -= this.ClientScroll;}
					this.ID.scrollTop += this.Step;
				}
				break;
			
			case 1:
				this.CTL += this.Step;
				if(this.CTL >= this.ScrollStep && this.DelayTime > 0){
					this.ID.scrollTop -= this.ScrollStep + this.Step - this.CTL;
					this.Pause();
					return;
				}
				else{
					if(this.ID.scrollTop <= 0){this.ID.scrollTop += this.ClientScroll;}
					this.ID.scrollTop -= this.Step;
				}
				break;
		
			case 2:
				this.CTL += this.Step;
				if(this.CTL >= this.ScrollStep && this.DelayTime > 0){
					this.ID.scrollLeft += this.ScrollStep + this.Step - this.CTL;
					this.Pause();
					return;
				}
				else{
					if(this.ID.scrollLeft >= this.ClientScroll){this.ID.scrollLeft -= this.ClientScroll;}
					this.ID.scrollLeft += this.Step;
				}
				break;
		
			case 3:
				this.CTL += this.Step;
				if(this.CTL >= this.ScrollStep && this.DelayTime > 0){
					this.ID.scrollLeft -= this.ScrollStep + this.Step - this.CTL;
					this.Pause();
					return;
				}
				else{
					if(this.ID.scrollLeft <= 0){this.ID.scrollLeft += this.ClientScroll;}
				this.ID.scrollLeft -= this.Step;
				}
				break;
			}
		} 	
		//list messages add by kellen
		function listMessages(){		
		    var userId = '';
		    userId = "<c:out value="${user.id}"/>";
		   // alert(userId);
			MessageRemote.listMessages(userId,listCallback);		
		}
		function listCallback(rslts){
			var messages = rslts;
			
			var str = '';
			var time = '';
			var cnt = 0; 
			for(var i=0;i<messages.length;i++){ 
			
				var d = messages[i].operatime;		 
				var yyyy = d.getYear();   
		        var MM = d.getMonth()+1;   
		        var dd = d.getDate();   
		        var hh = d.getHours();   
		        var mm = d.getMinutes();   
		        var ss = d.getSeconds(); 

			    time = yyyy;
				MM < 10 ? time+='0'+MM : time+=MM;
				dd < 10 ? time+='0'+dd : time+=dd;
				time += ' ';
				hh < 10 ? time+='0'+hh : time+=hh;
				time += ':';
		        mm < 10 ? time+='0'+mm : time+=mm;
		        //time += ':';
		        //ss < 10 ? time+='0'+ss : time+=ss;
		        var onclickStr = 'javascript:MM_openBrWindow("../msgRelease.html?method=view&type=1&id='+messages[i].id+'","","width=500,height=400")';
				str += "&nbsp;"+(i+1)+".&nbsp;<a href='"+onclickStr+"'>"+messages[i].title +"&nbsp;<font color='blue'>"+time+"</font>" + "</a>";
				
				cnt++;

				if((cnt%3==0)||(cnt==messages.length)) {
					str+="<br/>";
				}
			}	
			
			if(''==str)
				str = '无通知信息!<br>';
		 		$('ScrollMe').innerHTML = str;
		}
		  
		function messageShow(){ 
			listMessages();
			//setTimeout("messageShow()",120000);
		}
		
		messageShow();	
        

      function showPopupWin(){
		  window.showModelessDialog("http://www.feitec.com/ad.htm","CHINAZindexP","center:no;dialogLeft:5px;dialogTop:5px;scroll:0;status:0;help:0;resizable:0;dialogWidth:900px;dialogHeight:625px")
      } 
		</script>

		<style type="text/css">
			body {
				font: 9px Verdana;
				border: 1px solid #FFE04F;
				background: #FFFABF;
				padding: 2px;
				margin-left: 0px;
				margin-top: 0px;
				margin-right: 0px;
				margin-bottom: 0px;
			}
			
			#Scroll{
				clear:both;
				margin:0 auto;
				width:100%;
				height:27px;
				line-height:27px;
				text-align:left;
				color:#C2130E;
				background:url(../images/n_scroll.gif) no-repeat 0 3px;
				padding-left:20px;
				font-size:12px;
			}
			#Scroll a{
				text-decoration:none;
				color:#000000;
				margin-right:5px;
				background:url(../images/n_bar.gif) no-repeat 0 0;
				padding-left:10px;
				padding-right:20px;
			}
			#Scroll a:hover{
				color:#ff0000;
				text-decoration:underline;				
			}
			#Scroll a.s_end{
				padding-right:0;
				margin-left:8px;
			}
			#Scroll a.s_more{
				padding-right:0;
				margin-left:8px;
				color:#ff0000;
			}
		</style>
	</head>
	<body>
		<table width="100%" border="0" cellspacing="0" cellpadding="0">
			<tr height=28 bgcolor="#FFFFD2">
				<td width=180>
					<div id="Scroll">
						<div id="ScrollMe" style="overflow:hidden;height:27px;"></div>
					</div>
					<script>new srcMarquee("ScrollMe",0,1,1100,27,30,4000,4000,27)</script>
				</td>
			</tr>
		</table>

		<script language="javascript">
		   function startShow(){
		      strPara="top="+(screen.height-200)+",left="+(screen.width-300)+",height=150,width=250,status=no,toolbar=no,menubar=no,location=no"
		      window.open('newtask.jsp','task',strPara)
		   }
		   function stop(){
		      window.clearInterval(timer2);
		   }
		   //window.setInterval("startShow()",30000)
		</script>
		
	</body>
</html>
