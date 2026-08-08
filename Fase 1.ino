/*
  Fotopedia 3000 - Fase 1
  Prueba de la pantalla LCD 1602 (sin I2C)
  conectada al Arduino Uno R4 WiFi
*/

#include <LiquidCrystal.h>

// Pines: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  // Le decimos a la librería que la pantalla tiene 16 columnas y 2 filas
  lcd.begin(16, 2);

  // Escribimos en la primera línea
  lcd.setCursor(0, 0);
  lcd.print("Fotopedia 3000");

  // Escribimos en la segunda línea
  lcd.setCursor(0, 1);
  lcd.print("Hola mundo!");
}

void loop() {
  // De momento no hacemos nada aqui,
  // solo queremos ver el mensaje fijo en pantalla
}
