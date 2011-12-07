<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<%@ include file="/common/taglibs.jsp"%>
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
<title><fmt:message key="page.title"/></title>
<link href="css/all.css" rel="stylesheet" type="text/css" />
<link href="css/style.css" rel="stylesheet" type="text/css" />
<script language="javascript" src="scripts/prototype.js"></script>
<script language="javascript" src="scripts/comparebox.js"></script>
</head>
<body>
<script language="javascript" src="scripts/wz_tooltip.js"></script>
<script language="javascript" src="scripts/divtips.js"></script>
<div class="content">
<%@ include file="/common/header.jsp"%>
<div id="Tips" style="DISPLAY: none; FONT-SIZE: 13px; Z-INDEX: 99; WIDTH: 200px; POSITION: absolute">
    
</div>
<div class="product_list">
	<h2><fmt:message key="page.productQuery"/></h2>
        <c:if test="${gbu != null}">
        <div><h3><fmt:message key="page.selectByCategory"><fmt:param value="${gbu.name}"/></fmt:message></h3>
	<ul>
        <c:forEach var="cate" items="${categories}">
	   <li> 
	   	<a href="category?categoryId=${cate.id}" onmouseover="DivTips.loadUrl('category?categoryId=${cate.id}')" onmouseout="DivTips.hidden();"><em>${cate.name}</em>(100)</a>
		 </li>
        </c:forEach>
        </ul>
	</div>
        </c:if>
        <c:if test="${gbu != null}">
        <div><h3><fmt:message key="page.selectByBrand"><fmt:param value="${gbu.name}"/></fmt:message></h3>
	<ul>
        <c:forEach var="brd" items="${brands}">
	   <li>
	   	<a href="#"><em>${brd.name}</em>(100)</a>
            </li>
        </c:forEach>
        </ul>
	</div>
        </c:if>
	<div><h3><fmt:message key="page.selectByGbu"/></h3>
	<ul>
     <c:forEach var="gbu" items="${gbus}">
	   <li>
	   	<a href="gbu?gbuId=${gbu.id}"><em>${gbu.name}</em>(100)</a>
		 </li>
     </c:forEach>
  </ul>
	</div>
</div>				
<div id="ComparePanel" class="content_right">
	<h5><fmt:message key="page.compareProduct"/></h5>
	<div id="CompareProduct">
	</div>		
	<span class="button"><a  href="#"><fmt:message key="page.compare"/></a></span>
	<span class="button"><a href="javascript:Comparer.clear();"><fmt:message key="page.clear"/></a></span>
</div>
<%@ include file="/common/footer.jsp" %>
</div>
</body>
</html>
