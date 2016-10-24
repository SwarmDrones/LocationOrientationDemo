import processing.serial.*;
import java.awt.datatransfer.*;
import java.awt.Toolkit;
import processing.opengl.*;
//import saito.objloader.*;
import g4p_controls.*;
import peasy.*;
 
Serial myPort; //creates a software serial port
PeasyCam cam;
float roll  = 0.0F;
float pitch = 0.0F;
float yaw   = 0.0F;
float deltaT = 0.0F;
float locX = 0.0F;
float locY = 0.0F;
float surfaceQuality = 0.0F;

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


void setup()
{
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
}

void draw()
{
  background(100,100,100);
  
  
  showCopter();
  showGrid();
  fill(0, 102, 153);
  String s = "Surface Quality: " + (surfaceQuality) + "\n" + "X: " + locX + "\n" + "Y: " + locY;
  textSize(18);
  text(s, locX+(w/2)+50, locY +(h/2));
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
  fill(68,169,10);
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
  pointLight(255, 200, 200,  400, 400,  500);//pointLight(200, 200, 255, -400, 400,  500);//pointLight(255, 255, 255,    0,   0, -500);
  
  translate((w/2)+locX, (h/2)+locY, h/1000);
  
  rotateX(0);
  rotateY(0);
  rotateZ(0);
  //rotateZ(radians((float)roll));
  //rotateX(radians((float)pitch));
  //rotateY(radians((float)yaw));
  //rotateZ(radians(roll));
  //rotateX(radians(yaw));
  //rotateY(radians(pitch));
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
    println(incoming);
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
    if ( (list.length > 0) && (list[0].equals("Location:")) ) 
    {
      locX  = float(list[1]); 
      locY = float(list[2]); 
      surfaceQuality= float(list[3]); 
    }
    
  }
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