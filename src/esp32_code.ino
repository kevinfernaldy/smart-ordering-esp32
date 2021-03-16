#include <LiquidCrystal.h>
#include <Keypad.h>
#include <EspMQTTClient.h>
#include <TimeOut.h>
#include <analogWrite.h>
#include <ArduinoJson.h>

// LED initialization
const int ledRedPin = 18;
const int ledGreenPin = 19;
const int ledBluePin = 21;
bool ledMQTTWaitState = 0;

// LCD Display initialization
const int rs = 17, en = 5, d4 = 15, d5 = 2, d6 = 4, d7 = 16;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Keypad initialization
const int ROW_NUM = 4;
const int COLUMN_NUM = 4;
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pin_rows[ROW_NUM] = {13, 12, 14, 27};
byte pin_column[COLUMN_NUM] = {26, 25, 33, 32};
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

// MQTT Client Initialization
EspMQTTClient client(
  "DICKY-ANNA",
  "dp234234",
  "202.148.1.57",
  "app-smartorderingsystem",
  "G4zwVj1B1qmTDR2V0oY7y2YVqUUe6o",
  "ESP32_01"
);

// Menu Initialization
String menu[4][9] = {
  /*             1                2                3                4                5                6                7                 8                9
  /* A */ {"Nasi Putih   ", "Nasi Uduk    ", "Nasi Kuning  ", "Nasi Merah   ", "Ayam Kuning  ", "Ayam Kalasan ", "Ayam Bakar   ",  "Ikan Nila    ", "Ikan Bawal   "},
  /* B */ {"Sayur Asem   ", "Sayur Lodeh  ", "Cah Kangkung ", "Sayur Bayam  ", "Tahu Kuning  ", "Tempe Goreng ", "Mendoan      ",  "Bakwan Jagung", "Telor Ceplok "},
  /* C */ {"Telor Dadar  ", "Telor Rebus  ", "Telor Balado ", "Tempe Orek   ", "Perkedel     ", "Kerang       ", "Kerupuk Putih",  "Jeruk        ", "Pisang       "},
  /* D */ {"Es Teh Tawar ", "Es Teh Manis ", "Teh Tawar    ", "Teh Manis    ", "Es Jeruk     ", "Jeruk Hangat ", "Air Putih    ",  "Kopi         ", "Teh Botol    "}
};

// Loop state
// 0 = idle / enter food, 1 = confirmation, 2 = sending MQTT and wait, 3 = accepted, 4 = rejected, 5 = timeout
int state = 0;

// Others
int keyArray[2] = {-1, -1}; // Key select storage
byte keyPosition = 0; // Key position
byte confirmationPosition = 0; // Food confirmation position
TimeOut MQTT_Timeout; // Timeout for MQTT
TimeOut AcceptRejectTimeout;  // Timeout for accepted or rejected message
int waitResponseLCDCounter = 0; // Counter for waiting in LCD

void setup() {
  // set up the LCD's number of columns and rows
  lcd.begin(16, 2);

  // Set up serial output
  Serial.begin(115200);

  // Set up LED
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledBluePin, OUTPUT);
  analogWrite(ledRedPin, 255.0);
  analogWrite(ledGreenPin, 255.0);
  analogWrite(ledBluePin, 255.0);
}

void onConnectionEstablished() {
  client.subscribe("01ESP32Publish", [] (const String &payload)  {
    if (state == 2) {
      MQTT_Timeout.cancel();
      lcd.clear();
  
      DynamicJsonDocument document(100);
      deserializeJson(document, payload);

      AcceptRejectTimeout.timeOut(5000, [] () {
        lcd.clear();

        delay(2000);
        keyArray[0] = -1;
        keyArray[1] = -1;
        keyPosition = 0;
        waitResponseLCDCounter = 0;
        state = 0;
      });
      
      if (document["payload"]["status"] == "OK") {
        state = 3; 
      } else if (document["payload"]["status"] == "REJECT") {
        state = 4;
      }
    }
  });
}

void loop() {
  client.loop();
  
  MQTT_Timeout.handler();
  
  switch (state) {
    case 0: {
      state_0();
      break;
    }
    case 1: {
      state_1();
      break;
    }
    case 2: {
      state_2();
      break;
    }
    case 3: {
      state_3();
      break;
    }
    case 4: {
      state_4();
      break;
    }
    case 5: {
      state_5();
      break;
    }
  }
}

// *****************
// *     State     *
// *****************
void state_0() {
  enter_selection_base_lcd();
   analogWrite(ledGreenPin, 255.0);
   analogWrite(ledRedPin, 255.0);
   analogWrite(ledBluePin, 255.0);
  
  char key = keypad.getKey();
  if (key) {
    if ((key == '0') || ((key == '*') && (keyPosition == 0))) {
      Serial.println("zero");
      return;
    } else if ((key == '#') && (keyPosition == 2)) {
      state = 1;
      lcd.clear();
      return;
    } else {
      if ((key == '*') && (keyPosition > 0)) {
        keyPosition--;
        keyArray[keyPosition] = -1;
        lcd.setCursor(2 + keyPosition, 1);
        lcd.print(" ");
        return;
      }
      int convertedKey = atoi(&key);
      if ((convertedKey == 0) && (keyPosition == 0)) {
        lcd.setCursor(2 + keyPosition, 1);
        switch (key) {
          case 'A': {
            keyArray[keyPosition] = 0;
            lcd.print("A");
            break;
          }
          case 'B': {
            keyArray[keyPosition] = 1;
            lcd.print("B");
            break;
          }
          case 'C': {
            keyArray[keyPosition] = 2;
            lcd.print("C");
            break;
          }
          case 'D': {
            keyArray[keyPosition] = 3;
            lcd.print("D");
            break;
          }
        }
        
        keyPosition++;
      } else if ((convertedKey > 0) && (keyPosition == 1)) {
        lcd.setCursor(2 + keyPosition, 1);
        lcd.print(key);
        keyArray[keyPosition] = convertedKey - 1;
        keyPosition++;
      }
    }
  }
}

