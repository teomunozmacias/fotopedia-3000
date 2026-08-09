/*
  Fotopedia 3000 - Fase 4
  Servidor HTTP local en Arduino Uno R4 WiFi

  Que hace este programa:
  1. Se conecta a la red WiFi de casa.
  2. Ensena su direccion IP en la pantalla LCD (linea de arriba).
  3. Se queda escuchando peticiones en el puerto 80.
  4. Cuando llega http://<ip>/?texto=Cat, saca "Cat" en la linea de abajo.

  Como probarlo sin el movil, desde el ordenador:
     curl "http://192.168.1.50/?texto=Hola+Mundo"

  IMPORTANTE: pon aqui abajo el nombre y la clave de tu WiFi,
  pero NO subas este archivo a GitHub con la clave de verdad escrita.
*/

#include <WiFiS3.h>
#include <LiquidCrystal.h>

// ===== CONFIGURACION =====
const char* SSID_WIFI  = "TU_RED_WIFI";       // Nombre de tu red WiFi
const char* CLAVE_WIFI = "TU_CONTRASENA";     // Clave de tu red WiFi
const int   PUERTO     = 80;

// Cuanto esperamos como maximo a que el movil nos mande la peticion (2 segundos).
// Sin esto, si alguien abre una conexion y no dice nada, la placa se quedaria
// esperando para siempre y habria que reiniciarla.
const unsigned long TIMEOUT_PETICION_MS = 2000;

// Cuanto esperamos como maximo a que el router nos preste una direccion IP.
const unsigned long ESPERA_DHCP_MS = 10000;

// "Direccion vacia": es lo que devuelve la placa cuando todavia no tiene IP.
const IPAddress IP_SIN_ASIGNAR(0, 0, 0, 0);

// Pines de la LCD: RS, E, D4, D5, D6, D7 (los mismos que en la Fase 1)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

WiFiServer server(PUERTO);

// Lo que hay escrito ahora mismo en cada linea de la pantalla.
// 16 letras + 1 hueco final que marca donde acaba el texto.
char lineaArriba[17] = "Arrancando...   ";
char lineaAbajo[17]  = "                ";

// ---------------------------------------------------------------------------
// PANTALLA
// ---------------------------------------------------------------------------

// Copia un texto a una de las lineas, cortando a 16 letras
// y rellenando con espacios si sobra sitio.
void guardarLinea(char* destino, const String& texto) {
  for (int i = 0; i < 16; i++) {
    destino[i] = (i < (int)texto.length()) ? texto[i] : ' ';
  }
  destino[16] = '\0';
}

// Vuelve a dibujar las dos lineas.
// Usamos lcd.begin() en vez de lcd.clear() porque begin() reenvia toda la
// configuracion a la pantalla: asi se arreglan los caracteres corruptos que
// a veces provoca el WiFi al meter ruido en los cables (lo vimos en la Fase 2).
void refrescarPantalla() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print(lineaArriba);
  lcd.setCursor(0, 1);
  lcd.print(lineaAbajo);
}

void escribirArriba(const String& texto) {
  guardarLinea(lineaArriba, texto);
  refrescarPantalla();
}

void escribirAbajo(const String& texto) {
  guardarLinea(lineaAbajo, texto);
  refrescarPantalla();
}

// ---------------------------------------------------------------------------
// TEXTO: decodificar la URL y quitar acentos
// ---------------------------------------------------------------------------

// En una direccion web no se pueden poner espacios ni acentos tal cual,
// asi que el movil los manda "disfrazados":
//    espacio -> "+"  o  "%20"
//    e con tilde -> "%C3%A9"
// Esta funcion les quita el disfraz.
String decodificarURL(const String& texto) {
  String resultado = "";
  for (unsigned int i = 0; i < texto.length(); i++) {
    char c = texto[i];
    if (c == '+') {
      resultado += ' ';
    } else if (c == '%' && i + 2 < texto.length()) {
      char hex[3] = { texto[i + 1], texto[i + 2], '\0' };
      char* sobra;
      long codigo = strtol(hex, &sobra, 16);
      if (*sobra == '\0') {          // los dos caracteres eran hexadecimal valido
        resultado += (char)codigo;
        i += 2;
      } else {
        resultado += c;              // era un "%" suelto, lo dejamos tal cual
      }
    } else {
      resultado += c;
    }
  }
  return resultado;
}

