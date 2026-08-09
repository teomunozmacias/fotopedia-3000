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

> 📌 **Pendiente**: falta subir a este repositorio el sketch de la Fase 2
> (`fase2_arduino_cloud.ino`, el de Arduino Cloud). El hueco está preparado más abajo.

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

> ⚠️ **Pendiente de subir al repositorio.**
> El sketch de esta fase existe y funciona, pero todavía no está en GitHub.
> Cuando se suba, irá aquí como `fase2_arduino_cloud.ino`.

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

Página de prueba local: [`fase3_google_vision_test.html`](./fase3_google_vision_test.html)

Se abre con doble clic en cualquier navegador, sin instalar nada. Arrastras una foto,
escribes tu clave de API, y te enseña **lado a lado** lo que devuelven las dos formas de
analizar, con la misma foto y en una sola llamada:

```
┌──────────────────────────┬──────────────────────────┐
│ LABEL_DETECTION          │ OBJECT_LOCALIZATION      │
├──────────────────────────┼──────────────────────────┤
│ ⭐ Textile      94% ←abst│ ⭐ Teddy bear        91%  │
│    Material p.  91% ←abst│                          │
│    Teddy bear   88%      │                          │
└──────────────────────────┴──────────────────────────┘
      Se mandaría a la pantalla: Teddy bear
```

Lo que hace por dentro (los mismos pasos que hará la app del móvil):

1. **Reduce la foto a 640 px** de ancho y te dice cuánto ha adelgazado
2. La convierte a **Base64**
3. Llama a Vision pidiendo **las dos features a la vez**
4. **Marca en amarillo las etiquetas abstractas** para que se vea el problema
5. Aplica el mismo recorte a 16 letras y quitado de tildes que hace el Arduino
6. Opcionalmente **envía el resultado a la LCD**, si le das la IP de la placa

> 🔒 La clave de API **no está dentro del archivo**: se escribe cada vez y solo se recuerda
> mientras la pestaña esté abierta. Por eso esta página sí se puede subir a GitHub.

Con el botón del paso 4 se puede probar la cadena entera (foto → IA → pantalla)
**sin haber montado todavía la app del móvil**.

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
- Si la foto no tiene nada reconocible, `labelAnnotations` puede venir vacío o no existir — hay que comprobarlo antes de leerlo.

### ⚠️ Problema: la primera etiqueta suele ser abstracta

Este es el fallo más visible del proyecto y conviene entenderlo bien.

`LABEL_DETECTION` no describe **el objeto**, describe **la imagen entera**: colores, texturas, estilo, composición… Por eso, si le haces una foto a un peluche, la lista puede empezar así:

```
1. Textile          (tela)          ← etiqueta abstracta
2. Material property (propiedad del material) ← etiqueta abstracta
3. Teddy bear       (osito)         ← ¡esta es la buena!
```

Coger siempre `labelAnnotations[0]` haría que en la pantalla saliera **"Textile"** en lugar de **"Teddy bear"**.

Otras etiquetas abstractas que aparecen mucho: `Font`, `Pattern`, `Rectangle`, `Line`, `Automotive design`, `Circle`, `Art`, `Sky`, `Wood`, `Metal`, `Plastic`, `Product`.

#### Tres formas de arreglarlo

**Opción A — Lista negra de genéricos** (la más sencilla)

Seguir usando `LABEL_DETECTION`, pero recorrer la lista y quedarse con la **primera etiqueta que no esté en la lista negra**. Si todas son genéricas, se usa la primera de todas.

- ✅ Sencillo, no cambia la llamada a la API
- ❌ Hay que ir ampliando la lista a mano según lo que vayas viendo

**Opción B — `OBJECT_LOCALIZATION`** (la que mejor funciona) ⭐ *recomendada*

Cambiar el tipo de análisis. Esta función busca **objetos físicos** y devuelve además dónde están en la foto:

```json
{
  "requests": [{
    "image": { "content": "<foto en Base64>" },
    "features": [{ "type": "OBJECT_LOCALIZATION", "maxResults": 5 }]
  }]
}
```

La respuesta cambia de sitio y de nombre:

```json
{
  "responses": [{
    "localizedObjectAnnotations": [
      { "name": "Teddy bear", "score": 0.91, "boundingPoly": { } }
    ]
  }]
}
```

El dato que interesa es `responses[0].localizedObjectAnnotations[0].name`.

- ✅ Devuelve cosas de verdad: `Cat`, `Chair`, `Bottle`, `Person`, `Teddy bear`
- ✅ Nunca devuelve `Font` ni `Material property`
- ❌ Conoce menos categorías: si la foto no tiene un objeto claro (un paisaje, una pared), la lista viene **vacía**

