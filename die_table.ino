#include "IRremote.h"
#include <FastLED.h>


/*-----( Global Constants )-----*/

#define NUM_LEDS 150

// The folLOWing two values were reversed for an unknown reason.
#define low HIGH
#define high LOW

#define DELAY_VAL 4 // Signal delay between switching digits
#define BLINK_CONSTANT 500
#define MILLISECOND 1
#define TEAM_1 1
#define TEAM_2 2
#define NO_SELECTION -1
#define DEFAULT_ 0
#define DISPLAY_NUM_DIGITS 4

/*
  Showing number 0-9 on a Common Anode 7-segment LED display
  Displays the numbers 0-9 on the display, with one second inbetween.
    A
   ---
F |   | B
  | G |
   ---
E |   | C
  |   |
   ---
    D
  This example code is in the public domain.
 */

// Pin assignments 
// Pin 2-8 is connected to the 7 segments of the display.
// Pin 9-12 is connected to the display digits.
#define PIN_A 2
#define PIN_B 3
#define PIN_C 4
#define PIN_D 5
#define PIN_E 6
#define PIN_F 7
#define PIN_G 8
#define D1 9
#define D2 10
#define D3 11
#define D4 12
#define RECEIVER 13      // Signal Pin of IR RECEIVER to Arduino Digital Pin 13
#define LED_PIN A0       // WS2812B LED Strip to Arduino Analog Pin 0


/*-----( Declare objects )-----*/

// Array containing light RGB values
CRGB leds[NUM_LEDS];

// Instance Vars
int functionListLength = 2;
int function = DEFAULT_;
String functionList [2] = {"scoreboardOn", "controlLights"}; // functionList length must equal functionListLength
bool scoreboardOn = true;
bool lightsOn = false;
int prevLights = DEFAULT_;
int selectedValue = NO_SELECTION;
int selectedTeam = NO_SELECTION;
int firstValue = DEFAULT_;
int secondValue = DEFAULT_;
int thirdValue = DEFAULT_;
int fourthValue = DEFAULT_;

IRrecv irrecv(RECEIVER);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'


/*----( SETUP: RUNS ONCE )----*/

void setup(){   
  Serial.begin(9600);
  Serial.println("IR RECEIVER Button Decode"); 
  irrecv.enableIRIn();           // Start the RECEIVER
  pinMode(PIN_A, OUTPUT);     
  pinMode(PIN_B, OUTPUT);     
  pinMode(PIN_C, OUTPUT);     
  pinMode(PIN_D, OUTPUT);     
  pinMode(PIN_E, OUTPUT);     
  pinMode(PIN_F, OUTPUT);     
  pinMode(PIN_G, OUTPUT);   
  pinMode(D1, OUTPUT);  
  pinMode(D2, OUTPUT);  
  pinMode(D3, OUTPUT);  
  pinMode(D4, OUTPUT);  

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
}


/*----( LOOP: RUNS CONSTANTLY )----*/

void loop(){   
  runRemote();
  runScoreboard();
  runSeismicSensor();
}


/*----( REMOTE FUNCTIONS )----*/

void runRemote() {
  if (irrecv.decode(&results))   // have we received an IR signal?
  {
    translateIR();                 // Do not get immediate repeat
    irrecv.resume();            // receive the next value
  }  
}

