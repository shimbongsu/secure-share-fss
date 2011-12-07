<%@ page language="java" errorPage="/error.jsp" pageEncoding="UTF-8" contentType="text/html;charset=utf-8" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"  %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>
<%@ taglib prefix="authz" uri="http://www.springframework.org/security/tags" %>
<%@ taglib prefix="ec" uri="http://www.extremecomponents.org" %>
<%@ taglib uri="/struts-tags" prefix="s" %>
<c:set var="ctx" value="${pageContext.request.contextPath}"/>
<c:set var="sessionId" value="${pageContext.request.session.id}"/>
<c:set var="datePattern"><fmt:message key="date.format"/></c:set>
<authz:authentication property="credentials" var="logged"/>	
<authz:authentication property="principal" var="loggedUser"/>	