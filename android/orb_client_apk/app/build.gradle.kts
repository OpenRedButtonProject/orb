import com.android.build.gradle.internal.tasks.factory.dependsOn

plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace = "org.orbtv.orbclient"
    compileSdk = 34

    defaultConfig {
        applicationId = "org.orbtv.orbclient"
        minSdk = 30
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }

    buildFeatures {
        aidl = true
    }
    project.tasks.preBuild.dependsOn("CopyAidl")
}

tasks.register("CopyAidl")
{
    val pkgPath = "org/chromium/content_shell_apk"
    val destPath = "app/src/main/aidl/$pkgPath"
    val destDir = File(destPath)
    val pathList = destPath.split("/")
    val paths = StringBuilder()
    for (folder in pathList) {
        paths.append(folder)
        val fldr = File(paths.toString())
        if (!fldr.exists()) {
            fldr.mkdir()
        }
        paths.append("/")
    }
    val srcDir = File("../shell_apk/src/$pkgPath")
    val aidls = listOf("IOrbClient.aidl", "IOrbService.aidl")
    for( aidlname in aidls) {
        File(srcDir, aidlname).copyTo(File(destDir, aidlname), true)
    }
}

dependencies {
    // This is temporary just for this test
    implementation(files("../../../../../out/Default/lib.java/content/shell/android/content_shell_shared_javalib.jar"))
}
