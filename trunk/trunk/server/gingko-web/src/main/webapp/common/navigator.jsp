<%@ include file="/common/taglibs.jsp"%>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<title></title>
<link rel="stylesheet" type="text/css" media="all"	href="../styles/emx_nav_right.css" />
</head>


<body> 
  <div id="globalNav"> 
    <img alt="" src="gblnav_left.gif" height="32" width="4" id="gnl"> <img alt="" src="glbnav_right.gif" height="32" width="4" id="gnr"> 
    <div id="globalLink"> 
      <a href="#" id="gl1" class="glink" ><fmt:message key="navigator.resource"/>
      </a><a href="#" id="gl2" class="glink" ><fmt:message key="navigator.product"/>
      </a><a href="#" id="gl3" class="glink" ><fmt:message key="navigator.ticket"/>
      </a><a href="#" id="gl4" class="glink"><fmt:message key="navigator.team"/>
      </a><a href="#" id="gl5" class="glink"><fmt:message key="navigator.form"/>
      </a><a href="#" id="gl6" class="glink"><fmt:message key="navigator.fax"/>
      </a> 
    </div> 
    <!--end globalLinks--> 
    <form id="search" action=""> 
      <input name="searchFor" type="text" size="10"> 
      <a href="">search</a> 
    </form> 
  </div> 
  <!-- end globalNav --> 
</html>
