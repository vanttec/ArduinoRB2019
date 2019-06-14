#include <Servo.h>

//X8R Receiver arduino pins
const int PIN_X8R_4 = 10; 
const int PIN_X8R_2 = 9;
const int PIN_X8R_5 = 11;

//Add LED pins
int red_light_pin= 16;
int green_light_pin = 15;
int blue_light_pin = 14;

//autonomous navigation string
String inputString;

//X8R Receiver channels
float channel4; //perpendicular axis lever - rotation
float channel2; //parallel axis lever - displacement
float channel5; //choice lever

//Thrusters declaration
Servo thrusterRight;
Servo thrusterLeft;

void setup() {
  //Pin modes
  pinMode(PIN_X8R_4, INPUT);
  pinMode(PIN_X8R_2, INPUT);
  pinMode(PIN_X8R_5, INPUT);

  //Add LED pin modes
  pinMode(red_light_pin, OUTPUT);
  pinMode(green_light_pin, OUTPUT);
  pinMode(blue_light_pin, OUTPUT);

  //PIN Thrusters
  thrusterRight.attach(5);
  thrusterLeft.attach(3);

  //Stop thrusters, required
  thrusterRight.writeMicroseconds(1500);
  thrusterLeft.writeMicroseconds(1500);
  
  //Driver setup
  delay(1000);
  Serial.begin(115200);
}

void read_values () {
  //Read channel frequecies
  channel4 = pulseIn(PIN_X8R_4, HIGH);
  //Serial.print("CH 4 ");
  //Serial.println(channel4);
  channel2 = pulseIn(PIN_X8R_2, HIGH);
  //Serial.print("CH 2 ");
  //Serial.println(channel2);
  channel5 = pulseIn(PIN_X8R_5, HIGH);
  //Serial.print("CH 5 ");
  //Serial.println(channel5);
}

void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  if (red_light_value == 1) {
    digitalWrite(red_light_pin, HIGH);
  }
  else {
    digitalWrite(red_light_pin, LOW);
  }
  
  if (green_light_value == 1) {
    digitalWrite(green_light_pin, HIGH);
  }
  else {
    digitalWrite(green_light_pin, LOW);
  }

  if (blue_light_value == 1) {
    digitalWrite(blue_light_pin, HIGH);
  }
  else {
    digitalWrite(blue_light_pin, LOW);
  }
  
 }

void select() {
  //Use channel 5 to select current mode
  if (channel5 < 1300) {
     power_Difference();
     RGB_color(1, 1, 0); // Yellow
  }
  else if ( channel5 > 1600) {
      autonomous_Mode();
      RGB_color(0, 0, 1); // Blue
  }
  else {
      thrusterRight.writeMicroseconds(1500);
      thrusterLeft.writeMicroseconds(1500);
      RGB_color(1, 0, 0); // Red
  }
}

void power_Difference() {
//void for controlled movement
  //float YR;
  //float YL;
  float R;
  float L;
  //uncommnent if you want to use the old version

  float Tx;
  float Tz;
  float Tport;
  float Tstbd;

    //Stop thrusters, required
  thrusterRight.writeMicroseconds(1500);
  thrusterLeft.writeMicroseconds(1500);
  
  //Driver setup
  //delay(1000);

  if ((channel4 > 1450 & channel4 < 1550) & (channel2 > 1450 & channel2 < 1550)){     //Control stable
    R=1500;
    L=1500;
    thrusterRight.writeMicroseconds(R);
    thrusterLeft.writeMicroseconds(L);      //thrusters at zero
  }
  /* old version
  else if ((channel4 > 1450 & channel4 < 1550) & (channel2 < 1450 || channel2 > 1550)) {    //Control in advance
    R=map(channel2, 975, 2025, 1100, 1900);
    L=map(channel2, 975, 2025, 1100, 1900);
    L = (L - 1500) / 1.27 + 1500;
    thrusterRight.writeMicroseconds(R);
    thrusterLeft.writeMicroseconds(L);      //thrusters forward or backward
  }
  else if ((channel4 < 1450 || channel4 > 1550) & (channel2 > 1450 & channel2 < 1550)) {    //Control for rotation both left and right
    R = map(channel4, 975, 2025, 1900, 1100);
    L = map(channel4, 975, 2025, 1100, 1900);
    L = (L - 1500) / 1.27 + 1500;
    thrusterRight.writeMicroseconds(R);
    thrusterLeft.writeMicroseconds(L);    //left thruster is reversed for rotations
  }
  else if ((channel4 < 1450 || channel4 > 1550) & (channel2 < 1450 || channel2 > 1550)) {    //Control for turning
    YR = (channel2/2 - channel4/2);
    YL = (channel2/2 + channel4/2);
    R = map(YR, 975, 2025, 1100, 1900);
    L = map(YL, 975, 2025, 1100, 1900);
    L = (L - 1500) / 1.27 + 1500;
    thrusterRight.writeMicroseconds(R);
    thrusterLeft.writeMicroseconds(L);
  } */
  else if ((channel4 < 1450 || channel4 > 1550) || (channel2 < 1450 || channel2 > 1550)) {    //Control for moving
    Tx = map(channel2, 975, 2025, -70, 70);
    Tz = map(channel4, 975, 2025, -5, 5);
    
    Tstbd = (Tx / 2) - (Tz / 0.41);
    if (Tstbd > 35){
      Tstbd = 35;
    }
    else if (Tstbd < -35){
      Tstbd = -35;
    }

    Tport = (Tx / (2 * 1.27)) + (Tz / (0.41 * 1.27));
    if (Tport > 27){
      Tport = 27;
    }
    else if (Tstbd < -27){
      Tstbd = -27;
    }

    R = round((Tstbd / 35 * 400)+1500);
    L = round((Tport / 35 * 400)+1500);
    thrusterRight.writeMicroseconds(R);
    thrusterLeft.writeMicroseconds(L);
  }

}

void autonomous_Mode() {
  //void for autonomous navigation communicated via serial
  // put your main code here, to run repeatedly:
    // serial read section
    char c;
    
    while (Serial.available() > 0) {
        char c = Serial.read();
        inputString += c;  
        //wait for the next byte, if after this nothing has arrived it means 
        //the text was not part of the same stream entered by the user
        delay(1); 
    }
    
  //0123456789012
  //%B,1500,1500%

  //01234567890123456
  //%B,1500.0,1500.0%
  
    if(inputString[0] == '%' && inputString.length() > 0 && inputString.length() < 18 && inputString[inputString.length() - 1] == '%' && inputString != ""){
      Serial.println(inputString);
      if(inputString[1] == 'B') {
          String valRight = inputString.substring(3,7);
          String valLeft = inputString.substring(10,14);
          //Serial.println(valLeft);
          int powerL = valLeft.toInt();
          int powerR = valRight.toInt();
          
          thrusterRight.writeMicroseconds(powerR);
         // Serial.print(powerR);
          thrusterLeft.writeMicroseconds(powerL);
          //Serial.print(powerL);
        }
        //Left thrusters
        else if(inputString[1] == 'L') {
          String valLeft = inputString.substring(3,7);
          //Serial.println(valLeft);
          int power = valLeft.toInt();
          thrusterLeft.writeMicroseconds(power); 
          //Serial.print(power);
        }
        //Right thrusters
        else if(inputString[1] == 'R') {
          String valRight = inputString.substring(3,7);
          //Serial.println(valRight);
          int power = valRight.toInt();
          thrusterRight.writeMicroseconds(power);
          //Serial.print(power);
        }
    }  
    //Delete Previous Message
     inputString = "";
}

void loop() {
  read_values();
  select();
}
