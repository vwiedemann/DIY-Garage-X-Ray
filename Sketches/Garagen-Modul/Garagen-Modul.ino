#include <SPI.h>
#include "RF24.h"

/* Radio */
#define CE_PIN   9
#define CSN_PIN 53

/* Ultrasonic */
#define TRIGGER 10
#define ECHO 11
#define MAX_DISTANCE 250

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins CE & CSN */
RF24 radio(CE_PIN,CSN_PIN);

byte addresses[][6] = {"1Node","2Node"};              // Radio pipe addresses for the 2 nodes to communicate.
// Role management: Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  
typedef enum { role_ping_out = 1, role_pong_back } role_e;                 // The various roles supported by this sketch
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};  // The debug-friendly names of those roles
role_e role = role_pong_back;                                              // The role of the current running sketch

byte carsIn = 4;
byte request = 42;
byte pipeNo = 0;                          // Declare variables for the pipe and the byte received

void setup(){
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
    
  Serial.begin(115200);
  // Serial.println(F("Starting Mega, waiting for Requests, Cars are:"));
  // Serial.print("CarsIn is: ");
  // Serial.println(carsIn);
 
  // Setup and configure radio
  radio.begin();
  radio.enableAckPayload();                     // Allow optional ack payloads
  radio.enableDynamicPayloads();                // Ack payloads are dynamic payloads
 
  radio.openWritingPipe(addresses[1]);        // Both radios listen on the same pipes by default, but opposite addresses
  radio.openReadingPipe(1,addresses[0]);      // Open a reading pipe on address 0, pipe 1

  radio.startListening();                       // Start listening  
  radio.writeAckPayload(pipeNo,&carsIn, 1); 

  attachInterrupt(0, check_radio, LOW); // Attach interrupt handler to interrupt
  
}
void loop(void) {
  updateUltrasonics();
  delay(4000);
}

void check_radio(void)
{
    while( radio.available(&pipeNo)){              // Read all available payloads
      radio.read( &request, 1 );                   
      if(request == 42){                                            // We do not want to respond to anyone
        radio.writeAckPayload(pipeNo,&carsIn, 1);  // This can be commented out to send empty payloads.
      }
      // Serial.print("Request content is: ");
      // Serial.println(request);
    }  

  // Serial.println("Damn, got interrupted, sleeping was so nice");
}

void updateUltrasonics(){
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);

  long duration = pulseIn(ECHO, HIGH);
  long distance = computeDistance(duration);
  if(distance < 120){ // car in garage
    carsIn = (byte) 1;
  }
  else{
    carsIn = (byte) 2;
  }
}

long computeDistance(long duration){
  // Schallgeschwindigkeit: 340 m/s
  // 29 Microsek pro cm
  // Strecke hin und zurück
  long distance = (duration/29)/2;
  if(distance > MAX_DISTANCE){
    distance = MAX_DISTANCE;
  }
  return distance;
}

