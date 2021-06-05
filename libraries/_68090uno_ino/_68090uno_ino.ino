
#define LCD_RD   A0
#define LCD_WR   A1     
#define LCD_RS   A2        
#define LCD_CS   A3       
#define LCD_REST A4

void Lcd_Writ_Bus(unsigned char VH)
{
  unsigned int i,temp,data; 
  data=VH;
  for(i=8;i<=9;i++)
  {
    temp=(data&0x01);
    if(temp)
      digitalWrite(i,HIGH);
    else
      digitalWrite(i,LOW);
    data=data>>1;
  }	
  for(i=2;i<=7;i++)
  {
    temp=(data&0x01);
    if(temp)
      digitalWrite(i,HIGH);
    else
      digitalWrite(i,LOW);
    data=data>>1;
  }	 

  digitalWrite(LCD_WR,LOW);
  digitalWrite(LCD_WR,HIGH);
}


void Lcd_Write_Com(unsigned char VH)  
{   
  digitalWrite(LCD_RS,LOW);
  Lcd_Writ_Bus(VH);
}

void Lcd_Write_Data(unsigned char VH)
{
  digitalWrite(LCD_RS,HIGH);
  Lcd_Writ_Bus(VH);
}

void Lcd_Write_Com_Data(unsigned char com,unsigned char dat)
{
  Lcd_Write_Com(com);
  Lcd_Write_Data(dat);
}

void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
{
	Lcd_Write_Com_Data(0x2a,x1>>8);
	Lcd_Write_Com_Data(0x2a,x1);
	Lcd_Write_Com_Data(0x2a,x2>>8);
	Lcd_Write_Com_Data(0x2a,x2);
	Lcd_Write_Com_Data(0x2b,y1>>8);
	Lcd_Write_Com_Data(0x2b,y1);
	Lcd_Write_Com_Data(0x2b,y2>>8);
	Lcd_Write_Com_Data(0x2b,y2);
	Lcd_Write_Com(0x2c); 							 
}

void Lcd_Init(void)
{
  digitalWrite(LCD_REST,HIGH);
  delay(5); 
  digitalWrite(LCD_REST,LOW);
  delay(15);
  digitalWrite(LCD_REST,HIGH);
  delay(15);

  digitalWrite(LCD_CS,HIGH);
  digitalWrite(LCD_WR,HIGH);
  digitalWrite(LCD_CS,LOW);  //CS

   Lcd_Write_Com_Data( 0x00e5,0x8000);
   Lcd_Write_Com_Data( 0x0000,0x0001);

   Lcd_Write_Com_Data( 0x0001,0x0100);
   Lcd_Write_Com_Data( 0x0002,0x0700);
   Lcd_Write_Com_Data( 0x0003,0x1030);
   Lcd_Write_Com_Data( 0x0004,0x0000);
   Lcd_Write_Com_Data( 0x0008,0x0202);
   Lcd_Write_Com_Data( 0x0009,0x0000);
   Lcd_Write_Com_Data( 0x000a,0x0000);  
   Lcd_Write_Com_Data( 0x000c,0x0000);
   Lcd_Write_Com_Data( 0x000d,0x0000);
   Lcd_Write_Com_Data( 0x000f,0x0000);
//*********************************************Power On
   Lcd_Write_Com_Data( 0x0010,0x0000);
   Lcd_Write_Com_Data( 0x0011,0x0000);
    Lcd_Write_Com_Data(0x0012,0x0000);
    Lcd_Write_Com_Data(0x0013,0x0000);

   Lcd_Write_Com_Data( 0x0010,0x17b0);
   Lcd_Write_Com_Data( 0x0011,0x0037);

    Lcd_Write_Com_Data(0x0012,0x0138);

   Lcd_Write_Com_Data( 0x0013,0x1700);
  Lcd_Write_Com_Data(  0x0029,0x000d);

    Lcd_Write_Com_Data(0x0020,0x0000);
   Lcd_Write_Com_Data( 0x0021,0x0000);
//********************************************Set gamma
    Lcd_Write_Com_Data(0x0030,0x0001);
   Lcd_Write_Com_Data( 0x0031,0x0606);
   Lcd_Write_Com_Data( 0x0032,0x0304);
   Lcd_Write_Com_Data( 0x0033,0x0202);
    Lcd_Write_Com_Data(0x0034,0x0202);
   Lcd_Write_Com_Data( 0x0035,0x0103);
   Lcd_Write_Com_Data( 0x0036,0x011d);
   Lcd_Write_Com_Data( 0x0037,0x0404);
   Lcd_Write_Com_Data( 0x0038,0x0404);
   Lcd_Write_Com_Data( 0x0039,0x0404);
   Lcd_Write_Com_Data( 0x003c,0x0700);
    Lcd_Write_Com_Data(0x003d,0x0a1f);
//**********************************************Set Gram aera
    Lcd_Write_Com_Data(0x0050,0x0000);
   Lcd_Write_Com_Data( 0x0051,0x00ef);
   Lcd_Write_Com_Data( 0x0052,0x0000);
   Lcd_Write_Com_Data( 0x0053,0x013f);
   Lcd_Write_Com_Data( 0x0060,0x2700);
   Lcd_Write_Com_Data( 0x0061,0x0001);
  Lcd_Write_Com_Data(  0x006a,0x0000);
//*********************************************Paratial display
   Lcd_Write_Com_Data( 0x0090,0x0010);
   Lcd_Write_Com_Data( 0x0092,0x0000);
   Lcd_Write_Com_Data( 0x0093,0x0003);
   Lcd_Write_Com_Data( 0x0095,0x0101);
   Lcd_Write_Com_Data( 0x0097,0x0000);
   Lcd_Write_Com_Data( 0x0098,0x0000);
//******************************************** Plan Control
   Lcd_Write_Com_Data (0x0007,0x0021);

   Lcd_Write_Com_Data (0x0007,0x0031);
   Lcd_Write_Com_Data (0x0007,0x0173); 
}

