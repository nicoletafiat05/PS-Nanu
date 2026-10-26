#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#define SENZOR_LM35 1

// ora initiala
volatile uint8_t ore = 12, minute = 30, secunde = 0;
volatile bool tickSecunda = false;

void adcInit() {
  ADMUX  = (1 << REFS0);
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adcCiteste() {
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  return ADC;
}

void timer1Init() {
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A  = 15624;                         // 1 secunda
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

ISR(TIMER1_COMPA_vect) {
  secunde++;
  if (secunde >= 60) {
    secunde = 0;
    minute++;
    if (minute >= 60) {
      minute = 0;
      ore++;
      if (ore >= 24) ore = 0;
    }
  }
  tickSecunda = true;
}

void afiseaza() {
  uint8_t h, m, s;
  cli();
  h = ore; m = minute; s = secunde;
  sei();

  // citire temperatura
  uint16_t adc = adcCiteste();
  float mV = adc * 5000.0 / 1024.0;
#if SENZOR_LM35
  float temp = mV / 10.0;
#else
  float temp = (mV - 500.0) / 10.0;
#endif

  // afisare pe lcd
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp, 1);
  lcd.print("`C   ");

  lcd.setCursor(0, 1);
  lcd.print("Ora: ");
  if (h < 10) lcd.print('0');
  lcd.print(h);
  lcd.print(':');
  if (m < 10) lcd.print('0');
  lcd.print(m);
  lcd.print(':');
  if (s < 10) lcd.print('0');
  lcd.print(s);
  lcd.print("  ");

  // afisare pe serial
  Serial.print("Temp: ");
  Serial.print(temp, 1);
  Serial.print(" C  |  ");
  Serial.print(h); Serial.print(':');
  Serial.print(m); Serial.print(':');
  Serial.println(s);
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  adcInit();
  timer1Init();
  afiseaza();
}

void loop() {
  if (tickSecunda) {
    tickSecunda = false;
    afiseaza();
  }
}