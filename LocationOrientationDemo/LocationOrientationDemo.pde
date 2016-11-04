import processing.serial.*;
import java.awt.datatransfer.*;
import java.awt.Toolkit;
import processing.opengl.*;
//import saito.objloader.*;
//import g4p_controls.*;
import controlP5.*;
import peasy.*;
 
Serial myPort; //creates a software serial port
PeasyCam cam;
ControlP5 cp5;
Slider abc;
float roll  = 0.0F;
float pitch = 0.0F;
float yaw   = 0.0F;

// only at recieving x and y
float pRoll = 0.0F;
float pPitch = 0.0F;
float pYaw = 0.0F;

float dRoll = 0.0F;
float dPitch = 0.0F;
float dYaw = 0.0F;

float deltaT = 0.0F;
float locX = 0.0F;
float pX = 0.0F;
float locY = 0.0F;
float pY = 0.0F;

float dX = 0.0F;
float dY = 0.0F;
float surfaceQuality = 0.0F;

float expDX = 0.0F;
float expDY = 0.0F;

float estDX = 0.0F;
float estDY = 0.0F;

float x_cm = 0.0F;
float y_cm = 0.0F;


float alt = 0.0F;
float conv_factor = 0.0F;
float radians_to_pixels = 0.0F;
float MAGIC_NUMBER1 = 1.0F;
float MAGIC_NUMBER2 = 1.0F;
//////////////////////////////////////////////////////////////////////////////////////////
/*//////////////////////Grid Stuff////////////////////////////////////////////////////////////
*///////////////////////////////////////////////////////////////////////////////////////////
int rows, cols, sclW, sclH;
final int w = 2000;
final int h = 1500;
float flying;
float terrain[][];



//OBJModel model;

// Serial port state.
Serial port;
final String serialConfigFile = "serialconfig.txt";
boolean      printSerial = true;
String fileName;

 // updates conversion factors that are dependent upon field_of_view
 // field of view of ADNS3080 sensor lenses
final float AP_OPTICALFLOW_ADNS3080_08_FOV = 0.202458F;        // 11.6 degrees

// scaler - value returned when sensor is moved equivalent of 1 pixel
final float AP_OPTICALFLOW_ADNS3080_SCALER_400 = 1.1F;       // when resolution set to 400
final float AP_OPTICALFLOW_ADNS3080_SCALER_1600 = 4.4F;       // when resolution set to 1600

// ADNS3080 hardware config
final float ADNS3080_PIXELS_X  = 30;
final float ADNS3080_PIXELS_Y = 30;
final float ADNS3080_CLOCK_SPEED = 24000000;

float[] gainMins = {1.0F, 1.0F, 1.0F};
float[] gainMaxs = {100.0F, 100.0F, 100.0F};
float[] disGain = {1.0F, 1.0F, 1.0F}; 


void setup()
{
  update_conversion_factors();
  size(900, 600, OPENGL);
  frameRate(60);
  String portName = Serial.list()[3]; 
  myPort = new Serial(this, portName, 115200); //set up your port to listen to the serial port
  // don't generate a serialEvent() unless you get a newline character:
  myPort.bufferUntil('\n');
  
  //////////////////////////////////// Camera setup////////////////////////////////
  cam = new PeasyCam(this,w/2, h/2, 0,1000);
  cam.setMinimumDistance(20);
  cam.setMaximumDistance(1000);
  
  ///////////////////////////////////Grid setup /////////////////////////////////
  sclH = h/200;//h / 1000;
  sclW = w/200;//w/ 100;
  cols = (w / sclW);
  rows = h /sclH;
  //frameRate(50);
  terrain = new float[cols][rows];
  //flying = 0;
  float yOff = 0;//flying;
  for(int y = 0; y < rows; y++)
  {
    float xOff = 0;
    for(int x = 0; x < cols; x++)
    {
      terrain[x][y] = map(noise(xOff, yOff), 0, -1, -150, 150);
      xOff += 0.009;
      
    }
    yOff += 0.009;
  }
  
  //////////////////////////////////// Gui setup////////////////////////////////
  cp5 = new ControlP5(this);
  cp5.addButton("button", 10, 100, 60, 80, 20).setId(1);
  cp5.addSlider("gainSlide")
     .setPosition(100,50)
     .setRange(gainMins[0],gainMaxs[0])
     .setColorForeground(100);
  cp5.setAutoDraw(false);
}

