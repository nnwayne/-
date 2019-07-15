#include <reg52.h>
#include <stdio.h>
#include <absacc.h>
#include <intrins.h>

#define SEGMENT  XBYTE[0xdfff]			//段码寄存器地址
#define BIT_LED  XBYTE[0xbfff]			//位码寄存器地
#define fosc	11.0592	
#define time0	3500
#define time1	3000

unsigned char data display_bit,display_buffer[16];
unsigned char data time0_h,time0_l,time1_h,time1_l;
unsigned int idata time0_times,time1_times;

unsigned int high_T=30;						   //温度报警上限
unsigned int low_T=15;						   //温度报警下限
unsigned char alarm_hour=18;				   //初始闹钟小时
unsigned char alarm_minute=0;				   //初始闹钟分钟

unsigned char min,hou;
unsigned int wendu,liangdu,j,flag,k,n;

unsigned char get_code(unsigned char i);
void int0_int(void);
void display(void);
void delay(int d);
void shine(void);

void xunhuan(void);  
void zoushi(void);						//正常走时
void tiaoshi(void);						//调时函数
void dingnao(void);						//订闹函数

void read_time();						//读取当前时间
void read_time1();						//读取当前日期
void set_time();						//置入时间函数

void baoshi(void);						//报时
void baowen(void);						//报温

//ds1302变量及函数定义
sbit SCL_DS1302 =P2^0;
sbit IO_DS1302  =P2^1;
sbit RST_DS1302 =P2^2;
sbit s1=P1^0;
sbit s2=P1^1;
sbit s3=P1^2;
sbit s4=P1^3;
sbit s5=P3^2;
unsigned int tes,teg;

void dmsec (unsigned int count);
void tmreset (void);
void tmstart (void);
void tmrtemp (void);


// ISD25120变量及函数定义
unsigned char bdata data_ds1302;
sbit TMDAT=P3^4;
sbit bit_data0=data_ds1302^0;
sbit bit_data7=data_ds1302^7;

unsigned char bdata x;
sbit x0=x^0;
sbit x7=x^7;

void keyscan(void);	
void initial_ds1302();
unsigned char read_ds1302(char command);
void open_write_bit();
void close_write_bit();



//ISD1760变量及函数定义
unsigned char bdata SR0_L;
unsigned char bdata SR0_H;
unsigned char bdata SR1;
unsigned char APCL=0,APCH=0;
unsigned char PlayAddL=0,PlayAddH=0;
unsigned char RecAddL=0,RecAddH=0;

sbit CMD=SR0_L^0;
sbit FULL=SR0_L^1;
sbit PU=SR0_L^2;
sbit EOM=SR0_L^3;
sbit INTT=SR0_L^4;
sbit RDY=SR1^0;
sbit ERASE=SR1^1;
sbit PLAY=SR1^2;
sbit REC=SR1^3;

unsigned char ISD_SendData(unsigned char dat);
void ISD_PU(void);
void ISD_Rd_Status(void);
void ISD_WR_APC2(unsigned char apcdatl,apcdath);
void ISD_SET_PLAY(unsigned char Saddl,Saddh,Eaddl,Eaddh);

sbit SS=P1^4;
sbit SCK=P1^7;
sbit MOSI=P1^5;
sbit MISO=P1^6;

void Cpu_Init(void);
void ISD_Init(void);
void delayms(unsigned int t);


