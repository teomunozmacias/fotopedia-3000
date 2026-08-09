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

- [ ] Arduino IDE con librerías: `LiquidCrystal`, `WiFiS3`
- [ ] Sketch `fase4_arduino_http_server.ino` cargado en Arduino
- [ ] MIT App Inventor con la app instalada en el móvil
- [ ] Clave de Google Vision API generada y guardada

### Red

- [ ] Arduino y móvil conectados a la **misma red WiFi**
- [ ] IP del Arduino anotada (aparece en la LCD y en Serial Monitor)

---

## Fase 5.1: Prueba de hardware (Arduino solo)

### Test 1: LCD muestra IP correctamente

- [ ] Cargue el sketch `fase4_arduino_http_server.ino`
- [ ] Abra Serial Monitor (115200 baud)
- [ ] Verifique que Arduino imprime: "WiFi conectado. IP: 192.168.X.X"
- [ ] Verifique que la LCD muestra: "IP: 192.168.X.X" (o similar) durante 3 segundos
- [ ] Tras 3 segundos, LCD muestra: "Escuchando..." (o texto anterior si hay petición)

### Test 2: Servidor HTTP responde localmente (curl desde PC)

```bash
# En una terminal (PC o Mac) en la misma red:
curl "http://192.168.X.X/?texto=TestHola"
```

