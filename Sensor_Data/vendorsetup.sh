#**************************************************************************************
#
#  FILE NAME	: vendorsetup.sh
#
#  DESCRIPTION  : vendro setup shell script for building libraries
#
#  DATE	                NAME	              REFERENCE	         REASON
#  23rd May,2018		Shashi Shivaraju	  Initial			 AOSP 
#
#*************************************************************************************/
export	CPU=4

makefw()
{
	if [ -f build/core/envsetup.mk -a -f Makefile ]; then
		
			local DIR="vendor/Sensor_Data";
			echo "**************************************************************"
			echo "BUILDING SHARED LIBRARY LIBRARIES"
			echo "**************************************************************"
	
			ONE_SHOT_MAKEFILE="$DIR/Sensors/jni/Android.mk" make -j$CPU libSensors || return 1
			ONE_SHOT_MAKEFILE="$DIR/SensorJNI/jni/Android.mk" make -j$CPU libSensorsJNI || return 1
			
			echo "**************************************************************"
			echo "BUILDING SHARED LIBRARY DONE"
			echo "**************************************************************"
			
			if [ -f out/target/product/generic/system/lib/libSensors.so ]; then
				echo "copied libSensors.so to app/src/main/jniLibs/armeabi-v7a "
				cp out/target/product/generic/system/lib/libSensors.so $DIR/SensorApp/app/src/main/jniLibs/armeabi-v7a/libSensors.so || return 1
			fi
			
			if [ -f out/target/product/generic/system/lib/libSensorsJNI.so ]; then
				echo "copied libSensorsJNI.so to app/src/main/jniLibs/armeabi-v7a"
				cp out/target/product/generic/system/lib/libSensorsJNI.so $DIR/SensorApp/app/src/main/jniLibs/armeabi-v7a/libSensorsJNI.so || return 1
			fi
		
	else
		echo "ERROR: Not at Android root"
	fi

	return 0
}

cleanfw()
{
	if [ -f build/core/envsetup.mk -a -f Makefile ]; then

			local DIR="vendor/Sensor_Data";

			echo "**************************************************************"
			echo "CLEANING SHARED LIBRARY"
			echo "**************************************************************"
			
			ONE_SHOT_MAKEFILE="$DIR/Sensors/jni/Android.mk" make clean-libSensors;
			ONE_SHOT_MAKEFILE="$DIR/SensorJNI/jni/Android.mk" make  clean-libSensorsJNI;
			
			if [ -f $DIR/SensorApp/app/src/main/jniLibs/armeabi-v7a/libSensors.so ]; then
				echo "removed app/src/main/jniLibs/armeabi-v7a/libSensors.so "
				rm $DIR/SensorApp/app/src/main/jniLibs/armeabi-v7a/libSensors.so
			fi
			
			if [ -f $DIR/SensorApp/app/src/main/jniLibs/armeabi-v7a/libSensorsJNI.so ]; then
				echo "removed app/src/main/jniLibs/armeabi-v7a/libSensorsJNI.so "
				rm $DIR/SensorApp/app/src/main/jniLibs/armeabi-v7a/libSensorsJNI.so
			fi
		
			echo "**************************************************************"
			echo "CLEANING SHARED LIBRARY DONE"
			echo "**************************************************************"

	else
		echo "ERROR: Not at Android root"
	fi

	return 0
}

rebuildfw()
{
	cleanfw
	makefw
	
	echo "**************************************************************"
	echo "REBUILD DONE;SHARED LIBRARY READY"
	echo "**************************************************************"
}