void main()
{
	s1=1;
	j=0;
	k=0;
	liangdu=1;
	wendu=28;
 	BIT_LED=0;
	TMOD=0x11;							//定时器0,1工作于方式1
							
	time0_times=-time0*fosc/12;
	time0_h=(time0_times/256);
	time0_l=(time0_times%256);

	time1_times=-time1*fosc/12;
	time1_h=(time1_times/256);
	time1_l=(time1_times%256);


	TR0=EA=ET0=1;						//启动定时器0
	TH0=time0_h;						//高8位和低8位时间常数
	TL0=time0_l;			
	EX0=1;									//启动外部中断0
	IT0=1;									//外部中断0下降沿触发

	TR1=ET1=1;							//启动定时器1
	TH1=time1_h;						//高8位和低8位时间常数
	TL1=time1_l;

	initial_ds1302();					//上电启用ds1302
	display_bit=0x01;					//从第一个数码管开始显示
	display_buffer[0]=0X01;
	display_buffer[1]=0X08;
	display_buffer[2]=0X05;
	display_buffer[3]=0X09;
	display_buffer[4]=0X05;
	display_buffer[5]=0X00;				//将18时59分50秒设为当前时间
	display_buffer[6]=0x00;				
	display_buffer[7]=0x00;				//预留数字
	display_buffer[8]=0x01;
	display_buffer[9]=0x06;
	display_buffer[10]=0x00;
	display_buffer[11]=0x05;
	display_buffer[12]=0x02;
	display_buffer[13]=0x04;			//将16年05月24日设为当前日期
	set_time();							//将数组中的时间置入DS1302
	xunhuan();
	
}


/*********************循环程序************************/

void xunhuan(void)
{
	while(1)
	{
	   switch(flag)
	   {
		  case 0: zoushi();	break;
		  case 1: tiaoshi();	break;
		  case 2: dingnao();	break;
		  default:  break;
		}
	 }
}

/******************语音判断与年月日切换模块************************/

void zoushi(void)						//走时，判断是否切换年月日显示，语音功能
{
	if(s2==0)									//如果按下S2，语音报时
	{
		baoshi();
		TMOD=0x11;
		TR0=TR1=EA=1;						//报时后重新启用定时器0,1
		TH0=time0_h;
		TL0=time0_l;
		TH1=time1_h;
		TL1=time1_l;}
		
	if(s3==0)									//如果按下S3，语音报温
	{
		baowen();
		TMOD=0x11;
		TR0=TR1=EA=1;						//报温后重新启用定时器0,1
		TH0=time0_h;
		TL0=time0_l;
		TH1=time1_h;
		TL1=time1_l;}
		
	if(s4==0)
	{
		read_time1();					//如果s4键按下，读取年月日温度
  	}
	else 
	{
		read_time();				   	//如果s4末按下，读取时分秒星期
	}									
}


/*********************调时、订闹模块************************/

void tiaoshi(void)						//调时模块
{
	if(s3==0)							//按S3调整分钟
	{
		while(s3==0);
		if(display_buffer[3]==9)
		{
			display_buffer[3]=0;
			if(display_buffer[2]==5)
			{
				display_buffer[2]=0;}
			else
			{
				display_buffer[2]++;}
		}
		else
		{
			display_buffer[3]++;}
		delay(50);						//防止抖动
	}
									   	
	if(s2==0)							//按S2调整时钟
	{
		while(s2==0);
		if(display_buffer[1]==9)
		{			
			display_buffer[1]=0;
			display_buffer[0]++;
		}
		else if(display_buffer[1]==3&&display_buffer[0]==2)
		{
			display_buffer[1]=0;
			display_buffer[0]=0;
		}
		else
		{
			display_buffer[1]++;}
		delay(50);						//防止抖动
	}
	set_time();		  			
}

void dingnao(void)						//订闹模块
{
	display_buffer[0]=alarm_hour/10;
	display_buffer[1]=alarm_hour%10;
	display_buffer[2]=alarm_minute/10;
	display_buffer[3]=alarm_minute%10;
	display_buffer[4]=16;
	display_buffer[5]=16;
	display_buffer[6]=16;
	display_buffer[7]=16;

	if(s3==0)							//按S3调整分钟
	{
		while(s3==0);	
		alarm_minute++;
		if(alarm_minute>=59)
		{
			alarm_minute=0;
			display_buffer[3]=alarm_minute%10;
			display_buffer[2]=alarm_minute/10;
		}
		delay(50);						//防止抖动
	}
	if(s2==0)							//按S2调整时钟
	{
		while(s2==0);
		alarm_hour++;
		if(alarm_hour>=23)
		{
			alarm_hour=0;
			display_buffer[1]=alarm_hour%10;
			display_buffer[0]=alarm_hour/10;
		}
		delay(50);						//防止抖动
	}		
}



/*****************************中断程序*********************************/

