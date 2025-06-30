#include <Adafruit_Fingerprint.h>  // Incluye la librería para controlar el sensor de huellas Adafruit

// Configuración de pines para comunicación serial con el sensor
SoftwareSerial mySerial(2, 3);  // Crea una comunicación serial en pines 2 (RX) y 3 (TX)
Adafruit_Fingerprint finger(&mySerial);  // Crea un objeto "finger" del tipo Adafruit_Fingerprint

// Variables globales
int temp;  // Se usa como temporizador durante la captura de huella

void setup() {
  Serial.begin(9600);  // Inicia la comunicación con el monitor serial a 9600 baudios
  while (!Serial);     // Espera a que el monitor serial esté listo (solo en ciertas placas como Leonardo)

  finger.begin(57600);  // Inicializa el sensor de huella a 57600 baudios

  if (finger.verifyPassword()) {  // Verifica conexión con el sensor
    Serial.println("Sensor biométrico conectado correctamente.");
  } else {
    Serial.println("Error: Sensor biométrico no detectado.");
    while (true) delay(1);  // Si falla, se queda en un bucle infinito
  }

  delay(1000);  // Espera 1 segundo antes de continuar
}

void loop() {
  int idEncontrado = identificarHuella();  // Intenta identificar al usuario por huella

  if (idEncontrado > 0) {  // Si se identificó un usuario
    Serial.print("✅ Usuario identificado. ID: ");
    Serial.println(idEncontrado);
  } 
  else if (idEncontrado == 0) {  // Si hay huella pero no se reconoce
    Serial.println("❌ Usuario no identificado.");
    int nuevoID = finger.templateCount + 1;  // Determina el próximo ID libre
    Serial.print("¿Desea registrar una nueva huella en el ID ");
    Serial.print(nuevoID);
    Serial.println("? (si/no)");

    esperarInput();  // Espera entrada del usuario
    String respuesta = Serial.readStringUntil('\n');  // Lee la respuesta del usuario
    respuesta.trim();  // Elimina espacios o saltos de línea

    if (respuesta.equalsIgnoreCase("si")) {  // Si responde "si"
      if (registrarHuella(nuevoID)) {  // Intenta registrar la nueva huella
        Serial.println("✅ Registro exitoso.");
      } else {
        Serial.println("⚠️ Registro fallido.");
      }
    } else {
      Serial.println("↪️ Registro cancelado.");
    }
  } else {
    Serial.println("⌛ Coloque su dedo.");  // No se detectó ninguna huella
  }

  delay(1000);  // Espera un segundo antes de la siguiente iteración
}

// ---------------------------------------------
// FUNCIONES

// Espera hasta que el usuario escriba algo por el monitor serial
void esperarInput() {
  while (!Serial.available());  // Espera a que haya datos disponibles
  delay(100);  // Breve pausa para asegurar que se recibió la información completa
}

// Intenta identificar una huella y devuelve:
// > 0 = ID encontrado, 0 = huella no reconocida, -1 = no hay dedo
int identificarHuella() {
  if (finger.getImage() != FINGERPRINT_OK) return -1;  // Si no se detecta dedo
  if (finger.image2Tz() != FINGERPRINT_OK) return -1;  // Si no se puede convertir la imagen
  if (finger.fingerFastSearch() != FINGERPRINT_OK) return 0;  // Si no se encuentra coincidencia

  return finger.fingerID;  // Retorna el ID si fue encontrado
}

// Registra una nueva huella para el ID dado
bool registrarHuella(int id) {
  Serial.println("🖐️ Coloque su dedo para registrar...");

  if (!leerHuella(1)) return false;  // Lee y convierte la primera imagen

  Serial.println("✋ Retire el dedo...");
  delay(2000);  // Espera a que el usuario retire el dedo

  Serial.println("🖐️ Coloque el mismo dedo nuevamente...");
  if (!leerHuella(2)) return false;  // Lee y convierte la segunda imagen

  if (finger.createModel() != FINGERPRINT_OK) {  // Crea el modelo a partir de las 2 lecturas
    Serial.println("❌ Error creando modelo.");
    return false;
  }

  if (finger.storeModel(id) != FINGERPRINT_OK) {  // Guarda el modelo en la posición de memoria
    Serial.println("❌ Error guardando modelo.");
    return false;
  }

  return true;  // Registro exitoso
}

// Lee una huella en el buffer especificado (1 o 2)
bool leerHuella(uint8_t buffer) {
  temp = 0;  // Reinicia el contador

  while (temp < 5000) {  // Intenta por un máximo de 5000 ciclos
    temp++;
    if (finger.getImage() == FINGERPRINT_OK) {  // Si se detecta imagen
      if (finger.image2Tz(buffer) == FINGERPRINT_OK) {  // Intenta convertirla
        Serial.println("✅ Imagen capturada y convertida.");
        return true;
      } else {
        Serial.println("❌ Fallo al convertir imagen.");
        return false;
      }
    }
    delay(1);  // Pausa mínima entre intentos
  }

  Serial.println("⏱️ Tiempo agotado.");  // No se logró la lectura a tiempo
  return false;
}