void translateIR() {          // takes action based on IR code received
// describing Remote IR codes 
  switch(results.value){
    case 0xFFA25D:  if (functionList[function] == "controlLights") { lightSwitch(); } if (functionList[function] == "scoreboardOn") { scoreboardOn = !scoreboardOn; if (scoreboardOn) { deselect(); }} Serial.println("POWER"); break;
    case 0xFFE21D:  if (functionList[function] == "scoreboardOn") { resetSelectedScore(); } Serial.println("FUNC/STOP"); break;
    case 0xFF629D:  Serial.println("VOL+"); break;
    case 0xFF22DD:  if (functionList[function] == "scoreboardOn") { previousNumber(); } Serial.println("FAST BACK");  break;
    case 0xFF02FD:  if (functionList[function] == "scoreboardOn") { selectNumber(); } Serial.println("PAUSE"); break;
    case 0xFFC23D:  if (functionList[function] == "scoreboardOn") { nextNumber(); } Serial.println("FAST FORWARD"); break;
    case 0xFFE01F:  if (functionList[function] == "scoreboardOn") { decrementSelectedValue(); } Serial.println("DOWN"); break;
    case 0xFFA857:  Serial.println("VOL-"); break;  
    case 0xFF906F:  if (scoreboardOn) { incrementSelectedValue(); } Serial.println("UP"); break; 
    case 0xFF9867:  switchFunction(); Serial.println("EQ"); break;    
    case 0xFFB04F:  if (functionList[function] == "controlLights") { flashLights(); } Serial.println("ST/REPT"); break;     
    case 0xFF6897:  if (functionList[function] == "controlLights") { lightsWhite(); } if (functionList[function] == "scoreboardOn") { deselect(); } Serial.println("0"); break; 
    case 0xFF30CF:  if (functionList[function] == "controlLights") { lightsRed(); } if (functionList[function] == "scoreboardOn") { selectTeam1(); } Serial.println("1"); break; 
    case 0xFF18E7:  if (functionList[function] == "controlLights") { lightsBlue(); } if (functionList[function] == "scoreboardOn") { selectTeam2(); } Serial.println("2"); break; 
    case 0xFF7A85:  if (functionList[function] == "controlLights") { lightsGreen(); } Serial.println("3"); break;
    case 0xFF10EF:  if (functionList[function] == "controlLights") { lightsGoBears(); }if (functionList[function] == "scoreboardOn") { selectTeam1(); incrementSelectedValue(); deselect(); } Serial.println("4"); break; 
    case 0xFF38C7:  if (functionList[function] == "controlLights" ) { lightsYouthfulAndWise(); } Serial.println("5");  break;   
    case 0xFF5AA5:  if (functionList[function] == "controlLights" ) { lightsWinnie(); } if (functionList[function] == "scoreboardOn") { selectTeam2(); incrementSelectedValue(); deselect(); } Serial.println("6"); break; 
    case 0xFF42BD:  if (functionList[function] == "controlLights" ) { lightsIreland(); } if (functionList[function] == "scoreboardOn") { selectTeam1(); decrementSelectedValue(); deselect(); } Serial.println("7"); break; 
    case 0xFF4AB5:  if (functionList[function] == "controlLights" ) { lightsRainbow(); } Serial.println("8"); break;    
    case 0xFF52AD:  if (functionList[function] == "controlLights" ) { lightsCycle(); } if (functionList[function] == "scoreboardOn") { selectTeam2(); decrementSelectedValue(); deselect(); } Serial.println("9"); break; 
    case 0xFFFFFFFF:  Serial.println(" REPEAT"); break;

  default: 
    break;
     Serial.print(" other button   ");
     Serial.println(results.value);

  }
} //END translateIR

void switchFunction() {
  deselect();
  if (function + 1 < functionListLength) {
    function += 1;
  } else {
    function = 0;
  }
}


/*----( SCOREBOARD HELPER FUNCTIONS )----*/

void runScoreboard() {
   display(MILLISECOND, false);
   if (scoreboardOn) {
    if (selectedValue != NO_SELECTION || selectedTeam != NO_SELECTION)  {
      display(BLINK_CONSTANT, false);
      display(BLINK_CONSTANT, true);
    }
  }
}

