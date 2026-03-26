plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
    id("org.jetbrains.kotlin.plugin.compose")
}

val releaseKeystorePath = System.getenv("FORGE_ANDROID_RELEASE_KEYSTORE")
    ?: file("${rootDir}/../../.local/android-signing/forge-test.keystore").absolutePath
val releaseAlias = System.getenv("FORGE_ANDROID_RELEASE_USER") ?: "forge-test"
val releasePassword = System.getenv("FORGE_ANDROID_RELEASE_PASSWORD") ?: "android"

android {
    namespace = "io.crowdware.bench"
    compileSdk = 35

    defaultConfig {
        applicationId = "io.crowdware.bench"
        minSdk = 24
        targetSdk = 35
        versionCode = 1
        versionName = "0.1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            val releaseCfg = signingConfigs.findByName("release")
            if (releaseCfg != null && releaseCfg.storeFile != null && releaseCfg.storeFile!!.exists()) {
                signingConfig = releaseCfg
            } else {
                signingConfig = signingConfigs.getByName("debug")
            }
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    kotlinOptions {
        jvmTarget = "17"
    }

    buildFeatures {
        compose = true
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
        }
    }

    signingConfigs {
        create("release") {
            val ks = file(releaseKeystorePath)
            if (ks.exists()) {
                storeFile = ks
                storePassword = releasePassword
                keyAlias = releaseAlias
                keyPassword = releasePassword
            }
        }
    }

    packaging {
        resources {
            excludes += "/META-INF/{AL2.0,LGPL2.1}"
        }
    }
}

dependencies {
    implementation("androidx.core:core-ktx:1.13.1")
    implementation("androidx.lifecycle:lifecycle-runtime-ktx:2.8.6")
    implementation("androidx.activity:activity-compose:1.9.2")
    implementation("androidx.compose.ui:ui:1.7.4")
    implementation("androidx.compose.ui:ui-tooling-preview:1.7.4")
    implementation("androidx.compose.material3:material3:1.3.0")

    debugImplementation("androidx.compose.ui:ui-tooling:1.7.4")
    debugImplementation("androidx.compose.ui:ui-test-manifest:1.7.4")

    testImplementation("junit:junit:4.13.2")
}
