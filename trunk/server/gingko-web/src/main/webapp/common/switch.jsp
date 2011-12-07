<%@ page pageEncoding="utf-8"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
		<title>sis</title>
		<script language="javascript">
			function switchSysBar(){
				if (parent.document.getElementById('main').cols=="180,10,*"){	
					document.getElementById('leftbar').style.display="";
					parent.document.getElementById('main').cols="0,10,*";		
				}
				else{
					parent.document.getElementById('main').cols="180,10,*";		
					document.getElementById('leftbar').style.display="none"
				}
			}
			function load(){
			    if (parent.document.getElementById('main').cols=="0,10,*"){	
					document.getElementById('leftbar').style.display="";		
				}
			}
		</script>
	</head>
	<body leftmargin=0 topmargin=0 marginwidth=0 marginheight=0 bgcolor="#000000" onload="load()">
		<center>
			<table width=100% height=100% border=0 cellpadding=0 cellspacing=0>
				<tr>
					<td bgcolor="#009FEF" width="1">
						<img src="../images/spot.gif" width=1 height=1>
					</td>
					<td id="leftbar" bgcolor="#F5F4F4" style="display: none">
						<a href="javascript:void(0);" onclick="switchSysBar()"><img
								src="../images/menu_open.gif" width="9"
								height="90" alt='展开左侧菜单' border="0">
						</a>
					</td>
					<td id="rightbar" bgcolor="#F5F4F4">
						<a href="javascript:void(0);" onclick="switchSysBar()"><img
								src="../images/menu_close.gif" width="9"
								height="90" alt='隐藏左侧菜单' border="0">
						</a>
					</td>
				</tr>
			</table>
		</center>
	</body>
</html>