**Opción C — Pedir las dos a la vez** (la más completa)

Se pueden pedir las dos `features` en la misma petición (cuesta lo mismo que dos llamadas en la cuota gratuita, ojo):

```json
"features": [
  { "type": "OBJECT_LOCALIZATION", "maxResults": 5 },
  { "type": "LABEL_DETECTION",     "maxResults": 5 }
]
```

Y luego decidir así:
1. ¿Hay `localizedObjectAnnotations`? → usar el primero.
2. Si no → usar la primera `labelAnnotation` que no esté en la lista negra.

#### Qué hacemos en este proyecto

**Opción B: `OBJECT_LOCALIZATION`.** ✅ *Comprobado con fotos nuestras.*

No es una decisión sobre el papel: lo probamos con la página de la Fase 3, comparando las
dos formas con las mismas fotos, y `OBJECT_LOCALIZATION` acierta claramente más con
objetos de casa. Además nos ahorra tener que mantener a mano la lista de palabras
abstractas.

Si algún día vemos que se queda en blanco demasiado a menudo (pasa con paisajes o
superficies lisas), pasaremos a la **Opción C**, que ya está preparada en la página de
prueba.

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

**Solución en el código**: antes de escribir en la pantalla, el Arduino quita los acentos:
- "Café" → "Cafe"
- "Niño" → "Nino"
- "Silla" → "Silla" (sin cambios)

### 🔍 Detalle curioso: una letra con tilde ocupa dos números

Dentro del ordenador, las letras se guardan como números. Las letras normales (`a`, `B`, `7`) ocupan **un solo número**, pero las que llevan tilde ocupan **dos números seguidos**. Este sistema se llama **UTF-8**.

| Letra | Números que ocupa |
|---|---|
| `a` | `0x61` |
| `á` | `0xC3` `0xA1` |
| `ñ` | `0xC3` `0xB1` |

Por eso la función `quitarAcentos()` del sketch va mirando los números **de dos en dos**: cuando ve un `0xC3`, sabe que la letra de verdad está en el número siguiente.

> 💡 Este fue un error real que tuvimos: la primera versión comparaba `letra == 'á'`, y como `'á'` no cabe en un solo número, **nunca funcionaba**.

### ℹ️ Nota: hay dos tipos de pantalla

El chip HD44780 viene con dos juegos de letras distintos según el modelo:

| Versión | ¿Tiene ñ y tildes? | Qué trae en su lugar |
|---|---|---|
| **A00** (la más común) | ❌ No | Caracteres japoneses (katakana) |
| **A02** | ✅ Sí | Letras europeas con tildes |

La mayoría de módulos 1602A que se venden sueltos son **A00**, y por eso quitamos los acentos. Si tu pantalla resulta ser **A02**, podrías saltarte ese paso.

**Cómo saber cuál tienes**: sube el sketch de la Fase 1 y añade `lcd.write(0xEE);`. Si sale un símbolo japonés es una A00; si sale una `ñ` es una A02.

---

## Fase 4 — App en MIT App Inventor + Servidor HTTP (en desarrollo)

Esta fase tiene dos mitades:

| Mitad | Estado |
|---|---|
| **Servidor HTTP en el Arduino** | ✅ Hecho y probado |
| **App en MIT App Inventor** | ⏳ En construcción |

### Componentes del Arduino

El Arduino corre un servidor web local que:

1. **Se conecta a WiFi** usando las credenciales de tu red (SSID y contraseña)
2. **Muestra la IP en la línea de arriba de la LCD**, y la deja ahí siempre
3. **Escucha en el puerto 80** (el puerto normal de las páginas web)
4. **Recibe peticiones GET** con parámetro: `http://<IP>/?texto=nombreDelObjeto`
5. **Descodifica la URL**: `%20` y `+` vuelven a ser espacios, `%C3%A9` vuelve a ser `é`
6. **Quita los acentos** y **corta a 16 caracteres**
7. **Escribe el texto en la línea de abajo** de la LCD
8. **Responde `OK`** a la app

**Archivo**: Ver [`fase4_arduino_http_server.ino`](./fase4_arduino_http_server.ino)

**Configuración necesaria en el sketch**:
```cpp
const char* SSID_WIFI  = "TU_RED_WIFI";
const char* CLAVE_WIFI = "TU_CONTRASENA";
```

> ⚠️ **No subas a GitHub el sketch con la clave del WiFi de casa escrita de verdad.**
> Este repositorio es público. Antes de hacer `git commit`, deja los valores de ejemplo.

### 📍 Cómo saber la IP de la placa

