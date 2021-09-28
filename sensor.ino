#include <Adafruit_Fingerprint.h>
#include <Servo.h>

#define SENSOR_TX 2
#define SENSOR_RX 3
#define RELE_ON 9
#define RELE_OFF 10



 


SoftwareSerial mySerial(SENSOR_TX, SENSOR_RX);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
char strBuf[50];
bool estadoPuerta = false;


/*
    El codigo se compone del setup, en donde dependiendo del modo se preparan los pines para el servo, o el latch, 
    luego se preparan los pines para el pin incluido en el arduino
*/

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RELE_ON, OUTPUT);
    pinMode(RELE_OFF, OUTPUT);
    
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

/**************************************************************************/
/*!
    @brief  Alterna los reles de la ultima vez que se activo, luego de 50
    milisengudos, apaga los reles
*/
/**************************************************************************/
void abrirPuerta() {
    estadoPuerta = !estadoPuerta;
    digitalWrite(RELE_ON, estadoPuerta);
    digitalWrite(RELE_OFF, !estadoPuerta);
    delay(50);
    digitalWrite(RELE_ON, LOW);
    digitalWrite(RELE_OFF, LOW);

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
    if (p != FINGERPRINT_OK) return -1;         //Aca no paso nada, andate y no hagas nada
    
    
    
    sprintf(strBuf, "ID#%d\n AUTORIZADA *** ", finger.fingerID);
    Serial.println(strBuf);
    abrirPuerta();

    return finger.fingerID;
}
