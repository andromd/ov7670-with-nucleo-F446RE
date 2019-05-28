#include "mbed.h"
#include "global.h"
#include "math.h"
#include "ov7670sreg.h"
#include "OV7670.h"
#include "FastPWM.h"
#include "ILI9341.h"

OV7670::OV7670(
    PinName siod,    // Camera I2C port data
    PinName sioc,  // Camera I2C port clock
    PinName xclk, // Pixelclock

    PortName port,
    int mask,
    /*
    XCLK
     VSYNC
     HREF
    PinName d7, // D7
    PinName d6, // D6
    PinName d5, // D5
    PinName d4, // D4
    PinName d3, // D3
    PinName d2, // D2
    PinName d1, // D1
    PinName d0, // D0
    */
    PinName rst // /RESET
):_siod(siod),_sioc(sioc),_xclk(xclk),data(port,mask),_rst(rst)
{
}



void OV7670::Reset(void)
{
    _sioc=1;
    //str=0;
    //str1=0;
    _siod.input();
    _xclk.period_us(CAMERA_CLK_PERIOD);
    _xclk.write(0.5);
    _rst=1;
    wait_us(1000);
    _rst=0;
    wait_us(1000);
    _rst=1;
    wait_us(1000);
    WriteReg(0x12,0x80) ; // RESET CAMERA
    wait_ms(300) ;
    WriteReg(0x12,0x00);
    wait_ms(300) ;
}

void OV7670::WriteReg(int subad,int data)
{
    Start();
    WriteBy(OV7670_WRITE);
    WriteBy(subad);
    WriteBy(data);
    End();
}

void OV7670::Start(void)
{
    _sioc=1;
    _siod.output();
    _siod=1;
    wait_us(OV7670_I2CFREQ);
    _siod=0;
    wait_us(OV7670_I2CFREQ);
    _sioc=0;
    wait_us(OV7670_I2CFREQ);

}

void OV7670::WriteBy(int addr)
{
    int tempo=0;
    _siod.output();
    wait_us(1);
    _sioc=0;
    _siod=1;
    for (int i=0; i<8; i++) {
        tempo=addr>>7-i;
        tempo=tempo&0x01;
        _sioc=0;
        _siod=tempo;;
        wait_us(OV7670_I2CFREQ);
        _sioc=1;
        wait_us(OV7670_I2CFREQ);
    }
    _sioc=0;
    _siod=1;
    _siod.input();
    wait_us(OV7670_I2CFREQ);
    _sioc=1;
    wait_us(OV7670_I2CFREQ);
    _sioc=0;
}

void OV7670::End(void)
{
    _sioc=0;
    _siod.output();
    _siod=0;
    wait_us(OV7670_I2CFREQ);
    _sioc=1;
    wait_us(OV7670_I2CFREQ);
    _siod=1;
    wait_us(OV7670_I2CFREQ);
    _siod.input();
    wait_us(OV7670_I2CFREQ);
}

int OV7670::ReadReg(int subad)
{
    int dr=0;
    WriteReg2(subad);
    Start();
    WriteBy(OV7670_READ);
    dr = ReadBy();
    End();
    return dr;
}

void OV7670::WriteReg2(int data)
{
    Start();
    WriteBy(OV7670_WRITE);
    WriteBy(data);
    End();
}

int OV7670::ReadBy(void)
{
    unsigned char data[8];
    int tt=0;
    _siod.input();
    _sioc=0;                     // 出力モードに戻す　SDA =Low
    for (int i=0; i<8; i++) {
        _sioc=0;
        wait_us(OV7670_I2CFREQ);
        _sioc=1;
        data[7-i]=_siod;
        wait_us(OV7670_I2CFREQ);
    }
    _sioc=0;
    for (int i=0; i<8; i++) {
        tt=tt|data[7-i];
        if (i<7) {
            tt=tt<<1;
        }
    }
    _siod.output();
    _siod=1;
    wait_us(OV7670_I2CFREQ);
    _sioc=1;
    wait_us(OV7670_I2CFREQ);
    _sioc=0;
    return tt;
}

