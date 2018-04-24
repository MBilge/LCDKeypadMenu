#ifndef H_LCDKEYPADMENU
#define H_LCDKEYPADMENU

#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

struct Setpoint {
  String description;
  unsigned int value;
  Setpoint *next;
  byte type;
};

struct Screen {
  void (*callback)(void);
  Screen *next;
};

class LCDKeypadMenu {
public:
	static const byte SETPOINT_INT = 0;
	static const byte SETPOINT_BOOL = 1;

	LCDKeypadMenu();
	void begin(LiquidCrystal_I2C *display, Keypad *k);
	void process();
	byte addSetpoint(String descr, byte type = SETPOINT_INT);
	String getDescription(byte i);
	unsigned int getValue(byte i);
	void setValue(byte i, unsigned int v);
	byte addScreenHandler(void (*callback)(void));
private:
	static const byte MENU_HOME = 0;
	static const byte MENU_SELECT = 1;
	static const byte MENU_EDIT = 2;
	LiquidCrystal_I2C *lcd;
	Keypad *keypad;
	byte menuState, prevMenuState;
	unsigned long lastKeyPressTime;
	unsigned long editedValue, prevEditedValue;
	byte currentSetpointIndex, prevSetpointIndex;
	byte currentScreenIndex, prevScreenIndex;
	bool repaint;
	byte setpointCount;
	byte screenCount;
	Setpoint *rootSetpoint;
	Screen *rootScreen;
	
	void print();
	void printHome();
	void read();
	Setpoint *getSetpoint(byte i);
	byte getSetpointType();
};

#endif