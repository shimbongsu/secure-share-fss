/* 
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */


/**
 * 控制图片大小
 *
 * @param imgSource
 *            图片对象
 * @param maxWidth
 *            最大宽度
 * @param maxHeight
 *            最大高度
 * @return
 */
function showImage(imgSource, maxWidth, maxHeight) {
	var tmp_img = new Image();
	tmp_img.src = imgSource.src;
	image_x = tmp_img.width;
	image_y = tmp_img.height;

	if (image_x > maxWidth) {
		tmp_img.height = image_y * maxWidth / image_x;
		tmp_img.width = maxWidth;

		if (tmp_img.height > maxHeight) {
			tmp_img.width = tmp_img.width * maxHeight / tmp_img.height;
			tmp_img.height = maxHeight;
		}
	} else if (image_y > maxHeight) {
		tmp_img.width = image_x * maxHeight / image_y;
		tmp_img.height = maxHeight;

		if (tmp_img.width > maxWidth) {
			tmp_img.height = tmp_img.height * maxWidth / tmp_img.width;
			tmp_img.width = maxWidth;
		}
	}

	imgSource.width = tmp_img.width;
	imgSource.height = tmp_img.height;
}
