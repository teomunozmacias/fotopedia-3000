// Plugins comunes a todo el proyecto. Aqui solo se declaran ("apply false");
// quien los usa de verdad es app/build.gradle.kts.
//
// Desde la version 9 del plugin de Android, Kotlin ya viene incluido: por eso no
// aparece un plugin "kotlin-android". El de Compose si hay que pedirlo aparte.

plugins {
    alias(libs.plugins.android.application) apply false
    alias(libs.plugins.kotlin.compose) apply false
}
