##Project Introduction

  	The project is a onvif client project,through the project,you can learn 
 	how to get the IPC(ip camera)'s RTSP URL, video configuration, WSDL address, 
	and the Device capabilities(such as PTZ).

##Tips:
	The compile platform is Debian, ubuntu, i didn't compile it int the Redhat or Centos
	if you have any question, you can contact me,before you Make the Makefile.
	you must make sure you have install the g++, openssl

##usage:

	1:cd gsoap-onvif

	2: make 

if you compile sucess, you can use the ipconvif like this

./ipconvif  172.18.4.100  (your iPC IP)

##ONVIF official website

http://www.onvif.org/

if you want to learn more onvif API 

http://www.onvif.org/onvif/ver20/util/operationIndex.html

##The Gsoap Toolkit
http://www.cs.fsu.edu/~engelen/soap.html

##download
http://sourceforge.net/projects/gsoap2/files/

for example,you get the RTSP URL, you can paly  the video use the player,such as VLC player.

##VLC offical website
http://www.videolan.org/

you can save the video use the ffmpeg tool,like this

	ffmpeg -i rtsp://admin:admin@172.18.4.100:554 -b 300 -s 320x240 -vcodec copy  -ab 32 -ar 24000 -acodec aac -strict experimental -f mp4 test.mp4


if you want learn more, you can visit VLC official website
##ffmpeg official website
https://www.ffmpeg.org/





