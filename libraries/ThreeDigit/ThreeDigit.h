#ifndef ThreeDigit_h
#define ThreeDigit_h

namespace ThreeDigit {
  static char led_codes[] = { 0xDE, 0x06, 0xBA, 0xAE, 0x66, 0xEC, 0xFC, 0xC6, 0xFE, 0xE6 };
  void display_number(int number_to_display);
  void display_time(unsigned long time_to_display);
  void display_time(unsigned long time_to_display, bool pad_zeros);
  void update_display(char one, char ten, char hundred);
  char tens_code(char min_val, char ten_val, bool pad_zeros);
  char min_code(char min_val, bool pad_zeros);
  void setup();
}

#endif