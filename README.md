# fotopedia-3000 📸🔤

Con este aparato puedes hacerle una foto a algo con el móvil y una IA reconoce qué es y lo pone en una pantalla LCD.

## Descripción del proyecto

El sistema funciona así:

1. Con el móvil se saca una foto a un objeto.
2. La foto se envía a una IA de reconocimiento de imágenes (Google Cloud Vision API), que identifica qué es el objeto.
3. El resultado (el nombre del objeto) se envía a un **servidor web en la placa Arduino** mediante HTTP.
4. La placa Arduino recibe el texto y lo muestra en una **pantalla LCD**.

## Arquitectura

```
[App móvil]                     [Nube]                [Arduino Uno R4 WiFi]
   📷 sacas foto
   → envías foto a Google Vision API
   ← recibe "silla", "taza", "perro"...
   → envía GET http://<ip-arduino>/?texto=Cat
                                              [Servidor WiFi local]
                                              → parsea URL
                                              → decodifica parámetro
                                              → trunca a 16 caracteres
                                              → muestra en LCD
```

## Componentes del sistema

| Pieza | Herramienta elegida |
|---|---|
| App móvil | MIT App Inventor (sin código) |
| Reconocimiento de imagen (IA) | Google Cloud Vision API |
| Placa | Arduino Uno R4 WiFi |
| Comunicación app → placa | HTTP (WiFiS3 + WiFiServer, sin Arduino Cloud) |
| Pantalla | LCD 1602A (sin módulo I2C, 16 pines directos) |

## Plan de trabajo

- [x] **Fase 1 – Hardware**: conectar la LCD 1602 al Arduino Uno R4 WiFi y mostrar un mensaje de prueba.
- [x] **Fase 2 – Arduino Cloud**: crear la "Thing", la variable de texto, y el sketch que sincroniza y actualiza la LCD automáticamente.
- [x] **Fase 3 – Google Vision API**: crear cuenta/clave y probar que identifica correctamente una foto.
- [ ] **Fase 4 – App en MIT App Inventor + servidor HTTP**: cámara + llamada a Vision + petición GET HTTP a Arduino.
- [ ] **Fase 5 – Integración final**: probarlo todo junto de extremo a extremo.

## Fase 1 — Conexión de la LCD (completada)

### Materiales necesarios

- Arduino Uno R4 WiFi
- Pantalla LCD 1602A (sin I2C)
- Potenciómetro de 10kΩ (control de contraste)
- Resistencia de 220Ω (protección de la luz de fondo)
- Protoboard y cables macho-macho / macho-hembra

### Esquema de conexión

| Pin de la LCD | Va conectado a |
|---|---|
| 1 – VSS | GND |
| 2 – VDD | 5V |
| 3 – V0 | Pata central (wiper) del potenciómetro |
| 4 – RS | Pin digital 12 |
| 5 – RW | GND |
| 6 – E | Pin digital 11 |
| 7 a 10 – D0 a D3 | No se conectan |
| 11 – D4 | Pin digital 5 |
| 12 – D5 | Pin digital 4 |
| 13 – D6 | Pin digital 3 |
| 14 – D7 | Pin digital 2 |
| 15 – A (luz de fondo, ánodo) | 5V a través de la resistencia de 220Ω |
| 16 – K (luz de fondo, cátodo) | GND |

Las dos patas externas del potenciómetro van a 5V y GND (en cualquier orden); la pata central va al pin V0 de la LCD y controla el contraste.

### Código de prueba

Ver [`fase1_lcd_test.ino`](./fase1_lcd_test.ino). Usa la librería `LiquidCrystal` (incluida por defecto en el IDE de Arduino) para mostrar el texto "Fotopedia 3000" / "Detec. objetos" en la pantalla y verificar que el cableado funciona.

## Fase 2 — Arduino Cloud (documentada, código pendiente)

### Configuración en Arduino Cloud

