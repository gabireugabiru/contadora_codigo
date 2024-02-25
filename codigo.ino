#include <LiquidCrystal.h>

const int PRINT_TIMEOUT = 250;
const int HOW_MANY = 1;
const int BUTTON_PIN = 53;
const int BUTTON2_PIN = 51;


int pins[HOW_MANY] = { 9 };
int count[HOW_MANY] = { 0 };
int last[HOW_MANY] = { 0 };
long int last_printed = 0;

const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int state = 0;
int selection = 0;

int confirm_button_value() {
  return !digitalRead(BUTTON_PIN);
}
int swap_button_value() {
  return !digitalRead(BUTTON2_PIN);
}
void print_selection_row() {
  lcd.setCursor(0, 1);
  if (!selection) {
    lcd.print("   Sim  X Nao");
  } else {
    lcd.print(" X Sim    Nao");
  }
}
void setup() {
  for (int i = 0; i < HOW_MANY; i++) {
    pinMode(pins[i], INPUT);
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  Serial.begin(9600);
  lcd.begin(16, 2);
}


void loop() {
  if (state == 0) {
    for (int i = 0; i < HOW_MANY; i++) {
      int x = digitalRead(pins[i]);
      if (x == 1 && last[i] == 0) {
        count[i] += 1;
      }
      last[i] = x;
    }
    long int elapsed = millis() - last_printed;
    if (elapsed >= PRINT_TIMEOUT) {
      lcd.setCursor(0, 0);
      for (int i = 0; i < HOW_MANY; i++) {
        lcd.print(count[i]);
        lcd.print(" ");
      }
      last_printed = millis();
    }
    if (confirm_button_value()) {
      selection = 0;
      lcd.clear();
      lcd.print("Deseja Resetar?");
      print_selection_row();
      state = 1;
      delay(500);
    }
  }
  if (state == 1) {
    int skip = 0;
    if (confirm_button_value()) {
      if (selection) {
        for (int i = 0; i < HOW_MANY; i++) {
          count[i] = 0;
          last[i] = 0;
        }
      }
      lcd.clear();
      selection = 0;
      state = 0;
      skip = 1;
      while (confirm_button_value()) {};
    }
    if (swap_button_value()) {
      selection = !selection;
      while (swap_button_value()) {};
    }
    if (!skip) {
      lcd.setCursor(1, 0);
      print_selection_row();
    }

  }
}