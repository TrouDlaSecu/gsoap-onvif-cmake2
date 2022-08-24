#!/bin/bash
#-*-ENCODING: UTF-8-*-
# $2 is the name of the downloaded file
# $1 is the URL to download the file i want to download
# the command used to dl with replace the file if the name is already taken

echo "Using cURL to download Snapshot ...."

echo "Placing in Snapshot directory.."
cd Snapshot

if [ -e $2 ]
then
	echo "Namefile already taken..."
	exit
	
else
	echo "Downloading Snapshot"
	curl -o $2 $1
	#check if downloaded succeed
	if [ -e $2 ]
	then
		echo "Downloaded with success"
		exit
	else
		echo "Error during the downloading"
		exit
	fi
	
fi