void OV7670::CaptureNext(void)
{
    __disable_irq();    // Disable Interrupts

    bool cc=true;
    bool h,c,v;
    bool cpr=false;
    int w;
    w=data.read();
    v=w>>12;
    //   waitnegedge(v);
    while(!v) {
        w=data.read();
        v=w>>12;
    };
    while(v) {
        w=data.read();
        v=w>>12;
    };
    if (captur) {
        cpr=true;
        captur=false;
    }
    //waitposedge(v);
    while(v) {
        w=data.read();
        v=w>>12;
    };
    while(!v) {
        w=data.read();
        v=w>>12;
    };
    PixelCounter=0;
    //Time.start();
    while (v) {
        w=data.read();
        v=w>>12;
        h=w>>11&0x01;               // 先にSCLを
        c=w>>10&0x01;

        if (!c && h ) {
            cc=true;
        }
        if (cc && c && h)  {
            bank[PixelCounter]=w>>2;
            PixelCounter++;
            cc=false;
            //pc.printf("pixel counter \n");

        }
    }
    //pc.printf("inside\n");

    __enable_irq();     // Enable Interrupts
    if (cpr) {
        cpr=false;
    }
}

void OV7670::exrgbf(int j)
{
   
    int ncl = nc*2;
    int jn,jn1,d1,d2,d3;
    jn =j*ncl/2;
    jn1=(j/2)*ncl*nl/2;



    for(int k=0; k <nl/2; k++) {

        for (int i = jn; i< (j+1)*ncl/2; i=i+2) {
            d1=2*(i-jn)+ncl*2*k;
            d2=i+ncl*k+jn1;
            d3=(2*(i-jn))+ ncl*(2*k+1);
            bankf[d1]=bank[d2];
            bankf[d1+2]=bank[d2];
            bankf[d1+1]=bank[d2+1];
            bankf[d1+3]=bank[d2+1];

            bankf[d3]=bank[d2];
            bankf[d3+2]=bank[d2];
            bankf[d3+1]=bank[d2+1];
            bankf[d3+3]=bank[d2+1];

        }

    }
}





















