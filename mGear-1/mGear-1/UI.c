#include "UI.h"
#include "input.h"

UI_WINDOW UI_Win[MAX_UIWINDOWS];
UI_SYSTEM UI_Sys;

void UILoadSystem(char *filename)
{
	FILE *file;
	int8 basic=1;
	char buf[256], *tok, *val;

	int16 value=0;

	if(filename==NULL)
		basic=0;
	else
	if((file=fopen(filename,"r"))==NULL)
		basic=0;

	if(basic)
	{
		memset(&UI_Sys,-1,sizeof(UI_SYSTEM));

		while(!feof(file))
		{
			fgets(buf,256,file);

			tok=strtok(buf,"=\"");
			val=strtok(NULL," =\"");

			if(strcmp(tok,"MGG ID")==NULL)
				UI_Sys.mgg_id=atoi(val);

			if(strcmp(tok,"Window2 Frame")==NULL)
				UI_Sys.window2_frame=atoi(val);

			if(strcmp(tok,"Window Frame0")==NULL)
				UI_Sys.window_frame0=atoi(val);
			if(strcmp(tok,"Window Frame1")==NULL)
				UI_Sys.window_frame1=atoi(val);
			if(strcmp(tok,"Window Frame2")==NULL)
				UI_Sys.window_frame2=atoi(val);

			if(strcmp(tok,"Button Frame0")==NULL)
				UI_Sys.button_frame0=atoi(val);
			if(strcmp(tok,"Button Frame1")==NULL)
				UI_Sys.button_frame1=atoi(val);
			if(strcmp(tok,"Button Frame2")==NULL)
				UI_Sys.button_frame2=atoi(val);

			if(strcmp(tok,"Subwindow Frame0")==NULL)
				UI_Sys.subwindow_frame0=atoi(val);
			if(strcmp(tok,"Subwindow Frame1")==NULL)
				UI_Sys.subwindow_frame1=atoi(val);
			if(strcmp(tok,"Subwindow Frame2")==NULL)
				UI_Sys.subwindow_frame2=atoi(val);

			if(strcmp(tok,"Tab Frame")==NULL)
				UI_Sys.tab_frame=atoi(val);
			if(strcmp(tok,"Close Frame")==NULL)
				UI_Sys.close_frame=atoi(val);

			if(strcmp(tok,"Scroll Frame0")==NULL)
				UI_Sys.scroll_frame0=atoi(val);
			if(strcmp(tok,"Scroll Frame1")==NULL)
				UI_Sys.scroll_frame1=atoi(val);
			if(strcmp(tok,"Scroll Up Frame")==NULL)
				UI_Sys.scroll_up_frame=atoi(val);
			if(strcmp(tok,"Scroll Down Frame")==NULL)
				UI_Sys.scroll_down_frame=atoi(val);

			if(strcmp(tok,"Resize Cursor Frame")==NULL)
				UI_Sys.resize_cursor=atoi(val);

		}

		if(UI_Sys.mgg_id==-1)
			UI_Sys.mgg_id=0;

		if(UI_Sys.window2_frame==-1)
			UI_Sys.window2_frame=4;

		if(UI_Sys.window_frame0==-1)
			UI_Sys.window_frame0=6;
		if(UI_Sys.window_frame1==-1)
			UI_Sys.window_frame1=7;
		if(UI_Sys.window_frame2==-1)
			UI_Sys.window_frame2=8;

		if(UI_Sys.button_frame0==-1)
			UI_Sys.button_frame0=9;
		if(UI_Sys.button_frame1==-1)
			UI_Sys.button_frame1=10;
		if(UI_Sys.button_frame2==-1)
			UI_Sys.button_frame2=11;

		if(UI_Sys.subwindow_frame0==-1)
			UI_Sys.subwindow_frame0=14;
		if(UI_Sys.subwindow_frame1==-1)
			UI_Sys.subwindow_frame1=15;
		if(UI_Sys.subwindow_frame2==-1)
			UI_Sys.subwindow_frame2=16;

		if(UI_Sys.tab_frame==-1)
			UI_Sys.tab_frame=12;
		if(UI_Sys.close_frame==-1)
			UI_Sys.close_frame=13;

		if(UI_Sys.resize_cursor==-1)
			UI_Sys.resize_cursor=6;


		fclose(file);

		LogApp("UI system loaded");
	}
	else
	{
		UI_Sys.mgg_id=0;
		UI_Sys.window2_frame=4;
		UI_Sys.window_frame0=6;
		UI_Sys.window_frame1=7;
		UI_Sys.window_frame2=8;
		UI_Sys.button_frame0=9;
		UI_Sys.button_frame1=10;
		UI_Sys.button_frame2=11;
		UI_Sys.tab_frame=12;
		UI_Sys.close_frame=13;
		UI_Sys.subwindow_frame0=14;
		UI_Sys.subwindow_frame1=15;
		UI_Sys.subwindow_frame2=16;
		UI_Sys.resize_cursor=6;

		LogApp("UI system loaded");
	}

}