void display(int t, bool blink) {
  for (int j = 0; j * DISPLAY_NUM_DIGITS * DELAY_VAL < t; j ++) { 
    runRemote();
    if (scoreboardOn) {
      for (int i = 1; i <= DISPLAY_NUM_DIGITS; i++) {
        int value = NO_SELECTION;
        switch(i) {
          case 1: writeFirst(); value = firstValue; break;
          case 2: writeSecond(); value = secondValue; break;
          case 3: writeThird(); value = thirdValue; break;
          case 4: writeFourth(); value = fourthValue; break;
      
          default: break;
        }
        
        if (blink) {
          if ((selectedTeam == TEAM_1 && (i == 1 || i == 2)) || (selectedTeam == TEAM_2 && (i == 3 || i == 4))) {
            value = NO_SELECTION;
          }
          if (selectedValue == i) {
            value = NO_SELECTION;
          }
        }
  
       switch(value) {
        case -1: writeBlank(); break;
        case 0: write0(); break;
        case 1: write1(); break;
        case 2: write2(); break;
        case 3: write3(); break;
        case 4: write4(); break;
        case 5: write5(); break;
        case 6: write6(); break;
        case 7: write7(); break;
        case 8: write8(); break;
        case 9: write9(); break;
      
        default: break;
       }
  
       delay(DELAY_VAL);
      }
    } else {
      writeAll();
      writeBlank();
    }
  }
}

// This function is overwritten by the display function and is thus not used
void powerDownScoreboard() {
  writeAll();
  writeBlank();
}

void resetSelectedScore() {
   Serial.println("resetting team "+ selectedTeam);
  if (selectedTeam == TEAM_1) {
    firstValue = DEFAULT_;
    secondValue = DEFAULT_;
  } 
  if (selectedTeam == TEAM_2) {
    thirdValue = DEFAULT_;
    fourthValue = DEFAULT_;
  }

  if (selectedValue != NO_SELECTION) {
    switch(selectedValue) {
        case 1: firstValue = DEFAULT_; break;
        case 2: secondValue = DEFAULT_; break;
        case 3: thirdValue = DEFAULT_; break;
        case 4: fourthValue = DEFAULT_; break;
    
        default: break;
      }
  }
}

void deselect() {
  if (selectedTeam != NO_SELECTION) {
     Serial.println("deselecting team " + selectedTeam);
    selectedTeam = NO_SELECTION;
  }
  if (selectedValue != NO_SELECTION) {
     Serial.println("deslecting digit number " + selectedValue);
    selectedValue = NO_SELECTION;
  }
}

void selectTeam1() {
  deselect();
  Serial.println("selecting team 1");
  selectedTeam = TEAM_1;
}

void selectTeam2() {
  deselect();
  Serial.println("selecting team 2");
  selectedTeam = TEAM_2;
}

void selectNumber() {
  deselect();
  selectedValue = 1;
}

void nextNumber() {
  if (selectedValue != NO_SELECTION && selectedValue != DISPLAY_NUM_DIGITS) {
    selectedValue += 1;
  }
}

void previousNumber() {
  if (selectedValue != NO_SELECTION && selectedValue != 1) {
    selectedValue -= 1;
  }
}

void incrementSelectedValue() {
  switch(selectedValue) {
    case 1: if (firstValue < 9) {firstValue += 1;} else { if (firstValue == 9) { firstValue = 0; }}; break;
    case 2: if (secondValue < 9) {secondValue += 1;} else { if (secondValue == 9) { secondValue = 0; }}; break;
    case 3: if (thirdValue < 9) {thirdValue += 1;} else { if (thirdValue == 9) { thirdValue = 0; }}; break;
    case 4: if (fourthValue < 9) {fourthValue += 1;} else { if (fourthValue == 9) { fourthValue = 0; }}; break;

    default: break;
  }

  if (selectedTeam == TEAM_1) {
    if (secondValue != 9) {
      secondValue += 1;
    } else {
      if (firstValue != 9) {
        firstValue += 1;
      } else {
        firstValue = 0;
      }
      secondValue = 0;
    }
  }

  if (selectedTeam == TEAM_2) {
    if (fourthValue != 9) {
      fourthValue += 1;
    } else {
      if (thirdValue != 9) {
        thirdValue += 1;
      } else {
        thirdValue = 0;
      }
      fourthValue = 0;
    }
  }
}

