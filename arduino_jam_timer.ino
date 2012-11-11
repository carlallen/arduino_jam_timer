#include <MsTimer2.h>
#include <avr/pgmspace.h>
#include <SPI.h>
#include <Ethernet.h>
#include <WebServer.h>
#include <ThreeDigit.h>
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

static unsigned long start_time;
static char total_time;
static char jam_state = -1;
static byte last_ip = -1;

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
      ThreeDigit::display_time(0, true);
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
          ThreeDigit::display_time(120);
        }
        else
        {
          ThreeDigit::display_time(0);
        }
      }
      else
      {
       ThreeDigit::display_time(total_time - elapsed_time);
      }
      break;
    case 5:
      ThreeDigit::display_time((millis() - start_time) /1000);
      break;
    default:
      break;
  }
}

void setup() {
  // set output mode for serial pins
  ThreeDigit::setup();
  ThreeDigit::update_display(0x04, 0x04, 0x04);
  Ethernet.begin(mac);
  ThreeDigit::display_number(Ethernet.localIP()[3]);
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
