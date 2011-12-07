<div class="w560" style="width: 550px;float: left">	
	<form id="uploadForm" name="uploadForm" action="index.php" method="post" enctype="multipart/form-data">
		<ul style="width: 580px; float: left">
			<li style="width: 80px; float: left">Name:</li>
			<li style="width: 500px; float: left"><input type="text" name="photo.name" value="" style="width: 200px;"/></li>
			<li style="width: 80px; float: left">Tags:</li>
			<li style="width: 500px; float: left"><input type="text" name="photo.tags" value="" style="width: 200px;"/></li>
			<li style="width: 80px; float: left">Description:</li>
			<li style="width: 500px; float: left"><textarea name="photo.description" style="width: 200px; height: 100px"></textarea></li>
		</ul>
		<fieldset class="flash" style="width: 500px; min-height: 50px;float: left" id="fsUploadProgress">
		<legend>Upload File List</legend>
		</fieldset>
		<div id="divStatus" style="width: 580px; float:left">0 Files Uploaded</div>
		<div>
			<input type="button" value="Upload file (Max 100 MB)" onclick="swfu.selectFiles()" style="font-size: 8pt;" />
			<input id="btnCancel" type="button" value="Cancel All Uploads" onclick="swfu.cancelQueue();" disabled="disabled" style="font-size: 8pt;" />
		</div>
	</form>
</div>