int16 UIMessageBox(int32 x, int32 y, UI_POS bpos, const char *text, uint8 num_options, uint8 font, size_t font_size, uint32 colorN, uint32 colorS, uint32 colorT)
{
	char text_f[8][33];
	int32 xsize, ysize, text_size, height_size;
	int16 str_len, lines=0, i, j, ch_stop=0;

	uint8 rn, gn, bn, rs, gs, bs, rt, gt, bt;

	rn=colorN>>16;
	gn=(colorN>>8) & 0xFF;
	bn=colorN & 0xFF;

	rs=colorS>>16;
	gs=(colorS>>8) & 0xFF;
	bs=colorS & 0xFF;

	rt=colorT>>16;
	gt=(colorT>>8) & 0xFF;
	bt=colorT & 0xFF;

	//Forget x and y positions if bpos is not custom
	if(bpos==CENTER)
	{
		x=8192;
		y=4096;
	}

	str_len=strlen(text);

	if(str_len>32)
	{
		lines=str_len/32;

		for(i=0;i<lines;i++)
		{
			for(j=0;j<32;j++)
			{
				if(text[j+ch_stop]=='\n' || j==31 || text[j+ch_stop]==0)
				{
					ch_stop=j+ch_stop+1;

					if(j==31)
					{
						text_f[i][j+1]=0;
						text_f[i][j]=text[j+ch_stop];
					}
					else
						text_f[i][j]=0;

					if(i==lines-1 && ch_stop<str_len)
						lines++;

					break;
				}
				else
					text_f[i][j]=text[j+ch_stop];
			}
		}

		text_size=32*((st.fonts[font].size_w_gm*font_size)/FONT_SIZE); //text size in game units

		height_size=lines*((st.fonts[font].size_h_gm*font_size)/FONT_SIZE);

		
	}
	else
	{
		text_size=str_len*((st.fonts[font].size_w_gm*font_size)/FONT_SIZE); //text size in game units

		height_size=((st.fonts[font].size_h_gm*font_size)/FONT_SIZE);
	}

	text_size+=128; //64 g.u of space between sides
	height_size+=2*((st.fonts[font].size_h_gm*font_size)/FONT_SIZE);

	UIData(x,y,text_size,height_size,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id].frames[SYS_BOX_TILE],255,7);

	if(!lines)
		StringUIData(text,x,y,0,0,0,rt,gt,bt,255,0,font_size,font_size,6);
	else
	{
		for(i=0;i<lines;i++)
			StringUIData(text_f[i],x,(y-((height_size-512)/2))+(i*((st.fonts[font].size_h_gm*font_size)/FONT_SIZE)),0,0,0,rt,gt,bt,255,0,font_size,font_size,6);
	}

	if(num_options==1 || !num_options)
	{
		if(CheckColisionMouse(x,y+((height_size-256)/2),2*((st.fonts[font].size_w_gm*font_size)/FONT_SIZE),((st.fonts[font].size_h_gm*font_size)/FONT_SIZE),0))
		{
			StringUIData("OK",x,y+((height_size-256)/2),0,0,0,rs,gs,bs,255,0,2048,2048,5);

			if(st.mouse1)
				return UI_OK;
		}
		else
			StringUIData("OK",x,y+((height_size-256)/2),0,0,0,rn,gn,bn,255,0,2048,2048,5);
	}
	else
	if(num_options==2)
	{
		if(CheckColisionMouse(x-((text_size-256)/2),y+((height_size-256)/2),2*((st.fonts[font].size_w_gm*font_size)/FONT_SIZE),((st.fonts[font].size_h_gm*font_size)/FONT_SIZE),0) && st.mouse1)
		{
			if(st.mouse1)
				return UI_YES;
		}
		else
			StringUIData("Yes",x-((text_size-256)/2),y+((height_size-256)/2),0,0,0,rn,gn,bn,255,0,2048,2048,5);

		if(CheckColisionMouse(x+((text_size-256)/2),y+((height_size-256)/2),2*((st.fonts[font].size_w_gm*font_size)/FONT_SIZE),((st.fonts[font].size_h_gm*font_size)/FONT_SIZE),0) && st.mouse1)
		{
			if(st.mouse1)
				return UI_NO;
		}
		else
			StringUIData("No",x+((text_size-256)/2),y+((height_size-256)/2),0,0,0,rn,gn,bn,255,0,2048,2048,5);
	}
	else
	if(num_options==3)
	{
		if(CheckColisionMouse(x-((text_size-128)/2),y+((height_size-256)/2),2*((st.fonts[font].size_w_gm*font_size)/FONT_SIZE),((st.fonts[font].size_h_gm*font_size)/FONT_SIZE),0) && st.mouse1)
		{
			StringUIData("Yes",x-((text_size-128)/2),y+((height_size-256)/2),0,0,0,rs,gs,bs,255,0,2048,2048,5);

			if(st.mouse1)
				return UI_YES;
		}
		else
			StringUIData("Yes",x-((text_size-128)/2),y+((height_size-256)/2),0,0,0,rn,gn,bn,255,0,2048,2048,5);

		if(CheckColisionMouse(x+((text_size-128)/2),y+((height_size-256)/2),2*((st.fonts[font].size_w_gm*font_size)/FONT_SIZE),((st.fonts[font].size_h_gm*font_size)/FONT_SIZE),0) && st.mouse1)
		{
			StringUIData("No",x+((text_size-128)/2),y+((height_size-256)/2),0,0,0,rs,gs,bs,255,0,2048,2048,5);

			if(st.mouse1)
				return UI_NO;
		}
		else
			StringUIData("No",x+((text_size-128)/2),y+((height_size-256)/2),0,0,0,rn,gn,bn,255,0,2048,2048,5);

		if(CheckColisionMouse(x,y+((height_size-256)/2),2*((st.fonts[font].size_w_gm*font_size)/FONT_SIZE),((st.fonts[font].size_h_gm*font_size)/FONT_SIZE),0) && st.mouse1)
		{
			StringUIData("Cancel",x,y+((height_size-256)/2),0,0,0,rs,gs,bs,255,0,2048,2048,5);

			if(st.mouse1)
				return UI_CANCEL;
		}
		else
			StringUIData("Cancel",x,y+((height_size-256)/2),0,0,0,rn,gn,bn,255,0,2048,2048,5);
	}

	return UI_NULLOP;
}