Resultado esperado:
- [ ] Respuesta: `HTTP 200 OK`
- [ ] LCD muestra: "Testshola" (truncado a 16 caracteres)
- [ ] Serial Monitor imprime: "LCD actualizada: Testshola"

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
curl "http://192.168.X.X/?texto=EstaEsUnaFraseQueTieneMusDe16Caracteres"
```

Resultado esperado:
- [ ] LCD muestra solo: "EstaEsUnaFraseQu" (16 caracteres exactos)
- [ ] El resto se corta sin error

### Test 5: Normalización de acentos

```bash
curl "http://192.168.X.X/?texto=Café"
# o con URL encoding:
curl "http://192.168.X.X/?texto=Caf%C3%A9"
```

Resultado esperado:
- [ ] LCD muestra: "Cafe" (sin acento)

---

## Fase 5.2: Prueba de Vision API (app solo, sin Arduino)

### Test 1: App toma foto

- [ ] Abre la app en el móvil
- [ ] Presiona "📷 Tomar foto"
- [ ] Espera a que la cámara abre, toma una foto de un objeto
- [ ] Verifica que la imagen aparece en la app

### Test 2: Vision reconoce la foto

- [ ] La app envía la foto a Google Vision
- [ ] Espera respuesta (15-30 segundos típicamente)
- [ ] Verifica que aparece una etiqueta: ej. "Cat", "Toy", "Plant"
- [ ] Si no aparece nada: comprobar que la clave de API es correcta

### Test 3: Etiqueta se muestra en app

- [ ] Verifica que el campo "Detectado: ..." muestra la etiqueta
- [ ] Ejemplos esperados: "Cat", "Dog", "Toy", "Plant", "Food", etc.

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
4. [ ] Verifica en Serial Monitor: "Cliente conectado", "Parámetro recibido: ..."
5. [ ] **Verifica que la LCD de Arduino muestra la etiqueta detectada**

**Resultado esperado:**
- App: "✓ Enviado a Arduino"
- Arduino Serial: etiqueta impresa
- Arduino LCD: muestra etiqueta en línea 2

### Test 2: Múltiples objetos

Repite Test 1 con diferentes objetos:

- [ ] Foto de un gato → LCD: "Cat"
- [ ] Foto de un teléfono → LCD: "Mobile phone" (truncado a 16: "Mobile phone")
- [ ] Foto de una taza → LCD: "Mug" o "Cup"
- [ ] Foto de un libro → LCD: "Book"
- [ ] Foto de una planta → LCD: "Plant"

**Tabla de resultados:**

| Objeto | Etiqueta Vision | En LCD | ¿Correcto? |
|---|---|---|---|
| ... | ... | ... | ☐ |
| ... | ... | ... | ☐ |
| ... | ... | ... | ☐ |

### Test 3: Casos límite — Texto largo

Toma foto de algo que Vision etiquete con **texto largo** (ej. "Furniture", "Electronic Device", "Clothing"):

- [ ] Etiqueta en app: ej. "Electronic Device"
- [ ] LCD: trunca a 16 caracteres → "Electronic Devic"
- [ ] Sin errores ni caracteres raros
- [ ] Serial Monitor no muestra fallos

### Test 4: Casos límite — Acentos

Si usas idioma español y tomas foto de algo con nombre acentuado (ej. "Sofá", "Lámpara"):

- [ ] Vision devuelve: "Sofa", "Lampara" (sin acentos, en inglés típicamente)
- [ ] LCD muestra correctamente
- [ ] Sin caracteres corruptos

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

### Test 1: Conexión WiFi se pierde

- [ ] App y Arduino conectados
- [ ] Apaga WiFi del router (o desconecta Arduino)
- [ ] Espera 30 segundos
- [ ] Reactiva WiFi
- [ ] Verifica que todo sigue funcionando

### Test 2: Vision no reconoce

Toma una foto de algo abstracto (luz, pared blanca, cielo):

- [ ] Vision devuelve etiqueta genérica o vacía
- [ ] App no crashea
- [ ] Puedes volver a intentar

### Test 3: IP incorrecta

- [ ] Escribe IP incorrecta en la app: ej. "192.168.1.999"
- [ ] Toma foto y envía a Arduino
- [ ] App debe mostrar timeout o error (no debe crashear)

### Test 4: Arduino cae

- [ ] App enviando peticiones normalmente
- [ ] Apaga Arduino (desenchufa)
- [ ] App debe mostrar error de conexión
- [ ] Enciende Arduino de nuevo
- [ ] Debe volver a funcionar

---

## Fase 5.5: Documentación y video

### Test 1: Documentar flujo con screenshots

- [ ] Foto de la LCD mostrando "IP: 192.168.X.X"
- [ ] Foto de la LCD con objeto detectado ("Cat", "Toy", etc.)
- [ ] Screenshot de la app mostrando la foto y la etiqueta

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

**Síntomas:** Serial dice "ERROR: No se pudo conectar a WiFi"

**Checklist:**
- [ ] SSID y contraseña correctos en el sketch
- [ ] Red WiFi es 2.4 GHz (Arduino R4 soporta 2.4 GHz, no 5 GHz)
- [ ] Router está encendido
- [ ] Otro dispositivo puede conectarse a esa red

**Solución:**
1. Verifica SSID y contraseña en `fase4_arduino_http_server.ino`
2. Recompila y carga
3. Abre Serial Monitor
4. Apaga WiFi del router, espera 5 seg, enciende
5. Arduino debe conectarse

---

### ❌ App no recibe respuesta de Vision

**Síntomas:** App muestra "Foto capturada, enviando a Vision..." pero no continúa

**Checklist:**
- [ ] Clave de API es correcta
- [ ] Cloud Vision API está habilitada en Google Cloud
- [ ] Móvil tiene conexión a Internet (no es red local)

**Solución:**
1. Abre Google Cloud Console
2. Verifica que Cloud Vision API está enabled
3. Copia la clave correcta
4. Hardcodea en la app temporalmente para probar

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

**Síntomas:** LCD parpadea o muestra caracteres ininteligibles

**Checklist:**
- [ ] Cables LCD bien conectados (revisar esquema)
- [ ] Potenciómetro bien ajustado (girar para cambiar contraste)
- [ ] Alimentación estable (5V sin caídas)

**Solución:**
1. Ajusta potenciómetro lentamente hasta que se lea bien
2. Revisa conexión de cables
3. Prueba sin WiFi (apagar WiFi de Arduino) para ver si hay interferencia

---

## Checklist final

- [ ] Arduino conecta a WiFi
- [ ] Arduino muestra IP en LCD
- [ ] Servidor HTTP responde localmente
- [ ] App toma fotos
- [ ] App reconoce con Vision
- [ ] App envía texto a Arduino
- [ ] LCD muestra texto de Arduino
- [ ] Funciona con múltiples objetos
- [ ] Texto se trunca correctamente
- [ ] Acentos se normalizan
- [ ] Latencia es aceptable (~3-8 seg)
- [ ] Errores se manejan sin crashear
- [ ] Documentado con fotos/video

---

## ✅ Proyecto completado

Si todo funciona, ¡felicidades! Haces:

1. ✓ Foto con móvil
2. ✓ IA reconoce objeto
3. ✓ Nombre aparece en LCD

**Mejoras futuras:**
- Sincronizar con Arduino Cloud (Fase 6)
- Histórico de detecciones
- Traducción a otros idiomas
- Interfaz para cambiar opciones de Vision (LABEL_DETECTION vs OBJECT_LOCALIZATION)
