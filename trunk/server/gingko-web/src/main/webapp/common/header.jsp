<%@ taglib uri="/struts-tags" prefix="s" %>
<div class="logo"><h1><span><fmt:message key="page.logo"/></span></h1></div>
<div class="ad"><img src="${ctx}/images/ad_03.jpg" /></div>
<div class="search">
<div class="mainnav">  
	<ul>
    <li <c:if test="${navigator == 'home'}">class="nav_on"</c:if><c:if test="${navigator != 'home'}">class="nav_off"</c:if>><a href="${ctx}/index.jsp"><fmt:message key="menu.home"/></a></li>
    <li <c:if test="${navigator == 'products'}">class="nav_on"</c:if><c:if test="${navigator != 'products'}">class="nav_off"</c:if>><a href="${ctx}/gbu.go"><fmt:message key="menu.products"/></a></li>
    <li <c:if test="${navigator == 'shops'}">class="nav_on"</c:if><c:if test="${navigator != 'shops'}">class="nav_off"</c:if>><a href="#" ><fmt:message key="menu.shops"/></a></li>
    <li <c:if test="${navigator == 'account'}">class="nav_on"</c:if><c:if test="${navigator != 'account'}">class="nav_off" </c:if>><a href="${ctx}/member/myprofile.go" style="padding-right:15px;padding-left:15px"><fmt:message key="page.accountCenter"/></a></li>
    <li <c:if test="${navigator == 'agents'}">class="nav_on"</c:if><c:if test="${navigator != 'agents'}">class="nav_off"</c:if>><a href="#"><fmt:message key="menu.agentBuyers"/></a></li>
    <li <c:if test="${navigator == 'groups'}">class="nav_on"</c:if><c:if test="${navigator != 'groups'}">class="nav_off"</c:if>><a href="#"><fmt:message key="menu.groupBuyers"/></a></li>
    <li <c:if test="${navigator == 'community'}">class="nav_on"</c:if><c:if test="${navigator != 'community'}">class="nav_off"</c:if>><a href="#"><fmt:message key="menu.community"/></a></li>
    <li <c:if test="${navigator == 'qas'}">class="nav_on"</c:if><c:if test="${navigator != 'qas'}">class="nav_off"</c:if>><a href="#"><fmt:message key="menu.questionAnwser"/></a></li>
	</ul>
</div>
<form name="search" action="${ctx}/search" method="get">
	<div class="welcome">
	<c:if test="${logged != ''}">
	<fmt:message key="page.welcome"/>, ${loggedUser.screenName} <fmt:message key="page.enter"/> <a href="${ctx}/account"><fmt:message key="page.accountCenter"/></a> |<a href="j_spring_security_logout"><fmt:message key="user.logout"/></a>
	</c:if>
	<c:if test="${logged == ''}">
	<a href="${ctx}/member/login.go"><fmt:message key="page.login"/></a><s:url action="register"><fmt:message key="page.register"/></s:url>
	</c:if>	
	</div>
	<div class="search_list">
			<select name="searchType">
			  <option value="product"><fmt:message key="page.searchType"/></option>
			</select>
			<input type="text" name="query" size="38" value="<c:out value="${query}"/>"/>
			<select name="category">
			  <option value="All" selected="selected"><fmt:message key="page.allCategories"/></option>
			</select>
			<input type="submit" name="Submit" value="<fmt:message key="page.search"/>" class="btn_search"/>
		  <span><a href="#"><fmt:message key="page.advanceSearch"/></a>  |     <a href="#"><fmt:message key="page.searchUsages"/></a> </span>
	</div>
</form>	
</div>