void int0_int(void) interrupt 0
{
	 flag++;
	 EX0=0;							//防止按键抖动，先关闭外部中断0
	 if(flag==3)
	 {
		flag=0;
		read_time();				//将回到时间界面，读取当前时间值
	}
}


void int1_int(void) interrupt 1		//读取温度并判断当前时间应显示的亮度 
{		
	j++;
	if(j>=1000)
	{
		tmstart();
		tmrtemp();					//间隔一段长时间后读取当前温度
		j=0;
	}
	TH0=time0_h;
	TL0=time0_l;
									
	if(EX0==0)					   	//若外部中断关闭，一段时间后，再启用外部中断0
	{												//防止抖动
		n++;
		if(n>=600)
		{
			EX0=1;
			n=0;}
	}
		

	if(hou>=19||hou<7)				//判断当前时间应显示的亮度模式
	{
		liangdu=0;
		display();
	}
	else
	{
		liangdu=1;
		display();}

}

/*********************ＬＥＤ闪烁模块************************/

void int3_int(void) interrupt 3
{
	TH1=time1_h;
	TL1=time1_l;
	k++;
	if(k>100)						  //控制LED灯闪烁
	{
		if(s1==0)
		{
			s1=1;}
	}
	if(k>=200)						  //判断LED是否该闪烁
	{	
		shine();
		k=0;
	}
}


void shine(void)
{
	unsigned int stemp,stime;
	if((wendu>=high_T)||(wendu<=low_T))			 //温度是否到达报警线
	{
		stemp=0;  
	}
	else
	{
		stemp=1;
	}
	if((alarm_minute==min)&&(alarm_hour==hou))	 //时间是否到达闹钟时间
	{
		stime=0;
	}
	else
	{
		stime=1;
	}
	if((stemp==0)||(stime==0))
	{
		s1=0;}
	else
	{
		s1=1;}
}	  

/*******************时间星期与日历温度读取置数模块***********************/

void read_time()					//读时分秒星期
{
	unsigned char second,minute,hour,d;
	
	display_buffer[6]=17;
	display_buffer[7]=2;	
									
	second=0;						//read second address
	d=read_ds1302(second);
	display_buffer[5]=d&0x0f;
	display_buffer[4]=d>>4;
									
	minute=1;						//read minute address
	d=read_ds1302(minute);
	display_buffer[3]=d&0x0f;
	display_buffer[2]=(d>>4);
	min=display_buffer[2]*10+display_buffer[3];
									
	hour=2;						   //read hour address
	d=read_ds1302(hour);
	display_buffer[1]=d&0x0f;
	display_buffer[0]=(d>>4);
	hou=display_buffer[0]*10+display_buffer[1];	
			
}

void read_time1()							//读年月日温度
{
	unsigned char date,month,year,d;
	tes=wendu/0x0a;							//置入温度
	teg=wendu%0x0a;
	display_buffer[6]=tes;
	display_buffer[7]=teg;

	date=3;									//read date address
	d=read_ds1302(date);
	display_buffer[5]=d&0x0f;
	display_buffer[4]=d>>4;

	month=4;								//read month address
	d=read_ds1302(month);
	display_buffer[3]=d&0x0f;
	display_buffer[2]=(d>>4);

	year=6;									//read year address
	d=read_ds1302(year);
	display_buffer[1]=d&0x0f;
	display_buffer[0]=(d>>4);	
}



/************************延时模块****************************/

void delay(int j)
 {
  while(j>=0){j--;}
 }      

/************************显示模块****************************/	
unsigned char get_code(unsigned char i)
{
	unsigned char p;	
	switch (i)
	{	    
		case  0:    p=0x3F;	break;	/*0*/
        case  1:    p=0x06;	break;	/*1*/
        case  2:    p=0x5B;	break;	/*2*/ 
        case  3:    p=0x4F;	break;	/*3*/
        case  4:    p=0x66;	break;	/*4*/
        case  5:    p=0x6D;	break;	/*5*/
        case  6:    p=0x7D;	break;	/*6*/
        case  7:    p=0x07;	break;	/*7*/
        case  8:    p=0x7F;	break;	/*8*/
        case  9:    p=0x67;	break;	/*9*/
        case 10:    p=0x77;	break;	/*A*/
        case 11:    p=0x7C;	break;	/*B*/
        case 12:    p=0x39;	break;	/*C*/
        case 13:    p=0x5E;	break;	/*D*/
        case 14:    p=0x79;	break;	/*E*/
        case 15:    p=0x71;	break;	/*F*/
		case 16:    p=0x00; break;  /*mie*/
		case 17:    p=0x40;	break;	/*-*/	
		default:            break;
	}	
	return (p);
}

