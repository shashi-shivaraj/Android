# 
#  FILE NAME	: Makefile
#
#  DESCRIPTION  : Make file to build the native libraries
#
#
#  DATE	                NAME	             REASON
#  2nd July,2018		Shashi Shivaraju	 BiteCounter

build :
	ndk-build -C ./Watch/BiteCounterAlgo
	ndk-build -C ./Watch/BiteCounterJNI
	
install :
	cp -R ./Watch/BiteCounterJNI/libs/* ./Watch/BiteCounter/app/src/main/jniLibs
clean:
	ndk-build -C ./Watch/BiteCounterJNI clean
	ndk-build -C ./Watch/BiteCounterAlgo clean
	
