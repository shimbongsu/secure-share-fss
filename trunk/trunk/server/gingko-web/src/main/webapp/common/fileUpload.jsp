<link href="${ctx}/css/swfupload.css" rel="stylesheet" type="text/css" />
<script type="text/javascript" src="${ctx}/scripts/swfupload.js"></script>
<script type="text/javascript" src="${ctx}/scripts/swfupload.queue.js"></script>
<script type="text/javascript" src="${ctx}/scripts/fileprogress.js"></script>
<script type="text/javascript" src="${ctx}/scripts/handlers.js"></script>
<script type="text/javascript">
		var swfu;
		window.onload = function() {
			if (typeof(SWFUpload) === "undefined") {
				return;
			}

			var settings = {
				flash_url : "${ctx}/scripts/swfupload_f9.swf",
				upload_url: "${ctx}/gallery/uploadPhoto;jsessionid=${sessionId}?mid=1200",
				post_params: {"returnUrl" : "${returnUrl}"<c:if test="${postParams!=null}">, ${postParams}</c:if>},
				file_post_name: "uploadFile",
				attach_form: "uploadForm",
				use_query_string: false,
				file_size_limit : "100 MB",
				sessionName: "ss",
				sessionValue: "${sessionId}",
				file_types : "${fileType}",
				file_types_description : "${fileTypeDescription}",
				file_upload_limit : 100,
				file_queue_limit : 0,
				custom_settings : {
					progressTarget : "fsUploadProgress",
					cancelButtonId : "btnCancel"
				},
				debug: false,
				// The event handler functions are defined in handlers.js
				file_queued_handler : fileQueued,
				file_queue_error_handler : fileQueueError,
				file_dialog_complete_handler : fileDialogComplete,
				upload_start_handler : uploadStart,
				upload_progress_handler : uploadProgress,
				upload_error_handler : uploadError,
				upload_success_handler : uploadSuccess,
				upload_complete_handler : uploadComplete,
				queue_complete_handler : queueComplete	// Queue plugin event
			};
			
			swfu = new SWFUpload(settings);
	  };
	  
	  function SWFCommitForm(){
	  	swfu.addPostParam(name, value)
			swfu.removePostParam(name)
	  }
	</script>


