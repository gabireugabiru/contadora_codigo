#include <LiquidCrystal.h>
#include <EEPROM.h>

const int PRINT_TIMEOUT = 250;
const int HOW_MANY = 5;
const int BUTTON_PIN = 53;
const int BUTTON2_PIN = 51;
const int TIME_BETWEEN_PUSHES = 500;
const int LIMIT = 99;
const char CENTS_LINE[16] = "1R 50 25 10 05";
const float MULTIPLIER[HOW_MANY] = { 1.00, 0.50, 0.25, 0.10, 0.05 };
int pins[HOW_MANY] = { 9, 10, 11, 12, 13 };
int count[HOW_MANY] = { 0 };
int last[HOW_MANY] = { 0 };
long int last_printed = 0;
long int time_confirmed = 0;
long int time_swaped = 0;
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int state = 0;
int selection = 0;
bool should_send = false;

int confirm_button_value() {
  int x = !digitalRead(BUTTON_PIN);
  if (x) {
    time_confirmed = millis();
  }
  return x;
}

int swap_button_value() {
  int x = !digitalRead(BUTTON2_PIN);
  if (x) {
    time_swaped = millis();
  }
  return x;
}

void print_selection_row() {
  lcd.setCursor(0, 1);
  if (!selection) {
    lcd.print("   Sim  X Nao");
  } else {
    lcd.print(" X Sim    Nao");
  }
}

void handle_serial() {
  if (Serial.available()) {
    // limpa a serial se nao for um comando valido
    if (Serial.available() > 1) {
      while (Serial.available()) { Serial.read(); };
    }

    int read = Serial.read();
    if (read == 'c') {
      for (int i = 0; i < HOW_MANY; i++) {
        Serial.print(count[i]);
        Serial.print(",");
        Serial.flush();
      }
    } else if (read == 't') {
      for (int i = 0; i < HOW_MANY; i++) {
        count[i] = 0;
      }
    }
  }
}
void print_total() {
  float total = 0.0;
  for (int i = 0; i < HOW_MANY; i++) {
    total += count[i] * MULTIPLIER[i];
  }
  lcd.print("R$");
  lcd.print(total);
}

void reset_state() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(CENTS_LINE);
  selection = 0;
  state = 0;
}

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < HOW_MANY; i++) {
    count[i] = EEPROM.read(i);
    pinMode(pins[i], INPUT);
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  lcd.begin(16, 2);
  lcd.setCursor(0, 1);
  lcd.print(CENTS_LINE);
}

void loop() {
  long int elapsed = millis() - last_printed;

  if (state == 0) {
    if (elapsed >= PRINT_TIMEOUT) {
      lcd.setCursor(0, 0);
      for (int i = 0; i < HOW_MANY; i++) {
        int x = digitalRead(pins[i]);
        if (x == 1 && last[i] == 0) {
          count[i] += 1;
        }
        last[i] = x;
        EEPROM.update(i, count[i]);

        //left number padding
        if (count[i] < 10) {
          lcd.print("0");
        }

        // print with limiter
        if (count[i] > LIMIT) {
          lcd.print(LIMIT);
        } else {
          lcd.print(count[i]);
        }
        lcd.print(" ");
      }
      last_printed = millis();
    } else {
      for (int i = 0; i < HOW_MANY; i++) {
        int x = digitalRead(pins[i]);
        if (x == 1 && last[i] == 0) {
          count[i] += 1;
        }
        last[i] = x;
      }
    }
    if ((millis() - time_swaped) > TIME_BETWEEN_PUSHES && swap_button_value()) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Total: ");
      lcd.setCursor(0, 1);
      print_total();
      state = 2;
    }
    if ((millis() - time_confirmed) > TIME_BETWEEN_PUSHES && confirm_button_value()) {
      lcd.clear();
      lcd.print("Deseja Resetar?");
      print_selection_row();
      selection = 0;
      state = 1;
    }
  }
  if (state == 1) {
    lcd.setCursor(1, 0);
    print_selection_row();
    if ((millis() - time_confirmed) > TIME_BETWEEN_PUSHES && confirm_button_value()) {
      // if selected yes, proceed to nuke everything
      if (selection) {
        for (int i = 0; i < HOW_MANY; i++) {
          count[i] = 0;
          last[i] = 0;
          EEPROM.update(i, 0);
        }
      }
      reset_state();
    }
    if ((millis() - time_swaped) > TIME_BETWEEN_PUSHES && swap_button_value()) {
      selection = !selection;
    }
  }
  if (state == 2) {
    if (elapsed >= PRINT_TIMEOUT) {
      lcd.setCursor(0, 1);
      print_total();
      lcd.print("            ");
    }
    if ((millis() - time_swaped) > TIME_BETWEEN_PUSHES && swap_button_value()) {
      reset_state();
    }
  }
  handle_serial();
}