#ifndef MBED_S6D04D1_H
#define MBED_S6D04D1_H



#include "mbed.h"
#include "TFT.h"

/** Class for S6D04D1 TFT display controller
* for BM8K5113 TFT1P3520-E TFT8K5113FPC-A1-E and TFT1P3520
* to be copypasted and adapted for other controllers
*/
class S6D04D1 : public TFT
{
 
 public:

    /** Create a PAR display interface
    * @param displayproto PAR_8 or PAR_16
    * @param port GPIO port name to use
    * @param CS pin connected to CS of display
    * @param reset pin connected to RESET of display
    * @param DC pin connected to data/command of display
    * @param WR pin connected to SDI of display
    * @param RD pin connected to RS of display
    * @param name The name used by the parent class to access the interface
    * @param LCDSIZE_X x size in pixel - optional
    * @param LCDSIZE_Y y size in pixel - optional
    */ 
    S6D04D1(proto_t displayproto, PortName port, PinName CS, PinName reset, PinName DC, PinName WR, PinName RD, const char* name , unsigned int LCDSIZE_X = 240, unsigned  int LCDSIZE_Y = 400);
    
    /** Create a BUS display interface
    * @param displayproto BUS_8 or BUS_16
    * @param buspins array of PinName to group as Bus
    * @param CS pin connected to CS of display
    * @param reset pin connected to RESET of display
    * @param DC pin connected to data/command of display
    * @param WR pin connected to SDI of display
    * @param RD pin connected to RS of display
    * @param name The name used by the parent class to access the interface
    * @param LCDSIZE_X x size in pixel - optional
    * @param LCDSIZE_Y y size in pixel - optional
    */ 
    S6D04D1(proto_t displayproto, PinName* buspins, PinName CS, PinName reset, PinName DC, PinName WR, PinName RD, const char* name ,const unsigned int LCDSIZE_X = 240, unsigned  int LCDSIZE_Y = 400);

    
    /** Create an SPI display interface
    * @param displayproto SPI_8 or SPI_16
    * @param Hz SPI speed in Hz
    * @param mosi SPI pin
    * @param miso SPI pin
    * @param sclk SPI pin
    * @param CS pin connected to CS of display
    * @param reset pin connected to RESET of display
    * @param DC pin connected to data/command of display
    * @param name The name used by the parent class to access the interface
    * @param LCDSIZE_X x size in pixel - optional
    * @param LCDSIZE_Y y size in pixel - optional
    */ 
    S6D04D1(proto_t displayproto, int Hz, PinName mosi, PinName miso, PinName sclk, PinName CS, PinName reset, PinName DC, const char* name , unsigned int LCDSIZE_X = 240, unsigned  int LCDSIZE_Y = 400);
    
   // unsigned int status();
  

  
protected:
    
    
    /** Init command sequence  
    */
    void init();


};
#endif