int16 UIOptionBox(int32 x, int32 y, UI_POS bpos, const char options[8][16], uint8 num_options, uint8 font, size_t font_size, uint32 colorN, uint32 colorS)
{
	int32 xsize, ysize, text_size, height_size, temp;
	int16 i, j, gsize;

	uint8 rn, gn, bn, rs, gs, bs;

	rn=colorN>>16;
	gn=(colorN>>8) & 0xFF;
	bn=colorN & 0xFF;

	rs=colorS>>16;
	gs=(colorS>>8) & 0xFF;
	bs=colorS & 0xFF;

	if(bpos==CENTER)
	{
		x=8192;
		y=4096;
	}

	height_size=(((st.fonts[font].size_h_gm*font_size)/FONT_SIZE)*num_options)+256;
	gsize=(st.fonts[font].size_h_gm*font_size)/FONT_SIZE;

	for(i=0;i<num_options;i++)
	{
		if(i==0)
			text_size=strlen(options[i])*((st.fonts[font].size_w_gm*font_size)/FONT_SIZE); //text size in game units
		else
		{
			temp=strlen(options[i])*((st.fonts[font].size_w_gm*font_size)/FONT_SIZE);

			if(temp>text_size)
				text_size=temp;
		}
	}

	UIData(x,y,text_size+128,height_size,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id].frames[SYS_BOX_TILE],255,7);

	for(i=0;i<num_options;i++)
	{
		if(CheckColisionMouse(x,(y+(i*(st.fonts[font].size_h_gm*font_size)/FONT_SIZE))-((height_size-128-gsize)/2),text_size,(st.fonts[font].size_h_gm*font_size)/FONT_SIZE,0))
		{
			StringUIData(options[i],x,(y+(i*(st.fonts[font].size_h_gm*font_size)/FONT_SIZE))-((height_size-128-gsize)/2),0,0,0,rs,gs,bs,255,font,font_size,font_size,5);

			if(st.mouse1)
				return 100+i;
		}
		else
			StringUIData(options[i],x,(y+(i*(st.fonts[font].size_h_gm*font_size)/FONT_SIZE))-((height_size-128-gsize)/2),0,0,0,rn,gn,bn,255,font,font_size,font_size,5);
	}

	return UI_NULLOP;
}

