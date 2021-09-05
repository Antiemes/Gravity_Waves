#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <avr/wdt.h>
#include "moon.h"

#define DEMOTEXT_LEN 250
#define NOISE_LEN 50
#define MAKERS_LEN 742
#define PLASMA_LEN 600
#define GREETS_LEN 650
#define SIN_LEN (GREETS_LEN+300)

//U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, 13, 11, 10, 8);
U8G2_SH1106_128X64_NONAME_F_HW_I2C g(U8G2_R0);
uint16_t fct=0;
uint8_t clearCalled=0;

void clear()
{
  if (!clearCalled)
  {
    g.clearBuffer();
    clearCalled=1;
  }
}

void soft_reset()
{
  wdt_enable(WDTO_15MS);  
  for(;;)
  {
  }
}

// -----------------------------
// |          FAST SIN/COS     |
// -----------------------------
int8_t sinTable[17];

int8_t fs(uint8_t a) //Full wave: 0..63, values: -127..127
{
  uint8_t n;
  a&=63;
  n=(a>=32);
  a&=31;
  a=(a<=16)?a:32-a;
  return n?-sinTable[a]:sinTable[a];
}

int8_t fc(uint8_t a)
{
  uint8_t n;
  a+=16;
  a&=63;
  n=(a>=32);
  a&=31;
  a=(a<=16)?a:32-a;
  return n?-sinTable[a]:sinTable[a];
}

// -----------------------------
// |          RAND             |
// -----------------------------
#define FASTLED_RAND16_2053  ((uint16_t)(2053))
#define FASTLED_RAND16_13849 ((uint16_t)(13849))
uint16_t rand16seed;

uint8_t random8()
{
  rand16seed = (rand16seed * FASTLED_RAND16_2053) + FASTLED_RAND16_13849;
  // return the sum of the high and low bytes, for better
  //  mixing and non-sequential correlation
  return (uint8_t)(((uint8_t)(rand16seed & 0xFF)) +
                   ((uint8_t)(rand16seed >> 8)));
}

uint16_t random16()
{
  rand16seed = (rand16seed * FASTLED_RAND16_2053) + FASTLED_RAND16_13849;
  return rand16seed;
}

// -----------------------------
// |          RECTANGLES       |
// -----------------------------
typedef struct rect
{
  uint8_t cx;
  uint8_t cy;
  uint8_t rot;
  uint8_t a;
  uint8_t dia;
};

void d4(int8_t cx, int8_t cy, int8_t sa, int8_t ca)
{
  g.drawLine(cx+ca, cy-sa, cx-sa, cy-ca);
  g.drawLine(cx-sa, cy-ca, cx-ca, cy+sa);
  g.drawLine(cx-ca, cy+sa, cx+sa, cy+ca);
  g.drawLine(cx+sa, cy+ca, cx+ca, cy-sa);
}

#define RECTS 2
rect rects[RECTS];
void recteff()
{
  uint8_t i;
  
  clear();
  g.setFont(u8g2_font_guildenstern_nbp_t_all);
  
  for (i=0; i<RECTS; i++)
  {
    int8_t sa, ca;
    sa=fs(rects[i].a)/rects[i].dia;
    ca=fc(rects[i].a)/rects[i].dia;
    d4(rects[i].cx, rects[i].cy, sa, ca);
    rects[i].a+=rects[i].rot;
    //rects[i].cy-=1;
  }

  g.drawStr(rects[0].cx-2, rects[0].cy+5, "C");
}

// -----------------------------
// |          INITIALIZATION   |
// -----------------------------
void setup(void)
{
  uint8_t i;

  //Serial.begin(9600);
  for (i=0; i<=16; i++)
  {
    sinTable[i]=sin(i/16.0*M_PI/2)*127;
  }
  g.begin();
  g.setContrast(1);
  rects[0].cx=115;
  rects[0].cy=51;
  rects[0].dia=10;
  rects[0].a=0;
  rects[0].rot=3;
  
  rects[1].cx=80;
  rects[1].cy=37;
  rects[1].dia=6;
  rects[1].a=0;
  rects[1].rot=-2;
}

// -----------------------------
// |          SINUS EFFECT     |
// -----------------------------

