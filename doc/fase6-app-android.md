# Fase 6 — La app del móvil, ahora en Android nativo

> 🎯 **Objetivo**: rehacer la app del móvil en **Kotlin + Jetpack Compose**, dentro de este
> repositorio, sin tocar el Arduino.

Igual que en la Fase 4, se hace **en dos etapas**:

| Etapa | Qué hace | Para qué |
|---|---|---|
| **1 — App mini** | Escribes una palabra a mano y sale en la LCD | Comprobar que el móvil llega a la placa |
| **2 — App completa** | Cámara + Google Vision | El proyecto de verdad |

La razón es la misma de antes: si montas todo de golpe y no funciona, el fallo puede estar
en cuatro sitios y no hay forma de saber en cuál.

---

## 1. Por qué dejamos MIT App Inventor

App Inventor fue la decisión correcta para empezar. El problema no es que sea malo, es que
**este proyecto se le quedó grande**. Cada una de estas cuatro peleas está documentada en
[`fase4-app-inventor.md`](./fase4-app-inventor.md):

| Problema en App Inventor | Cómo se resuelve en Android nativo |
|---|---|
| No sabe hacer **Base64**: hay que instalar la extensión `KIO4_Base64`, que devuelve Base64 *Web Safe* (`-`, `_`, `*`) y hay que traducir a `+`, `/`, `=` carácter a carácter | `android.util.Base64.encodeToString(bytes, Base64.NO_WRAP)` — ya viene en el sistema y da Base64 estándar |
| **Reducir la foto** exige el truco de pintarla en un `Canvas` invisible y guardarla | `BitmapFactory` con `inSampleSize` y `Bitmap.scale()`: redimensionado de verdad |
| La cámara devuelve una ruta con **`file://`** delante que el lector de ficheros no entiende, y hay que quitárselo a mano | La cámara devuelve un `Uri` y se lee con `contentResolver.openInputStream(uri)` |
| **El proyecto no es un fichero**: vive en la web de MIT, sin `git`, sin historial, sin poder revisar un cambio, y no cabe en este repositorio | Código Kotlin en `android/`, versionado como todo lo demás — y editable con Claude Code |

## 2. Qué hay que instalar

No hace falta Android Studio. Con estas tres cosas se compila y se instala desde VS Code:

