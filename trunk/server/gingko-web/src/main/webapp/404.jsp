<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<%@ page language="java" errorPage="/error.jsp" pageEncoding="UTF-8" contentType="text/html;charset=utf-8" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"  %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>
<%@ taglib prefix="authz" uri="http://www.springframework.org/security/tags" %>
<c:set var="ctx" value="${pageContext.request.contextPath}"/>
<c:set var="datePattern"><fmt:message key="date.format"/></c:set>
<c:set var="navigator">home</c:set>
<authz:authentication property="credentials" var="logged"/>	
<authz:authentication property="principal" var="loggedUser"/>	
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
<title><fmt:message key="page.title"/></title>
<link href="${ctx}/css/all.css" rel="stylesheet" type="text/css" />
<link href="${ctx}/css/style.css" rel="stylesheet" type="text/css" />
<script language="javascript" src="${ctx}/scripts/comparebox.js"></script>
</head>
<body>
<div class="content">
<%@ include file="/common/header.jsp"%>
<div class="content_left">
	<h2><fmt:message key="page.profileInfo"/></h2>
	<div class="content_body">
		<!-- Webshops -->
		<div class="row" style="width:745px">
			<h3><fmt:message key="page.detailInfo" /></h3>
			<div class="basic" style="width:745px;float:left">
				<span>You access url is not found.</span>
			</div>		
		</div>
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