int8 UIWin2_StringButton(int8 uiwinid, int8 pos,char *text, int32 colorN, int32 colorS)
{
	int16 lenght, gsize;

	uint8 rn, gn, bn, rs, gs, bs;

	char *text2;

	lenght=strlen(text);

	text2=malloc(lenght+5);

	strcpy(text2,text);

	rn=colorN>>16;
	gn=(colorN>>8) & 0xFF;
	bn=colorN & 0xFF;

	rs=colorS>>16;
	gs=(colorS>>8) & 0xFF;
	bs=colorS & 0xFF;

	gsize=(st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE;

	lenght=strlen(text2);

		if(CheckColisionMouse(UI_Win[uiwinid].pos.x,(pos*((st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE))+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),
			lenght*((st.fonts[UI_Win[uiwinid].font].size_w_gm*UI_Win[uiwinid].font_size)/FONT_SIZE),(st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE,0))
		{
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*((st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE))+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rs,gs,bs,255,
				UI_Win[uiwinid].font,UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

			if(st.mouse1)
			{
				st.mouse1=0;
				return UI_SEL;
			}
		}
		else
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*((st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE))+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rn,gn,bn,255,
				UI_Win[uiwinid].font,UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

	return UI_NULLOP;
}

int8 UIStringButton(int32 x, int32 y,char *text, int8 font, int16 font_size, int8 layer, int32 colorN, int32 colorS)
{
	int32 text_size;
	int16 gsize, gsizew;

	uint8 rn, gn, bn, rs, gs, bs;

	rn=colorN>>16;
	gn=(colorN>>8) & 0xFF;
	bn=colorN & 0xFF;

	rs=colorS>>16;
	gs=(colorS>>8) & 0xFF;
	bs=colorS & 0xFF;

	gsize=(st.fonts[font].size_h_gm*font_size)/FONT_SIZE;
	gsizew=(st.fonts[font].size_w_gm*font_size)/FONT_SIZE;

	text_size=gsizew*strlen(text);

	if(CheckColisionMouse(x,y,text_size,gsize,0))
	{
		StringUIData(text,x,y,text_size,gsize,0,rs,gs,bs,255,font,font_size,font_size,layer);

		if(st.mouse1)
		{
			st.mouse1=0;
			return UI_SEL;
		}
	}
	else
		StringUIData(text,x,y,text_size,gsize,0,rn,gn,bn,255,font,font_size,font_size,layer);

	return UI_NULLOP;
}

int8 UIStringButtonWorld(int32 x, int32 y,char *text, int8 font, int16 font_size, int8 layer, int32 colorN, int32 colorS)
{
	int32 text_size;
	int16 gsize, gsizew;

	uint8 rn, gn, bn, rs, gs, bs;

	rn=colorN>>16;
	gn=(colorN>>8) & 0xFF;
	bn=colorN & 0xFF;

	rs=colorS>>16;
	gs=(colorS>>8) & 0xFF;
	bs=colorS & 0xFF;

	gsize=(st.fonts[font].size_h_gm*font_size)/FONT_SIZE;
	gsizew=(st.fonts[font].size_w_gm*font_size)/FONT_SIZE;

	text_size=gsizew*strlen(text);

	if(CheckColisionMouseWorld(x,y,text_size,gsize,0,0))
	{
		String2Data(text,x,y,text_size,gsize,0,rs,gs,bs,255,font,font_size,font_size,layer);

		if(st.mouse1)
		{
			st.mouse1=0;
			return UI_SEL;
		}
	}
	else
		String2Data(text,x,y,text_size,gsize,0,rn,gn,bn,255,font,font_size,font_size,layer);

	return UI_NULLOP;
}

int8 UICreateWindow(int32 x, int32 y, int32 xsize, int32 ysize, UI_POS bpos, int8 layer, uint8 window_frame)
{
	int8 ID, i;

	if(bpos==CENTER)
	{
		x=8192;
		y=4096;
	}

	if(st.num_uiwindow<MAX_UIWINDOWS)
	{
		for(i=0;i<MAX_UIWINDOWS;i++)
		{
			if(!UI_Win[i].stat)
			{
				UI_Win[i].stat=1;
				UI_Win[i].pos.x=x;
				UI_Win[i].pos.y=y;
				UI_Win[i].size.x=xsize;
				UI_Win[i].size.y=ysize;
				UI_Win[i].layer=layer;
				UI_Win[i].num_options=0;
				UI_Win[i].current=-1;

				if(window_frame==0 || window_frame==1)
					UI_Win[i].window_frame=UI_Sys.window_frame0;
				else
				if(window_frame==2)
					UI_Win[i].window_frame=UI_Sys.window_frame1;
				else
				if(window_frame==3)
					UI_Win[i].window_frame=UI_Sys.window_frame2;

				ID=st.num_uiwindow;
				st.num_uiwindow++;
				break;
			}
		}
	}
	else
		return -1;

	return ID;
}

int8 UICreateWindow2(int32 x, int32 y, UI_POS bpos, int8 layer, uint8 num_avail_options, int16 font_size, int8 num_charsperoption, int8 font)
{
	int8 ID, i;

	if(bpos==CENTER)
	{
		x=8192;
		y=4096;
	}

	if(st.num_uiwindow<MAX_UIWINDOWS)
	{
		for(i=0;i<MAX_UIWINDOWS;i++)
		{
			if(!UI_Win[i].stat)
			{
				UI_Win[i].stat=2;
				UI_Win[i].pos.x=x;
				UI_Win[i].pos.y=y;
				UI_Win[i].size.x=(num_charsperoption*st.fonts[font].size_w_gm*font_size)/FONT_SIZE;
				UI_Win[i].size.y=(num_avail_options*(st.fonts[font].size_h_gm*font_size)/FONT_SIZE)+256;
				UI_Win[i].layer=layer;
				UI_Win[i].num_options=0;
				UI_Win[i].font=font;
				UI_Win[i].font_size=font_size;
				UI_Win[i].current=-1;
				ID=st.num_uiwindow;
				st.num_uiwindow++;
				break;
			}
		}
	}
	else
		return -1;

	return ID;
}

void UIDestroyWindow(int8 id)
{
	UI_Win[id].stat=0;
	UI_Win[id].current=-1;
	st.num_uiwindow--;
}

int8 UIWin2_MarkBox(int8 uiwinid, int8 pos, uint8 marked, char *text, int32 colorN, int32 colorS)
{
	int16 lenght, gsize;

	uint8 rn, gn, bn, rs, gs, bs;

	char *text2;

	lenght=strlen(text);

	text2=malloc(lenght+5);

	strcpy(text2,text);

	rn=colorN>>16;
	gn=(colorN>>8) & 0xFF;
	bn=colorN & 0xFF;

	rs=colorS>>16;
	gs=(colorS>>8) & 0xFF;
	bs=colorS & 0xFF;

	if(marked==1)
		strcat(text2," [X]");
	else
	if(!marked || marked==2)
		strcat(text2," [ ]");

	gsize=(st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE;

	lenght=strlen(text2);

	if(!marked || marked==1)
	{
		if(CheckColisionMouse(UI_Win[uiwinid].pos.x,(pos*((st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE))+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),
			lenght*((st.fonts[UI_Win[uiwinid].font].size_w_gm*UI_Win[uiwinid].font_size)/FONT_SIZE),(st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE,0))
		{
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*((st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE))+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rs,gs,bs,255,
				UI_Win[uiwinid].font,UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

			if(st.mouse1 && !marked)
			{
				st.mouse1=0;
				return 2;
			}
			else
			if(st.mouse1 && marked==1)
			{
				st.mouse1=0;
				return 1;
			}
		}
		else
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*((st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE))+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rn,gn,bn,255,
				UI_Win[uiwinid].font,UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);
	}
	else
	if(marked==2)
		StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*((st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE))+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rn,gn,bn,128,
				UI_Win[uiwinid].font,UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

	return 0;
}

void UIWin2_NumberBoxui8(int8 uiwinid, int8 pos, uint8 *value, char *text, int32 colorN, int32 colorS, int32 colorC)
{
	int16 lenght, gsize;

	uint8 rn, gn, bn, rs, gs, bs, rc, gc, bc;

	char text2[32];

	rn=colorN>>16;
	gn=(colorN>>8) & 0xFF;
	bn=colorN & 0xFF;

	rs=colorS>>16;
	gs=(colorS>>8) & 0xFF;
	bs=colorS & 0xFF;

	rc=colorC>>16;
	gc=(colorC>>8) & 0xFF;
	bc=colorC & 0xFF;

	gsize=(st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE;

	sprintf(text2,"%s: %u",text, *value);

	lenght=strlen(text2);

	if(UI_Win[uiwinid].current!=pos)
	{
		if(CheckColisionMouse(UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),
			lenght*((st.fonts[UI_Win[uiwinid].font].size_w_gm*UI_Win[uiwinid].font_size)/FONT_SIZE),gsize,0))
		{
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rs,gs,bs,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

			if(st.mouse1)
			{
				UI_Win[uiwinid].current=pos;
				sprintf(st.TextInput,"%u",*value);
				StartText();
				st.mouse1=0;
			}
		}
		else
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rn,gn,bn,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);
	}
	else
	if(UI_Win[uiwinid].current==pos)
	{
		StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rc,gc,bc,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

		*value=atoi(st.TextInput);

		if(st.keys[RETURN_KEY].state)
		{
			StopText();
			st.keys[RETURN_KEY].state=0;
			UI_Win[uiwinid].current=-1;
		}
	}
}

void UIWin2_NumberBoxi8(int8 uiwinid, int8 pos, int8 *value, char *text, int32 colorN, int32 colorS, int32 colorC)
{
	int16 lenght, gsize;

	uint8 rn, gn, bn, rs, gs, bs, rc, gc, bc;

	char text2[32];

	rn=colorN>>16;
	gn=(colorN>>8) & 0xFF;
	bn=colorN & 0xFF;

	rs=colorS>>16;
	gs=(colorS>>8) & 0xFF;
	bs=colorS & 0xFF;

	rc=colorC>>16;
	gc=(colorC>>8) & 0xFF;
	bc=colorC & 0xFF;

	gsize=(st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE;

	sprintf(text2,"%s: %d",text, *value);

	lenght=strlen(text2);

	if(UI_Win[uiwinid].current!=pos)
	{
		if(CheckColisionMouse(UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),
			lenght*((st.fonts[UI_Win[uiwinid].font].size_w_gm*UI_Win[uiwinid].font_size)/FONT_SIZE),gsize,0))
		{
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rs,gs,bs,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

			if(st.mouse1)
			{
				UI_Win[uiwinid].current=pos;
				sprintf(st.TextInput,"%d",*value);
				StartText();
				st.mouse1=0;
			}
		}
		else
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rn,gn,bn,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);
	}
	else
	if(UI_Win[uiwinid].current==pos)
	{
		StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rc,gc,bc,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

		*value=atoi(st.TextInput);

		if(st.keys[RETURN_KEY].state)
		{
			StopText();
			st.keys[RETURN_KEY].state=0;
			UI_Win[uiwinid].current=-1;
		}
	}
}

void UIWin2_NumberBoxui16(int8 uiwinid, int8 pos, uint16 *value, char *text, int32 colorN, int32 colorS, int32 colorC)
{
	int16 lenght, gsize;

	uint8 rn, gn, bn, rs, gs, bs, rc, gc, bc;

	char text2[32];

	rn=colorN>>16;
	gn=(colorN>>8) & 0xFF;
	bn=colorN & 0xFF;

	rs=colorS>>16;
	gs=(colorS>>8) & 0xFF;
	bs=colorS & 0xFF;

	rc=colorC>>16;
	gc=(colorC>>8) & 0xFF;
	bc=colorC & 0xFF;

	gsize=(st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE;

	sprintf(text2,"%s: %u",text, *value);

	lenght=strlen(text2);

	if(UI_Win[uiwinid].current!=pos)
	{
		if(CheckColisionMouse(UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),
			lenght*((st.fonts[UI_Win[uiwinid].font].size_w_gm*UI_Win[uiwinid].font_size)/FONT_SIZE),gsize,0))
		{
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rs,gs,bs,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

			if(st.mouse1)
			{
				UI_Win[uiwinid].current=pos;
				sprintf(st.TextInput,"%u",*value);
				StartText();
				st.mouse1=0;
			}
		}
		else
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rn,gn,bn,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);
	}
	else
	if(UI_Win[uiwinid].current==pos)
	{
		StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rc,gc,bc,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

		*value=atoi(st.TextInput);

		if(st.keys[RETURN_KEY].state)
		{
			StopText();
			st.keys[RETURN_KEY].state=0;
			UI_Win[uiwinid].current=-1;
		}
	}
}

