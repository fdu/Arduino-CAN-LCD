#include <Wire.h>
#include "rgb_lcd.h"
#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>

#define   LCD_COLUMNS         16
#define   LCD_ROWS            2
#define   CAN_SPEED           CANSPEED_500
#define   CAN_ID_HVAC         0x100
#define   CAN_ID_LCD_TEXT1    0x200
#define   CAN_ID_LCD_TEXT2    0x201
#define   CAN_ID_LCD_COLOR    0x300
#define   CAN_ID_UP           0x400
#define   CAN_ID_DOWN         0x401
#define   CAN_ID_LEFT         0x402
#define   CAN_ID_RIGHT        0x403
#define   CAN_ID_CLICK        0x404
#define   UP                  A1
#define   DOWN                A3
#define   LEFT                A2
#define   RIGHT               A5
#define   CLICK               A4

rgb_lcd lcd;

void setup() 
{
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  lcd.setRGB(127, 127, 127);
  if(Canbus.init(CAN_SPEED)) {
    lcd.print("CAN shield ready");
  } else {
    lcd.print("CAN shield error");
  }

  pinMode(UP,INPUT);
  pinMode(DOWN,INPUT);
  pinMode(LEFT,INPUT);
  pinMode(RIGHT,INPUT);
  pinMode(CLICK,INPUT);
  
  digitalWrite(UP, HIGH);
  digitalWrite(DOWN, HIGH);
  digitalWrite(LEFT, HIGH);
  digitalWrite(RIGHT, HIGH);
  digitalWrite(CLICK, HIGH);
}

void loop() 
{
  tCAN received_message;
  if(mcp2515_check_message()) {
    if(mcp2515_get_message(&received_message)) {
      handle_received_message(&received_message);
    }
  }
  if(digitalRead(UP) == 0) send_message(UP);
  if(digitalRead(DOWN) == 0) send_message(DOWN);
  if(digitalRead(LEFT) == 0) send_message(LEFT);
  if(digitalRead(RIGHT) == 0) send_message(RIGHT);
  if(digitalRead(CLICK) == 0) send_message(CLICK);
}

void handle_received_message(const tCAN *message) {
  if(message->id == CAN_ID_HVAC) {
    String state_AC, state_temperature;
    if(message->data[0] == 0) {
      state_AC = "off";
    } else {
      state_AC = "on ";
    }
    state_temperature = message->data[1];
    lcd.setCursor(0, 1);
    lcd.print("AC:" + state_AC + "   temp:" + state_temperature);
  } else if(message->id == CAN_ID_LCD_TEXT1 || message->id == CAN_ID_LCD_TEXT2) {
    char text[9];
    for(int i = 0; i < 8; i++) {
      text[i] = message->data[i];
    }
    text[8] = '\0';
    if(message->id == CAN_ID_LCD_TEXT1) {
      lcd.setCursor(0, 0);
    } else {
      lcd.setCursor(8, 0);
    }
    lcd.print(text);
  } else if(message->id == CAN_ID_LCD_COLOR) {
    lcd.setRGB(message->data[0], message->data[1], message->data[2]);
  }
}

void send_message(const byte direction) {
  tCAN message;
  uint16_t id = 1;
  switch(direction) {
    case UP:    id = CAN_ID_UP; break;
    case DOWN:  id = CAN_ID_DOWN; break;
    case LEFT:  id = CAN_ID_LEFT; break;
    case RIGHT: id = CAN_ID_RIGHT; break;
    case CLICK: id = CAN_ID_CLICK; break;  
  }
  message.id = id;
  message.header.length = 8;
  for(int i = 0; i < 8; i++) {
    message.data[i] = i;
  }
  mcp2515_send_message(&message);
  delay(250);
}
