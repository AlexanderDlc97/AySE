#include <SoftwareSerial.h>   // Incluye la librería para crear puertos seriales en otros pines (además del Serial estándar)
#include <TinyGPS.h>          // Incluye la librería para procesar datos NMEA del GPS (coordenadas, fecha, velocidad, etc.)

TinyGPS gps;                  // Crea un objeto `gps` de tipo TinyGPS que se encargará de decodificar los datos del GPS
SoftwareSerial serialgps(4, 3); // Crea un puerto serie "virtual" en los pines 4 (RX) y 3 (TX) para comunicarse con el módulo GPS

// Variables para almacenar fecha y hora
int year;                     // Año (ej: 2025)
byte month, day, hour, minute, second, hundredths; // Mes, día, hora, minuto, segundo y centésimas

// Variables para estadísticas del GPS
unsigned long chars;          // Número total de caracteres recibidos desde el GPS
unsigned short sentences;     // Número de sentencias (frases) válidas recibidas
unsigned short failed_checksum; // Número de frases con errores de validación (checksum)

void setup() {
  Serial.begin(115200);       // Inicia la comunicación serial con la PC a 115200 baudios
  serialgps.begin(9600);      // Inicia la comunicación con el módulo GPS a 9600 baudios (estándar para GY-GPS6MV2)

  Serial.println();                           // Línea en blanco para separar la salida
  Serial.println("GPS GY-GPS6MV2 - Sistema Iniciado"); // Mensaje de inicio del sistema
  Serial.println("Buscando señal GPS...");    // Indica que el GPS aún está buscando señal
}

void loop() {
  // Mientras haya datos disponibles desde el módulo GPS
  while (serialgps.available()) {
    char c = serialgps.read(); // Lee un carácter del buffer serial del GPS

    // Si el carácter completa una sentencia válida del protocolo NMEA
    if (gps.encode(c)) {
      mostrarPosicion();       // Llama a la función que muestra latitud y longitud
      mostrarFechaHora();      // Muestra la fecha y hora obtenida del GPS
      mostrarDatosExtras();    // Muestra altitud, velocidad, rumbo y satélites
      mostrarEstadisticas();   // Muestra estadísticas de recepción del GPS
      Serial.println("--------------------------\n"); // Línea separadora entre ciclos
      delay(2000);             // Espera 2 segundos antes de leer nuevamente (para no saturar)
    }
  }
}

// 🧭 Función que muestra latitud y longitud
void mostrarPosicion() {
  float latitude, longitude; // Variables para almacenar coordenadas

  gps.f_get_position(&latitude, &longitude); // Obtiene la posición actual (en grados decimales)

  // Verifica si los datos son válidos
  if (latitude == TinyGPS::GPS_INVALID_F_ANGLE || longitude == TinyGPS::GPS_INVALID_F_ANGLE) {
    Serial.println("Latitud/Longitud: No disponible"); // Mensaje de error si no hay datos válidos
  } else {
    Serial.print("Latitud: "); Serial.print(latitude, 6);    // Imprime latitud con 6 decimales
    Serial.print(" | Longitud: "); Serial.println(longitude, 6); // Imprime longitud
  }
}

// ⏰ Función que muestra la fecha y hora actual
void mostrarFechaHora() {
  // Extrae la fecha y hora del objeto GPS y las guarda en las variables
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);

  if (year == 0) { // Si no se obtuvo una fecha válida
    Serial.println("Fecha y hora: No disponible");
  } else {
    // Muestra la fecha en formato DD/MM/AAAA
    Serial.print("Fecha: ");
    if (day < 10) Serial.print("0"); Serial.print(day);
    Serial.print("/");
    if (month < 10) Serial.print("0"); Serial.print(month);
    Serial.print("/");
    Serial.print(year);

    // Muestra la hora en formato HH:MM:SS
    Serial.print(" | Hora: ");
    if (hour < 10) Serial.print("0"); Serial.print(hour); Serial.print(":");
    if (minute < 10) Serial.print("0"); Serial.print(minute); Serial.print(":");
    if (second < 10) Serial.print("0"); Serial.print(second);
    Serial.println(); // Salto de línea
  }
}

// 📡 Función que muestra altitud, velocidad, rumbo y cantidad de satélites
void mostrarDatosExtras() {
  float alt = gps.f_altitude();        // Obtiene altitud en metros
  float spd = gps.f_speed_kmph();      // Obtiene velocidad en km/h
  float crs = gps.f_course();          // Obtiene rumbo (ángulo de movimiento)
  int sats = gps.satellites();         // Obtiene número de satélites conectados

  // Verificaciones para cada parámetro y su impresión
  Serial.print("Altitud: ");
  (alt == TinyGPS::GPS_INVALID_F_ALTITUDE) ? Serial.println("N/A") : Serial.println(String(alt, 2) + " m");

  Serial.print("Velocidad: ");
  (spd == TinyGPS::GPS_INVALID_F_SPEED) ? Serial.println("N/A") : Serial.println(String(spd, 2) + " km/h");

  Serial.print("Rumbo: ");
  (crs == TinyGPS::GPS_INVALID_F_ANGLE) ? Serial.println("N/A") : Serial.println(String(crs, 2) + "°");

  Serial.print("Satélites: ");
  Serial.println((sats == TinyGPS::GPS_INVALID_SATELLITES) ? "N/A" : String(sats));
}

// 📊 Función que muestra estadísticas de transmisión
void mostrarEstadisticas() {
  gps.stats(&chars, &sentences, &failed_checksum); // Actualiza los valores de estadísticas

  Serial.print("Caracteres procesados: "); Serial.println(chars);       // Total de caracteres NMEA recibidos
  Serial.print("Frases válidas: "); Serial.println(sentences);          // Frases completas y válidas
  Serial.print("Errores de checksum: "); Serial.println(failed_checksum); // Frases con errores de validación
}
