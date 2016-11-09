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

double alt_cm = 0.00;
double roll = 0.00, pitch = 0.00, yaw = 0.00;
double droll = 0.00, dpitch = 0.00, dyaw = 0.00;
double proll = 0.00, ppitch = 0.00, pyaw = 0.00;
double x_cm = 0.00, y_cm = 0.00;    // 
double dx_cm = 0.00, dy_cm = 0.00;  // displacement
double px_cm = 0.00, py_cm = 0.00;  // previous values
double ix_cm = 0.00, iy_cm = 0.00;  // integral
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
  Serial.println((double)dt);

  /* The processing sketch expects data as roll, pitch, heading */
  proll = roll;
  ppitch = pitch;
  pyaw = yaw;
  roll = event.orientation.z;
  pitch = event.orientation.y;
  yaw = event.orientation.x;
  droll = roll - proll;
  dpitch = pitch - ppitch;
  dyaw = yaw - pyaw;
  
  Serial.print(F("Orientation: "));
  Serial.print((double)yaw);
  Serial.print(F(" "));
  Serial.print((double)pitch);
  Serial.print(F(" "));
  Serial.print((double)roll);
  Serial.println(F(""));
}

/*
 * Lidar  Functions
 */
 void lidarSetup()
 {
    pinMode(2, OUTPUT); // Set pin 2 as trigger pin
    pinMode(3, INPUT); // Set pin 3 as monitor pin
    digitalWrite(2, LOW); // Set trigger LOW for continuous read
 }

 void getAlt_cm()
 {
    double pulse_width = pulseIn(3, HIGH); // Count how long the pulse is high in microseconds
    if(pulse_width != 0)
    { // If we get a reading that isn't zero, let's print it
      alt_cm = pulse_width/10; // 10usec = 1 cm of distance for LIDAR-Lite
      px_cm = x_cm;
      py_cm = y_cm;
      
      x_cm = alt_cm*sin(radians(roll));
      y_cm = alt_cm*sin(radians(pitch));
    
      dx_cm = alt_cm*sin(radians(droll));//x_xm - px_cm;
      dy_cm = alt_cm*sin(radians(dpitch));//y_cm - py_cm;
    
      ix_cm += dx_cm;
      iy_cm += dy_cm;
      
      Serial.print(F("Alt: "));
      Serial.print((double)alt_cm);
      Serial.print(F(" "));
      Serial.print((double)dx_cm);
      Serial.print(F(" "));
      Serial.print((double)dy_cm);
      Serial.println(F(""));
    }
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
  
  /******************************************************************************
   * THE Lidar lite SENSOR SETUP
   *****************************************************************************/
  lidarSetup();
  
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

//////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////// 
  /* altitude measurement */
  getAlt_cm();

  
  
}