| Qué | Versión | Cómo |
|---|---|---|
| **JDK** | 17 (Temurin) | `winget install EclipseAdoptium.Temurin.17.JDK` |
| **Android SDK command-line tools** | la última | Descargar de [developer.android.com/studio#command-line-tools-only](https://developer.android.com/studio#command-line-tools-only) y descomprimir en `%LOCALAPPDATA%\Android\Sdk\cmdline-tools\latest` |
| **Plataforma + build-tools + adb** | API 36 y 37 | `sdkmanager "platform-tools" "platforms;android-36" "platforms;android-37.0" "build-tools;36.0.0"` |

Las licencias del SDK se aceptan una vez con `sdkmanager --licenses`.

> 📌 **Versiones comprobadas en noviembre de 2026**: JDK 17.0.21, command-line tools
> 15859902, platform-tools 37.0.1. `sdkmanager` sigue funcionando, pero ya avisa de que
> está obsoleto y de que su sustituto es `android sdk`.

> 💡 Este fue un error real que tuvimos: el plan decía API 35, instalamos la 36 por ser la
> estable… y la primera compilación falló con once avisos iguales: *"requires libraries and
> applications that depend on it to compile against version 37 or later"*. Las librerías
> actuales (Compose, `core-ktx`) exigen **compilar** contra la 37. Se arregló instalando
> `platforms;android-37.0` y poniendo `compileSdk = 37`, sin tocar `targetSdk`.

Variables de entorno (una vez, en el usuario de Windows):

```
ANDROID_HOME = %LOCALAPPDATA%\Android\Sdk
JAVA_HOME    = C:\Program Files\Eclipse Adoptium\jdk-17...
PATH         += %ANDROID_HOME%\platform-tools
```

Extensiones recomendadas de VS Code: **Kotlin** (syntax + language server) y **Gradle for
Java**.

### Comprobar que está todo

```bash
java -version          # 17.x
adb --version          # Android Debug Bridge
sdkmanager --list      # sin errores
```

Y con el móvil enchufado por USB y la **depuración USB** activada (Ajustes → Opciones de
desarrollador):

```bash
adb devices            # tiene que aparecer tu móvil como "device", no "unauthorized"
```

> 💡 La primera vez, el móvil enseña un diálogo de "¿Permitir depuración USB?". Si dice
> `unauthorized`, es que ese diálogo está esperando en la pantalla del móvil.

## 3. Cómo queda el proyecto

```
android/
  settings.gradle.kts        ← qué módulos hay (solo :app)
  build.gradle.kts           ← plugins y versiones comunes
  gradle.properties          ← ajustes de Gradle (memoria, AndroidX)
  gradle/libs.versions.toml  ← versión de cada librería, en un solo sitio
  gradle/wrapper/            ← qué versión de Gradle usa el proyecto
  gradlew / gradlew.bat      ← el "wrapper": trae el Gradle correcto solo
  local.properties           ← 🔒 rutas y claves de esta máquina — NO se sube
  local.properties.example   ← plantilla del anterior, con valores de ejemplo — sí se sube
  app/
    build.gradle.kts         ← config de la app: minSdk, dependencias, BuildConfig
    src/main/
      AndroidManifest.xml    ← permisos (INTERNET, CAMERA) y config de red
      res/values/strings.xml ← el nombre de la app
      res/xml/network_security_config.xml   ← ⚠️ ver apartado 5
      java/es/fotopedia/app/
        MainActivity.kt      ← la pantalla
        ArduinoClient.kt     ← GET http://<ip>/?texto=...      (pendiente)
        VisionClient.kt      ← POST a Google Vision            (pendiente)
        ImagenUtils.kt       ← reducir a 640 px + Base64       (pendiente)
```

Versiones con las que compila hoy (están en `gradle/libs.versions.toml`):

| Pieza | Versión |
|---|---|
| Gradle (wrapper) | 9.9.1 |
| Plugin de Android (AGP) | 9.5.1 |
| Kotlin / plugin de Compose | 2.4.21 |
| Compose BOM | 2026.09.00 |
| `compileSdk` / `targetSdk` / `minSdk` | 37 / 36 / 26 |

> 📌 Desde la versión 9 del plugin de Android, **Kotlin ya viene dentro**: por eso en
> `build.gradle.kts` no aparece el plugin `kotlin-android` que sale en casi todos los
> tutoriales. El de Compose sí hay que pedirlo aparte.

Órdenes del día a día:

```bash
cd android
./gradlew assembleDebug                                   # compilar
adb install -r app/build/outputs/apk/debug/app-debug.apk   # instalar
adb logcat -s Fotopedia                                    # ver los mensajes de la app
./gradlew test                                             # tests unitarios
```

## 4. La clave de API y la IP de la placa

La clave de la Vision API **no se escribe en el código**. Va en `local.properties`, que
está en el `.gitignore` y por tanto nunca sale de tu ordenador:

```properties
# android/local.properties  — NO se sube a GitHub
sdk.dir=C\:\\Users\\tu-usuario\\AppData\\Local\\Android\\Sdk
VISION_API_KEY=AIzaSy...
ARDUINO_IP=192.168.1.50
```

Gradle la lee al compilar y la mete en `BuildConfig`:

```kotlin
// app/build.gradle.kts
val props = Properties().apply {
    rootProject.file("local.properties").inputStream().use { load(it) }
}
buildConfigField("String", "VISION_API_KEY", "\"${props.getProperty("VISION_API_KEY")}\"")
```

Y desde el código se usa como `BuildConfig.VISION_API_KEY`.

Para empezar en un ordenador nuevo: copiar `android/local.properties.example` como
`android/local.properties` y rellenarlo. Si falta la clave, **la compilación se para** y lo
dice (`Falta VISION_API_KEY en android/local.properties…`), en vez de generar un APK que se
instala bien y luego no reconoce nada.

La clave sale de la [consola de Google Cloud](https://console.cloud.google.com) →
*APIs y servicios* → *Credenciales*. Al crearla hay que **restringirla a Cloud Vision API**
(*Restricciones de API*), como ya se hizo en la Fase 3.

> 🔒 Esto **no** hace la clave secreta para quien tenga el APK (se puede sacar de dentro).
> Lo que evita es publicarla en un repositorio público, que es el error de verdad: una clave
> en GitHub la encuentran los robots en minutos. Por eso además la clave está **restringida
> a Cloud Vision API** en la consola de Google.

La **IP del Arduino** también empieza en `local.properties` como valor por defecto, pero
tiene que poder cambiarse sin recompilar (el router se la cambia al reiniciarse): se guarda
en el móvil con DataStore y se edita desde la app.

## 5. ⚠️ La trampa de esta fase: cleartext HTTP

**Esto es el `file://` de la Fase 6.** Si no lo sabes, te cuesta una tarde.

Desde **Android 9 (API 28)**, el sistema **bloquea las peticiones `http://` sin cifrar** por
defecto. Nuestra petición al Arduino es exactamente eso: `http://192.168.1.50/?texto=Cat`.

Y lo peor: **no falla de forma clara**. La petición simplemente no sale, y lo que ves es una
excepción genérica de red, como si la placa no estuviera. Te pones a mirar el WiFi, la IP y
el sketch… y el problema está en el móvil.

La solución es declarar que **sí** permitimos HTTP sin cifrar:

```xml
<!-- app/src/main/res/xml/network_security_config.xml -->
<network-security-config>
    <base-config cleartextTrafficPermitted="true" />
</network-security-config>
```

Y engancharlo en el manifiesto:

```xml
<application android:networkSecurityConfig="@xml/network_security_config" ... >
```

> 💡 El Arduino no puede hablar `https://`: cifrar es demasiado para esa placa. Por eso la
> excepción es correcta aquí.

> ⚠️ **El plan original no habría funcionado.** La primera idea era permitirlo *solo a la
> red de casa*, escribiendo `<domain includeSubdomains="true">192.168.1.0</domain>`. Pero
> ese fichero **no entiende rangos de direcciones**: eso autoriza la dirección
> `192.168.1.0` exacta y ninguna más, así que la petición a `192.168.1.50` habría seguido
> bloqueada. Y como la IP de la placa la reparte el router y cambia, tampoco se puede
> escribir fija. Por eso se permite en general.
>
> No se pierde seguridad con Google: su dirección empieza por `https://` y se sigue
> cifrando igual. El permiso solo afecta a direcciones `http://`, y la única que usa la app
> es la de la placa.

## 6. El flujo por dentro

Los mismos siete pasos de la app anterior, con la herramienta de cada uno:

| Paso | Qué se usa | Permiso que necesita |
|---|---|---|
| 1. Sacar la foto | CameraX (`ImageCapture`) | `CAMERA` (hay que pedirlo en marcha, no basta el manifiesto) |
| 2. Reducir a 640 px | `BitmapFactory` + `Bitmap.scale()` | — |
| 3. Codificar a Base64 | `android.util.Base64` con `NO_WRAP` | — |
| 4. Llamar a Vision | OkHttp: `POST https://vision.googleapis.com/v1/images:annotate?key=…` con `OBJECT_LOCALIZATION` | `INTERNET` |
| 5. Sacar el nombre | `responses[0].localizedObjectAnnotations[0].name` | — |
| 6. Enviar al Arduino | OkHttp: `GET http://<ip>/?texto=<nombre>` | `INTERNET` + cleartext (apartado 5) |
| 7. Enseñar el resultado | Compose: estado `Cargando` / `Resultado` / `Error` | — |

Dos avisos:

- **`NO_WRAP` no es opcional.** Sin él, `Base64` mete saltos de línea cada 76 caracteres y
  el JSON que se manda a Google sale roto.
- **El texto hay que codificarlo para la URL** antes de metérselo al Arduino: un objeto como
  `Coffee cup` tiene un espacio, y `Teddy bear` también. `URLEncoder.encode(texto, "UTF-8")`.
  El sketch ya sabe deshacer `+` y `%20`.

## 7. Checklist de integración de la Fase 6

Calcado del de la [Fase 5](./fase5-integracion-checklist.md), para poder comparar la app
nueva con la vieja.

### 6.1 — Entorno

- [x] `java -version` dice 17
- [ ] `adb devices` ve el móvil como `device`
- [x] `./gradlew assembleDebug` compila sin errores
- [x] Sin `VISION_API_KEY`, la compilación se para y dice qué falta
- [ ] El APK se instala y la app abre sin cerrarse

### 6.2 — Etapa 1: el móvil habla con la placa

- [ ] El Arduino está encendido y muestra su IP en la LCD
- [ ] Escribo `Prueba` en la app y le doy al botón
- [ ] `Prueba` sale en la línea de abajo de la LCD
- [ ] La app enseña la respuesta `OK` de la placa
- [ ] Con una palabra con espacios (`Coffee cup`) llega **entera**, no solo la primera parte
- [ ] Con una palabra con tildes (`Café`) llega como `Cafe`
- [ ] Con la IP mal puesta, la app dice qué ha pasado (no se cierra ni se queda colgada)

### 6.3 — Vision API (sin Arduino)

- [ ] La app pide permiso de cámara la primera vez
- [ ] Se saca una foto y se ve la previsualización
- [ ] La foto reducida pesa bastante menos (se ve en el log)
- [ ] Google devuelve un objeto y la app lo enseña en pantalla
- [ ] Con una foto sin objeto claro (una pared), la app dice "no he reconocido nada" en vez de fallar

### 6.4 — Flujo completo

- [ ] Foto → objeto en la LCD, sin tocar nada más
- [ ] Probado con cinco objetos distintos
- [ ] El tiempo total sigue siendo de 3–8 segundos
- [ ] Un nombre largo se corta a 16 caracteres en la pantalla

### 6.5 — Robustez

- [ ] Sin WiFi: la app avisa
- [ ] Con la clave de API mal: la app distingue el error 403 y lo dice
- [ ] Girar el móvil no pierde el resultado que había en pantalla
- [ ] Dos fotos seguidas rápidas no dejan la app en un estado raro

### 6.6 — Cierre

- [ ] `README.md` marca la Fase 6 como completada
- [ ] APK de release generado y instalado en el móvil de casa
- [ ] Vídeo corto del aparato funcionando

---

## Enlaces

- [Issues de la Fase 6](https://github.com/teomunozmacias/fotopedia-3000/issues?q=is%3Aissue+label%3Afase-6)
- [Documentación de Jetpack Compose](https://developer.android.com/develop/ui/compose/documentation)
- [CameraX](https://developer.android.com/media/camera/camerax)
- [Configuración de seguridad de red (cleartext)](https://developer.android.com/privacy-and-security/security-config)
- [Google Cloud Vision — Object localization](https://cloud.google.com/vision/docs/object-localizer)
- [`fase4-app-inventor.md`](./fase4-app-inventor.md) — la app anterior, archivada
