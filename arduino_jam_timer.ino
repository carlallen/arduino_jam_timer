#include <MsTimer2.h>
#include <avr/pgmspace.h>
#include <SPI.h>
#include <Ethernet.h>
#include <WebServer.h>
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static uint8_t ip[] = { 192, 168, 1, 64 };

static char latchPin = 5;
static char clockPin = 6;
static char dataPin = 7;
static unsigned long start_time;
static char total_time;
static char jam_state = -1;
static byte last_ip = -1;

static char led_codes[] = { 0xDE, 0x06, 0xBA, 0xAE, 0x66, 0xEC, 0xFC, 0xC6, 0xFE, 0xE6 };

#define PREFIX ""
WebServer webserver(PREFIX, 80);

void indexPage(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  URLPARAM_RESULT rc;
  server.httpSuccess();
  char name[16], value[16];
    if (strlen(url_tail))
    {
      rc = server.nextURLparam(&url_tail, name, 16, value, 16);
      if (strcmp(name, "jam_start") == 0)
      {
        start_time = millis();
        total_time = 120;
        jam_state = 1;
      }
      else if (strcmp(name, "jam_delay") == 0)
      {
        start_time = millis();
        total_time = 30;
        jam_state = 2;
      }
      else if (strcmp(name, "5_countdown") == 0)
      {
        start_time = millis();
        total_time = 5;
        jam_state = 3;
      }
      else if (strcmp(name, "timeout") == 0)
      {
        start_time = millis();
        total_time = 60;
        jam_state = 4;
      }
      else if (strcmp(name, "official") == 0) 
      {
        start_time = millis();
        jam_state = 5;
      }
      else if (strcmp(name, "stop") == 0)
      {
        total_time = 0;
        jam_state = 0;
      }
    }
    P(indexMsg) = "<meta name='viewport' content='width=320;' />"
      "<style type='text/css'>form{text-align:center;}button{width: 300px;height: 40px;}</style>"
      "<form action='index.html' method='GET'>"
      "<button type='submit' name='jam_start' value='jam_start'>Start Jam</button><br/>"
      "<button type='submit' name='jam_delay' value='jam_delay'>30 Second Countdown</button><br/>"
      "<button type='submit' name='5_countdown' value='5_countdown'>5 Second Countdown</button><br/>"
      "<button type='submit' name='timeout' value='timeout'>Timeout (1 Minute)</button><br/>"
      "<button type='submit' name='official' value='official'>Official Timeout</button><br/>"
      "<button type='submit' name='stop' value='stop'>Stop Clock</button>"
      "</form>";
    server.printP(indexMsg);
}
void update_leds() {
  char mins;
  char secs;
  unsigned long elapsed_time;
  
  switch (jam_state) {
    case 0:
      display_time(0);
      break;
    case 1:
    case 2:
    case 3:
    case 4:
      elapsed_time = (millis() - start_time) / 1000;
      if (elapsed_time >= total_time) 
      {
        if (jam_state == 2 || jam_state == 3)
        {
          start_time = millis();
          total_time = 120;
          jam_state = 1;
          display_time(120);   
        }
        else
        {
          display_time(0);
        }
      }
      else
      {
       display_time(total_time - elapsed_time);
      }
      break;
    case 5:
      display_time((millis() - start_time) /1000);
      break;
    default:
      if (last_ip == -1)
      {
        update_display(0x01, 0x0, 0x04);
      }
      else {
        display_number(last_ip);
      }
  }
}

void display_time(unsigned long time_to_display) {
  char mins = time_to_display / 60;
  char secs = time_to_display % 60;
  update_display(led_codes[secs % 10], tens_code(mins, secs / 10), min_code(mins));
}

void display_number(int number_to_display) {
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
  update_display(led_codes[number_to_display % 10], tens, hundreds);
}

void update_display(char one, char ten, char hundred) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, one);
  shiftOut(dataPin, clockPin, LSBFIRST, ten);
  shiftOut(dataPin, clockPin, LSBFIRST, hundred);
  digitalWrite(latchPin, HIGH); 
}

char tens_code(char min_val, char ten_val)
{
  if (min_val == 0 && jam_state != 0 && ten_val == 0) 
  {
    return 0x00;
  }
  else 
  {
    return led_codes[ten_val];
  }
}

char min_code(char min_val)
{
  if (min_val == 0 && jam_state != 0)
  {
    return 0x00;
  }
  else
  {
    return led_codes[min_val] | 0x01;
  }
}

void setup() {
  // set output mode for serial pins
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  update_display(0x20, 0x20, 0x20);
  Ethernet.begin(mac);
  last_ip = Ethernet.localIP()[3];
  webserver.setDefaultCommand(&indexPage);
  webserver.addCommand("index.html", &indexPage);
  MsTimer2::set(33, update_leds);
  MsTimer2::start();
}

void loop() {
  char buff[64];
  int len = 64;
  webserver.processConnection(buff, &len);
}