void UIWin2_NumberBoxi16(int8 uiwinid, int8 pos, int16 *value, char *text, int32 colorN, int32 colorS, int32 colorC)
{
	int16 lenght, gsize;

	uint8 rn, gn, bn, rs, gs, bs, rc, gc, bc;

	char text2[32];

	rn=colorN>>16;
	gn=(colorN>>8) & 0xFF;
	bn=colorN & 0xFF;

	rs=colorS>>16;
	gs=(colorS>>8) & 0xFF;
	bs=colorS & 0xFF;

	rc=colorC>>16;
	gc=(colorC>>8) & 0xFF;
	bc=colorC & 0xFF;

	gsize=(st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE;

	sprintf(text2,"%s: %d",text, *value);

	lenght=strlen(text2);

	if(UI_Win[uiwinid].current!=pos)
	{
		if(CheckColisionMouse(UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),
			lenght*((st.fonts[UI_Win[uiwinid].font].size_w_gm*UI_Win[uiwinid].font_size)/FONT_SIZE),gsize,0))
		{
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rs,gs,bs,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

			if(st.mouse1)
			{
				UI_Win[uiwinid].current=pos;
				sprintf(st.TextInput,"%d",*value);
				StartText();
				st.mouse1=0;
			}
		}
		else
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rn,gn,bn,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);
	}
	if(UI_Win[uiwinid].current==pos)
	{
		StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rc,gc,bc,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

		*value=atoi(st.TextInput);

		if(st.keys[RETURN_KEY].state)
		{
			UI_Win[uiwinid].current=-1;
			StopText();
			st.keys[RETURN_KEY].state=0;
		}
	}
}