// La pantalla HD44780 no tiene letras con tilde ni la "n" con virgulilla.
//
// Ojo a un detalle importante: en el ordenador una "a con tilde" NO ocupa
// una letra, ocupa DOS numeros seguidos (se llama UTF-8). La "a" con tilde
// es la pareja 0xC3 0xA1, y la "n" con virgulilla es 0xC3 0xB1.
// Por eso aqui miramos los numeros de dos en dos: si vemos un 0xC3,
// leemos el siguiente para saber que letra era y la cambiamos por su
// version sin tilde.
String quitarAcentos(const String& texto) {
  String resultado = "";
  for (unsigned int i = 0; i < texto.length(); i++) {
    byte b = (byte)texto[i];

    if (b == 0xC3 && i + 1 < texto.length()) {
      byte segundo = (byte)texto[i + 1];
      i++;  // esta pareja ya la hemos gastado entera

      if      (segundo >= 0xA0 && segundo <= 0xA5) resultado += 'a';  // a a a a a a
      else if (segundo == 0xA7)                    resultado += 'c';  // c cedilla
      else if (segundo >= 0xA8 && segundo <= 0xAB) resultado += 'e';  // e e e e
      else if (segundo >= 0xAC && segundo <= 0xAF) resultado += 'i';  // i i i i
      else if (segundo == 0xB1)                    resultado += 'n';  // n virgulilla
      else if (segundo >= 0xB2 && segundo <= 0xB6) resultado += 'o';  // o o o o o
      else if (segundo >= 0xB9 && segundo <= 0xBC) resultado += 'u';  // u u u u
      else if (segundo >= 0x80 && segundo <= 0x85) resultado += 'A';
      else if (segundo == 0x87)                    resultado += 'C';
      else if (segundo >= 0x88 && segundo <= 0x8B) resultado += 'E';
      else if (segundo >= 0x8C && segundo <= 0x8F) resultado += 'I';
      else if (segundo == 0x91)                    resultado += 'N';
      else if (segundo >= 0x92 && segundo <= 0x96) resultado += 'O';
      else if (segundo >= 0x99 && segundo <= 0x9C) resultado += 'U';
      // Cualquier otra pareja rara la ignoramos.

    } else if (b == 0xC2 && i + 1 < texto.length()) {
      i++;  // simbolos tipo "!" invertido o grados: los saltamos

    } else if (b >= 32 && b < 127) {
      resultado += (char)b;  // letra, numero o signo normal: pasa tal cual
    }
    // Lo que no sea imprimible se descarta, para que la pantalla no saque basura.
  }
  return resultado;
}

// Deja el texto listo para la pantalla y lo escribe en la linea de abajo.
void mostrarObjeto(const String& textoBruto) {
  String limpio = quitarAcentos(textoBruto);

  if (limpio.length() > 16) {
    limpio = limpio.substring(0, 16);   // solo caben 16 letras por linea
  }

  escribirAbajo(limpio);

  Serial.print("LCD linea 2: ");
  Serial.println(limpio);
}

// ---------------------------------------------------------------------------
// WIFI
// ---------------------------------------------------------------------------

// Si algo falla, aqui dejamos escrito que fue, para poder avisar en la pantalla.
String ultimoErrorWiFi = "";

// Intenta conectarse a la WiFi y conseguir una direccion IP.
// Devuelve true solo si consigue las dos cosas.
bool conectarWiFi() {
  Serial.print("Conectando a la WiFi: ");
  Serial.println(SSID_WIFI);

  WiFi.begin(SSID_WIFI, CLAVE_WIFI);

  // --- Paso 1: entrar en la red ---
  for (int intentos = 0; intentos < 20 && WiFi.status() != WL_CONNECTED; intentos++) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No se ha podido entrar en la red WiFi.");
    Serial.println("Revisa el nombre de la red y la clave.");
    ultimoErrorWiFi = "Clave o red mal";
    return false;
  }

  // --- Paso 2: esperar a que el router nos preste una IP ---
  //
  // MUY IMPORTANTE: estar "conectado" NO quiere decir que ya tengamos direccion.
  // Son dos cosas distintas y pasan una detras de otra:
  //
  //   1. La placa entra en la red     -> WiFi.status() == WL_CONNECTED
  //   2. El router le presta una IP   -> WiFi.localIP() deja de ser 0.0.0.0
  //
  // Es como entrar en un hotel: primero cruzas la puerta, y despues, en
  // recepcion, te dan el numero de habitacion. Si preguntamos el numero nada
  // mas cruzar la puerta, todavia no lo tienen y nos dicen "0.0.0.0".
  //
  // Ese reparto de direcciones se llama DHCP y tarda un poco, asi que hay que
  // esperar a que termine.
  Serial.print("Dentro de la red. Esperando la IP del router");

  IPAddress ip = WiFi.localIP();
  unsigned long inicio = millis();

  while (ip == IP_SIN_ASIGNAR && millis() - inicio < ESPERA_DHCP_MS) {
    delay(250);
    Serial.print(".");
    ip = WiFi.localIP();
  }
  Serial.println();

  if (ip == IP_SIN_ASIGNAR) {
    Serial.println("El router no nos ha dado ninguna IP (fallo de DHCP).");
    ultimoErrorWiFi = "Sin IP: router?";
    return false;
  }

  Serial.print("Listo. IP: ");
  Serial.println(ip);

  // La IP se queda fija arriba: es la direccion que hay que escribir
  // en la app. Puede cambiar si reinicias el router.
  escribirArriba(ip.toString());
  ultimoErrorWiFi = "";
  return true;
}

// ---------------------------------------------------------------------------
// SERVIDOR HTTP
// ---------------------------------------------------------------------------

