<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<%@ include file="/common/taglibs.jsp"%>
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
<title>S7turn Community</title>
<link href="/rsm/css/all.css" rel="stylesheet" type="text/css" />
<link href="/rsm/css/style.css" rel="stylesheet" type="text/css" />
<link href="/js/resources/css/ext-all.css" rel="stylesheet" type="text/css" />
<link href="/js/resources/css/viewstyle.css" rel="stylesheet" type="text/css" />
<script language="javascript" src="/js/ext/adapter/ext/ext-base.js"></script>
<script language="javascript" src="/js/ext/ext-all.js"></script>

<script language="javascript" src="/js/apps/ServiceGrid.js"></script>
<script language="javascript" src="/js/apps/ServiceStores.js"></script>
<script language="javascript" src="/js/apps/GingkoForms.js"></script>
<script language="javascript" src="/js/apps/JPoetTree.js"></script>
<script language="javascript" src="/js/apps/ServiceForms.js"></script>
<script language="javascript" src="/js/apps/FileUploadPanel.js"></script>
<script language="javascript" src="/js/apps/ServiceLaunch.js"></script>
<script language="javascript" src="/js/apps/ServiceMain.js"></script>
<script language="javascript" src="/js/apps/ServicePanel.js"></script>
<script language="javascript" src="/js/swfuploads/swfupload.js"></script>
<script language="javascript">
JPoet.GINGKO_UPLOAD_URL = "${ctx}/gingko/jupload;jsessionid=${sessionId}";
</script>
</head>
<body>
<div id="header"><div style="float:right;margin:5px;" class="x-small-editor"><input type="text" id="search" /></div></div>	
<div class="content">
<div id="content_body" style="height: 800px">
</div>
</div>
</body>
</html>