void display(void)
{
	unsigned char i;
	switch (display_bit)
	{
		case   1: i=0;break;
		case   2: i=1;break;
		case   4: i=2;break;
		case   8: i=3;break;
		case  16: i=4;break;
		case  32: i=5;break;
		case  64: i=6;break;
		case 128: i=7;break;
		default :	  break;	
	}
	if((i+1)%2==0)				//在时、分、秒、年、月、日的个位加小数点以区分
	{
		SEGMENT=get_code(display_buffer[i])|0x80;		//送段码，加小数点
		BIT_LED=display_bit;   							//送位码
	}
	else
	{
		SEGMENT=get_code(display_buffer[i]);
		BIT_LED=display_bit;
	}

	if(liangdu==1)				//调整亮度
	{
		delay(150);
	   	BIT_LED=0;}
	else
	{
		delay(50);
		BIT_LED=0;}

	if (display_bit<=64)
	{
		display_bit=display_bit*2;
	}
	else
	{
		display_bit=0x01;
	}
}	

/**************************时钟ds1302基本驱动程序************************/
void close_write_bit()
{
	char i;
	SCL_DS1302=0;
	_nop_();
	RST_DS1302=1;
	_nop_();_nop_();
	data_ds1302=0x8e;			//write control redister
	for (i=1;i<=8;i++)
	{
		SCL_DS1302=0;IO_DS1302=bit_data0;
		_nop_();SCL_DS1302=1;
		data_ds1302=data_ds1302>>1;	
	}
	data_ds1302=0x80;			//close write protect bit
	IO_DS1302=0;
	for (i=1;i<=8;i++)	
	{
		SCL_DS1302=0;IO_DS1302=bit_data0;
		_nop_();SCL_DS1302=1;
		data_ds1302=data_ds1302>>1;	
	}
}


void open_write_bit()
{
	char i;
	SCL_DS1302=0;
	_nop_();
	RST_DS1302=1;
	_nop_();_nop_();
	data_ds1302=0x8e;			//write control redister
	for (i=1;i<=8;i++)	
	{
		SCL_DS1302=0;IO_DS1302=bit_data0;
		_nop_();SCL_DS1302=1;
		data_ds1302=data_ds1302>>1;	
	}
	data_ds1302=0x00;			//open write protect bit
	IO_DS1302=0;
	for (i=1;i<=8;i++)
	{
		SCL_DS1302=0;IO_DS1302=bit_data0;
		_nop_();SCL_DS1302=1;
		data_ds1302=data_ds1302>>1;	
	}
}

