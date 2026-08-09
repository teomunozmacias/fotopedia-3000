# Fase 5: Integración y pruebas finales

Checklist completo para verificar que todo funciona de extremo a extremo.

---

## Preparación

### Hardware

- [ ] Arduino Uno R4 WiFi conectado a la protoboard
- [ ] LCD 1602 conectada con los 16 pines (sin I2C)
- [ ] Potenciómetro y resistencia de 220Ω en su sitio
- [ ] Cable USB conectado a Arduino (para poder ver Serial Monitor)

### Software

- [ ] Arduino IDE con las librerías `LiquidCrystal` y `WiFiS3`
- [ ] Sketch `fase4_arduino_http_server.ino` cargado, con `SSID_WIFI` y `CLAVE_WIFI` puestos
- [ ] Monitor Serie configurado a **115200 baudios**
- [ ] App de MIT App Inventor instalada en el móvil
- [ ] Clave de Google Vision API generada y con Cloud Vision API activada

### Red

- [ ] Arduino y móvil en la **misma red WiFi de 2,4 GHz**
- [ ] IP del Arduino anotada (se ve en la LCD y en el Monitor Serie)

---

## Fase 5.1: Prueba de hardware (Arduino solo)

### Test 1: LCD muestra IP correctamente

- [ ] Carga el sketch `fase4_arduino_http_server.ino` (con tu SSID y clave puestos)
- [ ] Abre el Monitor Serie a **115200 baudios**
- [ ] El Monitor Serie imprime: `Conectado. IP: 192.168.X.X`
- [ ] La LCD queda así, y **la IP se queda ahí de forma permanente**:

```
┌────────────────┐
│192.168.1.50    │
│Esperando foto  │
└────────────────┘
```

- [ ] Anota la IP: la necesitas para todos los tests siguientes

### Test 2: Servidor HTTP responde localmente (curl desde PC)

```bash
# En una terminal (PC o Mac) en la misma red:
curl "http://192.168.X.X/?texto=TestHola"
```

Resultado esperado:
- [ ] Respuesta: `OK`
- [ ] La LCD muestra `TestHola` en la línea de abajo (la IP sigue arriba)
- [ ] El Monitor Serie imprime: `LCD linea 2: TestHola`

### Test 3: Decodificación de URL

```bash
# Probar espacio codificado
curl "http://192.168.X.X/?texto=Hola%20Mundo"
# o
curl "http://192.168.X.X/?texto=Hola+Mundo"
```

Resultado esperado:
- [ ] LCD muestra: "Hola Mundo"

### Test 4: Truncado a 16 caracteres

```bash
curl "http://192.168.X.X/?texto=EstaEsUnaFraseQueTieneMasDe16Caracteres"
```

Resultado esperado:
- [ ] La LCD muestra solo: `EstaEsUnaFraseQu` (16 caracteres exactos)
- [ ] El resto se corta sin dar error

### Test 5: Se quitan los acentos

```bash
curl "http://192.168.X.X/?texto=Cafe%CC%81"     # no, este no
curl "http://192.168.X.X/?texto=Caf%C3%A9"      # este si: %C3%A9 es la "e" con tilde
curl "http://192.168.X.X/?texto=Ni%C3%B1o"      # %C3%B1 es la "n" con virgulilla
```

Resultado esperado:
- [ ] Primera línea de la LCD: sigue la IP
- [ ] Segunda línea: `Cafe` y luego `Nino`, **sin símbolos raros**

> Si aquí ves basura en pantalla, el problema está en `quitarAcentos()` del sketch.
> Recuerda: una letra con tilde son **dos números** (`0xC3` + otro), no uno.

### Test 6: La placa no se cuelga con una conexión vacía

Este test comprueba el *timeout*. Abre una conexión y no envíes nada:

```bash
# Deja este comando abierto 5 segundos y luego cortalo con Ctrl+C
telnet 192.168.X.X 80
```

Resultado esperado:
- [ ] El Monitor Serie dice: `Peticion vacia (se agoto el tiempo de espera).`
- [ ] **La placa sigue respondiendo**: repite el Test 2 y debe funcionar
- [ ] Si la placa se queda muerta, el timeout no está funcionando

---

## Fase 5.2: Prueba de Vision API (app solo, sin Arduino)

### Test 1: App toma foto

- [ ] Abre la app en el móvil
- [ ] Presiona "📷 Tomar foto"
- [ ] Espera a que la cámara abre, toma una foto de un objeto
- [ ] Verifica que la imagen aparece en la app

### Test 2: Vision reconoce la foto