void sineff()
{
  static uint16_t p=0, q=0, r=0, s=0;
  static int8_t bdisp=-45;
  static int8_t tdisp=70;
  uint8_t i;
  //bdisp=-100;
  clear();
  for (i=0; i<128; i+=2)
  {
    int16_t k=(fc(i+p/16)+128)/8 + fc(i*1.8+q/16)/12 + bdisp;
    if (k<0)
    {
      k=0;
    }
    if (k<64)
    {
      g.drawVLine(i, k, 63-k);
    }
  }
  for (i=1; i<128; i+=2)
  {
    int8_t k=(fc(i*1.7+r/16)+128)/9 + fc(-i*1.1+s/16)/13 + tdisp;
    if (k>63)
    {
      k=63;
    }
    if (k>0)
    {
      g.drawVLine(i, 0, k);
    }
  }
  p-=23;
  q-=13;
  r-=30;
  s-=17;
  if (bdisp<34)
  {
    bdisp++;
  }
  if (tdisp>20)
  {
    tdisp--;
  }
}

// -----------------------------
// |          GREETINGS        |
// -----------------------------

void greeteff()
{
  static const char PROGMEM greets[]="       Greetings to LFT - Musk - Adt - adj. - TGD - Umlaut Design - UFDD - Dilemma - Desire - Rebels - Cybernetic Genetics - Singular Crew                             ";
  static char gr[2];
  static int8_t groffs=0;
  static uint16_t grstart=0, grend=0;
  clear();
  g.setFont(u8g2_font_logisoso46_tr);

  uint8_t grl=0;
  uint8_t gri=grstart;
  while (grl<150)
  {
    uint8_t wid;
    gr[0]=pgm_read_byte(&(greets[gri]));
    wid=g.drawStr(grl-groffs, 54, gr);

    if (gri==grstart)
    {
      if (groffs>=wid)
      {
        grstart++;
        groffs-=wid;
        grl-=wid; // EHH
      }
      //else
      {
        groffs+=5;
      }
    }
    grl+=wid;
    gri++;
  }
}

// -----------------------------
// |          DEMOTEXT         |
// -----------------------------

void demotext()
{
#define TW1 81
#define TW2 47
#define TW3 75
#define TX1 0
#define TY1 16

#define TX2 80
#define TY2 34

#define TX3 26
#define TY3 60

#define TH 15
#define RISE 40

  static uint8_t dtpr1=((TW1+1)/2);
  static uint8_t dtpr2=((TW2+1)/2);
  static uint8_t dtpr3=((TW3+1)/2);
  static uint8_t nothing=30;
  static uint8_t rise=0;

  clear();
  g.setDrawColor(1);
  g.setFont(u8g2_font_guildenstern_nbp_t_all);

  if (rise==RISE)
  {
    g.drawXBMP(0, 0, 64, 64, moon);
    g.drawStr(50, 40, "a  micro-");
    g.drawStr(71, 52, "controller");
    g.drawStr(95, 63, "demo");
  }
  if (!rise)
  {
    g.drawStr(TX1, TY1, "The Bad Sectors");
    g.drawStr(TX2, TY2, "Presents");
  }
  g.drawStr(TX3+rise/4, TY3-rise, "GRAVITY  WAVES");
  //g.drawBox(0, 20, 81, 15);
  
  g.setDrawColor(0);
  
  g.drawBox(TX1, TY1-TH, dtpr1, TH);
  g.drawBox(TX1+TW1-dtpr1, TY1-TH, dtpr1, TH);
  
  g.drawBox(TX2, TY2-TH, dtpr2, TH);
  g.drawBox(TX2+TW2-dtpr2, TY2-TH, dtpr2, TH);
  
  g.drawBox(TX3, TY3-TH, dtpr3, TH);
  g.drawBox(TX3+TW3-dtpr3, TY3-TH, dtpr3, TH);

  g.setDrawColor(1);
  if (dtpr1>0)
  {
    dtpr1--;
  }
  else if (dtpr2>0)
  {
    dtpr2--;
  }
  else if (dtpr3>0)
  {
    dtpr3--;
  }
  else if (nothing>0)
  {
    nothing--;
  }
  else if (rise<RISE)
  {
    rise++;
  }
}

