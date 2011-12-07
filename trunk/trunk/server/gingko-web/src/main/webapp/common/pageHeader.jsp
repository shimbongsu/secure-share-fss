<%-- 
    Document   : pageHeader
    Created on : 2009-3-18, 22:56:21
    Author     : leo
--%>
<%@ include file="/common/taglibs.jsp"%>
<%@page import="com.s7turn.common.struts.Constants"%>
</head>
<body>
<div id="container">
  <!-- Header -->
  <div id="header" <c:if test="${greybox=='true'}" >style="display:none"</c:if>>
    <!-- Your logo
    <h1 id="logo"><span></span>logo</h1>-->
    <!-- Your slogan
    <p id="slogan">We can achieve your's thing!</p> -->
    <ul class="searchbox" >
	   <input type="text" id="search"/><a href="/search.go#"><fmt:message key="top.search"/></a>
    </ul>
  </div>
  <!-- /header -->
  <!-- mainNav -->
  <div id="mainNav" <c:if test="${greybox=='true'}" >style="display:none"</c:if>>
    <ul>
        ${navigator}
      <li <c:if test="${ navigator=='share_resource' }">class="current"</c:if> id="share_resource"><a href="${ctx}/"><fmt:message key="nav.share.resource"/></a></li>
      <li <c:if test="${ navigator=='my_resource' }">class="current"</c:if> id="my_resource"><a href="${ctx}/"><fmt:message key="nav.my.resource"/></a></li>
      <li <c:if test="${ navigator=='my_friend' }">class="current"</c:if> id="my_friends"><a  href="${ctx}/friends/list.go"><fmt:message key="nav.my.friend"/></a></li>
      <li <c:if test="${ navigator=='account' }">class="current"</c:if> id="member"><a  href="${ctx}/"><fmt:message key="nav.member"/></a></li>
      <li <c:if test="${ navigator=='my_profile' }">class="current"</c:if> id="profile"><a  href="${ctx}/member/myprofile.go"><fmt:message key="nav.my.profile"/></a></li>
      <li class="current" style="visibility: hidden;"></li>
    <li style="left: 694px; width: 0px; visibility: visible; opacity: 1;" class="background">
	<div class="left"></div>
	</li>
	</ul>
  </div>
  <!-- /mainNav -->
<%@ include file="/common/messages.jsp"%>