void decrementSelectedValue() {
  switch(selectedValue) {
    case 1: if (firstValue > 0) {firstValue -= 1;} else { if (firstValue == 0) { firstValue = 9; }}; break;
    case 2: if (secondValue > 0) {secondValue -= 1;} else { if (secondValue == 0) { secondValue = 9; }}; break;
    case 3: if (thirdValue > 0) {thirdValue -= 1;} else { if (thirdValue == 0) { thirdValue = 9; }}; break;
    case 4: if (fourthValue > 0) {fourthValue -= 1;} else { if (fourthValue == 0) { fourthValue = 9; }}; break;

    default: break;
  }

  if (selectedTeam == TEAM_1) {
    if (secondValue != 0) {
      secondValue -= 1;
    } else {
      if (firstValue != 0) {
        firstValue -= 1;
      } else {
        firstValue = 9;
      }
      secondValue = 9;
    }
  }

  if (selectedTeam == TEAM_2) {
    if (fourthValue != 0) {
      fourthValue -= 1;
    } else {
      if (thirdValue != 0) {
        thirdValue -= 1;
      } else {
        thirdValue = 9;
      }
      fourthValue = 9;
    }
  }
}


/*----( 4 DIGIT DISPLAY ENCODINGS )----*/

void writeFirst() {
  digitalWrite(D1, high);
  digitalWrite(D2, low);
  digitalWrite(D3, low);
  digitalWrite(D4, low); 
}

void writeSecond() {
  digitalWrite(D1, low);
  digitalWrite(D2, high);
  digitalWrite(D3, low);
  digitalWrite(D4, low); 
}

void writeThird() {
  digitalWrite(D1, low);
  digitalWrite(D2, low);
  digitalWrite(D3, high);
  digitalWrite(D4, low); 
}

void writeFourth() {
  digitalWrite(D1, low);
  digitalWrite(D2, low);
  digitalWrite(D3, low);
  digitalWrite(D4, high); 
}

void writeAll() {
  digitalWrite(D1, high);
  digitalWrite(D2, high);
  digitalWrite(D3, high);
  digitalWrite(D4, high); 
}

void writeBlank() {
  digitalWrite(PIN_A, high);   
  digitalWrite(PIN_B, high);   
  digitalWrite(PIN_C, high);   
  digitalWrite(PIN_D, high);   
  digitalWrite(PIN_E, high);   
  digitalWrite(PIN_F, high);   
  digitalWrite(PIN_G, high);    
}

void write0() {
  digitalWrite(PIN_A, low);   
  digitalWrite(PIN_B, low);   
  digitalWrite(PIN_C, low);   
  digitalWrite(PIN_D, low);   
  digitalWrite(PIN_E, low);   
  digitalWrite(PIN_F, low);   
  digitalWrite(PIN_G, high); 
}

void write1() {
  digitalWrite(PIN_A, high);   
  digitalWrite(PIN_B, low);   
  digitalWrite(PIN_C, low);   
  digitalWrite(PIN_D, high);   
  digitalWrite(PIN_E, high);   
  digitalWrite(PIN_F, high);   
  digitalWrite(PIN_G, high); 
}

void write2() {
  digitalWrite(PIN_A, low);   
  digitalWrite(PIN_B, low);   
  digitalWrite(PIN_C, high);   
  digitalWrite(PIN_D, low);   
  digitalWrite(PIN_E, low);   
  digitalWrite(PIN_F, high);   
  digitalWrite(PIN_G, low);
}