void draw()
{
  background(255,255,255);
  
  showCopter();
  //showGrid();
  fill(0, 102, 153);
  String s = "Alt: " + (alt) + "\n" + "X: " + locX + "\n" + "Y: " + locY;
  textSize(18);
  text(s, ((w/2)+50)-locX, locY +(h/2));
  
  gui(); // makes gui stay on top of everything else
}
void gui() {
  hint(DISABLE_DEPTH_TEST);
  cam.beginHUD();
  cp5.draw();
  cam.endHUD();
  hint(ENABLE_DEPTH_TEST);
}

boolean overCircle(double x, double y, int diameter) {
  double disX = x - mouseX;
  double disY = y - mouseY;
  if (sqrt(sq((float)disX) + sq((float)disY)) < diameter/2 ) {
    return true;
  } else {
    return false;
  }
}
void showGrid()
{
  /*flying -= 0.001;
  float yOff = flying;
  
  
  for(int y = 0; y < rows; y++)
  {
    float xOff = 0;
    for(int x = 0; x < cols; x++)
    {
      terrain[x][y] = map(noise(xOff, yOff), 0, -1, -100, 100);
      xOff += 0.06;
      
    }
    yOff += 0.06;
  }*/
  pushMatrix();
  noStroke();
  //stroke(51,38,29);
  fill(68,169,10,80);
  //translate(w/2, h/2, -h);
  rotateX(0);
  rotateY(0);
  rotateZ(0);
  translate(0, 0, 0);
  
  for(int y = 0; y < rows-1; y++)
  {
    beginShape(TRIANGLE_STRIP);
    
    for(int x = 0; x < cols; x++)
    {
      vertex(x*sclW, y *sclH, terrain[x][y]);
      vertex(x*sclW, (y+1) *sclH, terrain[x][y+1]);
    }
    endShape();
  }
  popMatrix();
}


