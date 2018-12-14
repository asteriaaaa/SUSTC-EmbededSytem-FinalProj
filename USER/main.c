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
#include "camera.h"
   	 
//Retrieve the number of target files
u16 pic_get_tnum(u8 *path)
{	  
	u8 res;
	u16 rval=0;
 	DIR tdir;	 		//tmp dir
	FILINFO tfileinfo;	//tmp file info	
	u8 *fn;	 			 			   			     
    res=f_opendir(&tdir,(const TCHAR*)path); 	//open the dir
  	tfileinfo.lfsize=_MAX_LFN*2+1;				//长文件名最大长度
	tfileinfo.lfname=mymalloc(tfileinfo.lfsize);//为长文件缓存区分配内存
	if(res==FR_OK&&tfileinfo.lfname!=NULL)
	{
		while(1)//Total number of files
		{
	        res=f_readdir(&tdir,&tfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||tfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出	  
     		fn=(u8*)(*tfileinfo.lfname?tfileinfo.lfname:tfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//To see if it's jpg	
			{
				rval++;
			}	    
		}  
	} 
	return rval;
}

void camera_on(){
 				 	 
 
}





 int main(void)
 { 
	u8 res;
 	DIR picdir;	 		//图片目录
	FILINFO picfileinfo;//文件信息
	u8 *fn;   			//长文件名
	u8 *pname;			//带路径的文件名
	u16 totpicnum; 		//图片文件总数
	u16 curindex;		//图片当前索引
	u16 tmpindex=0;
	u8 key=0;				//键值
	u8 pause=0;			//暂停标记
	u8 t;
	u16 temp;
	u16 *picindextbl;	//图片索引表 		  	    
	delay_init();	    	 //延时函数初始化
  	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	uart_init(9600);	 	//串口初始化为9600			 
	LCD_Init();			//初始化液晶
	OV7670_Init();	
	tp_dev.init(); // Initialize touch screen
	LED_Init();         //LED 
	KEY_Init();			//按键初始化	 	  													    
	usmart_dev.init(72);//usmart初始化	
 	mem_init();			//初始化内部内存池	
 	exfuns_init();		//为fatfs相关变量申请内存 
  	f_mount(fs, "0:", 1); //挂载SD卡
	POINT_COLOR=RED;      		   

MENU:
delay_ms(1000);
	while (1){
		
		tp_dev.scan(0);
	
		LCD_ShowString(60,150,200,16, 16,"LEFT ALBUM");	
		LCD_ShowString(60,60,200,16, 16,"RIGHT CAMERA");	
		if(tp_dev.sta&TP_PRES_DOWN&&tp_dev.x[0]>100) {
		while(1){
		delay_ms(100);
		POINT_COLOR=RED;
		while(OV7670_Init())//初始化OV7670
		{
		LCD_ShowString(60,150,200,200,16,"OV7670 Error!!");
		delay_ms(200);
	  LCD_Fill(60,150,239,166,WHITE);
		delay_ms(200);
		}
		LCD_ShowString(60,150,200,200,16,"OV7670 Init OK");
		delay_ms(1500);	 	   	  					  
		EXTI15_Init();						//使能定时器捕获
		OV7670_Window_Set(10,174,240,320);	//设置窗口	  
  	OV7670_CS=0;		
		while(1)
		{	
		tp_dev.scan(0);
			camera_refresh();	//更新显示	 
			LED0=!LED0;  //DS0闪烁. 		
		if (tp_dev.y[0] > 240&&tp_dev.sta & TP_PRES_DOWN)
			break;
		else if (tp_dev.y[0] < 100&&tp_dev.sta & TP_PRES_DOWN){
			LCD_Clear(WHITE);
			goto MENU;
		}
		}	   
		save_picture();
		delay_ms(500);
		}
	}
		else if(tp_dev.sta&TP_PRES_DOWN&&tp_dev.x[0]<100){
			LCD_Clear(RED);
		delay_ms(1000);
			break;
		}
	}
	    	 
 	while(f_opendir(&picdir,"0:/PICTURE"))//´ò¿ªÍ¼Æ¬ÎÄ¼þ¼Ð
 	{	    
		LCD_ShowString(60, 150, 200, 16, 16, "SD Card Error!");
		delay_ms(200);				  
		LCD_Fill(60,170,240,186,WHITE);////清除显示     
		delay_ms(200);				  
	}  
READLIST: // Used for solving problems after deleting a pic.
	totpicnum=pic_get_tnum("0:/PICTURE"); //µÃµ½×ÜÓÐÐ§ÎÄ¼þÊý

  	while(totpicnum==NULL)////图片文件为0			
 	{	  
			
		LCD_ShowString(60, 150, 200, 16, 16, "NO PICTURE");
		delay_ms(200);				  
		LCD_Fill(60,170,240,186,RED);//清除显示    
		delay_ms(200);				  
	}
  	picfileinfo.lfsize=_MAX_LFN*2+1;						//长文件名最大长度
	picfileinfo.lfname=mymalloc(picfileinfo.lfsize);	//为长文件缓存区分配内存
 	pname=mymalloc(picfileinfo.lfsize);				//为带路径的文件名分配内存
 	picindextbl=mymalloc(2*totpicnum);				//申请2*totpicnum个字节的内存,用于存放图片索引
 	while(picfileinfo.lfname==NULL||pname==NULL||picindextbl==NULL)//内存分配出错
 	{	    
		LCD_ShowString(60, 150, 200, 16, 16, "Memory Error!");
		delay_ms(200);				  
		LCD_Fill(60,170,240,186,BLUE);	     
		delay_ms(200);				  
	}  	
	//记录索引
    res=f_opendir(&picdir,"0:/PICTURE"); //´ò¿ªÄ¿Â¼
	if(res==FR_OK)
	{
		curindex=0;//当前索引为0
		while(1)//全部查询一遍
		{
			temp=picdir.index;								//记录当前index
	        res=f_readdir(&picdir,&picfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||picfileinfo.fname[0]==0)break;	//错误了/到末尾了,退出	  
     		fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			 
			res=f_typetell(fn);	
			if((res&0XF0)==0X50)//取高四位,看看是不是图片文件
			{
				picindextbl[curindex]=temp;//记录索引
				curindex++;
			}	    
		} 
	}   
	delay_ms(1500);
	piclib_init();		//初始化画图	
	if (tmpindex) 
		curindex = tmpindex;	 // Recover index from delete
	else
		curindex=0;											//从0开始显示
	
   	res=f_opendir(&picdir,(const TCHAR*)"0:/PICTURE"); 	//打开目录

	while(res==FR_OK)//打开成功
	{	

		dir_sdi(&picdir,picindextbl[curindex]);			//改变当前目录索引   
				LCD_Clear(BLACK);
	delay_ms(100);
        res=f_readdir(&picdir,&picfileinfo);   		//读取目录下的一个文件
	
        if(res!=FR_OK||picfileinfo.fname[0]==0){
					break;	//错误了/到末尾了,退出
				}
     	fn=(u8*)(*picfileinfo.lfname?picfileinfo.lfname:picfileinfo.fname);			
		strcpy((char*)pname,"0:/PICTURE/");				//复制路径(目录)
		strcat((char*)pname,(const char*)fn);  			//将文件名接在后面
 		LCD_Clear(BLACK);
 		ai_load_picfile(pname,0,0,lcddev.width,lcddev.height,1);//显示图片    
		t=0;

		while(1) 
		{
			LCD_ShowString(20, 20, 200, 16, 16, "BACK");
			

			printf("%d",pause);
			tp_dev.scan(0);		// Scan Touchscreen
			 if (t > 250) { //下一张图片
               curindex++;
               if (curindex >= totpicnum)
                  curindex = 0; //到末尾的时候,自动从头开始
               break;
            }
			if((t%20)==0)
				LED0=!LED0;//LED0闪烁,提示程序正在运行.
			   if (tp_dev.sta & TP_PRES_DOWN) {
               if (tp_dev.y[0] <= 50 && tp_dev.x[0] <= 50) // go back
               {
								 LCD_Clear(WHITE);
                  goto MENU;
               } else if (tp_dev.x[0] <= 50 &&
                          tp_dev.y[0] >= lcddev.height - 50) //触摸左下角，删除窗口
               {
                  f_unlink((char *)pname);
                  LCD_ShowString(60, 150, 200, 16, 16, "Deleted!");
                  delay_ms(500);
                  break;
               } else if (tp_dev.y[0] < 120) //触摸上部分的1/3，上一张
               {
                  if (curindex)
                     curindex--;
                  else
                     curindex = totpicnum - 1;
                  break;
               } else if (tp_dev.sta & TP_PRES_DOWN&&tp_dev.y[0] > 120 &&
                          tp_dev.y[0] < 240) //触摸中间的1/3，暂停
               {
                  pause = !pause;
                  LED1 = !pause;        //暂停的时候LED1亮.
               } else if (tp_dev.y[0] > 240) //触摸下面的1/3，下一张
               {
                  curindex++;
                  if (curindex >= totpicnum)
                     curindex = 0; //到末尾的时候,自动从头开始
                  break;
               }
            }
			if(pause==0)t++;
			delay_ms(10); 
		}	
		res=0;  
	} 											  
	myfree(picfileinfo.lfname);		//释放内存		    
	myfree(pname);							    
	myfree(picindextbl);					    
}