void initial_ds1302()
{
	unsigned char i;
	SCL_DS1302=0;
	_nop_();
	RST_DS1302=1;
	_nop_();_nop_();
	data_ds1302=0x8e;					//write control register
	for (i=1;i<=8;i++)
	{
		SCL_DS1302=0;IO_DS1302=bit_data0;
		_nop_();SCL_DS1302=1;
		data_ds1302=data_ds1302>>1;	
	}
	data_ds1302=0x00;//0x80;			//close write protect bit
	IO_DS1302=0;
	for (i=1;i<=8;i++)	
	{
		SCL_DS1302=0;IO_DS1302=bit_data0;_nop_();
		SCL_DS1302=1;data_ds1302=data_ds1302>>1;	
	}
	RST_DS1302=0;
	_nop_();
	SCL_DS1302=0;
	SCL_DS1302=0;
	_nop_();
	RST_DS1302=1;
	_nop_();_nop_();
	data_ds1302=0x90;					//recharge register 
	for (i=1;i<=8;i++)	
	{
		SCL_DS1302=0;IO_DS1302=bit_data0;_nop_();
		SCL_DS1302=1;data_ds1302=data_ds1302>>1;	
	}
	data_ds1302=0xa4;					//no rechaarge for battery
	for (i=1;i<=8;i++)	
	{
		SCL_DS1302=0;IO_DS1302=bit_data0;
		_nop_();SCL_DS1302=1;
		data_ds1302=data_ds1302>>1;	
	}
	RST_DS1302=0;
	_nop_();
	SCL_DS1302=0;
	SCL_DS1302=0;
	_nop_();
	RST_DS1302=1;
	_nop_();_nop_();
	data_ds1302=0x8e;
	for (i=1;i<=8;i++)	
	{
		SCL_DS1302=0;IO_DS1302=bit_data0;
		_nop_();SCL_DS1302=1;
		data_ds1302=data_ds1302>>1;	
	}
	data_ds1302=0x80;
	for (i=1;i<=8;i++)	
	{
		SCL_DS1302=0;IO_DS1302=bit_data0;
		_nop_();SCL_DS1302=1;
		data_ds1302=data_ds1302>>1;	
	}
	RST_DS1302=0;
	_nop_();
	SCL_DS1302=0;
}
unsigned char read_ds1302(char command)
{
	char i;
	data_ds1302=(command<<1)|0x81;
	SCL_DS1302=0;
	_nop_();
	RST_DS1302=1;
	for (i=1;i<=8;i++)	
	{
		SCL_DS1302=0;IO_DS1302=bit_data0;
		_nop_();SCL_DS1302=1;
		data_ds1302=data_ds1302>>1;	
	}
	SCL_DS1302=1;
	for (i=1;i<=8;i++)	
	{
		data_ds1302=data_ds1302>>1;
		SCL_DS1302=0;_nop_();
		bit_data7=IO_DS1302;SCL_DS1302=1;
	}
	RST_DS1302=0;
	_nop_();
	SCL_DS1302=0;
	return(data_ds1302);
}

void write_ds1302(unsigned char address,unsigned char numb)	//写入时分秒年月日星期
{
	char i;
	RST_DS1302=0;
	SCL_DS1302=0;
	RST_DS1302=0;
	RST_DS1302=1;
	data_ds1302=0x80|(address<<1);
	for (i=1;i<=8;i++)	
	{
		SCL_DS1302=0;IO_DS1302=bit_data0;
		_nop_();SCL_DS1302=1;
		data_ds1302=data_ds1302>>1;	
	}
	data_ds1302=numb;
	for (i=1;i<=8;i++)	
	{
		SCL_DS1302=0;IO_DS1302=bit_data0;
		_nop_();SCL_DS1302=1;
		data_ds1302=data_ds1302>>1;	
	}
	RST_DS1302=0;
	SCL_DS1302=1;
}


void set_time(void)
{
	unsigned char data temp;
	unsigned char data hour_address,minute_address,second_address,date_address,month_address,year_address;
	year_address=6;month_address=4;date_address=3;hour_address=2;minute_address=1;second_address=0;
	open_write_bit();
	temp=(display_buffer[8]<<4)|display_buffer[9];
	write_ds1302(year_address,temp);
	temp=(display_buffer[10]<<4)|display_buffer[11];
	write_ds1302(month_address,temp);
	temp=(display_buffer[12]<<4)|display_buffer[13];
	write_ds1302(date_address,temp);
	temp=(display_buffer[0]<<4)|display_buffer[1];
	write_ds1302(hour_address,temp);
	temp=(display_buffer[2]<<4)|display_buffer[3];
	write_ds1302(minute_address,temp);
	temp=(display_buffer[4]<<4)|display_buffer[5];
	write_ds1302(second_address,temp);
	close_write_bit();
}

/*************************温度ds18b20驱动程序************************/

void dmsec (unsigned int count) 	   	// mSec Delay 11.0592 Mhz
{      	
    unsigned int i;		       	// 1MS 延时
    while (count--) {
    for (i=0;i<125;i++){}
        }
   }

void tmreset (void) 		// Reset TX
{                  	
    unsigned int i;
    TMDAT = 0;
    i = 103; while (i>0) i--;          	// Approx 900 uS
    TMDAT = 1;
    i = 4; while (i>0) i--;
}

