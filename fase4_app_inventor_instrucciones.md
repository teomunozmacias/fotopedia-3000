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
cuando BtnTomarFoto.Click
   llamar Camera1.TakePicture
```

Este bloque ya viene con el componente Camera1 en App Inventor.

### 3.2 – Evento: Foto capturada (Camera1.AfterPicture)

```
cuando Camera1.AfterPicture image (ruta de la foto)
   llamar ResizeImage( image, 640 )
   guardar en variable "fotoActual"
   llamar ConvertirABase64( fotoActual )
   guardar resultado en variable "base64Foto"
   mostrar ImgFoto.Source = fotoActual
   LblResultado.Text = "Foto capturada, enviando a Vision..."
   llamar Vision_API( base64Foto )
```

**Nota**: Necesitas la extensión `Base64` (ver paso 4.1)

### 3.3 – Evento: Respuesta de Google Vision

```
cuando Web1.GotText response (respuesta JSON)
   analizar JSON: respuesta
   extraer: labelAnnotations[0].description
   guardar en variable "etiquetaDetectada"
   LblResultado.Text = etiquetaDetectada
   LblEstado.Text = "Detectado: " + etiquetaDetectada
```

### 3.4 – Evento: Enviar a Arduino (BtnEnviar.Click)

```
cuando BtnEnviar.Click
   ip = TxtIPArduino.Text
   texto = etiquetaDetectada
   url = "http://" + ip + "/?texto=" + URLEncode(texto)
   llamar Web2.Get( url )
   LblEstado.Text = "Enviando a Arduino..."
```

### 3.5 – Evento: Confirmación desde Arduino

```
cuando Web2.GotText response
   si response contiene "OK"
      LblEstado.Text = "✓ Enviado a Arduino"
   sino
      LblEstado.Text = "✗ Error: " + response
```

---

## Paso 4: Configuración de componentes

### 4.1 – Extensión Base64

MIT App Inventor **no tiene Base64 nativo**, necesitas una extensión:

1. En Designer, abajo a la izquierda: **Extensions**
2. Busca y carga: `Base64` (hay varias, usa una de confianza)
   - Ej: "MIT App Inventor Base64 Extension" o similar de la comunidad
3. Esto te da el bloque `Base64.encode(datos)`

**Alternativa simple**: Si no encuentras, usa JavaScript en un **WebView** para codificar a Base64 (más complicado).

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
    "features": [{ "type": "LABEL_DETECTION", "maxResults": 5 }]
  }]
}
```

Usa **Do It > Web1.PostText()** para enviar.

### 4.4 – Componente Web2 (llamar a Arduino)

Configura:
- **Url**: `http://<IP_ARDUINO>/?texto=<ETIQUETA>`
- **Method**: GET

---

## Paso 5: Funciones helper (Bloques personalizados)

Crea procedimientos (bloques personalizados) para que el código sea más limpio:

### Función: ResizeImage

```
procedimiento ResizeImage( imagenOriginal, maxAncho )
   // En App Inventor, típicamente la librería Canvas o Image maneja esto
   // Alternativa: usar servidor de redimensionado online
   return imagenRedimensionada
```

### Función: ConvertirABase64

```
procedimiento ConvertirABase64( rutaImagen )
   extension.Base64.encode(imagenArchivo)
   return codigoBase64
```

### Función: URLEncode

```
procedimiento URLEncode( texto )
   // App Inventor tiene built-in
   return Web1.URLDecode() (hay built-in)
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

- Arduino y móvil **deben estar en la MISMA red WiFi**
- Si tienes red "2.4 GHz" y "5 GHz" separadas, conecta ambos a la misma
- Algunos routers separado "IoT" en una red aparte: verifica que la app pueda llegar

### ⚠️ Clave de Google Vision API

- **NO la guardes hardcodeada** en la app
- Usa un servidor intermedio que maneje las claves, o
- Pide la clave manualmente cuando abra la app (peor UX pero más seguro)

### ⚠️ URL encoding

Google Vision API devuelve etiquetas que pueden tener espacios:
- "Pet Supplies" → URL encode → "Pet%20Supplies"
- Arduino decodifica automáticamente en el sketch

### ℹ️ Mejoras futuras

- Guardar histórico de detecciones
- Ajustar calidad de imagen (reducir más para envíos más rápidos)
- Interfaz para cambiar modos (LABEL_DETECTION vs OBJECT_LOCALIZATION)
- Botón para sincronizar con Arduino Cloud (Fase futura)

---

## Recursos

- [MIT App Inventor Docs](https://appinventor.mit.edu/explore/ai2/tutorials)
- [Google Vision API](https://cloud.google.com/vision/docs)
- [WiFiS3 Arduino](https://docs.arduino.cc/libraries/wifi-s3/)