void UIWin2_NumberBoxui32(int8 uiwinid, int8 pos, uint32 *value, char *text, int32 colorN, int32 colorS, int32 colorC)
{
	int16 lenght, gsize;

	uint8 rn, gn, bn, rs, gs, bs, rc, gc, bc;

	char text2[32];

	rn=colorN>>16;
	gn=(colorN>>8) & 0xFF;
	bn=colorN & 0xFF;

	rs=colorS>>16;
	gs=(colorS>>8) & 0xFF;
	bs=colorS & 0xFF;

	rc=colorC>>16;
	gc=(colorC>>8) & 0xFF;
	bc=colorC & 0xFF;

	gsize=(st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE;

	sprintf(text2,"%s: %lu",text, *value);

	lenght=strlen(text2);

	if(UI_Win[uiwinid].current!=pos)
	{
		if(CheckColisionMouse(UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),
			lenght*((st.fonts[UI_Win[uiwinid].font].size_w_gm*UI_Win[uiwinid].font_size)/FONT_SIZE),gsize,0))
		{
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rs,gs,bs,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

			if(st.mouse1)
			{
				UI_Win[uiwinid].current=pos;
				sprintf(st.TextInput,"%lu",*value);
				StartText();
				st.mouse1=0;
			}
		}
		else
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rn,gn,bn,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);
	}
	else
	if(UI_Win[uiwinid].current==pos)
	{
		StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rc,gc,bc,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

		*value=atoi(st.TextInput);

		if(st.keys[RETURN_KEY].state)
		{
			StopText();
			st.keys[RETURN_KEY].state=0;
			UI_Win[uiwinid].current=-1;
		}
	}
}

void UIWin2_NumberBoxi32(int8 uiwinid, int8 pos, int32 *value, char *text, int32 colorN, int32 colorS, int32 colorC)
{
	int16 lenght, gsize;

	uint8 rn, gn, bn, rs, gs, bs, rc, gc, bc;

	char text2[32];

	rn=colorN>>16;
	gn=(colorN>>8) & 0xFF;
	bn=colorN & 0xFF;

	rs=colorS>>16;
	gs=(colorS>>8) & 0xFF;
	bs=colorS & 0xFF;

	rc=colorC>>16;
	gc=(colorC>>8) & 0xFF;
	bc=colorC & 0xFF;

	gsize=(st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE;

	sprintf(text2,"%s: %ld",text, *value);

	lenght=strlen(text2);

	if(UI_Win[uiwinid].current!=pos)
	{
		if(CheckColisionMouse(UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),
			lenght*((st.fonts[UI_Win[uiwinid].font].size_w_gm*UI_Win[uiwinid].font_size)/FONT_SIZE),gsize,0))
		{
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rs,gs,bs,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

			if(st.mouse1)
			{
				UI_Win[uiwinid].current=pos;
				sprintf(st.TextInput,"%ld",*value);
				StartText();
				st.mouse1=0;
			}
		}
		else
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rn,gn,bn,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);
	}
	else
	if(UI_Win[uiwinid].current==pos)
	{
		StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rc,gc,bc,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

		*value=atoi(st.TextInput);

		if(st.keys[RETURN_KEY].state)
		{
			StopText();
			st.keys[RETURN_KEY].state=0;
			UI_Win[uiwinid].current=-1;
		}
	}
}

void UIWin2_NumberBoxf(int8 uiwinid, int8 pos, float *value, char *text, int32 colorN, int32 colorS, int32 colorC)
{
	int16 lenght, gsize;

	uint8 rn, gn, bn, rs, gs, bs, rc, gc, bc;

	char text2[32];

	rn=colorN>>16;
	gn=(colorN>>8) & 0xFF;
	bn=colorN & 0xFF;

	rs=colorS>>16;
	gs=(colorS>>8) & 0xFF;
	bs=colorS & 0xFF;

	rc=colorC>>16;
	gc=(colorC>>8) & 0xFF;
	bc=colorC & 0xFF;

	gsize=(st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE;

	sprintf(text2,"%s: %.3f",text, *value);

	lenght=strlen(text2);

	if(UI_Win[uiwinid].current!=pos)
	{
		if(CheckColisionMouse(UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),
			lenght*((st.fonts[UI_Win[uiwinid].font].size_w_gm*UI_Win[uiwinid].font_size)/FONT_SIZE),gsize,0))
		{
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rs,gs,bs,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

			if(st.mouse1)
			{
				UI_Win[uiwinid].current=pos;
				sprintf(st.TextInput,"%.3f",*value);
				StartText();
				st.mouse1=0;
			}
		}
		else
			StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rn,gn,bn,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);
	}
	else
	if(UI_Win[uiwinid].current==pos)
	{
		StringUIData(text2,UI_Win[uiwinid].pos.x,(pos*gsize)+UI_Win[uiwinid].pos.y-((UI_Win[uiwinid].size.y/2)-128-gsize),0,0,0,rc,gc,bc,255,UI_Win[uiwinid].font,
				UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

		*value=atof(st.TextInput);

		if(st.keys[RETURN_KEY].state)
		{
			StopText();
			st.keys[RETURN_KEY].state=0;
			UI_Win[uiwinid].current=-1;
		}
	}
}