void tmpre (void) 			// Wait for Presence RX
{                    
    unsigned int i;
    while (TMDAT);
    while (~TMDAT);
    i = 4; while (i>0) i--;
}

bit tmrbit (void) 	 		// read one bit
{                   
    unsigned int i;
    bit dat;
    TMDAT = 0; i++;
    TMDAT = 1; i++; i++;
    dat = TMDAT;
    i = 8; while (i>0) i--;            
    return (dat);
}

unsigned char tmrbyte (void) 	 // read one byte
{         
    unsigned char i,j,dat;
    dat = 0;
    for (i=1;i<=8;i++) 
	{
        j = tmrbit ();
        dat = (j << 7) | (dat >> 1);
    }
    return (dat);
}

void tmwbyte (unsigned char dat) 	 // write one byte
{      
    unsigned int i;
    unsigned char j;
    bit testb;
    for (j=1;j<=8;j++) 
	{
        testb = dat & 0x01;
        dat = dat >> 1;
        if (testb) 				   // Write 1
		{						   
            TMDAT = 0;             
            i++; i++; 
            TMDAT = 1;
            i = 8; while (i>0) i--; 
        }
        else 						// Write 0
		{							
            TMDAT = 0;              
            i = 8; while (i>0) i--;             
            TMDAT = 1;
            i++; i++;                          
        }
    }
}

void tmstart (void) 	// ds1820 start convert
{          	        
    tmreset ();
    tmpre ();
    dmsec (1);
    tmwbyte (0xcc);       // skip rom
    tmwbyte (0x44);       //转换命令
}

void tmrtemp (void) 	  // read temp
{   
    unsigned char a,b;    
    tmreset ();
    tmpre ();
    dmsec (1);
    tmwbyte (0xcc);                    // skip rom
    tmwbyte (0xbe);                    //读存储器命令
    a = tmrbyte ();                   	 // LSB
    b = tmrbyte ();                   	 // MSB
		wendu=a+b*256;
    wendu=wendu/16;
		tes=wendu/0x0a;	
		teg=wendu%0x0a;
}


/**********************语音isd1760驱动程序*************************/
void Cpu_Init(void)
{
  P0=P1=P2=P3=0XFF;
  TMOD=0X01;
  EA=0;
}

void ISD_Init(void)
{
  unsigned char i=2;
  SS=1;
  SCK=1;
  MOSI=0;
 do
 {
   ISD_PU();
   delayms(50);
   ISD_Rd_Status();
 }
while(CMD||(!PU));

  ISD_WR_APC2(0x40,0x04);

 do
 {
   ISD_Rd_Status();
 }while(RDY==0);
 do
 {
   delayms(300);
   delayms(300);
   i--;
 }while(i>0);
}


unsigned char ISD_SendData(unsigned char dat)
{
  unsigned char i,j,BUF_ISD=dat;
  SCK=1;
  SS=0;
  for(j=4;j>0;j--)
   {;}
  for(i=0;i<8;i++)
  {
   SCK=0;
   for(j=2;j>0;j--)
    {;}
   if(BUF_ISD&0x01)
     MOSI=1;
   else
     MOSI=0;
     BUF_ISD>>=1;
   if(MISO)
    BUF_ISD|=0x80;
    SCK=1;
   for(j=6;j>0;j--)
   {;}
  }
  MOSI=0;
  return(BUF_ISD);
}


void ISD_PU(void)
{
ISD_SendData(0x01);
ISD_SendData(0x00);
SS=1;
}

void ISD_Rd_Status(void)
{
  unsigned char i;
  ISD_SendData(0x05);
  ISD_SendData(0x00);
  ISD_SendData(0x00);
  SS=1;
  for(i=2;i>0;i--){;}
  SR0_L=ISD_SendData(0x05);
  SR0_H=ISD_SendData(0x00);
  SR1=ISD_SendData(0x00);
  SS=1;
}

void ISD_WR_APC2(unsigned char apcdatl,apcdath)
{
  ISD_SendData(0x65);
  ISD_SendData(apcdatl);
  ISD_SendData(apcdath);
  SS=1;
}

