#Initialisation du projet
#cmake_minimum_required(VERSION 3.10)

#set(CMAKE_BUILD_TYPE Release)


add_library(proxycpp 
	#soapAdvancedSecurityServiceBindingProxy.cpp
	#soapAdvancedSecurityServiceBindingService.cpp
	soapDeviceBindingProxy.cpp
	#soapDeviceBindingService.cpp 
	#soapDeviceIOBindingProxy.cpp
	#soapDeviceIOBindingService.cpp 
	#soapImagingBindingProxy.cpp
	#soapImagingBindingService.cpp
	soapMediaBindingProxy.cpp
	#soapMediaBindingService.cpp
	soapPTZBindingProxy.cpp
	#soapPTZBindingService.cpp
	soapPullPointSubscriptionBindingProxy.cpp
	#soapPullPointSubscriptionBindingService.cpp
	soapRemoteDiscoveryBindingProxy.cpp
	#soapRemoteDiscoveryBindingService.cpp
	#soapwsddProxy.cpp
	#soapwsddService.cpp
	#soapwsrmProxy.cpp
	#soapwsrmService.cpp
	)

target_include_directories(proxycpp PUBLIC 
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                          "${PROJECT_SOURCE_DIR}/../"
)
target_compile_options(proxycpp PUBLIC -Wall -g -w -fPIC -DWITH_NONAMESPACES -fno-use-cxa-atexit -fexceptions -DWITH_DOM  -DWITH_OPENSSL -DSOAP_DEBUG -v)
