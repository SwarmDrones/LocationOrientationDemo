//
// Created by Ariel Feliz on 10/23/16.
//


#ifndef CPE810_CPP_ADNS3080_H
#define CPE810_CPP_ADNS3080_H

#include <SPI.h>

// ADNS3080 hardware config
#define ADNS3080_PIXELS_X                 30
#define ADNS3080_PIXELS_Y                 30
#define ADNS3080_CLOCK_SPEED			  24000000

// Register Map for the ADNS3080 Optical OpticalFlow Sensor
#define ADNS3080_PRODUCT_ID            0x00
#define ADNS3080_REVISION_ID           0x01
#define ADNS3080_MOTION                0x02
#define ADNS3080_DELTA_X               0x03
#define ADNS3080_DELTA_Y               0x04
#define ADNS3080_SQUAL                 0x05
#define ADNS3080_PIXEL_SUM             0x06
#define ADNS3080_MAXIMUM_PIXEL         0x07
#define ADNS3080_CONFIGURATION_BITS    0x0a
#define ADNS3080_EXTENDED_CONFIG       0x0b
#define ADNS3080_DATA_OUT_LOWER        0x0c
#define ADNS3080_DATA_OUT_UPPER        0x0d
#define ADNS3080_SHUTTER_LOWER         0x0e
#define ADNS3080_SHUTTER_UPPER         0x0f
#define ADNS3080_FRAME_PERIOD_LOWER    0x10
#define ADNS3080_FRAME_PERIOD_UPPER    0x11
#define ADNS3080_MOTION_CLEAR          0x12
#define ADNS3080_FRAME_CAPTURE         0x13
#define ADNS3080_SROM_ENABLE           0x14
#define ADNS3080_FRAME_PERIOD_MAX_BOUND_LOWER      0x19
#define ADNS3080_FRAME_PERIOD_MAX_BOUND_UPPER      0x1a
#define ADNS3080_FRAME_PERIOD_MIN_BOUND_LOWER      0x1b
#define ADNS3080_FRAME_PERIOD_MIN_BOUND_UPPER      0x1c
#define ADNS3080_SHUTTER_MAX_BOUND_LOWER           0x1e
#define ADNS3080_SHUTTER_MAX_BOUND_UPPER           0x1e
#define ADNS3080_SROM_ID               0x1f
#define ADNS3080_OBSERVATION           0x3d
#define ADNS3080_INVERSE_PRODUCT_ID    0x3f
#define ADNS3080_PIXEL_BURST           0x40
#define ADNS3080_MOTION_BURST          0x50
#define ADNS3080_SROM_LOAD             0x60

// Configuration Bits
#define ADNS3080_LED_MODE_ALWAYS_ON        0x00
#define ADNS3080_LED_MODE_WHEN_REQUIRED    0x01

#define ADNS3080_RESOLUTION_400			400
#define ADNS3080_RESOLUTION_1600		1600

// Extended Configuration bits
#define ADNS3080_SERIALNPU_OFF	0x02

#define ADNS3080_FRAME_RATE_MAX         6469
#define ADNS3080_FRAME_RATE_MIN         2000

// Id returned by ADNS3080_PRODUCT_ID register
#define ADNS3080_PRODUCT_ID_VALUE      0x17

SPISettings spiSettings(2e6, MSBFIRST, SPI_MODE3); // 2e6 = 2 MHz, mode 3
class myADNS3080
{
public:
    myADNS3080()
    {
        //RESET_PIN = resetPin;
        //SS_PIN = ssPin;
        x = 0;
        y = 0;
    }
    void updateLocation()
    {
        #if 1
                updateSensor();
        #else
                Serial.println(F("image data --------------"));
                printPixelData();
                Serial.println(F("-------------------------"));
                Serial.flush();
                delay(1500);
        #endif
    }
    void setup()
    {
        SPI.begin();
        pinMode(SS_PIN, OUTPUT);
        pinMode(RESET_PIN, OUTPUT);
        reset();
        uint8_t id = spiRead(ADNS3080_PRODUCT_ID);
        if (id == ADNS3080_PRODUCT_ID_VALUE)
            Serial.println(F("ADNS-3080 found"));
        else {
            Serial.print(F("Could not find ADNS-3080: "));
            Serial.println(id, HEX);
            while (1);
        }
        
        uint8_t config = spiRead(ADNS3080_CONFIGURATION_BITS);
        spiWrite(ADNS3080_CONFIGURATION_BITS, config | 0x10); // Set resolution to 1600 counts per inch
    }
    int32_t getX()
    {
        return x;
    }
    int32_t getY()
    {
        return y;
    }

private:
    int32_t x;
    int32_t y;
    const uint8_t RESET_PIN = 9;
    const uint8_t SS_PIN = 10;
    
