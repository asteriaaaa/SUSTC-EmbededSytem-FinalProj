#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "key.h"
#include "usmart.h" 
#include "malloc.h"  
#include "MMC_SD.h" 
#include "ff.h"  
#include "exfuns.h"
#include "fontupd.h"
#include "text.h"	
#include "piclib.h"	
#include "string.h"	
#include "timer.h"
#include "ov7670.h" 
#include "exti.h" 
#include "touch.h"	
   	 
//µÃµ½pathÂ·¾¶ÏÂ,Ä¿±êÎÄ¼þµÄ×Ü¸öÊý
//path:Â·¾¶		    
//·µ»ØÖµ:×ÜÓÐÐ§ÎÄ¼þÊý
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//ÁÙÊ±Ä¿Â¼
	FILINFO tfileinfo;	//ÁÙÊ±ÎÄ¼þÐÅÏ¢	
	u8 *fn;	 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); 	//´ò¿ªÄ¿Â¼
  	tfileinfo.lfsize=_MAX_LFN*2+1;				//³¤ÎÄ¼þÃû×î´ó³¤¶È
	tfileinfo.lfname=mymalloc(tfileinfo.lfsize);//Îª³¤ÎÄ¼þ»º´æÇø·ÖÅäÄÚ´æ
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//²éÑ¯×ÜµÄÓÐÐ§ÎÄ¼þÊý
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//¶ÁÈ¡Ä¿Â¼ÏÂµÄÒ»¸öÎÄ¼þ
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//´íÎóÁË/µ½Ä©Î²ÁË,ÍË³ö		  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//È¡¸ßËÄÎ»,¿´¿´ÊÇ²»ÊÇÍ¼Æ¬ÎÄ¼þ	
			{
				rval++;//ÓÐÐ§ÎÄ¼þÊýÔö¼Ó1
			}	    
		}  
	} 
	return rval;
}
extern u8 ov_sta;	//?exit.c????
//extern u8 ov_frame;	//?timer.c????		 
//??LCD??
void camera_refresh(void)
{
	u32 j;
 	u16 color;	 
	if(ov_sta==2)
	{
		LCD_Scan_Dir(U2D_L2R);		//????,???? 
		LCD_SetCursor(0x00,0x0000);	//?????? 
		LCD_WriteRAM_Prepare();     //????GRAM	
 		OV7670_CS=0;	 
 		OV7670_RRST=0;				//??????? 
		OV7670_RCK=0;
		OV7670_RCK=1;
		OV7670_RCK=0;
		OV7670_RRST=1;				//??????? 
		OV7670_RCK=1;  
		for(j=0;j<76800;j++)
		{
			GPIOB->CRL=0X88888888;		   
			OV7670_RCK=0; 
			color=OV7670_DATA;		//???
			OV7670_RCK=1; 	
			color<<=8;					  
 			OV7670_RCK=0;
			color|=OV7670_DATA;		//???		  
			OV7670_RCK=1; 
			GPIOB->CRL=0X33333333;						 	 
			LCD_WR_DATA(color);	 
		}  
 		OV7670_CS=1; 							 
		OV7670_RCK=0; 
		OV7670_RCK=1; 
		EXTI->PR=1<<15;     		//??LINE8???????
		ov_sta=0;					//???????
 		//ov_frame++; 
		LCD_Scan_Dir(DFT_SCAN_DIR);	//???????? 	  				 	 
	} 
}	  

