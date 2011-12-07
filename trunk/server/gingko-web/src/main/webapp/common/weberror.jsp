<%-- 
    Document   : weberror
    Created on : 2009-3-27, 1:28:19
    Author     : leo
--%>


<%@ include file="/common/taglibs.jsp"%>
<%@ include file="/common/pageTop.jsp"%>
<%@ include file="/common/greybox.jsp"%>
<html>
	<head>
		<title>Error</title>
		
	</head>
	<body>
		<table width="90%" border="0" align="center" class="tableCss">
			<tr class="table_title">
				<td>
					Error
				</td>
			</tr>
			<tr>
				<td align="center">
					Operation failed, please try again!
					<s:if test="exception.message != null">
						<br>
						<font color="red">May cause by:<s:property value="exception.message" /> </font>
					</s:if>
					<div id="detail_error_msg" style="display:none">
						<pre>
							<s:property value="exceptionStack" />
						</pre>
					</div>
				</td>
			</tr>
		</table>
	</body>
</html>