<div class="pagenumber">
    <a href="${firstUrl}"><fmt:message key="page.first"/></a>
		<c:forEach var="pg" items="${pagings}">
    <a href="${pg.url}" <c:if test="${pg.current}">class="current"</c:if>>${pg.name}</a>
    </c:forEach>  
    <a href="${lastUrl}"><fmt:message key="page.last"/></a>
</div>