void camera_on(){
 	POINT_COLOR=RED;//ÉèÖÃ×ÖÌåÎªºìÉ« 
	LCD_ShowString(60,50,200,200,16,"Mini STM32");	
	LCD_ShowString(60,70,200,200,16,"OV7670 TEST");	
	LCD_ShowString(60,90,200,200,16,"ATOM@ALIENTEK");
	LCD_ShowString(60,110,200,200,16,"2014/3/27");  	 
	LCD_ShowString(60,130,200,200,16,"Use USMART To Set!");	 
  	LCD_ShowString(60,150,200,200,16,"OV7670 Init...");	  
	while(OV7670_Init())//³õÊ¼»¯OV7670
	{
		LCD_ShowString(60,150,200,200,16,"OV7670 Error!!");
		delay_ms(200);
	    LCD_Fill(60,150,239,166,WHITE);
		delay_ms(200);
	}
 	LCD_ShowString(60,150,200,200,16,"OV7670 Init OK");
	delay_ms(1500);	 	   	  					  
	EXTI15_Init();						//Ê¹ÄÜ¶¨Ê±Æ÷²¶»ñ
	OV7670_Window_Set(10,174,240,320);	//ÉèÖÃ´°¿Ú	  
  	OV7670_CS=0;						 	 
 	while(1)
	{	
 		camera_refresh();	//¸üÐÂÏÔÊ¾	 
			LED0=!LED0;
 		
	}	   
}


 int main(void)
 { 
	u8 res;
 	DIR picdir;	 		//Í¼Æ¬Ä¿Â¼
	FILINFO picfileinfo;//ÎÄ¼þÐÅÏ¢
	u8 *fn;   			//³¤ÎÄ¼þÃû
	u8 *pname;			//´øÂ·¾¶µÄÎÄ¼þÃû
	u16 totpicnum; 		//Í¼Æ¬ÎÄ¼þ×ÜÊý
	u16 curindex;		//Í¼Æ¬µ±Ç°Ë÷Òý
	u8 key;				//¼üÖµ
	u8 pause=0;			//ÔÝÍ£±ê¼Ç
	u8 t;
	u16 temp;
	u16 *picindextbl;	//Í¼Æ¬Ë÷Òý±í 		  	    
	delay_init();	    	 //ÑÓÊ±º¯Êý³õÊ¼»¯
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// ÉèÖÃÖÐ¶ÏÓÅÏÈ¼¶·Ö×é2
	uart_init(9600);	 	//´®¿Ú³õÊ¼»¯Îª9600			 
	LCD_Init();			//³õÊ¼»¯Òº¾§ 
	OV7670_Init();	
	tp_dev.init(); // Initialize touch screen
	LED_Init();         //LED³õÊ¼»¯	 
	KEY_Init();			//°´¼ü³õÊ¼»¯	  													    
	usmart_dev.init(72);//usmart³õÊ¼»¯	
 	mem_init();			//³õÊ¼»¯ÄÚ²¿ÄÚ´æ³Ø	
 	exfuns_init();		//ÎªfatfsÏà¹Ø±äÁ¿ÉêÇëÄÚ´æ  
  f_mount(fs, "0:", 1);
	POINT_COLOR=RED;      
	//LCD_ShowString(60,70,200,16,"Í¼Æ¬ÏÔÊ¾³ÌÐò",16,0);				    	 
	//LCD_ShowString(60,90,200,16,"KEY0:NEXT KEY1:PREV",16,0);				    	 
	//LCD_ShowString(60,110,200,16,"WK_UP:PAUSE",16,0);	
	//LCD_ShowString(60,130,200,16, "Touch upper left to delete",16,0);			    	 				    	 
 	while(f_opendir(&picdir,"0:/PICTURE"))//´ò¿ªÍ¼Æ¬ÎÄ¼þ¼Ð
 	{	    
		LCD_ShowString(60, 150, 200, 16, 16, "SD Card Error!");
		//Show_Str(60,170,240,16,"PICTUREÎÄ¼þ¼Ð´íÎó!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,170,240,186,WHITE);//Çå³ýÏÔÊ¾	     
		delay_ms(200);				  
	}  
	totpicnum=pic_get_tnum("0:/PICTURE"); //µÃµ½×ÜÓÐÐ§ÎÄ¼þÊý
  	while(totpicnum==NULL)//Í¼Æ¬ÎÄ¼þÎª0		
 	{	  
			
		//Show_Str(60,170,240,16,"Ã»ÓÐÍ¼Æ¬ÎÄ¼þ!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,170,240,186,RED);//Çå³ýÏÔÊ¾	     
		delay_ms(200);				  
	}
  	picfileinfo.lfsize=_MAX_LFN*2+1;						//³¤ÎÄ¼þÃû×î´ó³¤¶È
	picfileinfo.lfname=mymalloc(picfileinfo.lfsize);	//Îª³¤ÎÄ¼þ»º´æÇø·ÖÅäÄÚ´æ
 	pname=mymalloc(picfileinfo.lfsize);				//Îª´øÂ·¾¶µÄÎÄ¼þÃû·ÖÅäÄÚ´æ
 	picindextbl=mymalloc(2*totpicnum);				//ÉêÇë2*totpicnum¸ö×Ö½ÚµÄÄÚ´æ,ÓÃÓÚ´æ·ÅÍ¼Æ¬Ë÷Òý
 	while(picfileinfo.lfname==NULL||pname==NULL||picindextbl==NULL)//ÄÚ´æ·ÖÅä³ö´í
 	{	    
		//Show_Str(60,170,240,16,"ÄÚ´æ·ÖÅäÊ§°Ü!",16,0);
		delay_ms(200);				  
		LCD_Fill(60,170,240,186,BLUE);//Çå³ýÏÔÊ¾	     
		delay_ms(200);				  
	}  	
	//¼ÇÂ¼Ë÷Òý
    res=f_opendir(&picdir,"0:/PICTURE"); //´ò¿ªÄ¿Â¼
	if(res==FR_OK)
	{
		curindex=0;//µ±Ç°Ë÷ÒýÎª0
		while(1)//È«²¿²éÑ¯Ò»±é
		{
			temp=picdir.index;								//¼ÇÂ¼µ±Ç°index
	        res=f_readdir(&picdir,&picfileinfo);       		//¶ÁÈ¡Ä¿Â¼ÏÂµÄÒ»¸öÎÄ¼þ
	        if(res!=FR_OK||picfileinfo.fname[0]==0)break;	//´íÎóÁË/µ½Ä©Î²ÁË,ÍË³ö		  
     		fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//È¡¸ßËÄÎ»,¿´¿´ÊÇ²»ÊÇÍ¼Æ¬ÎÄ¼þ	
			{
				picindextbl[curindex]=temp;//¼ÇÂ¼Ë÷Òý
				curindex++;
			}	    
		} 
	}   
	//Show_Str(60,170,240,16,"¿ªÊ¼ÏÔÊ¾...",16,0); 
	delay_ms(1500);
	piclib_init();										//³õÊ¼»¯»­Í¼	   	   
	curindex=0;											//´Ó0¿ªÊ¼ÏÔÊ¾
   	res=f_opendir(&picdir,(const TCHAR*)"0:/PICTURE"); 	//´ò¿ªÄ¿Â¼
	while(res==FR_OK)//´ò¿ª³É¹¦
	{	
		dir_sdi(&picdir,picindextbl[curindex]);			//¸Ä±äµ±Ç°Ä¿Â¼Ë÷Òý	   
        res=f_readdir(&picdir,&picfileinfo);       		//¶ÁÈ¡Ä¿Â¼ÏÂµÄÒ»¸öÎÄ¼þ
        if(res!=FR_OK||picfileinfo.fname[0]==0)break;	//´íÎóÁË/µ½Ä©Î²ÁË,ÍË³ö
     	fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
		strcpy((char*)pname,"0:/PICTURE/");				//¸´ÖÆÂ·¾¶(Ä¿Â¼)
		strcat((char*)pname,(const char*)fn);  			//½«ÎÄ¼þÃû½ÓÔÚºóÃæ
 		LCD_Clear(BLACK);
 		ai_load_picfile(pname,0,0,lcddev.width,lcddev.height,1);//ÏÔÊ¾Í¼Æ¬    
		//Show_Str(2,2,240,16,pname,16,1); 				//ÏÔÊ¾Í¼Æ¬Ãû×Ö
		t=0;
		while(1) 
		{
			LCD_ShowString(20, 20, 200, 16, 16, "DEL");
			
			key=KEY_Scan(0);//É¨Ãè°´¼ü
			tp_dev.scan(0);		// Scan Touchscreen
			if(t>250)key=1;			//Ä£ÄâÒ»´Î°´ÏÂKEY0    
			if((t%20)==0)LED0=!LED0;//LED0ÉÁË¸,ÌáÊ¾³ÌÐòÕýÔÚÔËÐÐ.
			if(key==KEY1_PRES)		//ÉÏÒ»ÕÅ
			{
				
				if(curindex)curindex--;
				else curindex=totpicnum-1;
				break;
			}else if(key==KEY0_PRES)//ÏÂÒ»ÕÅ
			{
				curindex++;		   	
				if(curindex>=totpicnum)curindex=0;//µ½Ä©Î²µÄÊ±ºò,×Ô¶¯´ÓÍ·¿ªÊ¼
				break;
			}else if(key==WKUP_PRES)
			{
				pause=!pause;
				LED1=!pause; 	//ÔÝÍ£µÄÊ±ºòLED1ÁÁ.  
			}
			if(pause==0)t++;
			if(tp_dev.x[0]<50&&tp_dev.y[0]<40) // Touch to remove current file
			{
				f_unlink((char *)pname);
				delay_ms(100);
				break;
			}
			delay_ms(10); 

		}					    
		res=0;  
	} 											  
	myfree(picfileinfo.lfname);	//ÊÍ·ÅÄÚ´æ			    
	myfree(pname);				//ÊÍ·ÅÄÚ´æ			    
	myfree(picindextbl);		//ÊÍ·ÅÄÚ´æ			    
}







