#ifndef __CAMERA_H
#define __CAMERA_H
#include "sys.h"

void save_picture(void);
void photo_name(u8 *pname);
extern u8 ov_sta;	//在exit.c里面定义
//extern u8 ov_frame;	//?timer.c????		 
//更新LCD显示
void camera_refresh(void);  


#endif
