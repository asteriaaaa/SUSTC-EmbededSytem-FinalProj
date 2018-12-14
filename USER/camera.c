#include "camera.h"
#include "ov7670.h"
#include "lcd.h"
#include "exti.h"
#include "exfuns.h"
#include "stdio.h"
#include "ff.h"
#include "malloc.h"
#include "delay.h"
#include "bmp.h"
#include "touch.h"	
#include "led.h"

void save_picture(void){
	u8 *pname;
	u8 error = 0;
	pname = mymalloc(32);
	while (!pname){ //Error in memory error
		LCD_ShowString(60,150,200,16,16,"Memory alloc error.");
		delay_ms(200);
	}
	photo_name(pname);
	printf("%s", pname);
	error = bmp_encode(pname, 0, 0, 240, 320, 1);
	 if (error)
      LCD_ShowString(60, 150, 200, 24, 24, "Write Error!");
   else 
      LCD_ShowString(60, 150, 200, 24, 24, "Success!");

}

void photo_name(u8 *pname){
	// 扫描已存在的文件, 决定文件名称
	u8 res;
	u16 index = 0;
	while (index < 0xFFFF){
		sprintf((char *) pname, "0:PICTURE/ASTERIA%05d.bmp", index);
		res = f_open(ftemp, (const TCHAR *) pname, FA_READ);
		if (res == FR_NO_FILE)
			break;
		index++;
	}
}

extern u8 ov_sta;	//在exit.c里面定义
//extern u8 ov_frame;	//?timer.c????		 
//更新LCD显示
void camera_refresh(void)
{
	u32 j;
 	u16 color;	 
	if(ov_sta==2)
	{
		LCD_Scan_Dir(U2D_L2R);		//从上到下,从左到右  
		LCD_SetCursor(0x00,0x0000);	//设置光标位置 
		LCD_WriteRAM_Prepare();     //开始写入GRAM	
 		OV7670_CS=0;	 
 		OV7670_RRST=0;				//开始复位读指针  
		OV7670_RCK=0;
		OV7670_RCK=1;
		OV7670_RCK=0;
		OV7670_RRST=1;				//复位读指针结束
		OV7670_RCK=1;  
		for(j=0;j<76800;j++)
		{
			GPIOB->CRL=0X88888888;		   
			OV7670_RCK=0; 
			color=OV7670_DATA;		//读数据
			OV7670_RCK=1; 	
			color<<=8;					  
 			OV7670_RCK=0;
			color|=OV7670_DATA;		//读数据		  
			OV7670_RCK=1; 
			GPIOB->CRL=0X33333333;						 	 
			LCD_WR_DATA(color);	 
		}  
 		OV7670_CS=1; 							 
		OV7670_RCK=0; 
		OV7670_RCK=1; 
		EXTI->PR=1<<15;     		//清除LINE8上的中断标志位
		ov_sta=0;					//清除LINE8上的中断标志位
 		//ov_frame++; 
		LCD_Scan_Dir(DFT_SCAN_DIR);	//恢复默认扫描方向   		
	} 
}	  