- [ ] La app envía la foto a Google Vision
- [ ] Espera la respuesta (**1–3 segundos** normalmente)
- [ ] Aparece un nombre de objeto: `Cat`, `Chair`, `Bottle`, `Teddy bear`…
- [ ] Si no aparece nada: comprueba que la clave de API es correcta y que
      **Cloud Vision API** está activada en Google Cloud

### Test 3: Comprobar que NO salen etiquetas abstractas

Este test verifica que `OBJECT_LOCALIZATION` está bien configurado.

- [ ] Haz una foto a un peluche o a un cojín (cosas con tela)
- [ ] El resultado debe ser un objeto (`Teddy bear`, `Pillow`), **no** `Textile`,
      `Font` ni `Material property`
- [ ] Si sale una de esas palabras abstractas, la app sigue pidiendo
      `LABEL_DETECTION`: revisa el `PostData` del componente `Web1`

### Test 4: Vision no reconoce nada

- [ ] Haz una foto a una pared lisa o al techo
- [ ] `OBJECT_LOCALIZATION` devolverá la lista **vacía**
- [ ] La app debe mostrar `No he reconocido nada` y **no cerrarse**
- [ ] Si la app se cierra, falta la comprobación de "lista vacía" del paso 3.3

---

## Fase 5.3: Flujo completo (Arduino + App)

### Test 1: Envío simple desde app a Arduino

**Precondiciones:**
- [ ] Arduino corriendo, mostrando IP
- [ ] App abierta en móvil en la misma red WiFi
- [ ] IP del Arduino escrita en `TxtIPArduino` de la app

**Pasos:**
1. [ ] Toma una foto con el móvil
2. [ ] Vision identifica objeto (espera a que aparezca la etiqueta)
3. [ ] Presiona "Enviar a Arduino"
4. [ ] En el Monitor Serie aparece: `--- Cliente conectado ---`, `Texto recibido: ...`
5. [ ] **La LCD del Arduino muestra el nombre del objeto**

**Resultado esperado:**
- App: `✓ Enviado a Arduino`
- Monitor Serie: el nombre impreso
- LCD: IP arriba, nombre del objeto abajo

### Test 2: Múltiples objetos

Repite Test 1 con diferentes objetos:

- [ ] Foto de un gato → LCD: "Cat"
- [ ] Foto de un teléfono → LCD: "Mobile phone" (truncado a 16: "Mobile phone")
- [ ] Foto de una taza → LCD: "Mug" o "Cup"
- [ ] Foto de un libro → LCD: "Book"
- [ ] Foto de una planta → LCD: "Plant"

**Tabla de resultados:**

| Objeto | Nombre de Vision | En LCD | ¿Correcto? |
|---|---|---|---|
| ... | ... | ... | ☐ |
| ... | ... | ... | ☐ |
| ... | ... | ... | ☐ |

### Test 3: Casos límite — Nombre largo

Busca un objeto cuyo nombre pase de 16 letras (`Chest of drawers` = 16, `Home appliance`, `Musical instrument`):

- [ ] En la app se ve el nombre completo: ej. `Musical instrument`
- [ ] En la LCD se corta a 16: `Musical instrum`
- [ ] No salen caracteres raros ni se apaga la pantalla

### Test 4: Casos límite — Nombres con espacio

Vision devuelve muchos nombres de dos palabras (`Teddy bear`, `Coffee cup`, `Mobile phone`):

- [ ] La LCD muestra las **dos palabras**, no solo la primera
- [ ] Si solo ves la primera palabra, falta el `Web1.UriEncode` en la app

### Test 5: Latencia (tiempo total)

Mide el tiempo desde que tomas la foto hasta que aparece en la LCD:

- [ ] Toma foto
- [ ] Cronómetro comienza
- [ ] Espera a que aparezca en LCD
- [ ] Anota el tiempo total

**Tiempo típico esperado:** 3-8 segundos
- ~1 seg: captura de foto
- ~1-2 seg: envío a Vision API
- ~1-2 seg: procesamiento Vision
- ~0.5 seg: envío a Arduino
- ~0.5 seg: actualización LCD

| Intento | Tiempo | Notas |
|---|---|---|
| 1 | __ s | |
| 2 | __ s | |
| 3 | __ s | |
| Media | __ s | |

---

## Fase 5.4: Robustez y errores

### Test 1: Se pierde la conexión WiFi

El sketch comprueba la WiFi cada 5 segundos y se reconecta solo.

- [ ] Arduino funcionando y mostrando la IP
- [ ] Apaga el WiFi del router (o aléjate mucho con la placa)
- [ ] En menos de ~10 segundos, la LCD muestra `Recuperando WiFi`
- [ ] El Monitor Serie imprime: `Se ha perdido la WiFi. Reconectando...`
- [ ] Vuelve a encender el router
- [ ] La placa se reconecta sola y **vuelve a mostrar la IP**
- [ ] Repite el Test 2 de la fase 5.1: debe responder igual que antes

