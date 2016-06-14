#!/bin/bash

echo $CXX --version
echo $CC --version

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

	if [[ $OPTION == 'modern' ]]; then
		export ROOT_FILENAME=${ROOT6_FILENAME_MAC}
	else
		export ROOT_FILENAME=${ROOT5_FILENAME_MAC}
	fi
	export CMAKE_FILENAME=${CMAKE_FILENAME_MAC}

	echo "Installing root now"
	wget https://root.cern.ch/download/$ROOT_FILENAME
	tar -xvf $ROOT_FILENAME
	source root/bin/thisroot.sh
	
	echo "Installing cmake now"
	wget ${CMAKE_DOWNLOAD_PATH}/$CMAKE_FILENAME
	tar xfz $CMAKE_FILENAME
	export PATH="`pwd`/${CMAKE_FILENAME%%.*}/CMake.app/Contents/bin":$PATH:	
	echo $PATH
	
	# OS X: update brew cache:
	brew update
	
	if [[ "$CC" == "gcc" ]]; then CC=gcc-4.9; fi
	
else
	if [[ $OPTION == 'modern' ]]; then
		export ROOT_FILENAME=${ROOT6_FILENAME_LINUX}
	else
		export ROOT_FILENAME=${ROOT5_FILENAME_LINUX}
	fi
	
	echo "Installing root now"
	wget https://root.cern.ch/download/$ROOT_FILENAME
	tar -xvf $ROOT_FILENAME
	source root/bin/thisroot.sh
	
	sudo apt-get -qq update
fi
	
