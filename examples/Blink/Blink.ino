#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <LCDKeypadMenu.h>

LiquidCrystal_I2C lcd(0x3f, 16, 2);
const byte ROWS = 4; 
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
}; 
byte rowPins[ROWS] = {11,10, 9, 8}; 
byte colPins[COLS] = {7, 6, 5, 4}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LCDKeypadMenu menu;

byte MENU_TURN_OFF, MENU_TURN_ON, MENU_BACKLIGHT;
bool ledState = false;
unsigned long ledSwitchTime = millis();

void screenHandler() {
  lcd.setCursor(0, 0);
  lcd.print("Turn off: ");
  lcd.print(menu.getValue(MENU_TURN_OFF));
  lcd.setCursor(0, 1);
  lcd.print("Turn on:  ");
  lcd.print(menu.getValue(MENU_TURN_ON));
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  menu.begin(&lcd, &keypad);
  MENU_TURN_OFF = menu.addSetpoint("Turn off period", LCDKeypadMenu::SETPOINT_INT);
  menu.setValue(MENU_TURN_OFF, 500);
  MENU_TURN_ON = menu.addSetpoint("Turn on period", LCDKeypadMenu::SETPOINT_INT);
  menu.setValue(MENU_TURN_ON, 1000);
  MENU_BACKLIGHT = menu.addSetpoint("Backlight", LCDKeypadMenu::SETPOINT_BOOL);
  menu.setValue(MENU_BACKLIGHT, 1);
  menu.installScreenHandler(screenHandler);
}

void loop() {
  menu.process();
  if (ledState and ledSwitchTime + menu.getValue(MENU_TURN_ON) < millis()) {
    ledState = false;
    ledSwitchTime = millis();
  } else if (!ledState and ledSwitchTime + menu.getValue(MENU_TURN_OFF) < millis()) {
    ledState = true;
    ledSwitchTime = millis();
  }
  digitalWrite(LED_BUILTIN, ledState); 
  if (menu.getValue(MENU_BACKLIGHT))
    lcd.backlight();
  else
    lcd.noBacklight();
}