> ⚠️ Al reconectar, el router **puede darle una IP distinta**.
> Mira siempre la pantalla antes de dar por hecho que es la misma.

### Test 2: IP incorrecta en la app

- [ ] Escribe una IP que no existe: `192.168.1.240`
- [ ] Haz una foto y pulsa "Enviar a Arduino"
- [ ] La app muestra un error o se queda esperando, pero **no se cierra**
- [ ] Vuelve a poner la IP buena y comprueba que funciona otra vez

### Test 3: Se apaga el Arduino

- [ ] Con todo funcionando, desenchufa el Arduino
- [ ] Envía desde la app: debe salir un error de conexión
- [ ] Enchufa el Arduino y espera a que muestre la IP
- [ ] **Comprueba que la IP es la misma que antes** (a veces cambia)
- [ ] Envía otra vez: debe funcionar

---

## Fase 5.5: Documentación y video

### Test 1: Documentar flujo con screenshots

- [ ] Foto de la LCD mostrando la IP al arrancar
- [ ] Foto de la LCD con un objeto detectado (`Cat`, `Teddy bear`…)
- [ ] Captura de la app con la foto y el nombre reconocido

> ⚠️ Antes de subir las fotos: comprueba que **no se vea tu IP pública ni la clave
> de la API** en ninguna captura. La IP local (`192.168.x.x`) no pasa nada.

### Test 2: Video de demostración (opcional)

- [ ] Toma video: tomar foto → detección en app → aparece en LCD
- [ ] Duración: ~1 minuto
- [ ] Audio con explicación breve: "Foto de un gato, Vision detecta 'Cat', Arduino muestra 'Cat' en la LCD"

### Test 3: Documentar en README

Actualizar README con:
- [ ] Foto del setup (Arduino + LCD)
- [ ] Screenshot de app
- [ ] Etiquetas probadas con éxito
- [ ] Latencia medida
- [ ] Instrucciones para reproducir

---

## Resolución de problemas

### ❌ Arduino no se conecta a WiFi

**Síntomas:** el Monitor Serie dice `No se ha podido conectar a la WiFi.` y la LCD muestra `ERROR WiFi`

**Comprobaciones:**
- [ ] `SSID_WIFI` y `CLAVE_WIFI` bien escritos en el sketch (ojo a mayúsculas)
- [ ] La red es de **2,4 GHz** — el Uno R4 WiFi **no funciona en 5 GHz**
- [ ] El router está encendido y otro aparato se conecta bien a esa red
- [ ] El nombre de la red no lleva acentos ni caracteres raros

**Solución:**
1. Revisa `SSID_WIFI` y `CLAVE_WIFI` en `fase4_arduino_http_server.ino`
2. Vuelve a subir el sketch
3. Abre el Monitor Serie **a 115200 baudios** (si pones otra velocidad, verás símbolos raros)
4. Si tu router emite 2,4 y 5 GHz con el mismo nombre, entra en su configuración
   y sepáralos temporalmente para asegurarte de que la placa usa la de 2,4

---

### ❌ La LCD muestra la IP `0.0.0.0`

**Síntomas:** la placa dice que está conectada, pero la dirección que aparece es `0.0.0.0`

Esto significa: **la placa ha entrado en la red, pero el router todavía no le ha dado
una dirección**. Son dos pasos distintos:

```
1. Entrar en la red WiFi   →  WiFi.status() == WL_CONNECTED
2. Recibir una dirección   →  WiFi.localIP() deja de ser 0.0.0.0   ← este falta
```

Es como entrar en un hotel: primero cruzas la puerta, y **después** en recepción te dan
el número de habitación. Si preguntas el número nada más entrar, todavía no lo tienen.
Ese reparto de direcciones se llama **DHCP**.

**Comprobaciones:**
- [ ] ¿La IP aparece bien pasados unos segundos? Entonces solo era falta de paciencia
- [ ] ¿Se queda en `0.0.0.0` para siempre? Entonces el router no está repartiendo IPs
- [ ] ¿El router tiene un límite de aparatos conectados o filtro por MAC?
- [ ] ¿Hay algún modo "invitados" o "aislamiento de dispositivos" activado?

**Solución:**
1. El sketch ya **espera hasta 10 segundos** a que llegue la IP (`ESPERA_DHCP_MS`).
   Si tu router es lento, sube ese número
2. Si tras esos 10 segundos sigue sin IP, la LCD muestra `Sin IP: router?` y el
   Monitor Serie lo explica
