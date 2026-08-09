/*
  Fotopedia 3000 - Fase 4
  Servidor HTTP local en Arduino Uno R4 WiFi

  Funcionamiento:
  1. Se conecta a tu red WiFi local
  2. Levanta un servidor en puerto 80
  3. Espera peticiones GET: http://<ip>/?texto=nombreObjeto
  4. Parsea la URL, decodifica caracteres especiales
  5. Trunca a 16 caracteres y elimina acentos (LCD no los soporta)
  6. Muestra en la LCD

  Configuración:
  - Cambiar SSID y PASSWORD con tus datos
  - Los pines de la LCD son los mismos que en Fase 1
*/

#include <WiFiS3.h>
#include <LiquidCrystal.h>

// ===== CONFIGURACIÓN =====
const char* ssid = "TU_RED_WIFI";           // Cambiar a tu SSID
const char* password = "TU_CONTRASEÑA";     // Cambiar a tu contraseña
const int PUERTO = 80;

// Pines LCD: RS, E, D4, D5, D6, D7 (mismo que Fase 1)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

WiFiServer server(PUERTO);
IPAddress ipLocal;

// Buffer para el texto actual en la LCD
char textoActual[17] = "Esperando...";  // Max 16 chars + null terminator

// ===== FUNCIÓN: Decodificar URL =====
// Convierte %20 → espacio, + → espacio, etc.
String decodificarURL(String texto) {
  String resultado = "";
  for (int i = 0; i < texto.length(); i++) {
    char c = texto[i];
    if (c == '+') {
      resultado += ' ';
    } else if (c == '%' && i + 2 < texto.length()) {
      // Convertir %XX a carácter
      char hex[3];
      hex[0] = texto[i + 1];
      hex[1] = texto[i + 2];
      hex[2] = '\0';
      int code = strtol(hex, NULL, 16);
      resultado += (char)code;
      i += 2;
    } else {
      resultado += c;
    }
  }
  return resultado;
}

// ===== FUNCIÓN: Eliminar acentos =====
// La LCD HD44780 no soporta acentos ni ñ
String normalizarTexto(String texto) {
  String resultado = "";
  for (int i = 0; i < texto.length(); i++) {
    char c = texto[i];
    // Tabla simplificada de acentos comunes
    if (c == 'á' || c == 'à' || c == 'ä' || c == 'â') resultado += 'a';
    else if (c == 'é' || c == 'è' || c == 'ë' || c == 'ê') resultado += 'e';
    else if (c == 'í' || c == 'ì' || c == 'ï' || c == 'î') resultado += 'i';
    else if (c == 'ó' || c == 'ò' || c == 'ö' || c == 'ô') resultado += 'o';
    else if (c == 'ú' || c == 'ù' || c == 'ü' || c == 'û') resultado += 'u';
    else if (c == 'ñ') resultado += 'n';
    else if (c == 'ç') resultado += 'c';
    // Mayúsculas
    else if (c == 'Á' || c == 'À' || c == 'Ä' || c == 'Â') resultado += 'A';
    else if (c == 'É' || c == 'È' || c == 'Ë' || c == 'Ê') resultado += 'E';
    else if (c == 'Í' || c == 'Ì' || c == 'Ï' || c == 'Î') resultado += 'I';
    else if (c == 'Ó' || c == 'Ò' || c == 'Ö' || c == 'Ô') resultado += 'O';
    else if (c == 'Ú' || c == 'Ù' || c == 'Ü' || c == 'Û') resultado += 'U';
    else if (c == 'Ñ') resultado += 'N';
    else if (c == 'Ç') resultado += 'C';
    else resultado += c;
  }
  return resultado;
}

// ===== FUNCIÓN: Truncar a 16 caracteres =====
String truncarALCD(String texto) {
  if (texto.length() > 16) {
    return texto.substring(0, 16);
  }
  return texto;
}

