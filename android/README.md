## Android

The Orb project TvBrowser has two Android reference integrations:

### A. The mock reference (tvbrowsershell)

In the mock reference the tvbrowser library is integrated with the tvbrowsershell application. The implementation mocks any calls that would normally use other components such as a broadcast stack.

#### Building

Open this repo as your project in Android studio. The mock reference can be built as a regular app in Android studio and run on an emulator or device.

### B. The DTVKit reference

In the DTVKit reference the tvbrowser library is integrated with the DTVKit Tv Input Source. The implementation calls the DTVKit broadcast stack which itself is integrated with the Android Tuner framework.

#### Building

The DTVKit reference must be built as a system app in the Android source tree.

#### 1. Add the DTVKit Tv Input Source and its dependencies to the source tree:

```
cd <Android 11 source tree>

git clone git@github.com:OceanBlueSoftware/Android_11_Reference.git external/dtvkit
git -C external/dtvkit checkout orb

git clone git@github.com:OceanBlueSoftware/DVBCore \
external/dtvkit/androidtunerlibrary/src/main/libs/dvbcore/DVBCore
git -C external/dtvkit/androidtunerlibrary/src/main/libs/dvbcore/DVBCore checkout orb

git clone git@github.com:OceanBlueSoftware/DSMCC \
external/dtvkit/androidtunerlibrary/src/main/libs/dsmcc/DSMCC
git -C external/dtvkit/androidtunerlibrary/src/main/libs/dsmcc/DSMCC orb

git clone git@github.com:OceanBlueSoftware/TvBrowser \
external/dtvkit/TvBrowser
git -C external/dtvkit/TvBrowser checkout master
```

#### 2. Add the prebuilt Chromium apks to the source tree. For arm:

```
wget -c http://192.168.10.200/android-bcm/TrichromeLibrary.apk \
external/dtvkit/chromium/prebuilt/arm/
wget -c http://192.168.10.200/android-bcm/TrichromeWebView.apk \
external/dtvkit/chromium/prebuilt/arm/
```

#### 3. Add jsoncpp_ORB to the Android source tree

```
git clone https://android.googlesource.com/platform/external/jsoncpp external/jsoncpp_ORB -b android-t-preview-1
git -C external/jsoncpp_ORB checkout ba7e6c39895f4a9468a93b26b5e3c730f1d43629
patch external/jsoncpp_ORB/Android.bp external/dtvkit/TvBrowser/external/jsoncpp/1.9.5/build.patch
mmma external/jsoncpp_ORB -j8
```

#### 4. Add libwebsockets to the Android source tree

```
git clone https://android.googlesource.com/platform/external/libwebsockets external/libwebsockets -b android-t-preview-1
git -C external/libwebsockets checkout ba7e6c39895f4a9468a93b26b5e3c730f1d43629
patch external/libwebsockets/Android.bp dtvkit/TvBrowser/external/libwebsockets/v4.3/build.patch
mmma external/libwebsockets -j8
```

#### 5. Add the packages to your product configuration (e.g. device/broadcom/cypress/cypress.mk):

```
PRODUCT_PACKAGES += org.dtvkit.inputsource
PRODUCT_PACKAGES += jsoncpp_ORB
PRODUCT_PACKAGES += libwebsockets
```

#### 6. Build the Android source tree

#### 7. (Optional) To rebuild just this project (e.g. for development):

```
Before executing these steps, set up your environment for building and make the system partition of your device writeable.

mmm external/dtvkit/tis/src/main -j8
adb push $ANDROID_PRODUCT_OUT/system/lib/liborg.dtvkit.tunerlibrary-jni.so /system/lib/
adb push $ANDROID_PRODUCT_OUT/system/lib/liborg.orbtv.tvbrowser.applicationmanager-jni.so /system/lib/
adb push $ANDROID_PRODUCT_OUT/system/app/org.dtvkit.inputsource/ /system/app/
```
