# CLAUDE.md

Instrucciones para trabajar en este repositorio.

## Qué es este proyecto

**Fotopedia 3000**: le haces una foto a un objeto con el móvil, una IA lo reconoce y el
nombre sale en una pantalla LCD conectada a un Arduino.

Es un **proyecto de aprendizaje padre-hijo**, y eso condiciona todo lo demás: importa
tanto que funcione como que se entienda por qué funciona. Cuando haya que elegir entre lo
listo y lo explicable, gana lo explicable.

## Regla principal: el README manda

**Todo cambio en el código debe reflejarse en [`README.md`](./README.md) en el mismo
commit.** El README es la única fuente de verdad del proyecto: es lo que se lee para
entender qué hay montado y en qué punto está.

Antes de dar por terminado un cambio, comprueba:

- [ ] Si cambia el **comportamiento** (pines, endpoint, formato de la petición, valores por
      defecto), está actualizado en el apartado de su fase.
- [ ] Si se **añade, mueve o renombra** un fichero, están actualizados el índice de "Cómo
      está organizado este repositorio" y los enlaces relativos que lo apuntan.
- [ ] Si se **completa** una tarea o una fase, están marcadas las casillas del "Plan de
      trabajo" y el `(completada)` / `(en desarrollo)` del título de la sección.
- [ ] Si se ha tropezado con un **error real** y ha costado tiempo, está anotado como aviso
      (ver "Documentar los errores reales" más abajo).
- [ ] Los enlaces relativos siguen resolviendo:
      `grep -oh '](\./[^)]*)' README.md doc/*.md`

Los detalles largos van a `doc/`; en el README va lo que hay que saber y el enlace.

## Estructura

```
README.md      La portada: el proyecto entero, fase por fase
CLAUDE.md      Esto
doc/           Documentacion: guias de fase y la pagina de prueba de Vision
arduino/       Sketches, cada uno en su carpeta con el mismo nombre (lo exige el IDE)
android/       App del movil, Kotlin + Jetpack Compose (Fase 6, aun no creada)
```

- `doc/` en kebab-case (`fase6-app-android.md`).
- `arduino/` en snake_case: el fichero `.ino` **tiene que llamarse igual que su carpeta**.

## Idioma y tono

- **Todo en castellano**: documentación, comentarios del código y mensajes de commit.
- Los **identificadores del código** (variables, funciones, clases) también en castellano:
  `quitarAcentos()`, `hayActualizacionPendiente`, `ArduinoClient.enviar()`.
- **Sin tildes ni ñ** en los mensajes de commit y en los comentarios de los sketches
  (`Ensena su direccion IP`, `Anadir pagina de prueba`): es la convención del repositorio.
  En el Markdown de `doc/` y del README **sí** se acentúa con normalidad.
- Explicar **por qué**, no solo qué. Las tablas comparativas y los diagramas ASCII son el
  formato habitual aquí; úsalos.
- Los avisos van con emoji, como en el resto del repo: ⚠️ trampa, 💡 detalle curioso,
  📌 nota, 🔒 seguridad, 📦 archivado, 🎯 objetivo.

### Documentar los errores reales

Cuando algo falla y cuesta averiguar por qué, **eso se escribe**, con esta forma:

> 💡 Este fue un error real que tuvimos: el programa preguntaba la IP justo después del
> paso 1, cuando el router todavía no la había dado, y la pantalla mostraba `0.0.0.0`.

Ya hay tres así (el `0.0.0.0` del DHCP, la tilde que ocupa dos bytes, el `file://` de la
cámara) y son de lo más útil que tiene el repositorio. No los borres al reorganizar.

## Nunca subir secretos

Este repositorio es **público**.

| Secreto | Dónde va |
|---|---|
| Clave del WiFi de casa | En el sketch **solo en local**; se commitea con los valores de ejemplo (`TU_RED_WIFI`) |
| Clave de la Vision API | `android/local.properties` (ignorado) → `BuildConfig`. Nunca en el código |
| Claves de firma (`.keystore`) | Ignoradas |

