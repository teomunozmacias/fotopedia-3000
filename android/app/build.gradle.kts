import java.util.Properties

plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.compose)
}

// --- Claves y datos de esta maquina -----------------------------------------
// Se leen de android/local.properties, que NO se sube a GitHub (ver .gitignore).
// Asi la clave de la Vision API nunca queda escrita en un fichero del repositorio.
val ficheroLocal = rootProject.file("local.properties")
val propiedadesLocales = Properties().apply {
    if (ficheroLocal.exists()) {
        ficheroLocal.inputStream().use { load(it) }
    }
}

val claveVision = propiedadesLocales.getProperty("VISION_API_KEY")?.trim().orEmpty()
// Si falta la clave, se para la compilacion aqui, con un mensaje que dice que hacer.
// Es mejor que generar un APK que se instala bien y luego no reconoce nada.
if (claveVision.isEmpty()) {
    throw GradleException(
        "Falta VISION_API_KEY en android/local.properties. " +
            "Copia android/local.properties.example como android/local.properties " +
            "y escribe ahi tu clave de la Vision API."
    )
}

// La IP de la placa es solo el valor de arranque: luego se podra cambiar desde la app.
val ipArduino = propiedadesLocales.getProperty("ARDUINO_IP")?.trim().orEmpty()
    .ifEmpty { "192.168.1.50" }

android {
    namespace = "es.fotopedia.app"
    // Tres numeros que se parecen pero no son lo mismo:
    //   compileSdk: contra que version de Android se compila. Las librerias de hoy piden la 37.
    //   targetSdk:  para que version esta probada la app (como se comporta al ejecutarse).
    //   minSdk:     el Android mas viejo donde se puede instalar.
    compileSdk = 37

    defaultConfig {
        applicationId = "es.fotopedia.app"
        minSdk = 26        // Android 8
        targetSdk = 36
        versionCode = 1
        versionName = "0.1"

        // Esto crea BuildConfig.VISION_API_KEY y BuildConfig.ARDUINO_IP para el codigo.
        buildConfigField("String", "VISION_API_KEY", "\"$claveVision\"")
        buildConfigField("String", "ARDUINO_IP", "\"$ipArduino\"")
    }

    buildFeatures {
        compose = true
        buildConfig = true
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
}

dependencies {
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.activity.compose)
    implementation(platform(libs.androidx.compose.bom))
    implementation(libs.androidx.compose.ui)
    implementation(libs.androidx.compose.material3)

    testImplementation(libs.junit)
}
