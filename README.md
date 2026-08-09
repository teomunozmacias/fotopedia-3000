# fotopedia-3000 📸🔤

Con este aparato puedes hacerle una foto a algo con el móvil y una IA reconoce qué es y lo pone en una pantalla LCD.

## Descripción del proyecto

El sistema funciona así:

1. Con el móvil se saca una foto a un objeto.
2. La foto se envía a una IA de reconocimiento de imágenes (Google Cloud Vision API), que identifica qué es el objeto.
3. El resultado (el nombre del objeto) se envía a **Arduino Cloud**.
4. La placa Arduino, conectada a Arduino Cloud, recibe ese texto y lo muestra en una **pantalla LCD**.

## Arquitectura

```
[App móvil]                [Nube]                    [Arduino Uno R4 WiFi]
   📷 sacas foto
   → envías foto a Google Vision API
   ← recibe "silla", "taza", "perro"...
   → envía ese texto a Arduino Cloud (variable)                          
                          [Arduino IoT Cloud] --sincroniza--> [Placa]
                                                              → muestra en LCD
```

## Componentes del sistema

| Pieza | Herramienta elegida |
|---|---|
| App móvil | MIT App Inventor (sin código) |
| Reconocimiento de imagen (IA) | Google Cloud Vision API |
| Placa | Arduino Uno R4 WiFi |
| Sincronización nube ↔ placa | Arduino IoT Cloud |
| Pantalla | LCD 1602A (sin módulo I2C, 16 pines directos) |

## Plan de trabajo

- [x] **Fase 1 – Hardware**: conectar la LCD 1602 al Arduino Uno R4 WiFi y mostrar un mensaje de prueba.
- [x] **Fase 2 – Arduino Cloud**: crear la "Thing", la variable de texto, y el sketch que sincroniza y actualiza la LCD automáticamente.
- [x] **Fase 3 – Google Vision API**: crear cuenta/clave y probar que identifica correctamente una foto.
- [ ] **Fase 4 – App en MIT App Inventor**: cámara + llamada a Vision + llamada a Arduino Cloud para actualizar la variable.
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

Ver [`fase1_test_lcd.ino`](./fase1_test_lcd.ino). Usa la librería `LiquidCrystal` (incluida por defecto en el IDE de Arduino) para mostrar el texto "Fotopedia 3000" / "Hola mundo!" en la pantalla y verificar que el cableado funciona.

## Próximos pasos

Montar la app en MIT App Inventor (Fase 4): cámara + llamada a Google Vision + llamada a la API de Arduino Cloud para actualizar la variable `objetoDetectado`.

## Fase 2 — Arduino Cloud (completada)

### Configuración en Arduino Cloud

- Dispositivo: Arduino Uno R4 WiFi dado de alta en [Arduino Cloud](https://cloud.arduino.cc)
- Thing creada, con la placa y la red WiFi asociadas
- Variable de nube: `objetoDetectado`, tipo **String**, permisos **Read & Write**, sincronización **On change**

### Código

Ver [`fase2_sketch_cloud.ino`](./fase2_sketch_cloud.ino). Usa la librería `ArduinoIoTCloud` (junto con `thingProperties.h`, generado automáticamente por Arduino Cloud) para sincronizar la variable, y `LiquidCrystal` para mostrarla en la LCD.

**Notas de la implementación:**
- La escritura en la LCD no se hace directamente dentro de `onObjetoDetectadoChange()`, sino que esa función solo activa una bandera (`hayActualizacionPendiente`); la escritura real ocurre en el `loop()`, después de `ArduinoCloud.update()`. Esto evita conflictos de tiempo con la actividad del WiFi.
- Se usa `lcd.begin(16, 2)` en vez de `lcd.clear()` para refrescar la pantalla: `begin()` reenvía toda la configuración interna de la LCD, no solo borra el contenido, lo que evita caracteres corruptos causados por interferencia del WiFi sobre los cables de datos.

## Fase 3 — Google Cloud Vision API (completada)

### Configuración en Google Cloud

- Proyecto creado en [Google Cloud Console](https://console.cloud.google.com)
- **Cloud Vision API** habilitada para el proyecto
- Clave de API generada y restringida únicamente a Cloud Vision API
- **Facturación activada** en el proyecto (requisito de Google para usar la API, aunque el uso se mantenga dentro del nivel gratuito de 1000 imágenes/mes). Se recomienda configurar una alerta de presupuesto.
- ⚠️ La clave de API no se sube al repositorio en texto plano.

### Cómo se probó

Página de prueba local ([`test_google_vision.html`](./test_google_vision.html)): permite subir una foto desde el navegador, la convierte a Base64, y llama a la API con el tipo de análisis `LABEL_DETECTION` (máximo 5 etiquetas).

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
- Si la foto no tiene nada reconocible, `labelAnnotations` puede venir vacío o no existir — hay que comprobarlo antes de leerlo (así lo hace `test_google_vision.html`).
