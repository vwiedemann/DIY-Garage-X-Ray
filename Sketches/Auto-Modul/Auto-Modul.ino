#include <SPI.h>
#include "RF24.h"

/* Radio */
#define CE_PIN   9
#define CSN_PIN 10

/* LEDs */
#define LED_RED 5
#define LED_GREEN 6

/* self maintaining */
#define SELF_PIN 7

RF24 radio(CE_PIN,CSN_PIN);

byte addresses[][6] = {"1Node","2Node"};
typedef enum { role_ping_out = 1, role_pong_back } role_e;                 // The various roles supported by this sketch
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};  // The debug-friendly names of those roles
role_e role = role_ping_out;                                               // The role of the current running sketch

byte carsIn = 0;


void setup(){

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(SELF_PIN, OUTPUT);
  pinMode(CE_PIN, OUTPUT);
  
  digitalWrite(SELF_PIN, HIGH);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  
  Serial.begin(115200);
  // Serial.println(F("Arduino Uno - Sending request from car"));
  // Serial.print(F("Initial carIn state:  "));
  //              Serial.print(carsIn);
 
  // Setup and configure radio
  radio.begin();
  radio.enableAckPayload();                     // Allow optional ack payloads
  radio.enableDynamicPayloads();                // Ack payloads are dynamic payloads
  
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1,addresses[1]);
  
  radio.startListening();
}
void loop(void) {
 byte request = 42;     
    
    // Radio is in ping mode
    
    radio.stopListening();                                  // First, stop listening so we can talk.      
    // Serial.println(F("Now sending "));
    // unsigned long time = micros();                          // Record the current microsecond count   
                                                            
    if ( radio.write(&request,1) ){                         // Send the counter variable to the other radio 
        if(!radio.available()){                             // If nothing in the buffer, we got an ack but it is blank
            // Serial.print(F("Got blank response. round-trip delay: "));
            // Serial.print(micros()-time);
            // Serial.println(F(" microseconds"));     
        }else{      
            while(radio.available() ){                      // If an ack with payload was received
                radio.read( &carsIn, 1 );                  // Read it, and display the response time
                // unsigned long timer = micros();

                if(carsIn == 1){ // car in garage
                  digitalWrite(LED_RED, HIGH);
                }
                else if(carsIn ==2){ // no car
                  digitalWrite(LED_GREEN, HIGH);
                }
                // Serial.print(F("Got personal response :) :) :) :)   "));
                // Serial.print(carsIn);
                // Serial.print(F(" round-trip delay: "));
                // Serial.print(timer-time);
                // Serial.println(F(" microseconds"));
            }
        }
    
    }else{        
      //Serial.println(F("Sending failed."));
      }          // If no ack response, sending failed
  
  delay(1000);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(SELF_PIN, LOW);
  delay(200);
}

