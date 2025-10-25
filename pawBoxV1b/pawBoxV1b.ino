#include "UnoJoy.h"
#include <EEPROM.h>

byte socdMode = 0;   // Guardará 1 a 4 según la configuración
byte stickMode = 0;  // Guardará 1 o 2 según Left-stick o D-Pad

byte socdAddress = 0;  // Dirección en la EEPROM donde se almacenará el socd mode
byte stickAddress = 1; // Dirección en la EEPROM donde se almacenará el movimiento digital(d-pad) o analogico (left stick)

const byte threshold = 100; // Umbral para determinar si el botón analogico está presionado

// Variables para control de tiempo
unsigned long l1PressedTime = 0;
unsigned long r1PressedTime = 0;
bool l1WasPressed = false;
bool r1WasPressed = false;

// Variables globales para temporizador
unsigned long comboStartTime = 0;
bool comboActive = false;

// Define our pins
byte SquarePin = 9; 
byte TrianglePin = 8; 
byte CrossPin = 7; 
byte CirclePin = 6; 

byte i=0;
byte r1=5;
byte r2=3;
byte l1=4;
byte l2=2;

byte LeftPin = 13;
byte UpPin = 10;
byte RightPin = 11;
byte DownPin = 12;

byte l3 = A5;
byte r3 = A4;

//firmware with just the right analog stick
byte RightX =A0;
byte RightY =A1;
//byte LeftX =A2;
//byte LeftY =A3;
byte StartPin = A2;
byte SelectPin = A3;



unsigned long currentMillis;//global

unsigned long leftMillis = 0;
unsigned long rightMillis = 0;

void setupPins(void){
  // Set all the digital pins as inputs
  // with the pull-up enabled, except for the 
  // two serial line pins
  for (byte i = 2; i <= 13; i++){
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }

  pinMode(l3, INPUT); // Configura el pin como entrada
  digitalWrite(l3, HIGH); // Activa la resistencia pull-up
  pinMode(r3, INPUT); // Configura el pin como entrada
  digitalWrite(r3, HIGH); // Activa la resistencia pull-up

  pinMode(SelectPin, INPUT); // Configura el pin como entrada
  digitalWrite(SelectPin, HIGH); // Activa la resistencia pull-up
  pinMode(StartPin, INPUT); // Configura el pin como entrada
  digitalWrite(StartPin, HIGH); // Activa la resistencia pull-up


}

void setup(){
  
  setupPins();
  setupUnoJoy();
  socdMode = EEPROM.read(socdAddress);
  stickMode = EEPROM.read(stickAddress);
    
}