int8 UIWin_Button(int8 uiwinid, int32 x, int32 y, char *text, uint8 font, uint8 font_size, int32 color, int8 blocked)
{
	uint8 r, g, b, frame;
	int16 lenght, gsize, gsizew;

	r=color>>16;
	g=(color>>8) && 0xFF;
	b=color & 0xFF;

	gsize=(st.fonts[UI_Win[uiwinid].font].size_h_gm*UI_Win[uiwinid].font_size)/FONT_SIZE;
	gsizew=(st.fonts[UI_Win[uiwinid].font].size_w_gm*UI_Win[uiwinid].font_size)/FONT_SIZE;


	lenght=strlen(text);

	if(!blocked)
	{
		if(CheckColisionMouse(x,y,lenght*gsizew,gsize,0) && st.mouse1)
		{
			UIData(x,y,lenght*gsizew,gsize,0,r,g,b,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.button_frame1],255,UI_Win[uiwinid].layer-1);
			StringUIData(text,x,y,0,0,0,255,255,255,255,UI_Win[uiwinid].font,UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);

			st.mouse1=0;

			return UI_SEL;
		}
		else
		{
			UIData(x,y,lenght*gsizew,gsize,0,r,g,b,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.button_frame0],255,UI_Win[uiwinid].layer-1);
			StringUIData(text,x,y,0,0,0,255,255,255,255,UI_Win[uiwinid].font,UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);
		}
	}
	else
	if(blocked==1)
	{
		UIData(x,y,lenght*gsizew,gsize,0,r,g,b,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.button_frame2],255,UI_Win[uiwinid].layer-1);
		StringUIData(text,x,y,0,0,0,128,128,128,255,UI_Win[uiwinid].font,UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);
	}
	else
	if(blocked==2)
	{
		UIData(x,y,lenght*gsizew,gsize,0,r,g,b,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.button_frame2],255,UI_Win[uiwinid].layer-1);
		StringUIData(text,x,y,0,0,0,r,g,b,255,UI_Win[uiwinid].font,UI_Win[uiwinid].font_size,UI_Win[uiwinid].font_size,UI_Win[uiwinid].layer-1);
	}

	return UI_NULLOP;

}

int8 UIWin_ButtonIcon(int8 uiwinid, int32 x, int32 y, int32 sizex, int32 sizey, int8 frame, int32 color, int8 blocked)
{
	uint8 r, g, b;

	r=color>>16;
	g=(color>>8) && 0xFF;
	b=color & 0xFF;

	if(!blocked)
	{
		if(CheckColisionMouse(x,y,sizex,sizey,0) && st.mouse1)
		{
			UIData(x,y,sizex,sizey,0,r,g,b,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id2].frames[UI_Sys.frames[frame]],255,UI_Win[uiwinid].layer-1);

			st.mouse1=0;

			return UI_SEL;
		}
		else
			UIData(x,y,sizex,sizey,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id2].frames[UI_Sys.frames[frame]],255,UI_Win[uiwinid].layer-1);
	}
	else
	if(blocked==1)
		UIData(x,y,sizex,sizey,0,128,128,128,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id2].frames[UI_Sys.frames[frame]],255,UI_Win[uiwinid].layer-1);
	else
	if(blocked==2)
		UIData(x,y,sizex,sizey,0,r,g,b,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id2].frames[UI_Sys.frames[frame]],255,UI_Win[uiwinid].layer-1);

	return UI_NULLOP;
}

void UIMain_DrawSystem()
{
	register uint8 i;
	Pos p;

	if(st.num_uiwindow!=0)
	{
		for(i=0;i<MAX_UIWINDOWS;i++)
		{
			if(!UI_Win[i].stat)
				continue;
			else
			if(UI_Win[i].stat==2)
				DrawUI(UI_Win[i].pos.x,UI_Win[i].pos.y,UI_Win[i].size.x,UI_Win[i].size.y,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.window2_frame],255,UI_Win[i].layer);
			else
			if(UI_Win[i].stat==1)
				DrawUI(UI_Win[i].pos.x,UI_Win[i].pos.y,UI_Win[i].size.x,UI_Win[i].size.y,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id].frames[UI_Win[i].window_frame],255,UI_Win[i].layer);
		}
	}

	if(st.cursor_type!=0)
	{
		SDL_ShowCursor(SDL_DISABLE);
		
		p=st.mouse;

		p.x=(p.x*16384)/st.screenx;
		p.y=(p.y*8192)/st.screeny;

		if(st.cursor_type==1)
			DrawUI(p.x,p.y,512,512,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.resize_cursor],255,0);
		else
		if(st.cursor_type==2)
			DrawUI(p.x,p.y,512,512,900,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.resize_cursor],255,0);
		else
		if(st.cursor_type==3)
			DrawUI(p.x,p.y,512,512,450,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.resize_cursor],255,0);
		else
		if(st.cursor_type==4)
			DrawUI(p.x,p.y,512,512,1350,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.resize_cursor],255,0);
	}
	else
		SDL_ShowCursor(SDL_ENABLE);

	if(!st.mouse1)
		UI_Sys.mouse_flag=0;
}


