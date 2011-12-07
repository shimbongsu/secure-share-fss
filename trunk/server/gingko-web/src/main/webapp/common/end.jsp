<%-- 
    Document   : end
    Created on : 2009-3-27, 0:25:04
    Author     : leo
--%>

<%@ include file="/common/taglibs.jsp"%>
<%@ include file="/common/pageTop.jsp"%>
<%@ include file="/common/greybox.jsp"%>



	<!-- HEADER INCLUDE AREA START-->
<script type="text/javascript">
    //<![CDATA[
	function refresh() {
		parent.location.reload(true);
		parent.parent.location.href = parent.parent.location.href;
		closeGreyboxDiv();
	}
        function winClose() {
            try {
                refresh();
            } catch (e) {
                try {
                    window.opener.location.href = window.opener.location.href;
                    window.opener.location.reload(true);
                    window.close();
                } catch (e) {
                    window.close();
                }
            }
        }
			function autoClose() {
				//setInterval("winClose()", 2000);
				winClose();
			}
	   // ]]>
</script>
	<!-- HEADER INCLUDE AREA END -->
<title>message</title>

</head>
<body onload="autoClose()">
	<table width="80%" border="0" align="center" class="tableCss">
		<tr class="table_title">
			<td>
                <fmt:message key="page.end.message"/>
			</td>
		</tr>
		<tr>
			<td align="center">
                <fmt:message key="page.end.finished"/>

			</td>
		</tr>
		<tr>
			<td align="center">
				<button onclick="winClose()">
                    <fmt:message key="page.end.close"/>
				</button>
			</td>
		</tr>
	</table>
</body>
</html>