#include "mbed.h"
#include "global.h"
#include "FastPWM.h"

#include "OV7670.h"
#include "ov7670sreg.h"

#include "ILI9341.h"
#include "Arial12x12.h"
#include "Arial24x23.h"
#include "Arial28x28.h"

#define CSTFT     PB_0
#define RESETTFT  PC_1
#define RSTFT     PA_4
#define WRTFT     PA_1
#define RDTFT     PA_0


#define PCLK    PC_10 //camera Pixel clock 
#define HREF    PC_11 //camera Href
#define VSYNC   PC_12 // camera Vsync
#define I2C_D   PB_3 // Camera SCCB port data
#define I2C_CLK PB_10 // Camera SCCB port clock
#define RESET   PB_12 // camera reset
#define XCLK    PA_10 // camera system clock


#define mD0     PC_2//camera Data
#define mD1     PC_3//camera Data
#define mD2     PC_4//camera Data
#define mD3     PC_5//camera Data
#define mD4     PC_6//camera Data
#define mD5     PC_7//camera Data
#define mD6     PC_8 // camera Data
#define mD7     PC_9 //camera Data


#define SCSD    PB_13   // clk SD
#define MISD    PB_14   // miso SD
#define MOSD    PB_15   // mosi SD


// to check his use
#define PD2 PD_2 /: CS SD

#define QQVGA   19200          //160*120



/*------------------------------------------------------------------------------
-----                      GLOBAL INSTANCES
-----                     * pull-up in ICD *
------------------------------------------------------------------------------*/
DigitalOut myled(LED1);

// communication serial
Serial pc(USBTX, USBRX);

PinName buspins[8] = {D11, D10, D8, D7, D5, D4, PB_2, PA_12};
ILI9341 TFT(BUS_8, buspins, PB_0, PC_1, PA_4, PA_1, PA_0, "TFT", 240, 320); 

// camera 
int mask =0x1FFC;
OV7670 _OV7670(I2C_D,I2C_CLK,XCLK,PortC,mask,RESET);



/*------------------------------------------------------------------------------
---                          GLOBAL VARIABLES
---
------------------------------------------------------------------------------*/
Timer Time;


char desfile[25];
char patfile[25];
char filename[25];
const int tmarray = nc*2*nl;
unsigned char bank[tmarray];
unsigned char bankt[nl][nc];
unsigned char bankta[nlta][ncta][nv];
unsigned char banktatc[nlta][ncta];
unsigned char bankf[tmarray];
unsigned char banktc[nl][nc];

float meas0,measold0,meas1,measold1,meas2,measold2, meas3, measold3;

int volatile statc=0;
bool volatile captur =false;

 
void capturecycle(const char des[])
{
    strcpy( desfile,des);
    if (statc==0||statc==1) {
        _OV7670.CaptureNext() ;
        _OV7670.exrgbf(0);
        TFT.Bitmap(0,120,160,120,bankf);
        _OV7670.exrgbf(1);
        TFT.Bitmap(160,120,160,120,bankf);
        _OV7670.exrgbf(2);
        TFT.Bitmap(0,0,160,120,bankf);
        _OV7670.exrgbf(3);
        TFT.Bitmap(160,0,160,120,bankf);
    }
    
} 
 
 
int main() {
    myled = 0;
    statc = 0;
    
    // prepare the TFT
    TFT.claim(stdout);
    TFT.background(Black);                      // Set background to black
    TFT.foreground(White);                      // Set chars to white
    TFT.cls();                                  // Clear the screen
    TFT.set_font((unsigned char*) Arial12x12);  // Select the font
    TFT.set_orientation(1);                     // Select orientation
    TFT.locate(0,0);
    
    _OV7670.Reset();
    _OV7670.Init("RGB", QQVGA);
    pc.printf("Hello World !\r\n");
    
    
    while (1) {
        capturecycle("picture_test");
    }
        
}