La página `doc/fase3-google-vision-test.html` pide la clave cada vez a propósito: por eso se
puede publicar. No añadas un valor por defecto.

Antes de commitear un `.ino` o algo de `android/`, mira que no llevas dentro la clave real.

## El Arduino

Sketch definitivo: `arduino/fase4_arduino_http_server/`. Uno R4 WiFi, LCD 1602A sin I2C.

Cosas que están así por una razón y no hay que "arreglar":

- **`lcd.begin(16, 2)` en vez de `lcd.clear()`** para refrescar: `begin()` reenvía la
  configuración interna de la pantalla, y eso evita los caracteres corruptos que provoca la
  interferencia del WiFi sobre los cables de datos.
- **La LCD no se escribe desde dentro del callback**: se activa una bandera y se escribe en
  el `loop()`, para no chocar con la actividad del WiFi.
- **`quitarAcentos()` recorre los bytes de dos en dos**: en UTF-8 una letra con tilde ocupa
  dos bytes (`0xC3` + otro), así que comparar `letra == 'á'` no funciona nunca.
- **Se espera a que DHCP dé una IP** antes de enseñarla; si no, sale `0.0.0.0`.
- El texto se **recorta a 16 caracteres y se le quitan las tildes en la placa**, no en el
  móvil. No dupliques eso en la app.

Estos sketches se compilan con el IDE de Arduino, a mano y con la placa delante: no hay
build automatizado ni forma de probarlos desde aquí. No inventes que se han probado.

## La app Android (Fase 6)

Kotlin + Jetpack Compose + CameraX + OkHttp, Gradle Kotlin DSL, `minSdk 26`. Se trabaja
desde **VS Code**, sin Android Studio:

```bash
cd android
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb logcat -s Fotopedia
./gradlew test
```

Tres cosas que fallan en silencio si se olvidan:

1. **Cleartext HTTP**: Android 9+ bloquea `http://` y el error parece "la placa no responde".
   Hace falta `network_security_config.xml` limitado a la red local.
2. **`Base64.NO_WRAP`**: sin él se cuelan saltos de línea cada 76 caracteres y el JSON que
   va a Google sale roto.
3. **`URLEncoder.encode(texto, "UTF-8")`**: sin él, `Coffee cup` llega a la placa como
   `Coffee`.

La guía completa, con el entorno y las trampas, está en
[`doc/fase6-app-android.md`](./doc/fase6-app-android.md).

## Google Vision

Se usa **`OBJECT_LOCALIZATION`**, no `LABEL_DETECTION`, y está comprobado con fotos reales:
`LABEL_DETECTION` describe la imagen entera y devuelve `Textile` o `Material property` antes
que `Teddy bear`. El dato bueno es
`responses[0].localizedObjectAnnotations[0].name`, y la lista **puede venir vacía** (paredes,
paisajes) — hay que comprobarlo siempre antes de leerla.

Las etiquetas vienen **en inglés** (`Cat`, `Chair`) y se dejan así a propósito.

## Commits e issues

**Commits**: en castellano, verbo en infinitivo, sin tildes en el asunto, y diciendo qué
cambia de cara al proyecto (no qué fichero se ha tocado):

```
Arreglar IP 0.0.0.0: esperar a que DHCP asigne direccion
Marcar la etapa 1 de la app como superada
Concretar la extension Base64: KIO4_Base64 y su codificacion Web Safe
```

Se trabaja directamente sobre `main`. **No commitees ni subas nada sin que se te pida.**

**Issues**: un issue por tarea, titulado `Fase N.M: qué hace`, con este cuerpo:

```markdown
## Objetivo
Una frase.

## Tareas
- [ ] ...

## Depende de
#N (por qué).
```

Etiquetas: `fase-6`, `android`, `arduino`, `doc`, `setup`.

## No inventes que algo está probado

Buena parte de este proyecto solo se puede verificar con el hardware delante: la LCD, el
WiFi de casa, el móvil, la cuota de Google. Si no se ha probado, se dice. Los checklists de
`doc/` distinguen a propósito entre `[ ]` y `[x] SUPERADA`, y esa distinción tiene valor
justamente porque es honesta.