void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c)                   
{	
  unsigned int i,j;
  Lcd_Write_Com(0x02c); //write_memory_start
  digitalWrite(LCD_RS,HIGH);
  digitalWrite(LCD_CS,LOW);
  l=l+x;
  Address_set(x,y,l,y);
  j=l*2;
  for(i=1;i<=j;i++)
  {
    Lcd_Write_Data(c);
  }
  digitalWrite(LCD_CS,HIGH);   
}

void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c)                   
{	
  unsigned int i,j;
  Lcd_Write_Com(0x02c); //write_memory_start
  digitalWrite(LCD_RS,HIGH);
  digitalWrite(LCD_CS,LOW);
  l=l+y;
  Address_set(x,y,x,l);
  j=l*2;
  for(i=1;i<=j;i++)
  { 
    Lcd_Write_Data(c);
  }
  digitalWrite(LCD_CS,HIGH);   
}

void Rect(unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int c)
{
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}

void Rectf(unsigned int x,unsigned int y,unsigned int w,unsigned int h,unsigned int c)
{
  unsigned int i;
  for(i=0;i<h;i++)
  {
    H_line(x  , y  , w, c);
    H_line(x  , y+i, w, c);
  }
}
int RGB(int r,int g,int b)
{return r << 16 | g << 8 | b;
}
void LCD_Clear(unsigned int j)                   
{	
  unsigned int i,m;
 Address_set(0,0,320,240);
  Lcd_Write_Com(0x02c); //write_memory_start
  digitalWrite(LCD_RS,HIGH);
  digitalWrite(LCD_CS,LOW);


  for(i=0;i<320;i++)
    for(m=0;m<240;m++)
    {
      Lcd_Write_Data(j>>8);
      Lcd_Write_Data(j);

    }
  digitalWrite(LCD_CS,HIGH);   
}

void setup()
{
  for(int p=2;p<10;p++)
  {
    pinMode(p,OUTPUT);
  }
  pinMode(A0,OUTPUT);
  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  pinMode(A3,OUTPUT);
  pinMode(A4,OUTPUT);
  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  digitalWrite(A4, HIGH);
  Lcd_Init();
 LCD_Clear(0xf800);
}

void loop()
{  

  for(int i=0;i<1000;i++)
  {
    Rect(random(300),random(300),random(300),random(300),random(65535)); // rectangle at x, y, with, hight, color
  }
  
//  LCD_Clear(0xf800);
}
