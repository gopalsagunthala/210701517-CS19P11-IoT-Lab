#define BLYNK_TEMPLATE_ID "TMPL3M4fqhU1J"
#define BLYNK_TEMPLATE_NAME "Health Monitoring System"

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

char auth[] = "SLuqZmzEEQJaEC0YiALtuUUP3MsTYMG2";
char ssid[] = "Siva";
char pass[] = "password";

#define REPORTING_PERIOD_MS 1000
#define DHT_READ_INTERVAL_MS 5000  // Read DHT sensor every 5 seconds
#define DHTPIN 23 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11

PulseOximeter pox;
DHT dht(DHTPIN, DHTTYPE);

uint32_t tsLastReportMax30100 = 0;
uint32_t tsLastReportDHT = 0;

float lastTemperature = 0;
float lastHumidity = 0;

int heartRate=0;
int spo2=0;

int lastHeartRate = 0;
int lastSpO2 = 0;

int flag1=0,flag2=0,alert_value=0;

int highflag=0,high_alert=0;
int lowflag=0,low_alert=0;
int spflag=0,sp_alert=0;

void onBeatDetected()
{
    Serial.println("Beat!");
}

void setup()
{
    Serial.begin(115200);
    Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
    Serial.print("Initializing pulse oximeter..");

    if (!pox.begin())
    {
        Serial.println("FAILED");
        for (;;)
            ;
    }
    else
    {
        Serial.println("SUCCESS");
    }

    pox.setOnBeatDetectedCallback(onBeatDetected);

    dht.begin();
}

void loop()
{
    // Update MAX30100 sensor
    pox.update();
    bool timeOut=false;
    float temp,humi;

    // Read MAX30100 sensor data and send to Blynk
    if (millis() - tsLastReportMax30100 > REPORTING_PERIOD_MS)
    {
        heartRate = pox.getHeartRate();
        spo2 = pox.getSpO2();
        if (heartRate != 0)
        {
            lastHeartRate = heartRate;
            Blynk.virtualWrite(V0, lastHeartRate);
            Serial.print("Heart rate: ");
            Serial.println(lastHeartRate);
            flag1=0;
            alert_value=0;
            if(heartRate>185)
              highflag=1;
            else
            {
              highflag=0;
              high_alert=0;
            }
            if(heartRate>0 && heartRate<50)
              lowflag=1;
            else
            {
              lowflag=0;
              low_alert=0;
            }
        }
        else
        {
            lastHeartRate = 0;
            Blynk.virtualWrite(V0, lastHeartRate);
            Serial.println("Heart rate not available");
            flag1=1;
        }

        if (spo2 != 0)
        {
            lastSpO2 = spo2;
            Blynk.virtualWrite(V1, lastSpO2);
            Serial.print("SpO2: ");
            Serial.println(lastSpO2);
            flag2=0;
            alert_value=0;
            if(spo2<90)
              spflag=1;
            else
            {
              spflag=0;
              sp_alert=0;
            }
        }
        else
        {
            lastSpO2 = 0;
            Blynk.virtualWrite(V1, lastSpO2);
            Serial.println("SpO2 not available");
            flag2=1;
        }
        if(flag1!=0 || flag2!=0)
        {
          alert_value++;
          flag1=0;
          flag2=0;
        }
        if(highflag!=0)
          high_alert++;
        if(lowflag!=0)
          low_alert++;

        tsLastReportMax30100 = millis();
    }

    // Read DHT sensor data and send to Blynk after a delay
    if (millis() - tsLastReportDHT > DHT_READ_INTERVAL_MS)
    {
         temp = dht.readTemperature();
         humi = dht.readHumidity();

        if (!isnan(temp))
        {
            lastTemperature = temp;
            Blynk.virtualWrite(V3, lastTemperature);
            Serial.print("Temperature: ");
            Serial.println(lastTemperature);
        }
        else
        {
            lastTemperature = 0;
            Blynk.virtualWrite(V3, lastTemperature);
            Serial.println("Temperature not available");
        }

        if (!isnan(humi))
        {
            lastHumidity = humi;
            Blynk.virtualWrite(V2, lastHumidity);
            Serial.print("Humidity: ");
            Serial.println(lastHumidity);
        }
        else
        {
            lastHumidity = 0;
            Blynk.virtualWrite(V2, lastHumidity);
            Serial.println("Humidity not available");
        }

        tsLastReportDHT = millis();
    }


  /*
        ALERTS : 
  */

    bool temptrigger=false;
    if(temptrigger==false && temp>=40)
    {
      temptrigger=true;
      Blynk.logEvent("alert","TEMPERATURE IS HIGH");
    }
    else
    {
      temptrigger=false;
    }
    //Serial.println(alert_value);
    if(alert_value==60)
    {
      alert_value=0;
      Blynk.logEvent("alert","Please wear the watch properly.");
    }
    if(high_alert==10)
    {
      high_alert=0;
      heartRate=0;
      Blynk.logEvent("alert","Continuous high heart rate is observed. Kindly take safety measures");
    }
    // if(low_alert>=10)
    // {
    //   low_alert=0;
    //   heartRate=0;
    //   Blynk.logEvent("alert","Continuous low heart rate is observed. Kindly take safety measures");
    // }
    if(sp_alert==10)
    {
      sp_alert=0;
      Blynk.logEvent("alert","Low blood oxygen level is detected.");
    }

    Blynk.run();
}