// ---- Set up for buttons and analogs----
dataForController_t getBaseControllerData() {
  // Set up a place for our controller data
  //  Use the getBlankDataForController() function, since
  //  just declaring a fresh dataForController_t tends
  //  to get you one filled with junk from other, random
  //  values that were in those memory locations before
    dataForController_t controllerData = getBlankDataForController();

    // Botones
    controllerData.triangleOn = !digitalRead(TrianglePin);
    controllerData.circleOn   = !digitalRead(CirclePin);
    controllerData.squareOn   = !digitalRead(SquarePin);
    controllerData.crossOn    = !digitalRead(CrossPin);
    controllerData.l1On       = !digitalRead(l1);
    controllerData.l2On       = !digitalRead(l2);
    controllerData.r1On       = !digitalRead(r1);
    controllerData.r2On       = !digitalRead(r2);

    // no analog sticks in this firmware

    controllerData.rightStickX = analogRead(RightX) >> 2;
    controllerData.rightStickY = analogRead(RightY) >> 2;
    /*
    controllerData.leftStickX  = analogRead(LeftX) >> 2;
    controllerData.leftStickY  = analogRead(LeftY) >> 2;
    */

  // Leer el valor analógico del pin
  int analogValuel3 = analogRead(l3);
  int analogValuer3 = analogRead(r3);
  int analogValueSelect = analogRead(SelectPin);
  int analogValueStart = analogRead(StartPin);

  // Determinar si el botón está presionado
  bool buttonPressedl3 = analogValuel3 < threshold;
  bool buttonPressedr3 = analogValuer3 < threshold;

  if (buttonPressedl3 && buttonPressedr3){
    controllerData.l3On=1;
    controllerData.r3On=1;

  if (!comboActive) {
      comboActive = true;
      comboStartTime = millis();  // guardamos el tiempo al iniciar combo
    } else {
      // Si la combinación sigue activa por más de 3 segundos
      if (millis() - comboStartTime >= 3000) {
        if (!digitalRead(SquarePin)) {
          EEPROM.update(socdAddress, 1);
          socdMode = 1;  //  actualizamos variable
          
        }
        if (!digitalRead(TrianglePin)) {
          EEPROM.update(socdAddress, 2);
          socdMode = 2;  //  actualizamos variable
          
        }
        if (!digitalRead(CrossPin)) {
          EEPROM.update(socdAddress, 3);
          socdMode = 3;  //  actualizamos variable
          
        }
        if (!digitalRead(CirclePin)) {
          EEPROM.update(socdAddress, 4);
          socdMode = 4;  //  actualizamos variable
          
        }
        comboActive = false; // evita múltiples escrituras hasta soltar
      }
    }

    // --- L1 ---
    if (!digitalRead(l1)) { // botón apretado (activo en LOW)
      if (!l1WasPressed) {
        l1WasPressed = true;
        l1PressedTime = millis(); // guardamos el tiempo de inicio
      } else {
        // Si pasaron 3 segundos y aún sigue apretado
        if (millis() - l1PressedTime >= 3000) {
          EEPROM.update(1, 2); // Guardar "digital mode"
          stickMode=2;

          l1WasPressed = false; // evitamos que escriba de nuevo hasta soltar
        }
      }
    } else {
      l1WasPressed = false; // se resetea al soltar el botón
    }

    // --- R1 ---
    if (!digitalRead(r1)) {
      if (!r1WasPressed) {
        r1WasPressed = true;
        r1PressedTime = millis();
      } else {
        if (millis() - r1PressedTime >= 3000) {
          EEPROM.update(1, 1); // Guardar "analog mode"
          stickMode=1;

          r1WasPressed = false;
        }
      }
    } else {
      r1WasPressed = false;
    }
    //no need to use combo activation for start and select
    /*
    if ( !digitalRead(l2) && !digitalRead(r2)){ //home button activation
      controllerData.l2On = 0;
      controllerData.r2On = 0;
      controllerData.l3On=0;
      controllerData.r3On=0; 
      controllerData.homeOn = 1;
    }else{

        if ((!digitalRead(l2)) == 1) { //Start button activation
          controllerData.l2On = 0;
          controllerData.l3On=0;
          controllerData.r3On=0; 
          controllerData.startOn = 1;
        }
        
        if ((!digitalRead(r2)) == 1) { //Select button activation
          controllerData.r2On = 0;
          controllerData.l3On=0;
          controllerData.r3On=0; 
          controllerData.selectOn = 1;
        }
    }
    */

  }else{
    comboActive = false;
    if (buttonPressedl3) {
      controllerData.l3On=1;
    }
    if (buttonPressedr3) {
      controllerData.r3On=1;
    }

  }

  
  // Determinar si el botón está presionado
  bool buttonPressedSelect = analogValueSelect < threshold;
  bool buttonPressedStart = analogValueStart < threshold;
  // Hacer algo si el botón está presionado  
  if (buttonPressedSelect && buttonPressedStart) {
    controllerData.homeOn=1;
  }else{
    if (buttonPressedStart) {
      controllerData.startOn=1;
    }

    if (buttonPressedSelect) {
      controllerData.selectOn=1;
    }
  }

    return controllerData;
}


dataForController_t getControllerDataLastPriority(void){ // codigo de ejecucion SOCD, con prioridad en el ultimo boton presionado.
  
  dataForController_t controllerData = getBaseControllerData(); 

  uint8_t left = !digitalRead(LeftPin);//se lee el pin left en caso de ser presionado guarda 1 sino 0
  uint8_t right =!digitalRead(RightPin);//se lee el pin right en caso de ser presionado guarda 1 sino 0
  uint8_t up =!digitalRead(UpPin);
  uint8_t down =!digitalRead(DownPin);


//Codigo de prioridad segundo presionado
  if (left == 1){ //si left presionado primero

    if (right == 1){ //si right presionado segundo
      
      if (leftMillis>rightMillis){ //si left presionado primero entonces
        //rightPriority=1;
        controllerData.leftStickX = 255;//controllerData.dpadRightOn = 1;   sale right    
      }
      
    }else{//si left es presionado y right no
      controllerData.leftStickX = 0;//controllerData.dpadLeftOn = 1; sale left
      leftMillis = currentMillis; // guardo el tiempo de left, si es presionado left primero
    }
  }

  if (right == 1){  //si right presionado primero

    if (left == 1){ //si left presionado segundo
      
      if (rightMillis>leftMillis){ //si right presionado primero entonces
        //leftPriority=1;
        controllerData.leftStickX = 0;//controllerData.dpadLeftOn = 1;      sale left  
      }    
 
    }else{//si right es presionado y left no
      controllerData.leftStickX = 255;//controllerData.dpadRightOn = 1;     sale right
      rightMillis = currentMillis; //guardo tiempo de right
    }
  }

  if (up && down){
    controllerData.leftStickX=128;
    controllerData.leftStickY=0;

  }else{
    if (up && !down){
      controllerData.leftStickY=0;

    }
    if (!up && down){
      controllerData.leftStickY=255;

    }

  }

  return controllerData;
}