void write3() {
  digitalWrite(PIN_A, low);   
  digitalWrite(PIN_B, low);   
  digitalWrite(PIN_C, low);   
  digitalWrite(PIN_D, low);   
  digitalWrite(PIN_E, high);   
  digitalWrite(PIN_F, high);   
  digitalWrite(PIN_G, low);
}

void write4() {
  digitalWrite(PIN_A, high);   
  digitalWrite(PIN_B, low);   
  digitalWrite(PIN_C, low);   
  digitalWrite(PIN_D, high);   
  digitalWrite(PIN_E, high);   
  digitalWrite(PIN_F, low);   
  digitalWrite(PIN_G, low); 
}

void write5() {
   digitalWrite(PIN_A, low);   
  digitalWrite(PIN_B, high);   
  digitalWrite(PIN_C, low);   
  digitalWrite(PIN_D, low);   
  digitalWrite(PIN_E, high);   
  digitalWrite(PIN_F, low);   
  digitalWrite(PIN_G, low);  
}

void write6() {
  digitalWrite(PIN_A, low);   
  digitalWrite(PIN_B, high);   
  digitalWrite(PIN_C, low);   
  digitalWrite(PIN_D, low);   
  digitalWrite(PIN_E, low);   
  digitalWrite(PIN_F, low);   
  digitalWrite(PIN_G, low);   
}

void write7() {
  digitalWrite(PIN_A, low);   
  digitalWrite(PIN_B, low);   
  digitalWrite(PIN_C, low);   
  digitalWrite(PIN_D, high);   
  digitalWrite(PIN_E, high);   
  digitalWrite(PIN_F, high);   
  digitalWrite(PIN_G, high);   
}

void write8() {
  digitalWrite(PIN_A, low);   
  digitalWrite(PIN_B, low);   
  digitalWrite(PIN_C, low);   
  digitalWrite(PIN_D, low);   
  digitalWrite(PIN_E, low);   
  digitalWrite(PIN_F, low);   
  digitalWrite(PIN_G, low);   
}

void write9() {
  digitalWrite(PIN_A, low);   
  digitalWrite(PIN_B, low);   
  digitalWrite(PIN_C, low);   
  digitalWrite(PIN_D, high);   
  digitalWrite(PIN_E, high);   
  digitalWrite(PIN_F, low);   
  digitalWrite(PIN_G, low); 
}


/*----( LIGHTS HELPER FUNCTIONS )----*/

//TODO: implement this func when sensor comes
void runSeismicSensor() {
  // if (Signal from seismic sensors) {
  //    flashLights();
  // }
}

void runLights() {
  if (lightsOn) {
    switch(prevLights) {
      case 0: lightsWhite(); break;
      case 1: lightsRed(); break;
      case 2: lightsGreen(); break;
      case 3: lightsBlue(); break;
      case 4: lightsGoBears(); break;
      case 5: lightsYouthfulAndWise(); break;
      case 6: lightsWinnie(); break;
      case 7: lightsIreland(); break;
      case 8: lightsRainbow(); break;
      case 9: lightsCycle(); break;
      default: break;
    }
  } else {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(0, 0, 0);
     }
     FastLED.show();
  }
}

void lightSwitch() {
  lightsOn = !lightsOn;
  runLights();
}

void flashLights() {
  writeAll();
  writeBlank();
  lightSwitch();
  delay(BLINK_CONSTANT);
  lightSwitch();
}

/*----( LIGHTS SETTINGS )----*/

void lightsWhite() {
  for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(100, 255, 255);
   }
   FastLED.show();
   prevLights = 0;
   lightsOn = true;
}

void lightsRed() {
  for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(255,0,0);
   }
   FastLED.show();
   prevLights = 1;
   lightsOn = true;
}

void lightsGreen() {
  for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(0,255,0);
   }
   FastLED.show();
   prevLights = 2;
   lightsOn = true;
}

void lightsBlue() {
  for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(0,0,255);
   }
   FastLED.show();
   prevLights = 3;
   lightsOn = true;
}

