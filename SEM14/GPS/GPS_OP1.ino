#include <SoftwareSerial.h>   // Librería para crear un puerto serial en pines digitales
#include <TinyGPS.h>          // Librería para procesar datos NMEA del GPS

// Objetos principales
TinyGPS gps;                            // Objeto para decodificar datos GPS
SoftwareSerial serialgps(4, 3);         // RX = pin 4, TX = pin 3 (comunicación con módulo GPS)

// Variables para fecha y hora
int year;
byte month, day, hour, minute, second, hundredths;

// Variables para estadísticas del GPS
unsigned long chars;
unsigned short sentences, failed_checksum;

void setup()
{
  Serial.begin(115200);       // Inicializa comunicación con el PC
  serialgps.begin(9600);      // Inicializa comunicación con el GPS

  // Mensajes iniciales
  Serial.println();
  Serial.println("GPS GY-GPS6MV2 Leantec");
  Serial.println(" ---Buscando señal--- ");
  Serial.println();
}

void loop()
{
  // Verifica si hay datos disponibles desde el GPS
  while (serialgps.available()) 
  {
    int c = serialgps.read(); // Lee un carácter

    // Si se ha recibido una oración NMEA completa y válida
    if (gps.encode(c))  
    {
      // Variables para latitud y longitud
      float latitude, longitude;

      // Obtener posición
      gps.f_get_position(&latitude, &longitude);
      Serial.print("Latitud/Longitud: "); 
      Serial.print(latitude, 5); 
      Serial.print(", "); 
      Serial.println(longitude, 5);

      // Obtener fecha y hora
      gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
      Serial.print("Fecha: "); Serial.print(day); Serial.print("/"); 
      Serial.print(month); Serial.print("/"); Serial.print(year);
      Serial.print("  Hora: "); Serial.print(hour); Serial.print(":"); 
      Serial.print(minute); Serial.print(":"); Serial.print(second); 
      Serial.print("."); Serial.println(hundredths);

      // Altitud
      Serial.print("Altitud (m): ");
      Serial.println(gps.f_altitude());

      // Rumbo
      Serial.print("Rumbo (°): ");
      Serial.println(gps.f_course());

      // Velocidad
      Serial.print("Velocidad (km/h): ");
      Serial.println(gps.f_speed_kmph());

      // Satélites
      Serial.print("Satélites: ");
      Serial.println(gps.satellites());

      // Línea en blanco
      Serial.println();

      // Estadísticas (no se imprimen pero pueden ser útiles para depuración)
      gps.stats(&chars, &sentences, &failed_checksum);
    }
  }
}
