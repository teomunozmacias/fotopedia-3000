# Fase 4 — La app del móvil (MIT App Inventor)

Vamos a construir la app en **dos etapas**, no de golpe.

| Etapa | Qué hace | Cuánto cuesta |
|---|---|---|
| **1 — App mini** | Escribes una palabra a mano y sale en la LCD | ~15 minutos |
| **2 — App completa** | Le añades la cámara y la IA | Un rato más |

### ¿Por qué en dos etapas?

Si montas la app entera de una vez y al final no funciona, el fallo puede estar en
**cuatro sitios distintos** (la red, la foto, el Base64 o la llamada a Google) y no hay
forma de saber en cuál.

Con la etapa 1 compruebas lo único que todavía no hemos probado: **que el móvil consigue
hablar con la placa**. Si eso funciona, ya nunca más tendrás que dudar de ello.

Y además se ve algo funcionando el primer día, que motiva bastante más que pelearse una
tarde entera con extensiones antes de que se encienda nada.

> ℹ️ Los nombres de los bloques van **en inglés**, igual que aparecen en el editor.

---
---

# ETAPA 1 — App mini

**Objetivo**: escribir `Cat` en el móvil y que aparezca en la pantalla del Arduino.

Sin cámara. Sin IA. Sin extensiones. Solo el móvil hablando con la placa.

## 1.1 Crear el proyecto

