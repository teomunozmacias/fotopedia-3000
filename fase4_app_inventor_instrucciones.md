# Fase 4: App en MIT App Inventor + Servidor HTTP

## Paso 1: Crear proyecto en MIT App Inventor

1. Ve a [MIT App Inventor](https://appinventor.mit.edu)
2. Haz login (con tu cuenta de Google)
3. Crea un nuevo proyecto: "Fotopedia 3000"

---

## Paso 2: Interfaz de usuario (Designer)

En la pestaña **Designer**, construye esto:

### Componentes necesarios

| Componente | Nombre | Propiedad |
|---|---|---|
| **Vertical Arrangement** | VirtualScreen | (contenedor principal) |
| **Button** | BtnTomarFoto | Text: "📷 Tomar foto" |
| **Image** | ImgFoto | (preview de la foto) |
| **Label** | LblResultado | Text: "Esperando..." |
| **Label** | LblIP | Text: "Introduce IP Arduino:" |
| **TextBox** | TxtIPArduino | Text: "192.168.1.X" (placeholder) |
| **Button** | BtnEnviar | Text: "Enviar a Arduino" |
| **Label** | LblEstado | Text: "" (para mostrar estado) |

### Layout sugerido

```
┌─────────────────────────┐
│  FOTOPEDIA 3000         │
├─────────────────────────┤
│  [📷 Tomar foto]        │
│  ┌─────────────────┐    │
│  │                 │    │
│  │   (imagen)      │    │
│  │                 │    │
│  └─────────────────┘    │
│  Detectado: "..."       │
│  IP Arduino: [     ]    │
│  [Enviar a Arduino]     │
│  Estado: "..."          │
└─────────────────────────┘
```

---

## Paso 3: Lógica (Blocks)

En la pestaña **Blocks**, programa los eventos:

### 3.1 – Evento: Tomar foto (BtnTomarFoto.Click)

```
when BtnTomarFoto.Click
   call Camera1.TakePicture
```

Este bloque ya viene con el componente Camera1 en App Inventor.

### 3.2 – Evento: Foto capturada (Camera1.AfterPicture)

```
when Camera1.AfterPicture image (ruta de la foto)
   call ResizeImage( image, 640 )
   set fotoActual to result
   call ConvertirABase64( fotoActual )
   set base64Foto to result
   set ImgFoto.Source to fotoActual
   set LblResultado.Text to "Foto capturada, enviando a Vision..."
   call Vision_API( base64Foto )
```

**Nota**: Necesitas la extensión `Base64` (ver paso 4.1)

### 3.3 – Evento: Respuesta de Google Vision

Usamos `OBJECT_LOCALIZATION`, así que el nombre del objeto está en
`responses[0].localizedObjectAnnotations[0].name` (ver el README para saber por qué).

```
when Web1.GotText responseContent
   set global respuesta to call JsonTextDecodeWithDictionaries responseContent
   set global objetos to get value for key "localizedObjectAnnotations"
                          in dictionary (first item of (get value for key "responses"))
                          or if not found create empty list

   if is list empty? (get global objetos)
      set LblResultado.Text to "No he reconocido nada"
      set LblEstado.Text to "Prueba a acercarte mas"
   else
      set global etiquetaDetectada to
            get value for key "name" in dictionary (select list item list (get global objetos) index 1)
      set LblResultado.Text to get global etiquetaDetectada
      set LblEstado.Text to join "Detectado: " (get global etiquetaDetectada)
```

**Importante**: comprueba **siempre** si la lista está vacía antes de leer el primer
elemento. Si Vision no reconoce nada (una pared, el cielo) y lo lees sin comprobar,
la app se cierra con un error.

### 3.4 – Evento: Enviar a Arduino (BtnEnviar.Click)

```
when BtnEnviar.Click
   set ip to TxtIPArduino.Text
   set texto to etiquetaDetectada
   set url to join "http://" ip "/?texto=" (call Web1.UriEncode texto)
   call Web2.Get url
   set LblEstado.Text to "Enviando a Arduino..."
```

### 3.5 – Evento: Confirmación desde Arduino

```
when Web2.GotText response
   if response contains "OK"
      set LblEstado.Text to "✓ Enviado a Arduino"
   else
      set LblEstado.Text to join "✗ Error: " response
```

---

## Paso 4: Configuración de componentes

### 4.1 – Extensión Base64 (imprescindible)

Google Vision no acepta la foto tal cual: hay que mandarla convertida a **Base64**, que
es una forma de escribir un archivo usando solo letras y números.

**MIT App Inventor no sabe hacer Base64 de serie.** Hay que instalar una extensión:

1. En la pestaña **Designer**, panel de la izquierda del todo, abajo: **Extension**
2. Pulsa **Import extension** → **From my computer** (o desde URL)
3. Descarga una extensión de ficheros/Base64 de la comunidad de App Inventor
   (busca *"App Inventor Base64 extension"* o *"File extension TaifunFile"*, que incluye
   `ReadFrom` en Base64)
4. Arrástrala a la pantalla; aparecerá como componente no visible

Eso te da un bloque del tipo `call Base64.encodeFile path`.

> ⚠️ Las extensiones son ficheros `.aix` hechos por gente de la comunidad, no por el MIT.
> Descárgalas solo de la web oficial de la comunidad de App Inventor.

### 4.1.b – Reducir la foto antes de enviarla

Una foto del móvil puede ocupar **3–8 MB**. Convertida a Base64 crece otro 33 %, y
mandar eso por WiFi es lento (y se come la cuota de Google). Con **640 px de ancho sobra**
para que Vision reconozca el objeto.

**La verdad**: App Inventor **tampoco sabe redimensionar imágenes de serie**. Opciones reales:

| Cómo | Qué hacer | Nota |
|---|---|---|
| **Bajar la calidad de la cámara** ⭐ *lo más fácil* | En el Designer, selecciona `Camera1` y baja la propiedad **Quality** | No cambia el tamaño en píxeles, pero reduce mucho el peso |
| **Usar `Canvas`** | Poner un `Canvas` de 640 px de ancho, cargar la foto como fondo y guardar con `Canvas.SaveAs` | Funciona, pero da algo de guerra |
| **Extensión de imagen** | Buscar una extensión tipo *Image* / *TaifunImage* con bloque de redimensionado | La más limpia si la encuentras |

Empieza por bajar **Quality**; si va rápido, no compliques más.

### 4.2 – Componente Camera1

Viene por defecto. Asegúrate de tener estos permisos en **AndroidManifest** (App Inventor los pide automáticamente):
- `android.permission.CAMERA`
- `android.permission.WRITE_EXTERNAL_STORAGE`

### 4.3 – Componente Web1 (llamar a Google Vision)

Configura:
- **Url**: `https://vision.googleapis.com/v1/images:annotate?key=TU_CLAVE_API`
- **Method**: POST
- **RequestHeaders**: 
  - `Content-Type: application/json`
- **PostData**: JSON formateado

```json
{
  "requests": [{
    "image": { "content": "<BASE64_DE_LA_FOTO>" },
    "features": [{ "type": "OBJECT_LOCALIZATION", "maxResults": 5 }]
  }]
}
```

Se envía con el bloque `call Web1.PostText text`.

> 💡 Usamos `OBJECT_LOCALIZATION` en vez de `LABEL_DETECTION` porque devuelve
> **objetos de verdad** (`Cat`, `Chair`) en lugar de etiquetas abstractas
> (`Font`, `Material property`). Está explicado en detalle en el README.

### 4.4 – Componente Web2 (llamar a Arduino)

Configura:
- **Url**: `http://<IP_ARDUINO>/?texto=<ETIQUETA>`
- **Method**: GET

---

## Paso 5: Funciones helper (Bloques personalizados)

Crea procedimientos (bloques personalizados) para que el código sea más limpio:

### Función: ResizeImage

⚠️ **App Inventor no tiene un bloque de redimensionar.** Este "procedimiento" es en
realidad una de las tres soluciones del apartado 4.1.b. Si bajas la calidad de la cámara
(lo más fácil), **no necesitas este procedimiento en absoluto**: borra la llamada del
bloque `Camera1.AfterPicture` y usa la ruta de la foto directamente.

Si optas por el `Canvas`:

```
procedure ResizeImage( rutaFoto )
   set Canvas1.Width  to 640
   set Canvas1.Height to 480
   set Canvas1.BackgroundImage to rutaFoto
   set global rutaReducida to call Canvas1.SaveAs "reducida.jpg"
   return get global rutaReducida
```

### Función: ConvertirABase64

Depende de la extensión que hayas instalado; el nombre del bloque cambia según cuál sea.

```
procedure ConvertirABase64( rutaImagen )
   set global codigoBase64 to call Base64Extension.encodeFile rutaImagen
   return get global codigoBase64
```

### Función: URLEncode

```
procedure URLEncode( texto )
   // App Inventor tiene built-in: Web1.UriEncode
   return call Web1.UriEncode texto
```

---

## Paso 6: Pruebas

### Test 1: Tomar foto y mostrar

1. Instala la app en el móvil (APK)
2. Presiona "Tomar foto"
3. Verifica que aparece en pantalla

### Test 2: Enviar a Vision (sin Arduino)

1. Toma una foto
2. Verifica que llega a Google Vision
3. Comprueba en la app que aparece la etiqueta detectada

### Test 3: Enviar a Arduino (flujo completo)

1. Asegúrate de que **Arduino corre el sketch Fase 4** y muestra la IP en pantalla
2. En la app, escribe la IP en el campo `TxtIPArduino`
3. Toma una foto
4. Presiona "Enviar a Arduino"
5. **Verifica que el nombre aparece en la LCD de Arduino**

---

## Paso 7: Consideraciones especiales

### ⚠️ Red WiFi local

- El Arduino y el móvil **tienen que estar en la MISMA red WiFi**
- El Arduino Uno R4 WiFi **solo funciona en 2,4 GHz**, no en 5 GHz. Si tu router tiene
  las dos redes con nombres distintos, conecta el móvil a la de 2,4 GHz
- Algunos routers ponen los aparatos "IoT" en una red aparte que no se ve desde el móvil.
  Si no consigues llegar a la placa, mira esa opción en la configuración del router

### ⚠️ Clave de Google Vision API

En este proyecto la clave va **dentro de la app**, porque es la única forma sencilla de
hacerlo sin montar un servidor. Hay que asumir lo que eso significa:

- La app es **solo para vosotros**; no la publiques en Google Play
- Restringe la clave en Google Cloud **solo a Cloud Vision API**
- Pon una **alerta de presupuesto** en Google Cloud, por si acaso
- Si algún día la clave se filtra, **bórrala y crea otra** — se hace en dos clics

### ⚠️ Los espacios en la URL

Vision devuelve nombres con espacios, como `Teddy bear` o `Coffee cup`. Un espacio no
puede ir tal cual en una dirección web, por eso usamos `Web1.UriEncode`:

- `Teddy bear` → `Teddy%20bear` → viaja por la red → el Arduino lo vuelve a convertir en `Teddy bear`

Si se te olvida el `UriEncode`, en la pantalla solo saldrá `Teddy` (la primera palabra).

### ℹ️ Mejoras futuras

- Guardar un histórico de los objetos detectados
- Botón para elegir entre `OBJECT_LOCALIZATION` y `LABEL_DETECTION`
- Enviar también el porcentaje de confianza (`score`) y mostrarlo en la LCD
- Sincronizar además con Arduino Cloud (fase opcional)

---

## Recursos

- [MIT App Inventor Docs](https://appinventor.mit.edu/explore/ai2/tutorials)
- [Google Vision API](https://cloud.google.com/vision/docs)
- [WiFiS3 Arduino](https://docs.arduino.cc/libraries/wifi-s3/)
