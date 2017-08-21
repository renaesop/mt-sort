/**
 * Created by fed on 2017/8/21.
 */
const qiniu = require('qiniu');

var config = new qiniu.conf.Config();
var path = require('path');
var localFile = path.join(__dirname, "result.tar.gz");
var formUploader = new qiniu.form_up.FormUploader(config);
var putExtra = new qiniu.form_up.PutExtra();
var key='result.tar.gz';

var options = {
  scope: 'react',
};
var putPolicy = new qiniu.rs.PutPolicy(options);
var accessKey = 'rR8CNdvF6coZYuom7o3jb2H2eg3ymSmkHEtDqssE';
var secretKey = '9O2Y_8nUiWBKjsAXuzlTpRfeVVU6aypbAkk1y68w';
var mac = new qiniu.auth.digest.Mac(accessKey, secretKey);
var uploadToken= putPolicy.uploadToken(mac);

// 文件上传
formUploader.putFile(uploadToken, key, localFile, putExtra, function(respErr,
                                                                     respBody, respInfo) {
  if (respErr) {
    throw respErr;
  }
  if (respInfo.statusCode == 200) {
    console.log(respBody);
  } else {
    console.log(respInfo.statusCode);
    console.log(respBody);
  }
});