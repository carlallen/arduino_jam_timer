#include <MsTimer2.h>
#include <avr/pgmspace.h>
#include <SPI.h>
#include "Ethernet.h"
#include "WebServer.h"
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static uint8_t ip[] = { 192, 168, 1, 64 };

static char latchPin = 5;
static char clockPin = 6;
static char dataPin = 7;
static unsigned long start_time;
static char total_time;
static char jam_state = 0;

static char led_codes[10];

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
      "<button type='submit' name='stop' value='stop'>Stop Clock/Official Timeout</button>"
      "</form>";
    server.printP(indexMsg);
}
void update_leds() {
  char mins;
  char secs;
  unsigned long display_time;
  if (jam_state > 0)
  {
    display_time = (millis() - start_time) / 1000;
    
    if (display_time >= total_time) 
    {
      if (jam_state == 2 || jam_state == 3)
      {
        start_time = millis();
        total_time = 119;
        jam_state = 1;
        mins = 2;
        secs = 0;        
      }
      else
      {
        mins = 0;
        secs = 0;
      }
    }
    else
    {
      display_time = total_time - display_time;
      mins = display_time / 60;
      secs = display_time % 60;
    }
  }
  else
  {
    mins = 0;
    secs = 0;
  }

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, led_codes[secs % 10]); //ones
  shiftOut(dataPin, clockPin, LSBFIRST, tens_code(mins, secs / 10)); //tens
  shiftOut(dataPin, clockPin, LSBFIRST, mim_code(mins)); //minutes
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

char mim_code(char min_val)
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
  // set led # codes
  led_codes[0] = 0xDE; 
  led_codes[1] = 0x06;
  led_codes[2] = 0xBA;
  led_codes[3] = 0xAE;
  led_codes[4] = 0x66;
  led_codes[5] = 0xEC;
  led_codes[6] = 0xFC;
  led_codes[7] = 0xC6;
  led_codes[8] = 0xFE;
  led_codes[9] = 0xE6;
  // set output mode for serial pins
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  Ethernet.begin(mac, ip);
  webserver.setDefaultCommand(&indexPage);
  webserver.addCommand("index.html", &indexPage);
  
  MsTimer2::set(100, update_leds);
  MsTimer2::start();
}

void loop() {
  char buff[64];
  int len = 64;
  webserver.processConnection(buff, &len);
}
