SET GSOAP_HOME=C:\WinDDK
%GSOAP_HOME%\wsdl2h.exe -v -t%GSOAP_HOME%\wtypemap.dat -y  -oGingkoWebService.h http://www.7turn.cn/gingko/gkws/GingkoLoginService?wsdl http://www.7turn.cn/gingko/gkws/GingkoUnitService?wsdl http://www.7turn.cn/gingko/gkws/GingkoUserService?wsdl http://www.7turn.cn/gingko/gkws/GingkoDigitalService?wsdl
if NOT ERRORLEVEL 1 (
	%GSOAP_HOME%\soapcpp2.exe -2 -L -C -I%GSOAP_HOME%\import  -x -w GingkoWebService.h
)

