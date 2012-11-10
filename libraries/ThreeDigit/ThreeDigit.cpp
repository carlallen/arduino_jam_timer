#include <ThreeDigit.h>
#include <avr/pgmspace.h>
#include <SPI.h>

#define LATCH_PIN 5
#define CLOCK_PIN 6
#define DATA_PIN 7


void ThreeDigit::display_number(int number_to_display) {
  char tens = number_to_display % 100 / 10;
  char hundreds = number_to_display % 1000 / 100;
  if (tens > 0) { tens = led_codes[tens]; }
  if (hundreds > 0) {
    hundreds = led_codes[hundreds];
    if (tens == 0)
    {
      tens = led_codes[0];
    }
  }
  ThreeDigit::update_display(led_codes[number_to_display % 10], tens, hundreds);
}

void ThreeDigit::display_time(unsigned long time_to_display) {
  ThreeDigit::display_time(time_to_display, false);
}
void ThreeDigit::display_time(unsigned long time_to_display, bool pad_zeros) {
  char mins = time_to_display / 60;
  char secs = time_to_display % 60;
  ThreeDigit::update_display(led_codes[secs % 10], tens_code(mins, secs / 10, pad_zeros), min_code(mins, pad_zeros));
}

void ThreeDigit::update_display(char one, char ten, char hundred) {
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, one);
  shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, ten);
  shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, hundred);
  digitalWrite(LATCH_PIN, HIGH);
}

void ThreeDigit::setup() {
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
}

char ThreeDigit::tens_code(char min_val, char ten_val, bool pad_zeros)
{
  if (min_val == 0 && !pad_zeros && ten_val == 0)
  {
    return 0x00;
  }
  else
  {
    return led_codes[ten_val];
  }
}

char ThreeDigit::min_code(char min_val, bool pad_zeros)
{
  if (min_val == 0 && !pad_zeros)
  {
    return 0x00;
  }
  else
  {
    return led_codes[min_val] | 0x80;
  }
}


