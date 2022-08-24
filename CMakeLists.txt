#Initialisation du projet
cmake_minimum_required(VERSION 3.10)

# set the project name
project(onvifdemo 
	VERSION 1.0
	LANGUAGES C CXX
	)

set(CMAKE_BUILD_TYPE Release)

add_executable(onvifdemo 
	stdsoap2.cpp
	soapC.cpp
	soapClient.cpp
	main.cpp
	)

find_package(OpenSSL REQUIRED)

add_subdirectory(plugin)
add_subdirectory(proxycpp)

target_include_directories(onvifdemo PUBLIC 
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}"
                          "${PROJECT_SOURCE_DIR}/include"
                          "${PROJECT_SOURCE_DIR}/plugin"
                          "${PROJECT_SOURCE_DIR}/proxycpp"
)

target_link_libraries (onvifdemo plugin proxycpp OpenSSL::SSL)
target_compile_options(onvifdemo PUBLIC -Wall -g -w -fPIC -DWITH_NONAMESPACES -fno-use-cxa-atexit -fexceptions -DWITH_DOM  -DWITH_OPENSSL -DSOAP_DEBUG -v)
