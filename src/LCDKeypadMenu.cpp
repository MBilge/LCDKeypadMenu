#include <LCDKeypadMenu.h>

LCDKeypadMenu::LCDKeypadMenu() {
	menuState = 0;
	prevMenuState = -1;
	currentSetpointIndex = 0;
	prevSetpointIndex = 0;
	rootSetpoint = NULL;
	setpointCount = 0;
	rootScreen = NULL;
	screenCount = 0;
	currentScreenIndex = 0;
}

void LCDKeypadMenu::begin(LiquidCrystal_I2C *display, Keypad *k) {
	lcd = display;
	keypad = k;
	lastKeyPressTime = millis();
	lcd->begin();
	lcd->clear();
	lcd->setCursor(0, 0);
}

void LCDKeypadMenu::process() {
	read();
	print();
	prevMenuState = menuState;
}

byte LCDKeypadMenu::addSetpoint(String descr, byte type) {
	Setpoint *s = new Setpoint;
	s->description = descr;
	s->value = 0;
	s->type = type;
	s->next = NULL;
	if (rootSetpoint) {
		Setpoint *last = rootSetpoint;
		while (last->next != NULL)
			last = last->next;
		last->next = s;
	} else 
		rootSetpoint = s;
	return setpointCount++;
}

String LCDKeypadMenu::getDescription(byte i) {
	return getSetpoint(i)->description;
}

unsigned int LCDKeypadMenu::getValue(byte i) {
	return getSetpoint(i)->value;
}

void LCDKeypadMenu::setValue(byte i, unsigned int v) {
	getSetpoint(i)->value = v;
}

byte LCDKeypadMenu::addScreenHandler(void (*callback)(void)) {
	Screen *s = new Screen;
	s->callback = callback;
	s->next = NULL;
	if (rootScreen) {
		Screen *last = rootScreen;
		while (last->next != NULL)
			last = last->next;
		last->next = s;
	} else {
		rootScreen = s;
	}
	return screenCount++;
}

void LCDKeypadMenu::print() {
	repaint = prevMenuState != menuState;
	if (menuState == MENU_HOME)
		repaint |= currentScreenIndex != prevScreenIndex;
	if (menuState == MENU_SELECT)
		repaint |= currentSetpointIndex != prevSetpointIndex;
	if (menuState == MENU_EDIT)
		repaint |= editedValue != prevEditedValue;
	if (!repaint)
		return;
	switch (menuState) {
	case MENU_HOME: {
		lcd->noBlink();
		lcd->clear();
		printHome();
		prevScreenIndex = currentScreenIndex;
		break;
	}
	case MENU_SELECT: {
		lcd->noBlink();
		lcd->clear();
		byte startIndex = min(prevSetpointIndex, setpointCount - 2);
		Serial.println(currentSetpointIndex);
		if (startIndex > currentSetpointIndex)
			startIndex = currentSetpointIndex;
		for (byte i = 0; i < 2; ++i) {
			if (i + startIndex == currentSetpointIndex) {
				lcd->setCursor(0,i);
				lcd->print("*");
			}
		  lcd->setCursor(1,i);
		  lcd->print(getDescription(i + startIndex));
		}
		prevSetpointIndex = currentSetpointIndex;
		break;
	}
	case MENU_EDIT: {
		lcd->blink();
		lcd->clear();
		lcd->setCursor(0, 0);
		lcd->print(getDescription(currentSetpointIndex));
		lcd->setCursor(0, 1);
		if (getSetpointType() == SETPOINT_INT) {
			lcd->print(editedValue);
		} else {
			if (editedValue)
				lcd->print("ON");
			else
				lcd->print("OFF");
		}
		prevEditedValue = editedValue;
		break;
	}
	}
}

void LCDKeypadMenu::printHome() {
	if (!rootScreen)
		return;
	byte index = 0;
	Screen *screen = rootScreen;
	while (index++ < currentScreenIndex and screen->next) {
		screen = screen->next;
	}
	screen->callback();
}

void LCDKeypadMenu::read() {
	char key = keypad->getKey();
	if (key)
		lastKeyPressTime = millis();
	if (key == '*') {
		if (menuState == MENU_HOME) {
			menuState = MENU_SELECT;
			currentSetpointIndex = prevSetpointIndex = 0;
		} else if (menuState == MENU_SELECT) {
			menuState = MENU_EDIT;
			editedValue = getValue(currentSetpointIndex);
		} else if (menuState == MENU_EDIT) {
			menuState = MENU_SELECT;
		}
	}
	if (key == 'C') {
		if (menuState == MENU_SELECT and currentSetpointIndex > 0)
		  --currentSetpointIndex;
	}
	if (key == 'D') {
		if (menuState == MENU_SELECT and currentSetpointIndex < setpointCount - 1)
			++currentSetpointIndex;
	}
	if (key >= '0' and key <= '9') {
		if (menuState == MENU_EDIT) {
			if (getSetpointType() == SETPOINT_INT) {
				if (editedValue < 9999) {
					editedValue *= 10;
					editedValue += int(key) - 48;
				}
			} else {
				editedValue = key == '0'? 0: 1;
			}
		}
	}
	if (key == '4') {
		if (menuState == MENU_HOME) {
			if (currentScreenIndex > 0)
				--currentScreenIndex;
			else
				currentScreenIndex = screenCount - 1;
		}
	}
	if (key == '6') {
		if (menuState == MENU_HOME) {
			if (currentScreenIndex < screenCount - 1)
				++currentScreenIndex;
			else
				currentScreenIndex = 0;
		}
	}
	if (key == '#') {
		if (menuState == MENU_SELECT) {
			menuState = MENU_HOME;
		} else if (menuState == MENU_EDIT) {
			setValue(currentSetpointIndex, editedValue);
			menuState = MENU_SELECT;
		}
	}
	if (key == 'A') {
		if (menuState == MENU_EDIT and getSetpointType() == SETPOINT_INT) {
			editedValue /= 10;
		}
	}
	int pause = menuState == MENU_EDIT? 10000: 5000;
	if (lastKeyPressTime + pause < millis())
		menuState = MENU_HOME;
}

Setpoint *LCDKeypadMenu::getSetpoint(byte i) {
	byte index = 0;
	Setpoint *result = rootSetpoint;
	while (index++ < i and result->next) {
		result = result->next;
	}
	return result;
}

byte LCDKeypadMenu::getSetpointType() {
	return getSetpoint(currentSetpointIndex)->type;
}