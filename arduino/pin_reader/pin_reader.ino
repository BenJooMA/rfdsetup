#include <stdio.h>

#define BUFF_SIZE   256

void setup()
{
  Serial.begin(9600);
}

int voltage_0 = 0;
int voltage_1 = 0;
int voltage_2 = 0;
int voltage_3 = 0;
int voltage_4 = 0;

float pin_2 = 0.0f;
float pin_3 = 0.0f;
float pin_4 = 0.0f;
float pin_5 = 0.0f;

char buff[BUFF_SIZE];

void loop()
{
  voltage_0 = analogRead(A0);
  voltage_1 = analogRead(A1);
  voltage_2 = analogRead(A2);
  voltage_3 = analogRead(A3);
  voltage_4 = analogRead(A4);

  pin_2 = (float)(voltage_1 - voltage_0) / 204.6f;
  pin_3 = (float)(voltage_2 - voltage_0) / 204.6f;
  pin_4 = (float)(voltage_3 - voltage_0) / 204.6f;
  pin_5 = (float)(voltage_4 - voltage_0) / 204.6f;

  Serial.print(pin_2);
  Serial.print("\t");
  Serial.print(pin_3);
  Serial.print("\t");
  Serial.print(pin_4);
  Serial.print("\t");
  Serial.print(pin_5);
  Serial.print("\n");

  delay(1000);
}