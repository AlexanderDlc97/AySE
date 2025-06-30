#include <Adafruit_Fingerprint.h>  // Incluye la librería para manejar el sensor de huellas digitales Adafruit

// CONFIGURACIONES BIOMÉTRICO
SoftwareSerial mySerial(2, 3);  // Crea una comunicación serial en los pines 2 (RX) y 3 (TX)
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);  // Crea un objeto del sensor de huellas usando la comunicación serial

int id_finding;  // Variable para guardar el ID encontrado al verificar una huella
int id;          // Variable para guardar el ID donde se registrará una nueva huella
int temp;        // Variable usada como temporizador
char resp;       // Variable para almacenar una respuesta del usuario (no se usa en este código)

void setup() {
  Serial.begin(9600);  // Inicia la comunicación serial con el monitor a 9600 baudios
  while (!Serial);  // Espera a que el monitor serial esté disponible (necesario en algunas placas)

  // SETUP BIOMÉTRICO
  finger.begin(57600);  // Inicia el sensor de huellas a 57600 baudios
  if (finger.verifyPassword()) {  // Verifica si el sensor está conectado correctamente
    Serial.println("Sensor biometrico encontrado!");
  } else {
    Serial.println("No se encontro el sensor biometrico :(");
    while (1) { delay(1); }  // Si no se encuentra, queda en un bucle infinito
  }
  delay(1000);  // Espera 1 segundo antes de continuar
}

void loop() {
  // LOOP PARA BIOMÉTRICO

  id_finding = getFingerprintIDez();  // Intenta identificar una huella
  if(id_finding==0){  // Si no se identificó (pero hay una huella)
      Serial.println("Usuario no identificado");
      finger.getTemplateCount();  // Obtiene el número actual de huellas almacenadas
      id = finger.templateCount +1;  // Asigna el siguiente ID disponible
      Serial.print("Desea registrar en el ID: ");
      Serial.println(id);
      while(!Serial.available());  // Espera que el usuario escriba algo
      if(Serial.readString()=="si"){  // Si el usuario responde "si"
        Serial.println("Registrando");
        getFingerprintEnroll(id);  // Llama a la función para registrar la nueva huella
        return;
      }
      Serial.println("Continua");
      
  } else if(id_finding > 0){  // Si se identificó correctamente una huella
      Serial.println("Usuario identificado");
      Serial.println("ID: "+String(id_finding));
  }

  delay(50);  // Pequeña pausa antes de volver a ejecutar el loop      
}

// FUNCIÓN PARA REGISTRAR UNA HUELLA
uint8_t getFingerprintEnroll(int id_enroll) {
  int p = -1;
  Serial.println("Coloque su dedo");
  temp=0;
  while(temp<5000){  // Espera hasta que se coloque un dedo, por un tiempo máximo
    temp++;
    p = finger.getImage();  // Intenta capturar una imagen de la huella
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Huella leida");
        break;
      default:
        break;
    }
    if (p == FINGERPRINT_OK) {
      break;  // Sale del bucle si se leyó correctamente
    }
  }
  if (temp >= 5000){
    Serial.println("Termino el tiempo");  // Si pasó el tiempo máximo
    return 0;
  }

  p = finger.image2Tz(1);  // Convierte la imagen leída a características y la almacena en el buffer 1
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Imagen convertida");
      break;
    default:
      Serial.println("Error convirtiendo imagen");
      return p;
  }

  Serial.println("Retire el dedo");
  delay(2000);  // Pausa para que el usuario retire el dedo
  Serial.println("Coloque el dedo nuevamente");
  temp = 0;
  while (temp<5000) {  // Espera a que el usuario vuelva a colocar el dedo
    temp++; 
    p = finger.getImage();
    if(p == FINGERPRINT_OK){
      Serial.println("Huella leida");
      break;
    }   
  }
  if (temp == 5000){
    Serial.println("Termino el tiempo");
    return 0;
  }

  p = finger.image2Tz(2);  // Convierte esta segunda imagen y la guarda en el buffer 2
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Imagen convertida");
      break;
    default:
      Serial.println("Error convirtiendo imagen");
      return p;
  }

  Serial.print("Creando modelo para el ID: ");  
  Serial.println(id_enroll);
  p = finger.createModel();  // Compara las dos imágenes y crea un modelo único
  if (p == FINGERPRINT_OK) {
    Serial.println("Modelo correcto!");
  } else {
    Serial.println("Error creando modelo");
    return p;
  }   

  p = finger.storeModel(id_enroll);  // Guarda el modelo en la memoria del sensor
  if (p == FINGERPRINT_OK) {
    Serial.println("Guardado!");
  } else {
    Serial.println("Error guardando");
    return p;
  }
  return 1;  // Indica éxito
}

// FUNCIÓN PARA IDENTIFICAR HUELLA
int getFingerprintIDez() {
  uint8_t p = finger.getImage();  // Intenta capturar una huella
  if (p != FINGERPRINT_OK)  return -1;  // No hay dedo

  p = finger.image2Tz();  // Convierte la imagen a características
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();  // Busca en la base de datos del sensor
  if (p != FINGERPRINT_OK)  return 0;  // No se encontró coincidencia

  return finger.fingerID;  // Retorna el ID si encontró coincidencia
}