void state_1() {
  confirmation_base_lcd(getFullFoodName());

  char key = keypad.getKey();
  if (key) {
    if (key == '4') {
      confirmationPosition = 0;
      lcd.setCursor(2, 1);
      lcd.print(">");
      lcd.setCursor(9, 1);
      lcd.print(" ");
    } else if (key == '6') {
      confirmationPosition = 1;
      lcd.setCursor(2, 1);
      lcd.print(" ");
      lcd.setCursor(9, 1);
      lcd.print(">");
    } else if (key == '#') {
      lcd.clear();
      if (confirmationPosition == 0) {
        client.publish("01ESP32Subscribe", "{ \"payload\": { \"menu\": \"" + menu[keyArray[0]][keyArray[1]] + "\" }}");
        MQTT_Timeout.timeOut(10000, [] () {
          lcd.clear();
          state = 5;
        });
        state = 2;
        lcd.clear();
      } else {
        state = 0;
      }
    }
  }
}

void state_2(){
  waiting_base_lcd();
  
  analogWrite(ledBluePin, 255.0);
  if (ledMQTTWaitState == 0) {
    analogWrite(ledRedPin, 192.0);
    analogWrite(ledGreenPin, 192.0);
  } else {
    analogWrite(ledRedPin, 255.0);
    analogWrite(ledGreenPin, 255.0);
  }
  ledMQTTWaitState = !ledMQTTWaitState;
  
  delay(1000);
}

void state_3() {
  accepted_base_lcd();

  analogWrite(ledRedPin, 255.0);
  analogWrite(ledBluePin, 255.0);
  analogWrite(ledGreenPin, 192.0);
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {
      AcceptRejectTimeout.cancel();
      lcd.clear();
      delay(2000);

      keyArray[0] = -1;
      keyArray[1] = -1;
      keyPosition = 0;
      waitResponseLCDCounter = 0;
      state = 0;
    }
  }
}

void state_4() {
  rejected_base_lcd();

  analogWrite(ledBluePin, 255.0);
  analogWrite(ledGreenPin, 255.0);
  analogWrite(ledRedPin, 192.0);
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {
      AcceptRejectTimeout.cancel();
      lcd.clear();
      delay(2000);

      keyArray[0] = -1;
      keyArray[1] = -1;
      keyPosition = 0;
      waitResponseLCDCounter = 0;
      state = 0;
    }
  }
}

void state_5() {
  timeout_base_lcd();
  
  ledMQTTWaitState = 0;
  waitResponseLCDCounter = 0;
  analogWrite(ledBluePin, 255.0);
  analogWrite(ledGreenPin, 255.0);
  
  analogWrite(ledRedPin, 192.0);
  delay(500);
  analogWrite(ledRedPin, 255.0);
  delay(500);
  analogWrite(ledRedPin, 192.0);
  delay(500);
  analogWrite(ledRedPin, 255.0);
  delay(500);
  analogWrite(ledRedPin, 192.0);
  delay(500);
  analogWrite(ledRedPin, 255.0);

  lcd.clear();
  delay(2000);

  keyArray[0] = -1;
  keyArray[1] = -1;
  keyPosition = 0;
  state = 0;
}

// *****************
// * LCD Functions *
// *****************
// Enter selection base screen
void enter_selection_base_lcd() {
  lcd.setCursor(0, 0);
  lcd.print("Input food code");
  lcd.setCursor(0, 1);
  lcd.print(">");
}
void confirmation_base_lcd(String food) {
  lcd.setCursor(0, 0);
  lcd.print(food);
  lcd.setCursor(3, 1);
  lcd.print("Yes");
  lcd.setCursor(10, 1);
  lcd.print("No");
}

void waiting_base_lcd() {
  lcd.setCursor(0,0);
  lcd.write("Waiting For");
  lcd.setCursor(0,1);
  lcd.write("Response");

  if (8 + waitResponseLCDCounter < 16) {
    lcd.setCursor(8 + waitResponseLCDCounter, 1);
    lcd.write(".");
    waitResponseLCDCounter++;
  }
}

void accepted_base_lcd() {
  lcd.setCursor(0,0);
  lcd.write(" Menu received. ");
  lcd.setCursor(0,1);
  lcd.write("     > Back     ");
}

void rejected_base_lcd() {
  lcd.setCursor(0,0);
  lcd.write(" Menu rejected. ");
  lcd.setCursor(0,1);
  lcd.write("     > Back     ");
}

void timeout_base_lcd() {
  lcd.setCursor(0,0);
  lcd.write("Request");
  lcd.setCursor(0,1);
  lcd.write("Timed out");
}

// *****************
// *    General    *
// *****************
String getFullFoodName() {
  String food = "";

  switch(keyArray[0]) {
    case 0: {
      food = food + "A";
      break;
    }
    case 1: {
      food = food + "B";
      break;
    }
    case 2: {
      food = food + "C";
      break;
    }
    case 3: {
      food = food + "D";
      break;
    }
  }

  food = food + String(keyArray[1]) + " " + menu[keyArray[0]][keyArray[1]];

  return food;
}