void showCopter()
{
  // Set a new co-ordinate space
  pushMatrix();
  stroke(0);
  // Simple 3 point lighting
  pointLight(255, 200, 200,  400, 400,  500);
  pointLight(255, 200, 255, w, h,  0);
  pointLight(255, 255, 255,    -500,   -500, -500);
  
  translate((w/2)-locX, (h/2)+locY, h/1000);
  
  rotateX(0);
  rotateY(0);
  rotateZ(0);
  rotateZ(radians(yaw));
  rotateX(radians(pitch));
  rotateY(radians(roll));
  fill(100, 100, 100);
  box(20);
  
  fill(255,0,0);
  box(5,5,100);    
  //pushMatrix();
  
  sphere(10);
  //popMatrix();
 
  fill(0,255,0);
  box(100,10,10);
  //pushMatrix();
  ellipse(50, 0, 30, 30);
  ellipse(-50, 0, 30, 30);
  sphere(10);
  
  //popMatrix();
 
  fill(0,0,255);
  box(10,100,10);
  //pushMatrix();
  ellipse(0, 50, 30, 30);
  ellipse(0, -50, 30, 30);
  sphere(10);
  //popMatrix();
  
  popMatrix();
  
}
void serialEvent(Serial p) 
{
  
  String incoming = p.readString();
  if (printSerial) {
    
    //println(incoming);
  }
  if ((incoming.length() > 8))
  {
    String[] list = split(incoming, " ");
    if ( (list.length > 0) && (list[0].equals("Orientation:")) ) 
    {
      roll  = float(list[3]); // Roll = Z
      pitch = float(list[2]); // Pitch = Y 
      yaw   = float(list[1]); // Yaw/Heading = X
      
    }
    if ( (list.length > 0) && (list[0].equals("Alt:")) ) 
    {
      alt  = float(list[1]);
      expDX += float(list[3]);
      expDY += float(list[2]);
    }
    if ( (list.length > 0) && (list[0].equals("Location:")) ) 
    {
      
      surfaceQuality= float(list[3]); 
      
      //dX = locX - pX;
      //dY = locY - pY;
      dX = float(list[2]);//4
      dY = float(list[3]);//5
      
      dRoll = roll - pRoll;
      dPitch = pitch - pPitch;
      dYaw = yaw - pYaw;
      pRoll = roll;
      pPitch = pitch;
      pYaw = yaw;
      
      //expDX = sin(radians(-dRoll))* alt;
      //expDY = sin(radians(dPitch)) * alt;
      
      estDX = (conv_factor*alt*dX*disGain[0]) - expDX;   //-(locY - ((sin(radians(dYaw))*cos(radians(dPitch)) *dX)));
      estDY = (conv_factor*alt*dY*disGain[0]) - expDY;   //-(locX - (cos(radians(dYaw))*sin(radians(dRoll)) * dY));
      
      
      
      x_cm = estDX ;// * altitude
      y_cm = estDY ; // * altitude
      //println("est: " + x_cm + '\t' + y_cm );
      println("CompDist: " + expDX + "\t motionDist: " + (conv_factor*alt*dX*disGain[0]));
      expDX = 0.0;
      expDY = 0.0;
      locX += x_cm;
      locY += y_cm;
      
      
    }
    
  }
}
void update_conversion_factors()
{
    // multiply this number by altitude and pixel change to get horizontal
    // move (in same units as altitude)
    conv_factor = ((1.0f / (float)(ADNS3080_PIXELS_X * AP_OPTICALFLOW_ADNS3080_SCALER_1600))
                   * 2.0f * tan(AP_OPTICALFLOW_ADNS3080_08_FOV / 2.0f));
    // 0.00615
    radians_to_pixels = (ADNS3080_PIXELS_X * AP_OPTICALFLOW_ADNS3080_SCALER_1600) / (AP_OPTICALFLOW_ADNS3080_08_FOV);
    // 162.99
}

// Set serial port to desired value.
void setSerialPort(String portName) {
  // Close the port if it's currently open.
  if (port != null) {
    port.stop();
  }
  try {
    // Open port.
    port = new Serial(this, portName, 115200);
    port.bufferUntil('\n');
    // Persist port in configuration.
    saveStrings(serialConfigFile, new String[] { portName });
  }
  catch (RuntimeException ex) {
    // Swallow error if port can't be opened, keep port closed.
    port = null; 
  }
}




// resetting the location of the box
void resetBox()
{
  roll  = 0.0F;
  pitch = 0.0F;
  yaw   = 0.0F;
  
  // only at recieving x and y
  pRoll = 0.0F;
  pPitch = 0.0F;
  pYaw = 0.0F;
  
  dRoll = 0.0F;
  dPitch = 0.0F;
  dYaw = 0.0F;
  
  deltaT = 0.0F;
  locX = 0.0F;
  pX = 0.0F;
  locY = 0.0F;
  pY = 0.0F;
  
  dX = 0.0F;
  dY = 0.0F;
  surfaceQuality = 0.0F;
  
  expDX = 0.0F;
  expDY = 0.0F;
  
  estDX = 0.0F;
  estDY = 0.0F;
  
  x_cm = 0.0F;
  y_cm = 0.0F;
  
  
  alt = 0.0F;
  conv_factor = 0.0F;
  radians_to_pixels = 0.0F;
  MAGIC_NUMBER1 = 1.0F;
  MAGIC_NUMBER2 = 1.0F;
  deltaT = 0.0F;
  /*if (port != null) {
    port.stop();
  }*/
  if (myPort != null) {
    myPort.stop();
  }
  String portName = Serial.list()[3]; 
  myPort = new Serial(this, portName,  115200); //set up your port to listen to the serial port
  // don't generate a serialEvent() unless you get a newline character:
  myPort.bufferUntil('\n');
}

void gainSlide(float value) {
  disGain[0] = value;
  println("disgain now: " + value);
}

void button(float theValue) {
  resetBox();
  
}