int OV7670::Init(char *format, int n)
{

    if (ReadReg(REG_PID) != 0x76) {         // check id camera
        return 0;
    }

    Reset();                                 // Resets all registers to default values
    Reset();                                 // Resets all registers to default values

    WriteReg(REG_RGB444, 0x00);              // Disable RGB444
    WriteReg(REG_COM10, 0x22);               // 0x02   VSYNC negative (http://nasulica.homelinux.org/?p=959)
    WriteReg(REG_MVFP, 0x27);                // mirror image

    WriteReg(REG_CLKRC, 0x80);               // prescaler x1
    WriteReg(DBLV, 0x0a);                    // bypass PLL

    WriteReg(REG_COM11, 0x0A) ;
    WriteReg(REG_TSLB, 0x0D);                // 0D = UYVY  04 = YUYV
    WriteReg(REG_COM13, 0x88);               // connect to REG_TSLB


    if((strcmp("BAW", format) == 0) || (strcmp("YUV", format) == 0)|| (strcmp("RAW", format) == 0)) {   // YUV
        WriteReg(REG_COM7, 0x00);           // YUV
        WriteReg(REG_COM17, 0x00);          // color bar disable
        WriteReg(REG_COM3, 0x04);
        WriteReg(REG_COM15, 0xC0);          // Set normal rgb with Full range

    } else if((strcmp("RGB", format) == 0)) {             // RGB565
        WriteReg(REG_COM7, 0x04);           // RGB + color bar disable
        WriteReg(REG_RGB444, 0x00);         // Disable RGB444
        WriteReg(REG_COM15, 0x10);          // Set rgb565 with Full range    0xD0
        WriteReg(REG_COM3, 0x04);//0x44
        //WriteReg(REG_COM1, 0x40);
        WriteReg(REG_CLKRC, 0x80);          // prescaler x1
    }

    WriteReg(0x70, 0x3A);                   // Scaling Xsc
    WriteReg(0x71, 0x35);                   // Scaling Ysc
    WriteReg(0xA2, 0x02);                   // pixel clock delay

    if(n == 19200) {            // 160*120
        WriteReg(REG_COM14, 0x1a);          // divide by 4
        WriteReg(0x72, 0x22);               // downsample by 4
        WriteReg(0x73, 0xf2);               // divide by 4
        WriteReg(REG_HREF, 0x24);//a4
        WriteReg(REG_HSTART, 0x16);
        WriteReg(REG_HSTOP, 0x04);
        WriteReg(REG_VREF, 0x0a);//0a
        WriteReg(REG_VSTART, 0x02);
        WriteReg(REG_VSTOP, 0x7a);


        WriteReg(0x7a, 0x20);
        WriteReg(0x7b, 0x1c);
        WriteReg(0x7c, 0x28);
        WriteReg(0x7d, 0x3c);
        WriteReg(0x7e, 0x5a);
        WriteReg(0x7f, 0x68);
        WriteReg(0x80, 0x76);
        WriteReg(0x81, 0x80);
        WriteReg(0x82, 0x88);
        WriteReg(0x83, 0x8f);
        WriteReg(0x84, 0x96);
        WriteReg(0x85, 0xa3);
        WriteReg(0x86, 0xaf);
        WriteReg(0x87, 0xc4);
        WriteReg(0x88, 0xd7);
        WriteReg(0x89, 0xe8);

        WriteReg(0x13, 0xe0);
        WriteReg(0x00, 0x00);
        WriteReg(0x10, 0x00);
        WriteReg(0x0d, 0x40);
        WriteReg(0x14, 0x18);
        WriteReg(0xa5, 0x05);
        WriteReg(0xab, 0x07);
        WriteReg(0x24, 0x95);
        WriteReg(0x25, 0x33);
        WriteReg(0x26, 0xe3);
        WriteReg(0x9f, 0x78);
        WriteReg(0xa0, 0x68);
        WriteReg(0xa1, 0x03);
        WriteReg(0xa6, 0xd8);
        WriteReg(0xa7, 0xd8);
        WriteReg(0xa8, 0xf0);
        WriteReg(0xa9, 0x90);
        WriteReg(0xaa, 0x94);
        WriteReg(0x13, 0xe5);

        WriteReg(0x0e, 0x61);
        WriteReg(0x0f, 0x4b);
        WriteReg(0x16, 0x02);

        WriteReg(0x21, 0x02);
        WriteReg(0x22, 0x91);
        WriteReg(0x29, 0x07);
        WriteReg(0x33, 0x0b);
        WriteReg(0x35, 0x0b);
        WriteReg(0x37, 0x1d);
        WriteReg(0x38, 0x71);
        WriteReg(0x39, 0x2a);
        WriteReg(0x3c, 0x78);
        WriteReg(0x4d, 0x40);
        WriteReg(0x4e, 0x20);
        WriteReg(0x69, 0x00);

        WriteReg(0x74, 0x10);
        WriteReg(0x8d, 0x4f);
        WriteReg(0x8e, 0x00);
        WriteReg(0x8f, 0x00);
        WriteReg(0x90, 0x00);
        WriteReg(0x91, 0x00);
        WriteReg(0x92, 0x00);

        WriteReg(0x96, 0x00);
        WriteReg(0x9a, 0x80);
        WriteReg(0xb0, 0x84);
        WriteReg(0xb1, 0x0c);
        WriteReg(0xb2, 0x0e);
        WriteReg(0xb3, 0x82);
        WriteReg(0xb8, 0x0a);

        WriteReg(0x43, 0x0a);
        WriteReg(0x44, 0xf0);
        WriteReg(0x45, 0x34);
        WriteReg(0x46, 0x58);
        WriteReg(0x47, 0x28);
        WriteReg(0x48, 0x3a);
        WriteReg(0x59, 0x88);
        WriteReg(0x5a, 0x88);
        WriteReg(0x5b, 0x44);
        WriteReg(0x5c, 0x67);
        WriteReg(0x5d, 0x49);
        WriteReg(0x5e, 0x0e);
        WriteReg(0x64, 0x04);
        WriteReg(0x65, 0x20);
        WriteReg(0x66, 0x05);
        WriteReg(0x94, 0x04);
        WriteReg(0x95, 0x08);

        WriteReg(0x6c, 0x0a);
        WriteReg(0x6d, 0x55);
        WriteReg(0x6e, 0x11);
        WriteReg(0x6f, 0x9f);
        WriteReg(0x6a, 0x40);
        WriteReg(0x01, 0x40);
        WriteReg(0x02, 0x40);
        WriteReg(0x13, 0xe7);
        WriteReg(0x15, 0x02);

        WriteReg(0x4f, 0x80);
        WriteReg(0x50, 0x80);
        WriteReg(0x51, 0x00);
        WriteReg(0x52, 0x22);
        WriteReg(0x53, 0x5e);
        WriteReg(0x54, 0x80);
        WriteReg(0x58, 0x9e);

        WriteReg(0x41, 0x08);
        WriteReg(0x3f, 0x00);
        WriteReg(0x75, 0x05);
        WriteReg(0x76, 0xe1);
        WriteReg(0x4c, 0x00);
        WriteReg(0x77, 0x01);
        WriteReg(0x3d, 0xc1);
        WriteReg(0x4b, 0x09);
        WriteReg(0xc9, 0x60);
        WriteReg(0x41, 0x38);
        WriteReg(0x56, 0x40);

        WriteReg(0x34, 0x11);
        WriteReg(0x3b, 0x02);
        WriteReg(0xa4, 0x88);
        WriteReg(0x96, 0x00);
        WriteReg(0x97, 0x30);
        WriteReg(0x98, 0x20);
        WriteReg(0x99, 0x30);
        WriteReg(0x9a, 0x84);
        WriteReg(0x9b, 0x29);
        WriteReg(0x9c, 0x03);
        WriteReg(0x9d, 0x4c);
        WriteReg(0x9e, 0x3f);
        WriteReg(0x78, 0x04);

        WriteReg(0x79, 0x01);
        WriteReg(0xc8, 0xf0);
        WriteReg(0x79, 0x0f);
        WriteReg(0xc8, 0x00);
        WriteReg(0x79, 0x10);
        WriteReg(0xc8, 0x7e);
        WriteReg(0x79, 0x0a);
        WriteReg(0xc8, 0x80);
        WriteReg(0x79, 0x0b);
        WriteReg(0xc8, 0x01);
        WriteReg(0x79, 0x0c);
        WriteReg(0xc8, 0x0f);
        WriteReg(0x79, 0x0d);
        WriteReg(0xc8, 0x20);
        WriteReg(0x79, 0x09);
        WriteReg(0xc8, 0x80);
        WriteReg(0x79, 0x02);
        WriteReg(0xc8, 0xc0);
        WriteReg(0x79, 0x03);
        WriteReg(0xc8, 0x40);
        WriteReg(0x79, 0x05);
        WriteReg(0xc8, 0x30);
        WriteReg(0x79, 0x26);
        WriteReg(0x09, 0x03);
        WriteReg(0x3b, 0x42);

        WriteReg(0xff, 0xff);   /* END MARKER */
        WriteReg(DBLV, 0x4a); // clock multiplication by 4 4a
        //WriteReg(REG_CLKRC, 0x80);          // prescaler x1
        WriteReg(REG_CLKRC, 0x03);          // prescaler x2
    }
    if(n == 76800) {            // 320*240


    }
    if(n == 307200) {           // 640*480

    }

    return 1;
}

