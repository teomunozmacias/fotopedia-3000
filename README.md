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
- [ ] **Fase 2 – Arduino Cloud**: crear la "Thing", la variable de texto, y el sketch que sincroniza y actualiza la LCD automáticamente.
- [ ] **Fase 3 – Google Vision API**: crear cuenta/clave y probar que identifica correctamente una foto.
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

Dar de alta la placa en Arduino IoT Cloud y crear la variable de texto que la Fase 2 usará para actualizar la pantalla de forma remota.