void lightsGoBears() {
  const int numLightsBeforeChange = 1;
  int count = 0;
  bool blue = true;
  for (int i = 0; i < NUM_LEDS; i++) {
      if (blue) {
        leds[i] = CRGB(0,0,150);
      } else {
        leds[i] = CRGB(50, 50,1);
      }
      count += 1; 
      if (count == numLightsBeforeChange) {
        count = 0;
        blue = !blue;
      }
   }
   FastLED.show();
   prevLights = 4;
   lightsOn = true;
}

void lightsYouthfulAndWise() {
  const int numLightsBeforeChange = 1;
  const int numColors = 3;
  int count = 0;
  int color = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    if (color == 0) {
      leds[i] = CRGB(50, 50, 0);
    }
    if (color == 1) {
      leds[i] = CRGB(0, 60, 0);
    }
    if (color == 2) {
      leds[i] = CRGB(0, 0, 155);
    }
     count += 1;
    if (count == numLightsBeforeChange) {
      count = 0;
      color += 1;
      if (color == numColors) {
        color = 0;
      }
    }
  }
  FastLED.show();
  prevLights = 5;
  lightsOn = true;
}

void lightsWinnie() {
  const int numLightsBeforeChange = 1;
  int count = 0;
  bool fuschia = true;
  for (int i = 0; i < NUM_LEDS; i++) {
    if (fuschia) {
      leds[i] = CRGB(50,10,80);
    }
    else {
      leds[i] = CRGB(0,0,200);
    }
    // Can't make LED black so commenting out
//    if (color == 2) {
//      leds[i] = CRGB(0, 0, 0);
//    }
  count += 1;
  if (count == numLightsBeforeChange) {
      count = 0;
      fuschia = !fuschia;
    }
  }
  FastLED.show();
  prevLights = 6;
  lightsOn = true;
}

void lightsIreland() {
  const int numLightsBeforeChange = 1;
  const int numColors = 3;
  int count = 0;
  int color = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    if (color == 0) {
      leds[i] = CRGB(50, 30, 0);
    }
    if (color == 1) {
      leds[i] = CRGB(100, 100, 100);
    }
    if (color == 2) {
      leds[i] = CRGB(0, 100, 0);
    }
     count += 1;
    if (count == numLightsBeforeChange) {
      count = 0;
      color += 1;
      if (color == numColors) {
        color = 0;
      }
    }
  }
  FastLED.show();
  prevLights = 7;
  lightsOn = true;
}

void lightsRainbow() {
  for (int i = 0; i < NUM_LEDS; i++) {
    for (int r = 0; r < 64; r += 63) {
      if (i == NUM_LEDS) {
            break;
          }
      for (int g = 0; g < 255; g += 63) {
        if (i == NUM_LEDS) {
            break;
          }
        for (int b = 0; b < 255; b += 63) {
          if (i == NUM_LEDS) {
            break;
          }
          leds[i] = CRGB(r,g,b);
          i += 1;
        }
      }
    }
  }
  FastLED.show();
  prevLights = 8;
  lightsOn = true;
}

// Remote signals will not be received and ran while the light cycle is in progress
void lightsCycle() {
  // turns scoreboard off because cannot operate scoreboard and changing lights at the same time
  writeAll();
  writeBlank();
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(155, 50, 70);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
    for (int j = 0; j < i; j++) {
      leds[j].b++;
    }
    FastLED.show();
    delay(50);
  }
  for (int i = NUM_LEDS - 1; i >= 0; i--) {
    leds[i] = CRGB(0, 255, 0);
    for (int j = 0; j < NUM_LEDS-i; j++) {
      leds[j].b--;
      leds[j].r++;
    }
    FastLED.show();
    delay(50);
  }
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(155, 50, 70);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(155, 50, 70);
  }
  FastLED.show();
  prevLights = 9;
  lightsOn = true;
}