// -----------------------------
// |          NOISE            |
// -----------------------------
void noise()
{
  uint8_t* buff=g.getBufferPtr();
  uint8_t i;
  //static uint8_t pre=20;
  //char t1[]="A MICRO-";
  //char t2[]="CONTROLLER";
  //char t3[]="DEMO";

  random16();
  for (i=0; i<47; i++)
  {
    *(buff + (random16() & 0x3ff)) |= random8();
    *(buff + ((random16() & 0x3ff)|1)) |= random8();
  }
  for (i=0; i<16; i++)
  {
    *(buff + (random16() & 0x3ff)) &= random8();
    *(buff + ((random16() & 0x3ff)|1)) &= random8();
  }
  //if (pre)
  //{
  //  pre--;
  //}
  //else
  //{
  //  g.setFont(u8g2_font_guildenstern_nbp_t_all);
  //  //g.setFont(u8g2_font_profont22_tf);
  //  //g.drawStr(30, 18, "A MICRO-");
  //  //g.drawStr(8, 37, "CONTROLLER");
  //  //g.setFont(u8g2_font_crox4hb_tr);
  //  g.setFontMode(1);
  //  g.setDrawColor(1);
  //  g.drawStr(39, 55, t3);
  //  g.drawStr(41, 55, t3);
  //  g.drawStr(40, 54, t3);
  //  g.drawStr(40, 56, t3);
  //  
  //  g.setDrawColor(0);
  //  g.drawStr(40, 55, t3);

  //  g.setFontMode(0);
  //}
}

// -----------------------------
// |          PLASMA           |
// -----------------------------
void plasma()
{
  static uint16_t psp=0;
  static uint8_t pbias=255;
  static uint8_t pbias2=0;
  static uint8_t a=0;
  int16_t x, y;
  int16_t v1, v2;
  uint16_t p4;
  uint8_t bits;
  uint16_t rnd;
  int16_t x2, x3, y2, y3;
  uint8_t* buff=g.getBufferPtr();
  rand16seed=fct;
  for (y=0; y<64; y+=8)
  {
    for (x=0; x<128; x+=4)
    {
      x2=((int16_t)fc(a)*x-(int16_t)fs(a)*(y+4))/128;
      x3=((int16_t)fc(a)*x-(int16_t)fs(a)*y)/128;
      y2=((int16_t)fs(a)*x+(int16_t)fc(a)*(y+4))/128;
      y3=((int16_t)fs(a)*x+(int16_t)fc(a)*(y))/128;
      v1=((int16_t)fs(x2*0.7+psp*0.5+fs(y2-9)/11)+fs(y2+psp+fs(x2+41)/8))*3/2;
      v2=((int16_t)fs(x3*0.7+psp*0.5+fs(y3-9)/11)+fs(y3+psp+fs(x3+41)/8))*3/2;
    
      if (v1<0)
      {
        v1=-v1;
      }
      if (v2<0)
      {
        v2=-v2;
      }
      v1+=pbias;
      v2+=pbias;
      if (v1>255)
      {
        v1=255;
      }
      v1-=pbias2;
      if (v1<0)
      {
        v1=0;
      }
      if (v2>255)
      {
        v2=255;
      }
      v2-=pbias2;
      if (v2<0)
      {
        v2=0;
      }
      for (p4=0; p4<4; p4++)
      {
        rnd=random16();
        bits=(v1<=(rnd&0xff));
        bits<<=1;
        bits|=(v1<=(rnd>>8));
        bits<<=1;
        rnd=random16();
        bits|=(v1<=(rnd&0xff));
        bits<<=1;
        bits|=(v1<=(rnd>>8));

        rnd=random16();
        bits<<=1;
        bits|=(v2<=(rnd&0xff));
        bits<<=1;
        bits|=(v2<=(rnd>>8));
        bits<<=1;
        bits|=(v2<=(rnd&0xff));
        bits<<=1;
        bits|=(v2<=(rnd>>8));

        *(buff+p4)=bits;
      }
      buff+=4;
    }
  }
  psp++;
  a++;
  if (psp<85)
  {
    pbias-=3;
  }
  else if (psp>(PLASMA_LEN-255/3) && pbias2<255)
  {
    pbias2+=3;
  }
}