    void printPixelData(void)
    {
        bool isFirstPixel = true;
        
        // Write to frame capture register to force capture of frame
        spiWrite(ADNS3080_FRAME_CAPTURE, 0x83);
        
        // Wait 3 frame periods + 10 nanoseconds for frame to be captured
        delayMicroseconds(1510); // Minimum frame speed is 2000 frames/second so 1 frame = 500 nano seconds. So 500 x 3 + 10 = 1510
        
        // Display the pixel data
        for (uint8_t i = 0; i < ADNS3080_PIXELS_Y; i++) {
            for (uint8_t j = 0; j < ADNS3080_PIXELS_X; j++) {
                uint8_t regValue = spiRead(ADNS3080_FRAME_CAPTURE);
                if (isFirstPixel && !(regValue & 0x40)) {
                    Serial.println(F("Failed to find first pixel"));
                    goto reset;
                }
                isFirstPixel = false;
                uint8_t pixelValue = regValue << 2; // Only lower 6 bits have data
                Serial.print(pixelValue);
                if (j != ADNS3080_PIXELS_X - 1)
                    Serial.write(',');
            }
            Serial.println();
            Serial.flush();
        }
        
    reset:
        reset(); // Hardware reset to restore sensor to normal operation
    }
    
    void updateSensor(void)
    {
        // Read sensor
        uint8_t buf[4];
        spiRead(ADNS3080_MOTION_BURST, buf, 4);
        uint8_t motion = buf[0];
        //Serial.print(motion & 0x01); // Resolution
        
        if (motion & 0x10) // Check if we've had an overflow
            Serial.println(F("ADNS-3080 overflow\n"));
        else if (motion & 0x80) {
            int8_t dx = buf[1];
            int8_t dy = buf[2];
            uint8_t surfaceQuality = buf[3];
            
            x += dx;
            y += dy;
            Serial.print(F("Location: "));
            Serial.print(x);
            Serial.print(F(" "));
            Serial.print(y);
            Serial.print(F(" "));
            Serial.print(surfaceQuality);
            Serial.println(F(" "));
            
            // Print values
            /*
            Serial.print(x);
            Serial.write(',');
            Serial.print(dx);
            Serial.write('\t');
            Serial.print(y);
            Serial.write(',');
            Serial.print(dy);
            Serial.write('\t');
            Serial.println(surfaceQuality);
             */
            //Serial.flush();
        }
        #if 0
                else
                    Serial.println(motion, HEX);
        #endif
                
                delay(10);
    }
    void reset(void)
    {
        digitalWrite(RESET_PIN, HIGH); // Set high
        delayMicroseconds(10);
        digitalWrite(RESET_PIN, LOW); // Set low
        delayMicroseconds(500); // Wait for sensor to get ready
    }
    void clearMotion()
    {
        spiWrite(ADNS3080_MOTION_CLEAR, 0xFF); // Writing anything to this register will clear the sensor's motion registers
        x = y = 0;
    }
    void spiWrite(uint8_t reg, uint8_t data)
    {
        spiWrite(reg, &data, 1);
    }
    void spiWrite(uint8_t reg, uint8_t *data, uint8_t length)
    {
        SPI.beginTransaction(spiSettings);
        digitalWrite(SS_PIN, LOW);
        
        SPI.transfer(reg | 0x80); // Indicate write operation
        delayMicroseconds(75); // Wait minimum 75 us in case writing to Motion or Motion_Burst registers
        SPI.transfer(data, length); // Write data
        
        digitalWrite(SS_PIN, HIGH);
        SPI.endTransaction();
    }
    uint8_t spiRead(uint8_t reg)
    {
        uint8_t buf;
        spiRead(reg, &buf, 1);
        return buf;
    }
    
    void spiRead(uint8_t reg, uint8_t *data, uint8_t length)
    {
        SPI.beginTransaction(spiSettings);
        digitalWrite(SS_PIN, LOW);
        
        SPI.transfer(reg); // Send register address
        delayMicroseconds(75); // Wait minimum 75 us in case writing to Motion or Motion_Burst registers
        memset(data, 0, length); // Make sure data buffer is 0
        SPI.transfer(data, length); // Write data
        
        digitalWrite(SS_PIN, HIGH);
        SPI.endTransaction();
    }
    
};

#endif //CPE810_CPP_ADNS3080_H