- Dispositivo: Arduino Uno R4 WiFi dado de alta en [Arduino Cloud](https://cloud.arduino.cc)
- Thing creada, con la placa y la red WiFi asociadas
- Variable de nube: `objetoDetectado`, tipo **String**, permisos **Read & Write**, sincronización **On change**

### Código

Ver [`fase2_arduino_cloud.ino`](./fase2_arduino_cloud.ino) (código pendiente de subir).

Usa la librería `ArduinoIoTCloud` (junto con `thingProperties.h`, generado automáticamente por Arduino Cloud) para sincronizar la variable, y `LiquidCrystal` para mostrarla en la LCD.

**Notas de la implementación:**
- La escritura en la LCD no se hace directamente dentro de `onObjetoDetectadoChange()`, sino que esa función solo activa una bandera (`hayActualizacionPendiente`); la escritura real ocurre en el `loop()`, después de `ArduinoCloud.update()`. Esto evita conflictos de tiempo con la actividad del WiFi.
- Se usa `lcd.begin(16, 2)` en vez de `lcd.clear()` para refrescar la pantalla: `begin()` reenvía toda la configuración interna de la LCD, no solo borra el contenido, lo que evita caracteres corruptos causados por interferencia del WiFi sobre los cables de datos.

## Fase 3 — Google Cloud Vision API (documentada, código pendiente)

### Configuración en Google Cloud

- Proyecto creado en [Google Cloud Console](https://console.cloud.google.com)
- **Cloud Vision API** habilitada para el proyecto
- Clave de API generada y restringida únicamente a Cloud Vision API
- **Facturación activada** en el proyecto (requisito de Google para usar la API, aunque el uso se mantenga dentro del nivel gratuito de 1000 imágenes/mes). Se recomienda configurar una alerta de presupuesto.
- ⚠️ La clave de API no se sube al repositorio en texto plano.

### Cómo se probó

Página de prueba local ([`fase3_google_vision_test.html`](./fase3_google_vision_test.html), código pendiente de subir): permite subir una foto desde el navegador, la convierte a Base64, y llama a la API con el tipo de análisis `LABEL_DETECTION` (máximo 5 etiquetas).

Endpoint usado:
```
POST https://vision.googleapis.com/v1/images:annotate?key=TU_CLAVE_DE_API
```

Cuerpo de la petición:
```json
{
  "requests": [{
    "image": { "content": "<foto en Base64>" },
    "features": [{ "type": "LABEL_DETECTION", "maxResults": 5 }]
  }]
}
```

### Formato de la respuesta

La API devuelve un JSON con esta forma:

```json
{
  "responses": [
    {
      "labelAnnotations": [
        {
          "mid": "/m/0jbk",
          "description": "Animal",
          "score": 0.98,
          "topicality": 0.98
        },
        {
          "mid": "/m/01yrx",
          "description": "Cat",
          "score": 0.95,
          "topicality": 0.95
        }
      ]
    }
  ]

}
```

- `responses[0].labelAnnotations` es una lista ordenada de mayor a menor confianza.
- Cada etiqueta trae `description` (el nombre en inglés, ej. "Cat"), `score` (confianza entre 0 y 1, ej. 0.95 = 95%) y `mid` (un ID interno de Google, no lo necesitamos para este proyecto).
- Para la app, el dato que nos interesa es `responses[0].labelAnnotations[0].description`: la etiqueta con más confianza, la primera de la lista.
- Si la foto no tiene nada reconocible, `labelAnnotations` puede venir vacío o no existir — hay que comprobarlo antes de leerlo (así lo hace `fase3_google_vision_test.html`).

---

## 💡 Decisión de diseño: ¿Por qué servidor HTTP local en lugar de Arduino Cloud?

En las primeras versiones se planteó usar **Arduino IoT Cloud** para sincronizar el nombre del objeto detectado. Sin embargo, hemos optado por un **servidor HTTP simple en la placa** por estas razones:

1. **Seguridad más simple**: Usar OAuth2 en una app móvil requiere guardar credenciales (`client_id`, `client_secret`) dentro de la aplicación. Aunque sea en una app privada, es frágil. HTTP en red local no necesita autenticación.

2. **Sin dependencias de terceros**: Arduino Cloud puede tener caídas o cambios de API. Un servidor en la placa es completamente controlado.

3. **Mejor para aprender**: El hijo entiende HTTP, IP, parámetros de URL. Está más cerca del funcionamiento real de Internet.

4. **Funcionamiento offline**: Si la red local se cae pero el WiFi sigue funcionando, Arduino Cloud podría sincronizar mal. HTTP local es directo.

**Nota**: Arduino Cloud queda como mejora opcional futura. El código HTTP es simple de expandir si luego se quiere también sincronizar a la nube.

---

## 🔤 Caracteres en la pantalla LCD 1602A (HD44780)

La pantalla HD44780 soporta el juego de caracteres **ASCII estándar + algunos caracteres especiales**, pero **NO soporta acentos ni ñ**. 

| Carácter | ¿Soportado? | Alternativa |
|---|---|---|
| a-z, A-Z, 0-9 | ✅ Sí | — |
| Espacios, puntuación (. , ! ? etc) | ✅ Sí | — |
| ñ | ❌ No | Usar "n" o abreviación |
| á, é, í, ó, ú | ❌ No | Quitar acento: a, e, i, o, u |
| À, È, Ì, Ò, Ù (mayúsculas acentuadas) | ❌ No | Quitar acento |

**Solución en el código**: Al truncar el texto en Arduino, se normalizan los acentos:
- "Café" → "Cafe"
- "Niño" → "Nino"
- "Silla" → "Silla" (sin cambios)

---

## Fase 4 — App en MIT App Inventor + Servidor HTTP (en desarrollo)

### Componentes del Arduino

El Arduino corre un servidor web local que:

1. **Se conecta a WiFi** usando las credenciales de tu red (SSID y contraseña)
2. **Muestra la IP en la LCD** al arrancar (para que sepas a dónde enviar las peticiones)
3. **Escucha en puerto 80** (HTTP estándar)
4. **Recibe peticiones GET** con parámetro: `http://<IP>/?texto=nombreDelObjeto`
5. **Parsea la URL**: decodifica caracteres especiales (%20 = espacio, + = espacio), elimina acentos
6. **Trunca a 16 caracteres** (ancho de la LCD)
7. **Muestra el texto en la LCD** en la segunda línea
8. **Responde OK** a la app

**Archivo**: Ver [`fase4_arduino_http_server.ino`](./fase4_arduino_http_server.ino)

**Configuración necesaria en el sketch**:
```cpp
const char* ssid = "TU_RED_WIFI";
const char* password = "TU_CONTRASEÑA";
```

### Aplicación en MIT App Inventor

La app hace esto:

1. **Cámara**: permite capturar una foto
2. **Reducir imagen**: redimensiona a ~640px de ancho (para que sea más rápido enviar a Google)
3. **Convertir a Base64**: usa la extensión `Base64` para codificar la foto (no es nativo en App Inventor)
4. **Llamar a Google Vision API**:
   - `POST https://vision.googleapis.com/v1/images:annotate?key=TU_CLAVE_API`
   - Parámetro: imagen en Base64
   - Tipo: `LABEL_DETECTION`
   - Respuesta: JSON con etiquetas
5. **Extraer etiqueta**: toma `responses[0].labelAnnotations[0].description`
6. **Enviar al Arduino**: `GET http://<IP-Arduino>/?texto=<etiqueta>`
7. **Mostrar confirmación**: "Enviado a Arduino" o icono

**Archivo**: Ver [`fase4_app_inventor_instrucciones.md`](./fase4_app_inventor_instrucciones.md)

---

## Fase 5 — Integración y pruebas (en desarrollo)

Checklist completo del flujo:

- [ ] Arduino corre, muestra IP en LCD
- [ ] App se conecta a la red WiFi
- [ ] Tomar una foto con la app
- [ ] Google Vision identifica el objeto (verificar en la app)
- [ ] La etiqueta aparece en la LCD del Arduino
- [ ] Probar con distintos objetos (animales, objetos, plantas, etc.)
- [ ] Medir latencia: foto → resultado en LCD (típicamente ~3-5 segundos)
- [ ] Casos límite: texto muy largo (truncado a 16 chars), acentos (normalizados)
- [ ] Si Vision se equivoca: documentar y evaluar si conviene cambiar a `OBJECT_LOCALIZATION`

**Archivo**: Ver [`fase5_integracion_checklist.md`](./fase5_integracion_checklist.md)