// -----------------------------
// |          MAKERS           |
// -----------------------------
void makers()
{
  uint8_t* buff=g.getBufferPtr();
  uint16_t rnd;
  uint8_t x, y;
  uint8_t bits;
  int16_t q;
  static const char* titles[3]={"CODE", "MUSIC", "GRAPHICS"};
  static const char* names[3]={"Antiemes", "Blueghost", "Leon"};
  static uint8_t state=0;
  static uint8_t hide1=60, hide2=60, wait=0;
  static uint8_t member=0;
  static int8_t bias=-20;

  g.setFont(u8g2_font_guildenstern_nbp_t_all);
  //g.setFont(u8g2_font_helvB14_tr);
  random16();

  if (bias<0)
  {
    bias++;
  }
  for (y=0; y<8; y++)
  {
    for (x=0; x<128; x++)
    {
      q=(y+bias)*38-48+x;
      if (q<0)
      {
        q=0;
      }
      else if (q>255)
      {
        q=255;
      }
      rnd=random16();
      bits=(q+34<=(rnd&0xff));
      bits<<=1;
      bits|=(q+29<=(rnd>>8));
      bits<<=1;
      rnd=random16();
      bits|=(q+24<=(rnd&0xff));
      bits<<=1;
      bits|=(q+19<=(rnd>>8));

      rnd=random16();
      bits<<=1;
      bits|=(q+14<=(rnd&0xff));
      bits<<=1;
      bits|=(q+10<=(rnd>>8));
      bits<<=1;
      bits|=(q+5<=(rnd&0xff));
      bits<<=1;
      bits|=(q<=(rnd>>8));

      *(buff++)=bits;
    }
  }
  if (state==0)
  {
    hide1=g.getStrWidth(titles[member])+5;
    hide2=g.getStrWidth(names[member])+5;
    state=1;
  }
  else if (state==1)
  {
    if (hide1)
    {
      hide1--;
    }
    else
    {
      wait=10;
      state=2;
    }
  }
  else if (state==2)
  {
    if (wait)
    {
      wait--;
    }
    else
    {
      state=3;
    }
  }
  else if (state==3)
  {
    if (hide2)
    {
      hide2--;
    }
    else
    {
      wait=50;
      state=4;
    }
  }
  else if (state==4)
  {
    if (wait)
    {
      wait--;
    }
    else
    {
      state=5;
    }
  }
  if (state==5)
  {
    if (hide2<g.getStrWidth(names[member])+5)
    {
      hide2++;
    }
    else
    {
      state=6;
    }
  }
  if (state==6)
  {
    if (hide1<g.getStrWidth(titles[member])+5)
    {
      hide1++;
    }
    else if (member<2)
    {
      member++;
      state=0;
    }
    else
    {
      state=7;
    }
  }
  else if (state==7)
  {
    if (bias<63)
    {
      bias++;
    }
    //else
    //{
    //  Serial.print(fct);
    //  Serial.print("\n");
    //}
  }
  g.setDrawColor(0);
  g.drawStr(5-hide1,11,titles[member]);
  g.setDrawColor(1);
  g.drawStr(128-g.getStrWidth(names[member])-5+hide2, 60, names[member]);
}

// -----------------------------
// |          MAIN LOOP        |
// -----------------------------
void loop(void)
{
  clearCalled=0;
  //u8g2.setFont(u8g2_font_ncenB14_tr);
  //u8g2.setFont(u8g2_font_u8glib_4_tf);
  //u8g2.drawStr(xpos, 24, "Boldog");
  //u8g2.drawUTF8(-xpos, 52, "Karácsonyt!");
  //u8g2.drawLine(20, 5, 5, 32);


  if (fct<DEMOTEXT_LEN)
  {
    demotext(); // 250 frames
  }
  if (fct>=DEMOTEXT_LEN && fct<DEMOTEXT_LEN+NOISE_LEN)
  {
    noise();
  }
  if (fct>=DEMOTEXT_LEN+NOISE_LEN && fct<DEMOTEXT_LEN+MAKERS_LEN+NOISE_LEN)
  {
    makers(); // 742 frames
  }
  if (fct>=DEMOTEXT_LEN+MAKERS_LEN+NOISE_LEN && fct<DEMOTEXT_LEN+MAKERS_LEN+PLASMA_LEN+NOISE_LEN)
  {
    plasma(); // 500 frames
  }
  if (fct>DEMOTEXT_LEN+MAKERS_LEN+PLASMA_LEN+NOISE_LEN+200 && fct<DEMOTEXT_LEN+MAKERS_LEN+PLASMA_LEN+SIN_LEN+NOISE_LEN-100)
  {
    greeteff();
  }
  if (fct>=DEMOTEXT_LEN+MAKERS_LEN+PLASMA_LEN+NOISE_LEN && fct<DEMOTEXT_LEN+MAKERS_LEN+PLASMA_LEN+SIN_LEN+NOISE_LEN)
  {
    sineff();
  }
  if (fct>=DEMOTEXT_LEN+MAKERS_LEN+PLASMA_LEN+SIN_LEN+NOISE_LEN)
  {
    soft_reset();
  }
  //if (fct>2000)
  //{
  //  recteff();
  //}
  g.sendBuffer();
  fct++;
}
