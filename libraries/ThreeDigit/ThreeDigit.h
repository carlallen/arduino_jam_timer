#ifndef ThreeDigit_h
#define ThreeDigit_h

namespace ThreeDigit {
  static char led_codes[] = { 0x7B, 0x041, 0x2F, 0x4F, 0x55, 0x5E, 0x7E, 0x59, 0x7F, 0x5D };
  void display_number(int number_to_display);
  void display_time(unsigned long time_to_display);
  void display_time(unsigned long time_to_display, bool pad_zeros);
  void update_display(char one, char ten, char hundred);
  char tens_code(char min_val, char ten_val, bool pad_zeros);
  char min_code(char min_val, bool pad_zeros);
  void setup();
}

#endif