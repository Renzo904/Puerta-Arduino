#include <Adafruit_Fingerprint.h>
#include <Servo.h>
#include "sensor.h"

#define SENSOR_TX 2
#define SENSOR_RX 3
#define BUZZER_PIN 7
#define SERVO_PIN 4
#define LATCH_PIN 6

#define BUZZER_ERROR_TIME 200

#define MODO 1

#if MODO == 0
    #define CERROJO     //elegir entre cerrojo o servo
#elif MODO == 1
    #define SERVO
#else 
    #error MODO IS NOT VALID
#endif    


SoftwareSerial mySerial(SENSOR_TX, SENSOR_RX);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
char strBuf[50];
#ifdef SERVO
    Servo puerta;
#endif


/*
    El codigo se compone del setup, en donde dependiendo del modo se preparan los pines para el servo, o el latch, 
    luego se preparan los pines para el pin incluido en el arduino
*/

void setup() {
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    
    if(MODO) {
        puerta.attach(SERVO_PIN);
        puerta.write(0); //Empeza en la posicion de cerrado
    }
    else {
        pinMode(LATCH_PIN, OUTPUT);
    }
    Serial.begin(9600);

    finger.begin(57601);
    delay(5);       //No tengo ni puta idea de porque estaba este delay, testealo y si no afecta en nada, boletealo
    verificarModulo();

    finger.getTemplateCount();

    sprintf(strBuf, "El sensor contiene %d plantillas", finger.templateCount);
    Serial.println(strBuf);
    Serial.println("Esperando por una huella valida...");
}

void loop() {
    autenticarHuella();       //Verifica la huella colocada en el Sensor de huella dactilar
    delay(50);              //Yamete kudasai!
}

/**************************************************************************/
/*!
    @brief  Verifica que el lector de huellas dactilares este conectado al
    arduino, en caso contrario, deja parpadeando loop infinito al led 
    integrado del arduino
*/
/**************************************************************************/
void verificarModulo() {
    if (finger.verifyPassword()) {
        Serial.println("Detectado un sensor de huella!");
    }
    else {
        Serial.println("No hay comunicacion con el sensor de huella");
        Serial.println("Revise las conexiones");
        while (1) {
            digitalWrite(LED_BUILTIN, HIGH);    //Hace parpadear el led del arduino 
            delay(750);                         //infinitamente, eso indica que no se detecto el sensor de huella 
            digitalWrite(LED_BUILTIN, LOW);
            delay(750);
        }
    }
}

#if (defined(SERVO))
/**************************************************************************/
/*!
    @brief  Gira 180 grados el servo por un tiempo para luego cerrarlo haciendo
    sonar la alarma
    TODO Eliminar el tiempo y hace que conmute entre cerrado y abierto con
    el sensor de huellas
*/
/**************************************************************************/
void abrirPuerta() {    //SERVO
    Serial.println(" AUTORIZADA *** ");
    digitalWrite(BUZZER_PIN, HIGH);     //Hace un sonido el buzzer
    puerta.write(180);                  //Gira el servo para abrir la puerta
    delay(1500);
    
    digitalWrite(BUZZER_PIN, LOW);      //Deja de sonar el buzzer
    delay(3000);
    puerta.write(0);                    //Gira el servo para cerrar la puerta
}

#elif (defined(CERROJO))
/**************************************************************************/
/*!
    @brief  Da corriente al pin en donde esta conectado el cerrojo haciendo
    sonar la alarma
*/
/**************************************************************************/
void abrirPuerta() {    //CERROJO
    
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LATCH_PIN, HIGH);
    delay(2500);

    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LATCH_PIN, HIGH);
}
#endif

/**************************************************************************/
/*!
    @brief  Hace sonar el buzzer avisando de que la huella es incorrecta
*/
/**************************************************************************/
void huellaIncorrecta() {
    for (int i = 0; i <= 5; i++) {      //*3,14 3,14 3,14 3,14*
        digitalWrite(BUZZER_PIN, i % 2);
        delay(BUZZER_ERROR_TIME);
    }
    digitalWrite(BUZZER_PIN, LOW);
}

/**************************************************************************/
/*!
    @brief  Verifica que la huella colocada este registrada en el modulo
    del arduino
    @returns El ID de la huella, en caso de no encontrarla, retorna -1
*/
/**************************************************************************/
int autenticarHuella() {
    uint8_t p = finger.getImage();
    if (p != FINGERPRINT_OK) return -1;
    
    p = finger.image2Tz();
    if (p != FINGERPRINT_OK) return -1;

    p = finger.fingerFastSearch();
    if (p == FINGERPRINT_NOMATCH) {
        huellaIncorrecta();     //Si se presiono el dedo, pero no coincide, hace sonar el buzzer
        return -1;
    }
    else if (p != FINGERPRINT_OK) {
        return -1;              //Aca no paso nada, andate y no hagas nada
    }
    
    
    sprintf(strBuf, "ID#%d\n AUTORIZADA *** ", finger.fingerID);
    Serial.println(strBuf);
    abrirPuerta();

    return finger.fingerID;
}