// ===== FUNCIÓN: Mostrar en LCD =====
void mostrarEnLCD(String texto) {
  // Normalizar y truncar
  texto = normalizarTexto(texto);
  texto = truncarALCD(texto);

  // Copiar a buffer
  texto.toCharArray(textoActual, 17);

  // Mostrar: usar begin() en lugar de clear() para evitar corrupción por WiFi
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Fotopedia 3000");

  lcd.setCursor(0, 1);
  lcd.print(textoActual);

  // Rellenar con espacios si es más corto que 16 caracteres
  int espacios = 16 - strlen(textoActual);
  for (int i = 0; i < espacios; i++) {
    lcd.print(" ");
  }

  // Debug por Serial
  Serial.print("LCD actualizada: ");
  Serial.println(textoActual);
}

// ===== SETUP =====
void setup() {
  Serial.begin(9600);
  delay(2000);  // Esperar a que se estabilice la placa

  Serial.println("\n\nFotopedia 3000 - Servidor HTTP");
  Serial.println("================================");

  // Inicializar LCD
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Fotopedia 3000");
  lcd.setCursor(0, 1);
  lcd.print("Conectando WiFi.");

  // Conectar a WiFi
  Serial.print("Conectando a WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
    intentos++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    ipLocal = WiFi.localIP();

    Serial.print("WiFi conectado. IP: ");
    Serial.println(ipLocal);

    // Mostrar IP en LCD
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("IP:");
    lcd.setCursor(3, 0);
    lcd.print(ipLocal);

    lcd.setCursor(0, 1);
    lcd.print("Escuchando...");

    // Iniciar servidor
    server.begin();
    Serial.println("Servidor HTTP iniciado en puerto 80");
  } else {
    Serial.println("ERROR: No se pudo conectar a WiFi");
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("ERROR WiFi");
    while (true) {
      delay(1000);  // Bloquear si hay error
    }
  }

  delay(3000);  // Mostrar IP durante 3 segundos
}

// ===== LOOP =====
void loop() {
  // Comprobar si hay cliente conectado
  WiFiClient client = server.available();

  if (client) {
    Serial.println("Cliente conectado");

    String requestLine = "";
    String parametroTexto = "";
    boolean finalizarLectura = false;

    // Leer la petición HTTP
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        requestLine += c;

        // Detectar fin de línea de petición
        if (c == '\n') {
          if (finalizarLectura) {
            break;  // Fin de los headers
          }
          if (requestLine.endsWith("\r\n")) {
            // Procesar la línea de petición
            if (requestLine.startsWith("GET")) {
              // Formato esperado: GET /?texto=nombreObjeto HTTP/1.1
              int posicionPregunta = requestLine.indexOf('?');
              int posicionEspacio = requestLine.indexOf(' ', posicionPregunta);

              if (posicionPregunta > 0 && posicionEspacio > posicionPregunta) {
                String parametros = requestLine.substring(posicionPregunta + 1, posicionEspacio);

                // Buscar parámetro "texto="
                int posTexto = parametros.indexOf("texto=");
                if (posTexto >= 0) {
                  parametroTexto = parametros.substring(posTexto + 6);
                  // Detener en & o espacio si hay más parámetros
                  int posAmpersand = parametroTexto.indexOf('&');
                  if (posAmpersand > 0) {
                    parametroTexto = parametroTexto.substring(0, posAmpersand);
                  }
                }
              }
            }
            finalizarLectura = true;
          }
          requestLine = "";
        }
      }
    }

    // Si recibimos un parámetro "texto", actualizar LCD
    if (parametroTexto.length() > 0) {
      parametroTexto = decodificarURL(parametroTexto);
      Serial.print("Parámetro recibido: ");
      Serial.println(parametroTexto);

      mostrarEnLCD(parametroTexto);

      // Responder con OK
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println("OK");
    } else {
      // Responder con error
      client.println("HTTP/1.1 400 Bad Request");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println("Error: falta parámetro 'texto'");
    }

    // Cerrar conexión
    delay(1);
    client.stop();
    Serial.println("Cliente desconectado");
  }

  // Cada 10 segundos sin cliente, mostrar la IP actual (para referencia)
  static unsigned long ultimoMuestreoIP = 0;
  unsigned long ahora = millis();
  if (ahora - ultimoMuestreoIP > 30000) {
    ultimoMuestreoIP = ahora;
    Serial.print("Servidor activo en: http://");
    Serial.print(ipLocal);
    Serial.println("/");
  }
}
