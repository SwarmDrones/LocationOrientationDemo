/* Board layout:
         +----------+
         |         *| RST   PITCH  ROLL  HEADING
     ADR |*        *| SCL
     INT |*        *| SDA     ^            /->
     PS1 |*        *| GND     |            |
     PS0 |*        *| 3VO     Y    Z-->    \-X
         |         *| VIN
         +----------+
  */
/*
 *  Uses the imu to gather orientation and motion flow camera to gather location along with the processing demo
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <ADNS3080.h>

double ct = 0.00;
double pt = 0.00;
double dt = 0.00;
/* Set the delay between fresh samples */
#define BNO055_SAMPLERATE_DELAY_MS (100)
Adafruit_BNO055 bno = Adafruit_BNO055(55);

  
void displaySensorDetails(void)
{
  sensor_t sensor;
  bno.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" xxx");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" xxx");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" xxx");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}
void printOrientation(sensors_event_t &event)
{
  Serial.print(F("DeltaTime: "));
  Serial.print((double)dt);
  Serial.print(F(" "));
  Serial.print((double)dt);
  Serial.print(F(" "));
  Serial.print((double)dt);
  Serial.println(F(" "));

  /* The processing sketch expects data as roll, pitch, heading */
  Serial.print(F("Orientation: "));
  Serial.print((double)event.orientation.x);
  Serial.print(F(" "));
  Serial.print((double)event.orientation.y);
  Serial.print(F(" "));
  Serial.print((double)event.orientation.z);
  Serial.println(F(""));
}
/*
 *  ADNS3080 object
 */

myADNS3080 moFlow;// lol

void setup()
{
  
  Serial.begin(115200);
  //while (!Serial);
  Serial.println("Orientation Sensor Test"); Serial.println("");
  /******************************************************************************
   * THE IMU SENSOR SETUP
   *****************************************************************************/
  if(!bno.begin())
  {
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  delay(1000);
  displaySensorDetails();
  /******************************************************************************
   * THE OPTICAL FLOW SENSOR SETUP
   *****************************************************************************/
  moFlow.setup();
}

void loop()
{
  /* Get a new sensor event */
  sensors_event_t event;
  bno.getEvent(&event);
  pt = ct;
  ct = millis();
  dt = ct-pt;
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
  /* Orientation from IMU sensor */
  printOrientation(event);
  //delay(BNO055_SAMPLERATE_DELAY_MS);
//////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////// 
  /* Location from ADNS3080 sensor */
  moFlow.updateLocation();
  
}