void ISD_SET_PLAY(unsigned char Saddl,Saddh,Eaddl,Eaddh)
{
  ISD_SendData(0x80);
  ISD_SendData(0x00);
  ISD_SendData(Saddl);
  ISD_SendData(Saddh);
  ISD_SendData(Eaddl);
  ISD_SendData(Eaddh);
  ISD_SendData(0x00);
  SS=1;
}


void delayms(unsigned int t)
{
  for(;t>0;t--)
  {
   TH0=0xfc;
   TL0=0x18;
   TR0=1;
   while(TF0!=1)
     {;}
   TF0=0;
   TR0=0;
  }
}


unsigned char get_yuyin(unsigned char i)
{
	switch (i)
	{	    
				case  0:    ISD_SET_PLAY(96,0,100,0);	break;	/*0*/
        case  1:    ISD_SET_PLAY(1,0,5,0);	break;	/*1*/
        case  2:    ISD_SET_PLAY(6,0,10,0);	break;	/*2*/ 
        case  3:    ISD_SET_PLAY(11,0,15,0);	break;	/*3*/
        case  4:    ISD_SET_PLAY(16,0,20,0);	break;	/*4*/
        case  5:    ISD_SET_PLAY(21,0,25,0);	break;	/*5*/
        case  6:    ISD_SET_PLAY(26,0,30,0);	break;	/*6*/
        case  7:    ISD_SET_PLAY(31,0,35,0);	break;	/*7*/
        case  8:    ISD_SET_PLAY(36,0,40,0);	break;	/*8*/
        case  9:    ISD_SET_PLAY(41,0,45,0);	break;	/*9*/
        case 10:    ISD_SET_PLAY(46,0,50,0);	break;	/*10*/
        case 11:    ISD_SET_PLAY(51,0,55,0);	break;	/*点*/
        case 12:    ISD_SET_PLAY(56,0,60,0);	break;	/*分*/
        case 13:    ISD_SET_PLAY(61,0,65,0);	break;	/*度*/
				case 14:		ISD_SET_PLAY(66,0,80,0);	break;	/*现在时间*/
				case 15:		ISD_SET_PLAY(81,0,95,0);	break;	/*现在温度*/
				default:            break;
	}	
	return 1;
}

/********************语音播报模块************************/

void baoshi(void)							//报时功能
{
	unsigned char tenshi,tenfen,geshi,gefen;
	tenshi=display_buffer[0];
	geshi=display_buffer[1];
	tenfen=display_buffer[2];
	gefen=display_buffer[3];
	Cpu_Init();
	ISD_Init();
	ISD_SET_PLAY(66,0,80,0);
	delayms(1000);
	if(tenshi==0)								//报小时
	{
		get_yuyin(geshi);
		delayms(1000);
	}
	else
	{
		get_yuyin(tenshi);
		delayms(1000);
		ISD_SET_PLAY(46,0,50,0);
		delayms(1000);
		if(geshi!=0)
		{		get_yuyin(geshi);
				delayms(1000);
		}
	}
	ISD_SET_PLAY(51,0,55,0);
	delayms(1000);
	
		
	if(tenfen==0)									//报分钟
	{
		get_yuyin(gefen);
		delayms(1000);
	}
	else
	{
		get_yuyin(tenfen);
		delayms(1000);
		ISD_SET_PLAY(46,0,50,0);
		delayms(1000);
		if(gefen!=0)
		{		get_yuyin(gefen);
				delayms(1000);
		}
	}
	ISD_SET_PLAY(56,0,60,0);
	delayms(1000);
}
	
void baowen(void)								//报温功能
{
	unsigned char tenwen,gewen;
	tenwen=tes;
	gewen=teg;
	Cpu_Init();
	ISD_Init();
	ISD_SET_PLAY(81,0,95,0);
	delayms(1000);
	get_yuyin(tenwen);
	delayms(1000);
	ISD_SET_PLAY(46,0,50,0);
	delayms(1000);
	if(gewen!=0)
	{		get_yuyin(gewen);
			delayms(1000);
	}
	ISD_SET_PLAY(61,0,65,0);
	delayms(1000);
}
	