La placa no tiene una dirección fija de fábrica: se la da el router al encenderse (esto se llama **DHCP**). Por eso el sketch **la enseña en la pantalla**:

```
┌────────────────┐
│192.168.1.50    │  ← línea 1: la IP (siempre visible)
│Esperando foto  │  ← línea 2: el objeto detectado
└────────────────┘
```

Esa es la dirección que hay que escribir en la app. También sale por el **Monitor Serie** (a 115200 baudios).

#### Conectarse y tener IP son dos cosas distintas

Al arrancar, la placa da **dos pasos seguidos**, y es fácil confundirlos:

| Paso | Qué pasa | Cómo se comprueba |
|---|---|---|
| 1️⃣ Entrar en la red | La placa dice la clave y el router la deja pasar | `WiFi.status() == WL_CONNECTED` |
| 2️⃣ Recibir una dirección | El router le presta un número (una IP) | `WiFi.localIP()` deja de ser `0.0.0.0` |

Es como entrar en un hotel: primero cruzas la puerta, y **después**, en recepción, te dan
el número de habitación. Ese reparto de direcciones se llama **DHCP** y tarda un poquito.

> 💡 Este fue otro error real que tuvimos: el programa preguntaba la IP justo después del
> paso 1, cuando el router todavía no la había dado, y la pantalla mostraba **`0.0.0.0`**.
> La solución es esperar a que la dirección deje de estar vacía antes de enseñarla.

**Cuidado**: la IP puede cambiar si reinicias el router o si la placa está mucho rato apagada. Si un día la app deja de funcionar, lo primero es **mirar la pantalla** por si la IP es otra.

**Cómo evitar que cambie** (opcional, no hace falta para que funcione):

| Método | Qué hay que hacer | Dificultad |
|---|---|---|
| **Mirar la pantalla** ✅ *lo que hacemos* | Nada. Leer la IP de la LCD y escribirla en la app | Ninguna |
| **IP reservada en el router** | Entrar en la configuración del router → DHCP → asignar siempre la misma IP a la MAC de la placa | Media |
| **IP fija en el sketch** | Usar `WiFi.config(IPAddress(192,168,1,50))` antes de `WiFi.begin()` | Media (si eliges una IP ocupada, se rompe) |

### 🔌 Si se cae el WiFi

El sketch comprueba cada 5 segundos si sigue conectado. Si se ha caído, lo intenta de nuevo solo y muestra `Recuperando WiFi` en pantalla. No hay que reiniciar nada a mano.

### Aplicación en MIT App Inventor

La app hace esto:

1. **Cámara**: permite capturar una foto
2. **Reducir la imagen** a ~640 px de ancho, para que se envíe rápido
3. **Convertir a Base64**: App Inventor **no sabe hacerlo de serie**, hace falta una extensión
4. **Llamar a Google Vision API**:
   - `POST https://vision.googleapis.com/v1/images:annotate?key=TU_CLAVE_API`
   - Tipo: `OBJECT_LOCALIZATION` (ver [por qué](#️-problema-la-primera-etiqueta-suele-ser-abstracta))
   - Respuesta: JSON con los objetos encontrados
5. **Extraer el nombre**: `responses[0].localizedObjectAnnotations[0].name`
6. **Enviar al Arduino**: `GET http://<IP-Arduino>/?texto=<nombre>`
7. **Mostrar confirmación**: "Enviado a Arduino"

**Archivo**: Ver [`fase4_app_inventor_instrucciones.md`](./fase4_app_inventor_instrucciones.md)

> 📌 Las etiquetas de Google Vision vienen **en inglés** (`Cat`, `Chair`, `Bottle`).
> En este proyecto las dejamos así a propósito: se entienden bien y no hace falta traducirlas.

---

## Fase 5 — Integración y pruebas (en desarrollo)

Checklist completo del flujo:

- [ ] Arduino arranca y muestra la IP en la LCD
- [ ] App y placa están en la misma red WiFi
- [ ] Tomar una foto con la app
- [ ] Google Vision identifica el objeto (se ve en la app)
- [ ] El nombre aparece en la LCD del Arduino
- [ ] Probar con distintos objetos (animales, muebles, plantas…)
- [ ] Medir el tiempo total: foto → texto en pantalla (**típicamente 3–8 segundos**)
- [ ] Casos límite: texto largo (se corta a 16), acentos (se quitan), Vision sin resultado
- [ ] Si `OBJECT_LOCALIZATION` se queda en blanco a menudo, pasar a la Opción C

**Archivo**: Ver [`fase5_integracion_checklist.md`](./fase5_integracion_checklist.md)
