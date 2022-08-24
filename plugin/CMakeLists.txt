#Initialisation du projet
#cmake_minimum_required(VERSION 3.10)

#set(CMAKE_BUILD_TYPE Release)


add_library(plugin 
	dom.cpp  
	duration.cpp  
	mecevp.cpp  
	smdevp.cpp  
	threads.cpp  
	wsaapi.cpp  
	wsseapi.cpp
	)

target_include_directories(plugin PUBLIC 
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                          "${PROJECT_SOURCE_DIR}/../"
)

target_compile_options(plugin PUBLIC -Wall -g -w -fPIC -DWITH_NONAMESPACES -fno-use-cxa-atexit -fexceptions -DWITH_DOM  -DWITH_OPENSSL -DSOAP_DEBUG -v)
