#define LINE_TOKEN "AOetYQRQXZgpKkkvoGoBLe9mOPyGE61s14vqfoIkSWI"
#define BLYNK_AUTH_TOKEN "nhpufa5_n0ktq3Uk3M6hEpnlef_nMi-O"
#define BLYNK_TEMPLATE_ID "TMPL6yC8OwH0v"
#define BLYNK_TEMPLATE_NAME "CarbonMonoxide"

#define MQ_7 34

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include <BlynkSimpleEsp32.h>

const int rs = 22, en = 21, d4 = 19, d5 = 18, d6 = 5, d7 = 15;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

float analysis(int);
float sensorResistance;
void sendLineNotify(String);

void setup()
{
  Serial.begin(115200);
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
  lcd.home();

  lcd.setCursor(3, 0);
  lcd.print("CO Sensor");
  lcd.setCursor(3, 1);
  lcd.print("System On");
  delay(1000);
  lcd.clear();

  lcd.setCursor(1, 0);
  lcd.print("Please Connect");
  lcd.setCursor(4, 1);
  lcd.print("WiFi....");
  delay(1000);
  lcd.clear();

  WiFiManager wm;
  bool res;

  IPAddress staticIP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  wm.setAPStaticIPConfig(staticIP, gateway, subnet);

  lcd.setCursor(0, 0);
  lcd.print("WiFi Connecting.");
  delay(1000);

  res = wm.autoConnect("!CarbonMonoxide", "11111111");
  if (!res)
  {
    Serial.println("Failed to connect");
    ESP.restart();
  }
  else
  {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("WiFi Connected");
    sendLineNotify("เชื่อมต่อกับเครื่องวัดระบบคาร์บอนมอนอกไซด์สำเร็จแล้ว✅!");
    Serial.println("เชื่อมต่อสำเร็จแล้ว!");
    delay(1000);
    lcd.clear();
    Blynk.config(BLYNK_AUTH_TOKEN);
  }
}

void loop()
{
  Blynk.run();
  int sensorValue = analogRead(MQ_7);
  float co_level = analysis(sensorValue);

  Serial.print("adc : ");
  Serial.print(sensorValue);
  Serial.print("\t");
  Serial.print("Carbon monoxide : ");
  Serial.print(co_level, 3);
  Serial.print(" ppm");
  Serial.print("\t");
  Serial.print("Sensor resistance : ");
  Serial.println(sensorResistance);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CO: ");
  lcd.print(co_level, 3);
  lcd.print(" ppm");

  Blynk.virtualWrite(V1, String(co_level, 2));

  lcd.setCursor(0, 1);
  lcd.print("Rs: ");
  lcd.print(sensorResistance, 3);

  if (co_level >= 35)
  {
    String message = "⚠️ระบบตรวจจับคาร์บอนมอนอกไซด์เริ่มมีระดับที่สูงขึ้น: " + String(co_level, 2) + " ppm";
    sendLineNotify(message);
    delay(2000);
  }
  else
  {
    delay(500);
  }
}

float analysis(int adc)
{
  float slope = -0.7516072988;
  float A = 45.87510694;
  float Rseries = 1000;
  float V_Rseries = ((float)adc * 3.3) / 4095;
  sensorResistance = ((3.3 - V_Rseries) / V_Rseries) * Rseries;
  float R0 = 400;
  float Y = sensorResistance / R0;
  float CO_ppm = pow(10, (log10(Y / A) / slope));
  return CO_ppm;
}

void sendLineNotify(String message)
{
  HTTPClient http;
  http.begin("https://notify-api.line.me/api/notify");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Authorization", "Bearer " + String(LINE_TOKEN));

  String httpRequestData = "message=" + message;
  int httpResponseCode = http.POST(httpRequestData);

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}