int8 Sys_ResizeController(int32 x, int32 y, int32 *sizex, int32 *sizey, uint8 keepaspect, int16 ang, int8 z)
{
	int32 sx=*sizex/2, sy=*sizey/2;
	static Pos size, size2;

	//Vertices first

	if(st.mouse1)
	{
		if(CheckColisionMouseWorld(x-sx,y-sy,256,256,ang,z) && !UI_Sys.mouse_flag)
		{
			st.cursor_type=CURSOR_RESIZE_DRIGHT;

			size=st.mouse;
			UI_Sys.mouse_flag=1;
		}
		else
		if(CheckColisionMouseWorld(x+sx,y-sy,256,256,ang,z) && !UI_Sys.mouse_flag)
		{
			st.cursor_type=CURSOR_RESIZE_DLEFT;

			size=st.mouse;
			UI_Sys.mouse_flag=2;
		}
		else
		if(CheckColisionMouseWorld(x+sx,y+sy,256,256,ang,z) && !UI_Sys.mouse_flag)
		{
			st.cursor_type=CURSOR_RESIZE_DRIGHT;

			size=st.mouse;
			UI_Sys.mouse_flag=3;
		}
		else
		if(CheckColisionMouseWorld(x-sx,y+sy,256,256,ang,z) && !UI_Sys.mouse_flag)
		{
			st.cursor_type=CURSOR_RESIZE_DLEFT;

			size=st.mouse;
			UI_Sys.mouse_flag=4;
		}
		else
		if(CheckColisionMouseWorld(x,y-sy,*sizex,256,ang,z) && !UI_Sys.mouse_flag && !keepaspect)
		{
			st.cursor_type=CURSOR_RESIZE_VERTICAL;

			size=st.mouse;
			UI_Sys.mouse_flag=5;
		}
		else
		if(CheckColisionMouseWorld(x,y+sy,*sizex,256,ang,z) && !UI_Sys.mouse_flag && !keepaspect)
		{
			st.cursor_type=CURSOR_RESIZE_VERTICAL;

			size=st.mouse;
			UI_Sys.mouse_flag=6;
		}
		else
		if(CheckColisionMouseWorld(x-sx,y,256,*sizey,ang,z) && !UI_Sys.mouse_flag && !keepaspect)
		{
			st.cursor_type=CURSOR_RESIZE_HORIZONTAL;

			size=st.mouse;
			UI_Sys.mouse_flag=7;
		}
		else
		if(CheckColisionMouseWorld(x+sx,y,256,*sizey,ang,z) && !UI_Sys.mouse_flag && !keepaspect)
		{
			st.cursor_type=CURSOR_RESIZE_HORIZONTAL;

			size=st.mouse;
			UI_Sys.mouse_flag=8;
		}
		
		if(CheckColisionMouseWorld(x-sx,y-sy,256,256,ang,z) && UI_Sys.mouse_flag==1)
		{
			size2=st.mouse;

			STW(&size2.x,&size2.y);
			STW(&size.x,&size.y);

			size2.x-=size.x;
			size2.y-=size.y;

			*sizex-=size2.x*2;
			*sizey-=size2.y*2;

			size=st.mouse;
		}
		else
		if(CheckColisionMouseWorld(x+sx,y-sy,256,256,ang,z) && UI_Sys.mouse_flag==2)
		{
			size2=st.mouse;

			STW(&size2.x,&size2.y);
			STW(&size.x,&size.y);

			size2.x-=size.x;
			size2.y-=size.y;

			*sizex+=size2.x*2;
			*sizey-=size2.y*2;

			size=st.mouse;
		}
		else
		if(CheckColisionMouseWorld(x+sx,y+sy,256,256,ang,z) && UI_Sys.mouse_flag==3)
		{
			size2=st.mouse;

			STW(&size2.x,&size2.y);
			STW(&size.x,&size.y);

			size2.x-=size.x;
			size2.y-=size.y;

			*sizex+=size2.x*2;
			*sizey+=size2.y*2;

			size=st.mouse;
		}
		else
		if(CheckColisionMouseWorld(x-sx,y+sy,256,256,ang,z) && UI_Sys.mouse_flag==4)
		{
			size2=st.mouse;

			STW(&size2.x,&size2.y);
			STW(&size.x,&size.y);

			size2.x-=size.x;
			size2.y-=size.y;

			*sizex-=size2.x*2;
			*sizey+=size2.y*2;

			size=st.mouse;
		}
		else
		if(CheckColisionMouseWorld(x,y-sy,*sizex,256,ang,z) && UI_Sys.mouse_flag==5)
		{
			size2=st.mouse;

			STW(&size2.x,&size2.y);
			STW(&size.x,&size.y);

			size2.x-=size.x;
			size2.y-=size.y;

			*sizey-=size2.y*2;

			size=st.mouse;
		}
		else
		if(CheckColisionMouseWorld(x,y+sy,*sizex,256,ang,z) && UI_Sys.mouse_flag==6)
		{
			size2=st.mouse;

			STW(&size2.x,&size2.y);
			STW(&size.x,&size.y);

			size2.x-=size.x;
			size2.y-=size.y;

			*sizey+=size2.y*2;

			size=st.mouse;
		}
		else
		if(CheckColisionMouseWorld(x-sx,y,256,*sizey,ang,z) && UI_Sys.mouse_flag==7)
		{
			size2=st.mouse;

			STW(&size2.x,&size2.y);
			STW(&size.x,&size.y);

			size2.x-=size.x;
			size2.y-=size.y;

			*sizex-=size2.x*2;

			size=st.mouse;
		}
		else
		if(CheckColisionMouseWorld(x+sx,y,256,*sizey,ang,z) && UI_Sys.mouse_flag==8)
		{
			size2=st.mouse;

			STW(&size2.x,&size2.y);
			STW(&size.x,&size.y);

			size2.x-=size.x;
			size2.y-=size.y;

			*sizex+=size2.x*2;

			size=st.mouse;
		}
	}
	else
	{
		UI_Sys.mouse_flag=0;
		st.cursor_type=CURSOR_NORMAL;
	}

	if(UI_Sys.mouse_flag>0)
		return 1;
	else
		return 0;
}

void Sys_ColorPicker(uint8 *r, uint8 *g, uint8 *b)
{
	Pos p;

	UIData(14336,1536,4096,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[10],255,0);

	if(CheckColisionMouse(14336,1536,4096,2048,0) && st.mouse1)
	{
		p=st.mouse;

		glReadPixels(p.x,st.screeny-p.y-1,1,1,GL_RED,GL_UNSIGNED_BYTE,r);
		glReadPixels(p.x,st.screeny-p.y-1,1,1,GL_GREEN,GL_UNSIGNED_BYTE,g);
		glReadPixels(p.x,st.screeny-p.y-1,1,1,GL_BLUE,GL_UNSIGNED_BYTE,b);
	}
}