// Saca el valor del parametro "texto" de una direccion como  /?texto=Cat
// Si no lo encuentra, devuelve "".
String extraerParametroTexto(const String& ruta) {
  int inicioConsulta = ruta.indexOf('?');
  if (inicioConsulta < 0) {
    return "";
  }

  String consulta = ruta.substring(inicioConsulta + 1);

  // La consulta puede traer varios parametros separados por "&":
  //    ?texto=Cat&otro=3
  int desde = 0;
  while (desde <= (int)consulta.length()) {
    int siguiente = consulta.indexOf('&', desde);
    if (siguiente < 0) siguiente = consulta.length();

    String pareja = consulta.substring(desde, siguiente);
    if (pareja.startsWith("texto=")) {
      return pareja.substring(6);
    }

    desde = siguiente + 1;
  }

  return "";
}

// Lee la primera linea de la peticion (la que trae la direccion),
// sin quedarse colgado si el cliente no manda nada.
String leerPrimeraLinea(WiFiClient& client) {
  String linea = "";
  unsigned long inicio = millis();

  while (millis() - inicio < TIMEOUT_PETICION_MS) {
    if (client.available()) {
      char c = client.read();
      if (c == '\n') break;
      if (c != '\r') linea += c;
      if (linea.length() > 250) break;   // proteccion: la placa tiene poca memoria
    } else if (!client.connected()) {
      break;
    }
  }

  return linea;
}

void responder(WiFiClient& client, const char* estado, const String& cuerpo) {
  client.print("HTTP/1.1 ");
  client.println(estado);
  client.println("Content-Type: text/plain; charset=utf-8");
  client.println("Connection: close");
  client.println();
  client.println(cuerpo);
}

void atenderCliente(WiFiClient& client) {
  String primeraLinea = leerPrimeraLinea(client);

  if (primeraLinea.length() == 0) {
    Serial.println("Peticion vacia (se agoto el tiempo de espera).");
    return;
  }

  Serial.print("Peticion: ");
  Serial.println(primeraLinea);

  // Nos tragamos el resto de la peticion para que el movil no vea un corte feo.
  unsigned long inicio = millis();
  while (client.available() && millis() - inicio < 100) {
    client.read();
  }

  // Formato de la primera linea:  GET /?texto=Cat HTTP/1.1
  int primerEspacio  = primeraLinea.indexOf(' ');
  int segundoEspacio = primeraLinea.indexOf(' ', primerEspacio + 1);

  if (primerEspacio < 0 || segundoEspacio < 0) {
    responder(client, "400 Bad Request", "No entiendo la peticion");
    return;
  }

  String ruta = primeraLinea.substring(primerEspacio + 1, segundoEspacio);

  // Los navegadores piden el iconito de la pestana; no nos interesa.
  if (ruta.startsWith("/favicon.ico")) {
    responder(client, "404 Not Found", "");
    return;
  }

  String parametro = extraerParametroTexto(ruta);

  if (parametro.length() == 0) {
    responder(client, "400 Bad Request",
              "Falta el parametro 'texto'. Prueba con /?texto=Hola");
    return;
  }

  String textoRecibido = decodificarURL(parametro);
  Serial.print("Texto recibido: ");
  Serial.println(textoRecibido);

  mostrarObjeto(textoRecibido);

  responder(client, "200 OK", "OK");
}

// ---------------------------------------------------------------------------
// SETUP Y LOOP
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(2000);   // damos tiempo a que el Monitor Serie se abra

  Serial.println();
  Serial.println("Fotopedia 3000 - Servidor HTTP");
  Serial.println("==============================");

  lcd.begin(16, 2);
  escribirArriba("Fotopedia 3000");
  escribirAbajo("Buscando WiFi...");

  if (!conectarWiFi()) {
    escribirArriba("ERROR de red");
    escribirAbajo(ultimoErrorWiFi);
    Serial.println("Mira la pantalla y el mensaje de arriba para saber que falla.");
    while (true) {
      delay(1000);   // aqui nos quedamos: sin IP no podemos hacer nada
    }
  }

  escribirAbajo("Esperando foto");

  server.begin();
  Serial.print("Servidor listo en http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void loop() {
  // 1) Atender al movil si nos esta llamando
  WiFiClient client = server.available();
  if (client) {
    Serial.println("--- Cliente conectado ---");
    atenderCliente(client);
    delay(1);          // margen para que salgan los ultimos bytes
    client.stop();
    Serial.println("--- Cliente desconectado ---");
  }

  // 2) Vigilar la WiFi: si se cae, volver a conectarse
  static unsigned long ultimaRevision = 0;
  if (millis() - ultimaRevision > 5000) {
    ultimaRevision = millis();

    // Damos por perdida la conexion si nos hemos salido de la red
    // O si nos hemos quedado sin direccion IP (las dos cosas importan).
    if (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IP_SIN_ASIGNAR) {
      Serial.println("Se ha perdido la conexion. Reconectando...");
      escribirArriba("Recuperando red");

      if (conectarWiFi()) {
        server.begin();          // el servidor necesita volver a arrancar
        Serial.println("Servidor de nuevo en marcha.");
      }
    }
  }
}