dataForController_t getControllerDataLastPriorityDPAD(void){ // codigo de ejecucion SOCD, con prioridad en el ultimo boton presionado.
  
  dataForController_t controllerData = getBaseControllerData(); 

  uint8_t left = !digitalRead(LeftPin);//se lee el pin left en caso de ser presionado guarda 1 sino 0
  uint8_t right =!digitalRead(RightPin);//se lee el pin right en caso de ser presionado guarda 1 sino 0
  uint8_t up =!digitalRead(UpPin);
  uint8_t down =!digitalRead(DownPin);


//Codigo de prioridad segundo presionado
  if (left == 1){ //si left presionado primero

    if (right == 1){ //si right presionado segundo
      
      if (leftMillis>rightMillis){ //si left presionado primero entonces
        //rightPriority=1;
        controllerData.dpadRightOn = 1;   //sale right    
      }
      
    }else{//si left es presionado y right no
      controllerData.dpadLeftOn = 1; //sale left
      leftMillis = currentMillis; // guardo tiempo de left
    }
  }

  if (right == 1){  //si right presionado primero

    if (left == 1){ //si left presionado segundo
      
      if (rightMillis>leftMillis){ //si right presionado primero entonces
        //leftPriority=1;
        controllerData.dpadLeftOn = 1;      //sale left  
      }    
 
    }else{//si right es presionado y left no
      controllerData.dpadRightOn = 1;     //sale right
      rightMillis = currentMillis; //guardo tiempo de right
    }
  }

  if (up && down){

    controllerData.dpadLeftOn = 0;
    controllerData.dpadRightOn = 0;
    controllerData.dpadUpOn = 1;

  }else{
    if (up && !down){
      controllerData.dpadUpOn = 1;

    }
    if (!up && down){

      controllerData.dpadDownOn = 1;

    }

  }

  return controllerData;
}


dataForController_t getControllerDataNeutral(void){

  dataForController_t controllerData = getBaseControllerData(); 

    //logica socd up + down = neutral
  if (!digitalRead(UpPin) && !digitalRead(DownPin)) {
    controllerData.leftStickY = 128;

    
  }else{
    if (!digitalRead(UpPin)){

      controllerData.leftStickY = 0;
    }
    if (!digitalRead(DownPin)){
      controllerData.leftStickY = 255;

    }
    
  }


  if ( (!digitalRead(RightPin)) && (!digitalRead(LeftPin)) ){
    controllerData.leftStickX = 128;
  
  }else{

  if(!digitalRead(LeftPin)) {

        controllerData.leftStickX = 0;
  } 

  if(!digitalRead(RightPin)) {

        controllerData.leftStickX = 255;

  }

  }


  return controllerData;
}


dataForController_t getControllerDataNeutralDPAD(void){

  dataForController_t controllerData = getBaseControllerData(); 

  //logica socd up + down = neutral
  if (!digitalRead(UpPin) && !digitalRead(DownPin)) {
    controllerData.dpadDownOn = 0;
    controllerData.dpadUpOn = 0;
    
  }else{
    if (!digitalRead(UpPin)){
      controllerData.dpadUpOn = 1;

    }
    if (!digitalRead(DownPin)){
      controllerData.dpadDownOn = 1;
    }
    
  }


  if ( (!digitalRead(RightPin)) && (!digitalRead(LeftPin)) ){
    controllerData.dpadRightOn = 0;
    controllerData.dpadLeftOn = 0;
  }else{

  if(!digitalRead(LeftPin)) {
    controllerData.dpadLeftOn = 1;
  } 

  if(!digitalRead(RightPin)) {
    controllerData.dpadRightOn = 1;
  }

  }

  return controllerData;
}

