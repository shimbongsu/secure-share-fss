The system provides the capability to share your files to other guys via the security channel in Internet. And the system can track back to how the files are shared to each other.

本软件建立起一个私人的互联网络，在这个私人的互联网络里，通过安全的加密手段对所有的需要共享 的数字化文件进行加密，确保数字文件内容的安全性。
本系统采用了AES-256的加密算法，确保了文件的安全级别，同时在客户端采用的是基于Windows文件系统驱动的技术来实现，用户在使用这些共享文件时，可以完全感觉不到加解密的过程。

This system build a SNS networks for sharing the digital files. As basically, the system encrypted the file by AES-256 and the file will be shared by internet. By the client side software, it provides a transparent way for decryption.

<a href='https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=ZT5MG2C4GDUMJ&submit=1'><img src='http://www.paypal.com/en_US/i/btn/btn_donate_LG.gif' /></a>


The software components includes:
  * A windows driver for transparent Encryption / Decryption.
  * A windows service.
  * A Set of DLL for injection function.
  * A Windows Form application to manage the Secure Digital files.
  * Server side software.

本软件包括以下组件：
  * 一个文件系统驱动，用于加解密；
  * 一个Windows服务，用于管理软件的运行状态；
  * 一组系统增强的功能DLL，用于管理用户行为；
  * 一个Windows Form应用程序，用于管理数字文件；
  * 一个服务器端软件，用于管理数字文件的共享；

If you would like to update the code or modify the code, please send email to me (long.zou@gmail.com).
我们欢迎你加入到源代码的维护工作中来，如果你们有兴趣，请发邮件给我！


# Important #
The system was built by Windows 6.0 DDK & SDK. You should install the Visual Studio 2008.
# Source Code #
Please check out the source  at <a href='http://code.google.com/p/secure-share-fss/source/checkout'> Source Code </a>.

# Setup Development Environment #
## Client Development ##
  1. Please install Windows DDK 7600 or above version.
  1. Please install Windows SDK (Supports Windows 6.0 or above).
  1. Please install libs that under tools folder.

## Server Development ##
  1. Please install NetBeans 6.5 or above version.
  1. The server side development uses Maven.