1. Entra en [MIT App Inventor](https://appinventor.mit.edu) y pulsa **Create Apps!**
2. Inicia sesión con tu cuenta de Google
3. **Projects → Start new project** → nombre: `Fotopedia3000`

> El nombre no puede llevar espacios ni acentos.

## 1.2 La pantalla (pestaña Designer)

Arrastra estos componentes desde el panel **Palette** de la izquierda:

| Dónde está | Componente | Renómbralo a | Propiedades |
|---|---|---|---|
| User Interface | Label | `LblTituloIP` | Text: `IP del Arduino:` |
| User Interface | TextBox | `TxtIP` | Hint: `192.168.1.50` |
| User Interface | Label | `LblTituloTexto` | Text: `Palabra a enviar:` |
| User Interface | TextBox | `TxtPalabra` | Text: `Cat` |
| User Interface | Button | `BtnEnviar` | Text: `Enviar a la pantalla` |
| User Interface | Label | `LblEstado` | Text: *(vacío)* |
| Connectivity | **Web** | `Web1` | *(no se ve en la pantalla)* |

Para cambiar el nombre de un componente: selecciónalo y pulsa **Rename** debajo del
panel **Components**.

Debería quedarte algo así:

```
┌─────────────────────────┐
│ IP del Arduino:         │
│ [192.168.1.50         ] │
│                         │
│ Palabra a enviar:       │
│ [Cat                  ] │
│                         │
│  [ Enviar a la pantalla]│
│                         │
│ Estado: ...             │
└─────────────────────────┘
```

## 1.3 La lógica (pestaña Blocks)

Solo hacen falta **dos bloques**.

### Bloque 1 — Al pulsar el botón

```
when BtnEnviar.Click
do  set Web1.Url to join
        "http://"
        TxtIP.Text
        "/?texto="
        call Web1.UriEncode  text  TxtPalabra.Text

    call Web1.Get

    set LblEstado.Text to "Enviando..."
```

**Qué hace cada trozo:**

| Trozo | Para qué sirve |
|---|---|
| `join` | Pega textos uno detrás de otro, como un tren de vagones |
| `TxtIP.Text` | La dirección que sale en la pantalla del Arduino |
| `/?texto=` | Así es como se le pasa un dato a una dirección web |
| `Web1.UriEncode` | Disfraza los espacios y las tildes para que viajen bien |
| `Web1.Get` | Manda la petición |

> ⚠️ **No te saltes el `UriEncode`.** Sin él, `Teddy bear` llegaría cortado y en la
> pantalla solo verías `Teddy`. Es un fallo muy típico y difícil de encontrar.

### Bloque 2 — Cuando el Arduino contesta

```
when Web1.GotText (url, responseCode, responseType, responseContent)
do  if  responseCode = 200
    then set LblEstado.Text to "Enviado correctamente"
    else set LblEstado.Text to join "Error: " responseCode
```

`responseCode` es el número que devuelve el Arduino: **200** significa "todo bien".

## 1.4 Probarlo

1. **Enciende el Arduino** y apunta la IP que sale en la pantalla
2. Móvil y Arduino, **en la misma red WiFi de 2,4 GHz**
3. En App Inventor: **Connect → AI Companion**, y escanea el código QR con la app
   *MIT AI2 Companion* del móvil
4. Escribe la IP y la palabra `Cat`
5. Pulsa el botón

### ✅ Si funciona

La LCD del Arduino muestra `Cat` en la línea de abajo y el móvil dice
`Enviado correctamente`.

**Enhorabuena: el móvil y la placa ya se hablan.** Esa es la parte que solía dar
problemas, y ya está resuelta. Prueba también con `Teddy bear` (con espacio) para
comprobar que el `UriEncode` hace su trabajo.

### ❌ Si no funciona

| Lo que ves | Qué suele ser |
|---|---|
| Se queda en `Enviando...` para siempre | El móvil no llega a la placa: mira lo de abajo |
| `Error: 400` | Llega, pero sin el parámetro. Revisa que pusiste `/?texto=` |
| En la LCD solo la primera palabra | Falta el `UriEncode` |
| `Error: 403` o similar | La IP escrita no es la del Arduino |

**Si no llega nada**, comprueba en este orden:

1. ¿La IP de la app es **exactamente** la de la pantalla? (cambia al reiniciar el router)
2. ¿El móvil está en el **WiFi de casa** y no en datos móviles? Desactiva los datos
3. Desde el ordenador, en la misma red:
   `curl "http://LA_IP/?texto=Prueba"` — si desde el PC sí funciona y desde el móvil no,
   el problema es del móvil o del router, no del Arduino
4. Algunos routers tienen **"aislamiento de clientes"** activado, que impide que los
   aparatos de la casa se hablen entre ellos. Búscalo en la configuración del router

**No sigas a la etapa 2 hasta que esto funcione.**

---
---

# ETAPA 2 — App completa

Ahora le añadimos la cámara y la IA a la app que ya funciona.

El camino que recorre la foto es este:

```
📷 foto  →  hacerla pequeña  →  convertir a Base64  →  Google Vision
                                                            ↓
                     LCD  ←  Arduino  ←  enviar el nombre  ←┘
```

## 2.1 Antes de empezar: dos avisos honestos

App Inventor **no sabe hacer de serie** dos de esas cosas:

| Lo que hace falta | ¿Viene incluido? | Solución |
|---|---|---|
| Convertir la foto a **Base64** | ❌ No | Instalar una extensión |
| **Reducir** el tamaño de la foto | ❌ No | Truco del Canvas, o extensión |

> 📌 **Corrección**: una versión anterior de este documento decía que bastaba con bajar la
> propiedad `Quality` del componente `Camera`. **Eso no es cierto**: el componente
> `Camera` de App Inventor no tiene esa propiedad. Lo dejamos escrito para que no se
> repita el error.

### ¿Por qué hay que hacer la foto pequeña?

Una foto del móvil ocupa **entre 3 y 8 MB**. Al convertirla a Base64 crece otro 33 %.
Mandar todo eso por WiFi es lento y consume cuota de Google sin ninguna necesidad:
con **640 píxeles de ancho, Vision reconoce igual de bien**.

En la página de la Fase 3 podéis verlo: al arrastrar una foto os dice cuánto adelgaza.

## 2.2 Instalar la extensión de Base64

1. En la pestaña **Designer**, panel **Palette** de la izquierda, baja del todo:
   **Extension → Import extension → From my computer**
2. Necesitas un fichero `.aix`. Búscalo en la
   [comunidad de App Inventor](https://community.appinventor.mit.edu) con
   *"base64 extension"* o *"file extension"*
3. Arrástralo a la pantalla: aparecerá abajo como componente **no visible**

> ⚠️ Las extensiones `.aix` las hace gente de la comunidad, **no el MIT**.
> Descárgalas solo del foro oficial y no de sitios raros.

> 🔍 **Los nombres de los bloques cambian según la extensión que instales.**
> Busca uno que diga algo como `Base64Encode`, `EncodeFile` o `FileToBase64`.
> Ajusta los bloques de abajo al nombre que veas en tu editor.

## 2.3 Componentes nuevos

Añade a lo que ya tenías:

| Dónde está | Componente | Renómbralo a | Notas |
|---|---|---|---|
| Media | Camera | `Camera1` | |
| User Interface | Button | `BtnFoto` | Text: `Hacer foto` |
| User Interface | Image | `ImgFoto` | Para ver la foto |
| Drawing and Animation | Canvas | `Canvas1` | Width: `640`, Height: `480` |
| Connectivity | **Web** | `Web2` | ⚠️ **Otro Web distinto**, para Google |
| *(el que instalaste)* | Base64 | `Base64` | |

> ⚠️ **Dos componentes Web, y esto importa.** `Web1` habla con el Arduino y `Web2` habla
> con Google. Si usas el mismo para las dos cosas, las respuestas se mezclan en el mismo
> bloque `GotText` y no hay quien lo entienda.

Pon el `Canvas1` con **Visible = false** cuando todo funcione: solo lo usamos para
encoger la foto, no hace falta verlo.

## 2.4 Variables

En **Blocks**, apartado **Variables**, crea estas tres:

```
initialize global rutaFoto      to ""
initialize global base64Foto    to ""
initialize global nombreObjeto  to ""
```

## 2.5 Hacer la foto

```
when BtnFoto.Click
do  call Camera1.TakePicture
    set LblEstado.Text to "Abriendo la camara..."
```

```
when Camera1.AfterPicture (image)
do  set global rutaFoto to image
    set ImgFoto.Picture to image

    // Encogerla usando el Canvas
    set Canvas1.BackgroundImage to image
    set global rutaFoto to call Canvas1.SaveAs  fileName "pequena.jpg"

    // Convertirla a Base64  (ajusta el nombre del bloque a tu extension)
    set global base64Foto to call Base64.EncodeFile  path (get global rutaFoto)

    set LblEstado.Text to "Preguntando a la IA..."
    call AnalizarConVision
```

## 2.6 Llamar a Google Vision

Crea un **procedimiento** (apartado **Procedures → to procedure do**) y llámalo
`AnalizarConVision`:

```
to AnalizarConVision
do  set Web2.Url to join
        "https://vision.googleapis.com/v1/images:annotate?key="
        "AQUI_TU_CLAVE_DE_API"

    set Web2.RequestHeaders to
        make a list (make a list "Content-Type" "application/json")

    call Web2.PostText  text (join
        "{\"requests\":[{\"image\":{\"content\":\""
        get global base64Foto
        "\"},\"features\":[{\"type\":\"OBJECT_LOCALIZATION\",\"maxResults\":5}]}]}"
    )
```

> 💡 En los bloques de texto de App Inventor las comillas se escriben **tal cual**, no
> hace falta ponerles nada delante. Los `\"` de arriba son solo para que se lea bien en
> este documento.

**Por qué `OBJECT_LOCALIZATION`**: devuelve objetos de verdad (`Cat`, `Chair`,
`Teddy bear`) en vez de etiquetas abstractas (`Font`, `Material property`).
Lo comprobamos con nuestras propias fotos en la Fase 3; está explicado en el README.

## 2.7 Leer la respuesta de Google

Aquí está la parte más delicada. La respuesta viene así:

```json
{
  "responses": [
    { "localizedObjectAnnotations": [ { "name": "Cat", "score": 0.94 } ] }
  ]
}
```

Hay que ir entrando capa por capa: primero `responses`, luego el primer elemento, luego
`localizedObjectAnnotations`, y por fin el `name` del primero.

```
when Web2.GotText (url, responseCode, responseType, responseContent)
do  if  responseCode ≠ 200
    then set LblEstado.Text to join "Error de Google: " responseCode
    else
        set global datos to
            call Web2.JsonTextDecodeWithDictionaries  jsonText responseContent

        set global respuestas to
            get value for key "responses" in dictionary (get global datos)
            or if not found (create empty list)

        set global objetos to
            get value for key "localizedObjectAnnotations"
            in dictionary (select list item  list (get global respuestas)  index 1)
            or if not found (create empty list)

        if  is list empty? (get global objetos)
        then set LblEstado.Text to "No he reconocido nada. Prueba a acercarte."
        else
            set global nombreObjeto to
                get value for key "name"
                in dictionary (select list item  list (get global objetos)  index 1)
                or if not found ""

            set LblEstado.Text to join "Es un/a: " (get global nombreObjeto)
            set TxtPalabra.Text to get global nombreObjeto
            call EnviarAlArduino
```

> ⚠️ **La comprobación `is list empty?` no es opcional.** Si haces una foto a una pared
> lisa o al cielo, Vision no encuentra ningún objeto y devuelve la lista vacía. Si lees
> el primer elemento de una lista vacía, **la app se cierra de golpe**.

## 2.8 Reutilizar el envío de la etapa 1

Convierte el bloque del botón de la etapa 1 en un procedimiento, para poder llamarlo
también desde aquí:

```
to EnviarAlArduino
do  set Web1.Url to join
        "http://"
        TxtIP.Text
        "/?texto="
        call Web1.UriEncode  text (get global nombreObjeto)

    call Web1.Get
```

Y deja el botón manual llamando al mismo sitio (viene bien para hacer pruebas sin gastar
cuota de Google):

```
when BtnEnviar.Click
do  set global nombreObjeto to TxtPalabra.Text
    call EnviarAlArduino
```

---

## 2.9 Dónde va la clave de API

En este proyecto la clave va **escrita dentro de la app**, porque es la única forma
sencilla sin montar un servidor. Hay que asumir lo que eso significa:

- La app es **solo para vosotros**. No la publiques en Google Play
- En Google Cloud, restringe la clave **solo a Cloud Vision API**
- Pon una **alerta de presupuesto**, por si acaso
- Si algún día se filtra: **bórrala y crea otra**, se hace en dos clics

---

## Resolución de problemas

### La app se cierra sola al hacer la foto

Casi siempre es la conversión a Base64 con una foto demasiado grande: se queda sin
memoria. Comprueba que el `Canvas1` está encogiendo de verdad (ponlo visible un momento
y mira si sale la foto dentro).

### `Error de Google: 403`

La clave está mal, **Cloud Vision API** no está activada en tu proyecto, o falta activar
la facturación. Pruébala primero en la página de la Fase 3: si allí tampoco funciona,
el problema es de la clave y no de la app.

### `Error de Google: 400`

El JSON va mal formado. Casi siempre es que `base64Foto` está vacío, o que en el texto
del `PostText` falta una comilla o una llave.

### Google responde bien pero la LCD no cambia

Entonces el fallo está entre la app y el Arduino, no en la IA. Vuelve a la **etapa 1**:
escribe una palabra a mano y pulsa el botón. Si tampoco va, el problema es de red.

### En la LCD solo sale la primera palabra

Falta el `Web1.UriEncode`.

### Sale algo abstracto tipo `Textile`

La app está pidiendo `LABEL_DETECTION` en vez de `OBJECT_LOCALIZATION`. Revisa el texto
del `PostText` del apartado 2.6.

---

## Ideas para después

- Enseñar también el `score` (el porcentaje de acierto) en la pantalla
- Guardar una lista de todo lo que habéis reconocido
- Un botón para elegir entre `OBJECT_LOCALIZATION` y `LABEL_DETECTION` y comparar
- Un sonido cuando el Arduino confirma que lo ha recibido

---

## Enlaces

- [Tutoriales de MIT App Inventor](https://appinventor.mit.edu/explore/ai2/tutorials)
- [Foro de la comunidad (extensiones)](https://community.appinventor.mit.edu)
- [Documentación de Google Vision](https://cloud.google.com/vision/docs)
- [Librería WiFiS3 del Arduino](https://docs.arduino.cc/libraries/wifi-s3/)