dataForController_t getControllerDataUpNeutral(void){
  
  dataForController_t controllerData = getBaseControllerData(); 

  //logica socd up + down = neutral
  if (!digitalRead(UpPin) && !digitalRead(DownPin)) {
    controllerData.leftStickY = 0;
 
    
  }else{
    if (!digitalRead(UpPin)){

      controllerData.leftStickY = 0;
    }
    if (!digitalRead(DownPin)){
      controllerData.leftStickY = 255;

    }
    
  }


  if ( (!digitalRead(RightPin)) && (!digitalRead(LeftPin)) ){
    controllerData.leftStickX = 128;

  }else{

  if(!digitalRead(LeftPin)) {

        controllerData.leftStickX = 0;
  } 

  if(!digitalRead(RightPin)) {

        controllerData.leftStickX = 255;

  }

  }

  return controllerData;
}

dataForController_t getControllerDataUpNeutralDPAD(void){
  
  dataForController_t controllerData = getBaseControllerData(); 

  //logica socd up + down = up
  if (!digitalRead(UpPin) && !digitalRead(DownPin)) {
    controllerData.dpadUpOn = 1;   
  }else{
    if (!digitalRead(UpPin)){
      controllerData.dpadUpOn = 1;

    }
    if (!digitalRead(DownPin)){
      controllerData.dpadDownOn = 1;
    }
    
  }


  if ( (!digitalRead(RightPin)) && (!digitalRead(LeftPin)) ){
    controllerData.dpadRightOn = 0;
    controllerData.dpadLeftOn = 0;
  }else{

  if(!digitalRead(LeftPin)) {
    controllerData.dpadLeftOn = 1;
  } 

  if(!digitalRead(RightPin)) {
    controllerData.dpadRightOn = 1;

  }

  }


  return controllerData;
}


dataForController_t getControllerDataDownNeutral(void){
  
  dataForController_t controllerData = getBaseControllerData(); 

  //logica socd up + down = neutral
  if (!digitalRead(UpPin) && !digitalRead(DownPin)) {
    controllerData.leftStickY = 255;
 
    
  }else{
    if (!digitalRead(UpPin)){

      controllerData.leftStickY = 0;
    }
    if (!digitalRead(DownPin)){
      controllerData.leftStickY = 255;


    }
    
  }


  if ( (!digitalRead(RightPin)) && (!digitalRead(LeftPin)) ){
    controllerData.leftStickX = 128;

  }else{

  if(!digitalRead(LeftPin)) {

        controllerData.leftStickX = 0;
  } 

  if(!digitalRead(RightPin)) {

        controllerData.leftStickX = 255;

  }

  }

  return controllerData;
}

dataForController_t getControllerDataDownNeutralDPAD(void){
  
  dataForController_t controllerData = getBaseControllerData(); 

  //logica socd up + down = down
  if (!digitalRead(UpPin) && !digitalRead(DownPin)) {

    controllerData.dpadDownOn = 1;
    
  }else{
    if (!digitalRead(UpPin)){

      controllerData.dpadUpOn = 1;
    }
    if (!digitalRead(DownPin)){
      controllerData.dpadDownOn = 1;

    }
    
  }

  if ( (!digitalRead(RightPin)) && (!digitalRead(LeftPin)) ){
    controllerData.dpadRightOn = 0;
    controllerData.dpadLeftOn = 0;
  }else{

  if(!digitalRead(LeftPin)) {
    controllerData.dpadLeftOn = 1;
  } 

  if(!digitalRead(RightPin)) {
    controllerData.dpadRightOn = 1;

  }

  }

  return controllerData;
}

void loop(){
  // Always be getting fresh data
  currentMillis = millis();

  if (stickMode != 2) { // analog mode
      switch(socdMode) {
          case 1: setControllerData(getControllerDataNeutral()); break;
          case 2: setControllerData(getControllerDataLastPriority()); break;
          case 3: setControllerData(getControllerDataUpNeutral()); break;
          case 4: setControllerData(getControllerDataDownNeutral()); break;
      }
  } else { // digital mode
      switch(socdMode) {
          case 1: setControllerData(getControllerDataNeutralDPAD()); break;
          case 2: setControllerData(getControllerDataLastPriorityDPAD()); break;
          case 3: setControllerData(getControllerDataUpNeutralDPAD()); break;
          case 4: setControllerData(getControllerDataDownNeutralDPAD()); break;
      }
  }

}