3. Reinicia el router y vuelve a probar
4. Comprueba en la configuración del router que el **DHCP está activado**
5. Si nada funciona, actualiza el **firmware del módulo WiFi** de la placa:
   Arduino IDE → *Herramientas* → *Firmware Updater*

---

### ❌ App no recibe respuesta de Vision

**Síntomas:** App muestra "Foto capturada, enviando a Vision..." pero no continúa

**Checklist:**
- [ ] Clave de API es correcta
- [ ] Cloud Vision API está habilitada en Google Cloud
- [ ] Móvil tiene conexión a Internet (no es red local)

**Solución:**
1. Abre Google Cloud Console y comprueba que **Cloud Vision API** está activada
2. Comprueba que la **facturación** está activada (Google la exige aunque no pagues nada)
3. Prueba la misma clave desde el ordenador con la página de la Fase 3: si ahí tampoco
   funciona, el problema es la clave, no la app
4. Mira el bloque `Web1.GotText`: el `responseCode` te dice qué pasa
   (`403` = clave mal o API sin activar, `400` = el JSON está mal formado)

---

### ❌ LCD no muestra el texto desde la app

**Síntomas:** App dice "Enviado a Arduino" pero LCD no cambia

**Checklist:**
- [ ] Arduino y móvil están en la misma red WiFi
- [ ] IP en la app es correcta (comparar con Serial Monitor de Arduino)
- [ ] Firewall del router no bloquea puerto 80

**Solución:**
1. Desde PC en la misma red, intenta: `curl http://<IP>/?texto=Test`
2. Si funciona desde PC pero no desde app: problema de red
3. Si no funciona desde PC: problema de Arduino

---

### ❌ LCD muestra caracteres raros

**Síntomas:** la LCD parpadea o muestra símbolos sin sentido

Hay **dos causas distintas** y conviene distinguirlas:

**Causa A — problema eléctrico** (símbolos raros en *toda* la pantalla, o cuadros negros)
- [ ] Cables bien conectados (revisa el esquema de la Fase 1)
- [ ] Potenciómetro ajustado (gíralo despacio hasta que se lea bien)
- [ ] Alimentación estable de 5 V

**Causa B — problema de texto** (el resto se lee bien, pero un nombre concreto sale mal)
- [ ] Es una letra con tilde o `ñ` que no se ha convertido
- [ ] Mira el Monitor Serie: si ahí se ve bien pero en la LCD no, es la pantalla
- [ ] Si en el Monitor Serie ya sale mal, el fallo está en `quitarAcentos()` del sketch

---

### ❌ La placa deja de responder y hay que reiniciarla

**Síntomas:** funcionaba, y de pronto no contesta a nada (ni a `curl` ni a la app)

**Comprobaciones:**
- [ ] ¿El Monitor Serie sigue imprimiendo algo? Si está mudo, la placa está colgada
- [ ] ¿Ha entrado alguna conexión rara justo antes? (otro aparato de la red, un escáner)

**Solución:**
El sketch tiene un *timeout* de 2 segundos (`TIMEOUT_PETICION_MS`) precisamente para
esto. Si aun así se cuelga, ejecuta el **Test 6 de la fase 5.1** para comprobar que el
timeout funciona.

---

## Checklist final

- [ ] El Arduino se conecta a la WiFi
- [ ] La IP se ve en la LCD y se queda ahí
- [ ] El servidor responde a `curl` desde el ordenador
- [ ] Aguanta una conexión vacía sin colgarse (timeout)
- [ ] La app hace fotos
- [ ] Vision reconoce objetos de verdad (no `Font` ni `Material property`)
- [ ] La app envía el nombre al Arduino
- [ ] La LCD muestra el nombre
- [ ] Funciona con varios objetos distintos
- [ ] Los nombres largos se cortan a 16 sin romper nada
- [ ] Los nombres de dos palabras llegan enteros
- [ ] Los acentos se quitan bien
- [ ] Si se cae la WiFi, la placa se reconecta sola
- [ ] Si Vision no reconoce nada, la app no se cierra
- [ ] Tiempo total aceptable (3–8 segundos)
- [ ] Documentado con fotos o vídeo

---

## ✅ Proyecto completado

Si todo esto funciona, ¡felicidades! Habéis construido:

1. ✓ Una foto hecha con el móvil
2. ✓ Una IA que reconoce lo que hay en la foto
3. ✓ Un aparato propio que lo escribe en una pantalla

**Mejoras futuras:**
- Guardar un histórico de los objetos detectados
- Enviar también el `score` (el porcentaje de confianza) y mostrarlo
- Botón en la app para elegir entre `OBJECT_LOCALIZATION` y `LABEL_DETECTION`
- Añadir Arduino IoT Cloud como camino alternativo (opcional, ver README)
