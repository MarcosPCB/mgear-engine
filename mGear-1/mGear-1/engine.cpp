#include "engine.h"
#include <math.h>
#include "quicklz.h"
#include "input.h"
#include <SDL_image.h>

_SETTINGS st;
_ENTITIES ent[MAX_GRAPHICS];
SDL_Event events;

_MGG movie;
_MGG mgg[MAX_MGG];

SDL_Window *wn;

#define _ENGINE_VERSION 0.5
const char WindowTitle[32]={"mGear-1 Engine PRE-ALPHA"};

#define timer SDL_Delay

double inline __declspec (naked) __fastcall sqrt14(double n)
{
	_asm fld qword ptr [esp+4]
	_asm fsqrt
	_asm ret 8
} 

void LogIn(void *userdata, int category, SDL_LogPriority, const char *message)
{
	FILE *file;
	size_t size;
	if((file=fopen("mgear.log","a+"))==NULL)
	{
		if(MessageBox(NULL,L"Opening log file failed",NULL,MB_OK | MB_ICONERROR)==IDOK)
			Quit();
	}

	fseek(file,0,SEEK_END);
	fprintf(file,"%s\n",message);

	fclose(file);
}

void CreateLog()
{
	FILE *file;
	float version=_ENGINE_VERSION;
	if((file=fopen("mgear.log","w"))==NULL)
	{
		if(MessageBox(NULL,L"Create log failed",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			exit(1);
	}

	SDL_LogSetOutputFunction(&LogIn,NULL);

	fprintf(file,"%s %.2f\n",WindowTitle,version);
	fprintf(file,"Engine started\n");
	fprintf(file,"Log created\n");

	fclose(file);
}

void Timer()
{
	st.time++;
	timer(1000/TICSPERSECOND);
}

void FPSCounter()
{
	if((SDL_GetTicks() - st.FPSTime)!=1000)
	{
		st.FPS=SDL_GetTicks()-st.FPSTime;
		st.FPS=1000/st.FPS;
		sprintf(st.WINDOW_NAME,"%s fps: %.2f",WindowTitle,st.FPS);
		st.FPS=0;
		st.FPSTime=SDL_GetTicks();
		SDL_SetWindowTitle(wn,st.WINDOW_NAME);
	}
}

void _fastcall STW(double *x, double *y)
{
	*x=((((*x*16384)/st.screenx)/st.Camera.dimension.x)+st.Camera.position.x);
	*y=((((*y*8192)/st.screeny)/st.Camera.dimension.y)+st.Camera.position.y);
}

uint32 POT(uint32 value)
{
	if(value != 0)
    {
        value--;
        value |= (value >> 1); //Or first 2 bits
        value |= (value >> 2); //Or next 2 bits
        value |= (value >> 4); //Or next 4 bits
        value |= (value >> 8); //Or next 8 bits
        value |= (value >> 16); //Or next 16 bits
        value++;
    }

	return value;
}

void _fastcall WTS(double *x, double *y)
{
	*x-=st.Camera.position.x;
	*y-=st.Camera.position.y;

	*x=((*x*st.screenx)/16384)*st.Camera.dimension.x;
	*y=((*y*st.screeny)/8192)*st.Camera.dimension.y;
}

void Quit()
{
	InputClose();
	SDL_DestroyWindow(wn);
	SDL_Quit();
	FMOD_System_Close(st.sound_sys.Sound_System);
	FMOD_System_Release(st.sound_sys.Sound_System);
	TTF_Quit();
	exit(1);
}

void Init()
{	
	CreateLog();

	int check;
	FMOD_RESULT result;

	//Initialize SDL
	if((SDL_Init(SDL_INIT_EVERYTHING))!=NULL)
	{
		LogApp("SDL Initilization failed %s",SDL_GetError());
			Quit();
	}

	LogApp("SDL 2.0 initialzed");
		
	if((result=FMOD_System_Create(&st.sound_sys.Sound_System))!=FMOD_OK)
	{
		LogApp("Error while initializing FMOD, Creating System : %s",FMOD_ErrorString(result));
			Quit();
	}
	LogApp("FMOD system created");
	
	if((result=FMOD_System_Init(st.sound_sys.Sound_System,MAX_CHANNELS,FMOD_INIT_NORMAL,NULL))!=FMOD_OK)
	{
		LogApp("Error while initializing FMOD : %s",FMOD_ErrorString(result));
			Quit();
	}

	LogApp("FMOD system initialzed, %d channels",MAX_CHANNELS);
	
	for(register uint8 i=0;i<MAX_SOUNDS;i++)
		st.sound_sys.slot_ID[i]=-1;

	for(register uint8 i=0;i<MAX_CHANNELS;i++)
		st.sound_sys.slotch_ID[i]=-1;

	if(TTF_Init()==-1)
	{
		LogApp("Error while initializing SDL TTF : %s",TTF_GetError());
			Quit();
	}

	LogApp("SDL TTF initialized");
	
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	//Set video mode
	if(st.fullscreen)
	{
		if((wn=SDL_CreateWindow(st.WINDOW_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, st.screenx, st.screeny, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL ))==NULL)
		{
			LogApp("Error setting fullscreen video mode %d x %d %d bits - %s",st.screenx,st.screeny,st.bpp,SDL_GetError());
				Quit();
		}
	}
	else
	if(!st.fullscreen)
	{
		if((wn=SDL_CreateWindow(st.WINDOW_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, st.screenx, st.screeny, SDL_WINDOW_OPENGL))==NULL)
		{
			LogApp("Error setting widowed video mode %d x %d %d bits - %s",st.screenx,st.screeny,st.bpp,SDL_GetError());
				Quit();
		}
	}

	LogApp("Window created, %d x %d, %d bits",st.screenx,st.screeny,st.bpp);

	if((st.glc=SDL_GL_CreateContext(wn))==NULL)
	{
		LogApp("Error setting renderer: %s",SDL_GetError());
				Quit();
	}

	LogApp("Opengl context created");
	
	//Initialize OpenGL
	glClearColor(0,0,0,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,st.screenx,st.screeny,0,0,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);

	LogApp("Opengl initialized");

	if(strstr((char const*) glGetString(GL_EXTENSIONS),"GL_ARB_texture_non_power_of_two")==NULL)
	{
		st.LOWRES=1;
		LogApp("Non power of two textures not supported, loading times might increase and video's fps might decrease");
	}

	if(strstr((char const*) glGetString(GL_EXTENSIONS),"GL_ARB_texture_rectangle")==NULL && strstr((char const*) glGetString(GL_EXTENSIONS),"GL_NV_texture_rectangle")==NULL && strstr((char const*) glGetString(GL_EXTENSIONS),"GL_NV_texture_rectangle")==NULL)
	{
		LogApp("Rectangle textures not supported, your video card is not supported or try updating your driver");
		Quit();
	}

	st.quit=0;

	st.time=0;

	st.PlayingVideo=0;

	InputInit();

	LogApp("Input initialized");

	st.Camera.position.x=0;
	st.Camera.position.y=0;
	st.Camera.dimension.x=1;
	st.Camera.dimension.y=1;
	st.Camera.angle=0.0;

	st.Current_Map.num_lights=0;
	st.Current_Map.num_mgg=0;
	st.Current_Map.num_obj=0;
	st.Current_Map.num_sector=0;
	st.Current_Map.num_sprites=0;

	st.num_hud=0;
	st.num_tex=0;
	st.num_ui=0;

	st.num_sprites=0;

	memset(&ent,0,MAX_GRAPHICS*sizeof(_ENTITIES));

}

uint8 OpenFont(const char *file,const char *name, uint8 index)
{
	if((st.fonts[index].font=TTF_OpenFont(file,64))==NULL)
	{
		LogApp("Error while opening TTF font : %s",TTF_GetError());
		return 0;
	}
	
	strcpy(st.fonts[index].name,name);

	return 1;
}

void RestartVideo()
{
	
	LogApp("Video restarted");

	SDL_DestroyWindow(wn);

	wn=SDL_CreateWindow(st.WINDOW_NAME,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,st.screenx,st.screeny, st.fullscreen==1 ? SDL_WINDOW_FULLSCREEN : NULL | SDL_WINDOW_OPENGL);

	if(wn==NULL)
	{
		if(st.fullscreen) LogApp("Error setting fullscreen video mode %d x %d %d bits - %s",st.screenx,st.screeny,st.bpp,SDL_GetError());
		else LogApp("Error setting windowed video mode %d x %d %d bits - %s",st.screenx,st.screeny,st.bpp,SDL_GetError());
	}

	LogApp("Window created, %d x %d, %d bits",st.screenx,st.screeny,st.bpp);

	SDL_GL_MakeCurrent(wn,st.glc);

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	glViewport(0,0,st.screenx,st.screeny);

	glClearColor(0,0,0,0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,st.screenx,st.screeny,0,0,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	
	if(SDL_GetRelativeMouseMode())
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	
}

static FILE *DecompressFile(const char *name)
{
	FILE *f;

	if((f=fopen(name,"rb"))==NULL)
		return NULL;

	FILE *file=tmpfile();
	size_t len;

	char *buf, *buf2;

	qlz_state_decompress *decomp=(qlz_state_decompress*) malloc(sizeof(qlz_state_decompress));

	fseek(f,0,SEEK_END);
	len=ftell(f);
	rewind(f);
	buf=(char*) malloc(len);
	fread(buf,len,1,f);
	len=qlz_size_decompressed(buf);

	buf2=(char*) malloc(len);
	len=qlz_decompress(buf,buf2,decomp);
	fwrite(buf2,len,1,file);
	fclose(f);

	free(buf);
	free(buf2);

	return file;
}

void createmgv()
{
	FILE *file, *file2, *file3;

	//file=tmpfile();

	if((file=fopen("movie.mgv","wb"))==NULL)
		if(MessageBox(NULL,L"Error creating MGG file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	_MGVFORMAT mgv;

	mgv.num_frames=290;
	mgv.fps=30;

	if((file3=fopen("movie.wav","rb"))==NULL)
		if(MessageBox(NULL,L"Error opening sound file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	fseek(file3,0,SEEK_END);
	mgv.sound_buffer_lenght=ftell(file3);

	rewind(file);
	fseek(file,0,SEEK_SET);

	fwrite(&mgv,sizeof(_MGVFORMAT),1,file);
	fseek(file,(sizeof(_MGVFORMAT)+512),SEEK_SET);
	
	uint32 *framesize;

	framesize=(uint32*) malloc(mgv.num_frames*sizeof(uint32));

	size_t totalsize;
	totalsize=((sizeof(_MGVFORMAT)+512)+(mgv.num_frames*sizeof(uint32)+512));

	uint32 j=0;
	for(register uint32 i=0;i<mgv.num_frames+1;i++)
	{
		//if(i==33) j=42;
		char filename[128];
		if(j<10) sprintf(filename,"logo\\frame000%d.tga",j); else
		if(j<100) sprintf(filename,"logo\\frame00%d.tga",j); else
		if(j<1000) sprintf(filename,"logo\\frame0%d.tga",j);

		if((file2=fopen(filename,"rb"))==NULL)
		{
			MessageBox(NULL,L"Error reading 0 file",NULL,MB_OK | MB_ICONERROR);
			continue;
		}
		else
		{
			fseek(file2,0,SEEK_END);

			size_t size=ftell(file2);
			rewind(file2);

			framesize[i]=size;
	
			void *buf=(void*) malloc(size);
			fread(buf,size,1,file2);
			rewind(file);

			if(i==0) fseek(file,totalsize,SEEK_CUR); 
			else
			{
				totalsize+=(framesize[i-1]+2048);
				fseek(file,totalsize,SEEK_CUR);
			}

			fwrite(buf,size,1,file);
			fclose(file2);
			free(buf);
		}
		j++;
	}

	//j=42;
	/*
	for(register uint32 i=33;i<53;i++)
	{
		char filename[128];
		if(j<10) sprintf(filename,"test\\frame000%d.tga",j); else
		if(j<100) sprintf(filename,"test\\frame00%d.tga",j); else
		if(j<1000) sprintf(filename,"test\\frame0%d.tga",j);

		if((file2=fopen(filename,"rb"))==NULL)
		{
			MessageBox(NULL,L"Error reading 0 file",NULL,MB_OK | MB_ICONERROR);
			continue;
		}
		else
		{
			fseek(file2,0,SEEK_END);

			size_t size=ftell(file2);
			rewind(file2);

			framesize[i]=size;
	
			char *buf=(char*) malloc(size);
			fread(buf,size,1,file2);
			rewind(file);

			if(i==0) fseek(file,totalsize,SEEK_CUR); 
			else
			{
				totalsize+=(framesize[i-1]+2048);
				fseek(file,totalsize,SEEK_CUR);
			}

			fwrite(buf,size,1,file);
			fclose(file2);
			free(buf);
		}
		j++;
	}
	*/
	rewind(file);
	fseek(file,((sizeof(_MGVFORMAT)+512)),SEEK_CUR);
	fwrite(framesize,sizeof(uint32),mgv.num_frames,file);

	rewind(file);
	fseek(file,totalsize+512,SEEK_CUR);

	rewind(file3);
	
	void *buffer=(void*) malloc(mgv.sound_buffer_lenght);
	fread(buffer,mgv.sound_buffer_lenght,1,file3);
	fwrite(buffer,mgv.sound_buffer_lenght,1,file);

	free(buffer);

	fclose(file3);

	fclose(file);

}

uint32 CheckMGGFile(const char *name)
{
	FILE *file;
	char header[21];

	if((file=DecompressFile(name))==NULL)
	{
		LogApp("Error reading MGG file %s",name);
			return false;
	}

	_MGGFORMAT mggf;

	rewind(file);

	fread(header,21,1,file);

	if(strcmp(header,"MGG File Version 1.0")!=NULL)
	{
		LogApp("Invalid MGG file header %s",name);
		fclose(file);
		return false;
	}

	rewind(file);

	fread(&mggf,sizeof(_MGGFORMAT),1,file);

	if((mggf.type!=SPRITEM && mggf.type!=TEXTUREM && mggf.type!=NONE) || (mggf.num_animations<0 || mggf.num_animations>MAX_ANIMATIONS) || (mggf.num_frames<0 || mggf.num_frames>MAX_FRAMES))
	{
		fclose(file);
		LogApp("Invalid MGG file %s",name);
		return false;
	}

	fclose(file);
	return true;
}

uint32 LoadMGG(_MGG *mgg, const char *name)
{
	FILE *file, *file2;
	void *data;

	char header[21];

	if((file=DecompressFile(name))==NULL)
	{
		LogApp("Error reading MGG file %s",name);
			return false;
	}

	_MGGFORMAT mggf;

	rewind(file);

	fread(header,21,1,file);

	if(strcmp(header,"MGG File Version 1.0")!=NULL)
	{
		LogApp("Invalid MGG file header %s",header);
		fclose(file);
		return false;
	}

	rewind(file);

	fseek(file,21,SEEK_SET);

	fread(&mggf,sizeof(_MGGFORMAT),1,file);

	if((mggf.type!=SPRITEM && mggf.type!=TEXTUREM && mggf.type!=NONE) || (mggf.num_animations<0 || mggf.num_animations>MAX_ANIMATIONS) || (mggf.num_frames<0 || mggf.num_frames>MAX_FRAMES))
	{
		fclose(file);
		LogApp("Invalid MGG file info %s",name);
		return false;
	}

	strcpy(mgg->name,mggf.name);

	mgg->num_frames=mggf.num_frames;

	mgg->type=mggf.type;
	
	mgg->num_anims=2;

	_MGGANIM *mga;

	uint32 framesize[MAX_FRAMES];

	mga=(_MGGANIM*) malloc(mgg->num_anims*sizeof(_MGGANIM));
	mgg->anim=(_MGGANIM*) malloc(mgg->num_anims*sizeof(_MGGANIM));

	rewind(file);
	fseek(file,((sizeof(_MGGFORMAT)+512))+21,SEEK_CUR);
	fread(mga,sizeof(_MGGANIM),mgg->num_anims,file);

	for(register uint16 i=0;i<mgg->num_anims;i++)
		mgg->anim[i]=mga[i];

	fseek(file,((sizeof(_MGGFORMAT)+512)+(512+(MAX_ANIMATIONS*sizeof(_MGGANIM))))+21,SEEK_SET);
	fread(framesize,sizeof(uint32),mgg->num_frames,file);

	size_t totalsize=((sizeof(_MGGFORMAT)+512)+(512+MAX_FRAMES*sizeof(uint32))+(512+(MAX_ANIMATIONS*sizeof(_MGGANIM))))+21;

	mgg->frames=(GLuint*) malloc(mgg->num_frames*sizeof(GLuint));
	mgg->size=(Pos*) malloc(mgg->num_frames*sizeof(Pos));
	mgg->sizefix=(Pos*) malloc(mgg->num_frames*sizeof(Pos));

	for(register uint32 i=0;i<mgg->num_frames;i++)
	{
		rewind(file);
		if(i==0) fseek(file,totalsize,SEEK_CUR);
		else
		{
			totalsize+=(framesize[i-1]+2048);
			fseek(file,totalsize,SEEK_CUR);
		}

		data=malloc(framesize[i]);

		if(data==NULL)
		{
			LogApp("Error allocating memory for texture %d, size %d, file %s",i,framesize[i],name);
			continue;
		}

		fread(data,framesize[i],1,file);

		mgg->frames[i]=SOIL_load_OGL_texture_from_memory((unsigned char*)data,framesize[i],SOIL_LOAD_AUTO,0,SOIL_FLAG_TEXTURE_REPEATS);

		if(mgg->frames[i]==NULL)
			LogApp("Error loading texture from memory");
		
		if (data)						
			free(data);
	}

	fclose(file);

	return true;
		
}

void FreeMGG(_MGG *file)
{
	file->type=NONE;

	for(register uint32 i=0; i<file->num_frames; i++)
	{
		glDeleteTextures(1, &file->frames[i]);
		file->size[i].x=NULL;
		file->size[i].y=NULL;
	}

	free(mgg->frames);
	free(mgg->anim);
	free(mgg->size);

	file->num_frames=NULL;
	
	file->num_anims=NULL;

	memset(mgg,0,sizeof(_MGG));
}

void InitMGG()
{
	for(register uint8 i=0; i<MAX_MGG; i++)
	{
		memset(&mgg[i],0,sizeof(_MGG));
		mgg[i].type=NONE;
	}
}

uint8 CheckCollisionSector(double x, double y, double xsize, double ysize, float ang, Pos vert[4])
{
	double xb, xl, yb, yl, xtb, xtl, ytb, ytl, tmpx, tmpy;

	x-=st.Camera.position.x;
	y-=st.Camera.position.y;

	vert[0].x-=st.Camera.position.x;
	vert[0].y-=st.Camera.position.y;

	vert[1].x-=st.Camera.position.x;
	vert[1].y-=st.Camera.position.y;

	vert[2].x-=st.Camera.position.x;
	vert[2].y-=st.Camera.position.y;

	vert[3].x-=st.Camera.position.x;
	vert[3].y-=st.Camera.position.y;

	for(register uint8 i=0;i<4;i++)
	{
			if(i==0)
			{
				tmpx=x+(((x-(xsize/2))-x)*cos((ang*pi)/180) - ((y-(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x-(xsize/2))-x)*sin((ang*pi)/180) + ((y-(ysize/2))-y)*cos((ang*pi)/180));
				xb=xl=tmpx;
				yb=yl=tmpy;
			}
			else
			if(i==1)
			{
				tmpx=x+(((x+(xsize/2))-x)*cos((ang*pi)/180) - ((y-(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x+(xsize/2))-x)*sin((ang*pi)/180) + ((y-(ysize/2))-y)*cos((ang*pi)/180));
				if(tmpx>xb) xb=tmpx;
				else if(tmpx<xl) xl=tmpx;

				if(tmpy>yb) yb=tmpy;
				else if(tmpy<yl) yl=tmpy;
			}
			else
			if(i==2)
			{
				tmpx=x+(((x+(xsize/2))-x)*cos((ang*pi)/180) - ((y+(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x+(xsize/2))-x)*sin((ang*pi)/180) + ((y+(ysize/2))-y)*cos((ang*pi)/180));
				if(tmpx>xb) xb=tmpx;
				else if(tmpx<xl) xl=tmpx;

				if(tmpy>yb) yb=tmpy;
				else if(tmpy<yl) yl=tmpy;
			}
			else
			if(i==3)
			{
				tmpx=x+(((x-(xsize/2))-x)*cos((ang*pi)/180) - ((y+(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x-(xsize/2))-x)*sin((ang*pi)/180) + ((y+(ysize/2))-y)*cos((ang*pi)/180));
				if(tmpx>xb) xb=tmpx;
				else if(tmpx<xl) xl=tmpx;

				if(tmpy>yb) yb=tmpy;
				else if(tmpy<yl) yl=tmpy;
			}
	}

	for(register uint8 i=0;i<4;i++)
	{
		if(i==0)
		{
			xtb=xtl=vert[i].x;
			ytb=ytl=vert[i].y;
		}
		else
		{
			if(vert[i].x<xtl) xtl=vert[i].x;
			else if(vert[i].x>xtb) xtb=vert[i].x;

			if(vert[i].y<ytl) ytl=vert[i].y;
			else if(vert[i].y>ytb) ytb=vert[i].y;
		}
	}

	if((xtb>xl && xtb<xb && ytb>yl && ytb<yb) || (xtl>xl && xtl<xb && ytl>yl && ytl<yb))
		return 1; //Collided
	else
		return 0; //No collision
}

uint8 CheckColision(double x, double y, double xsize, double ysize, double tx, double ty, double txsize, double tysize, float ang, float angt)
{
	double xb, xl, yb, yl, xtb, xtl, ytb, ytl, tmpx, tmpy;

	x-=st.Camera.position.x;
	y-=st.Camera.position.y;

	tx-=st.Camera.position.x;
	ty-=st.Camera.position.y;

	for(register uint8 i=0;i<8;i++)
	{
		if(i<4)
		{
			if(i==0)
			{
				tmpx=x+(((x-(xsize/2))-x)*cos((ang*pi)/180) - ((y-(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x-(xsize/2))-x)*sin((ang*pi)/180) + ((y-(ysize/2))-y)*cos((ang*pi)/180));
				xb=xl=tmpx;
				yb=yl=tmpy;
			}
			else
			if(i==1)
			{
				tmpx=x+(((x+(xsize/2))-x)*cos((ang*pi)/180) - ((y-(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x+(xsize/2))-x)*sin((ang*pi)/180) + ((y-(ysize/2))-y)*cos((ang*pi)/180));
				if(tmpx>xb) xb=tmpx;
				else if(tmpx<xl) xl=tmpx;

				if(tmpy>yb) yb=tmpy;
				else if(tmpy<yl) yl=tmpy;
			}
			else
			if(i==2)
			{
				tmpx=x+(((x+(xsize/2))-x)*cos((ang*pi)/180) - ((y+(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x+(xsize/2))-x)*sin((ang*pi)/180) + ((y+(ysize/2))-y)*cos((ang*pi)/180));
				if(tmpx>xb) xb=tmpx;
				else if(tmpx<xl) xl=tmpx;

				if(tmpy>yb) yb=tmpy;
				else if(tmpy<yl) yl=tmpy;
			}
			else
			if(i==3)
			{
				tmpx=x+(((x-(xsize/2))-x)*cos((ang*pi)/180) - ((y+(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x-(xsize/2))-x)*sin((ang*pi)/180) + ((y+(ysize/2))-y)*cos((ang*pi)/180));
				if(tmpx>xb) xb=tmpx;
				else if(tmpx<xl) xl=tmpx;

				if(tmpy>yb) yb=tmpy;
				else if(tmpy<yl) yl=tmpy;
			}
		}
		else
		if(i>3)
		{
			if(i==4)
			{
				tmpx=tx+(((tx-(txsize/2))-tx)*cos((angt*pi)/180) - ((ty-(tysize/2))-ty)*sin((angt*pi)/180));
				tmpy=ty+(((tx-(txsize/2))-tx)*sin((angt*pi)/180) + ((ty-(tysize/2))-ty)*cos((angt*pi)/180));
				xtb=xtl=tmpx;
				ytb=ytl=tmpy;
			}
			else
			if(i==5)
			{
				tmpx=tx+(((tx+(txsize/2))-tx)*cos((angt*pi)/180) - ((ty-(tysize/2))-ty)*sin((angt*pi)/180));
				tmpy=ty+(((tx+(txsize/2))-tx)*sin((angt*pi)/180) + ((ty-(tysize/2))-ty)*cos((angt*pi)/180));
				if(tmpx>xtb) xtb=tmpx;
				else if(tmpx<xtl) xtl=tmpx;

				if(tmpy>ytb) ytb=tmpy;
				else if(tmpy<ytl) ytl=tmpy;
			}
			else
			if(i==6)
			{
				tmpx=tx+(((tx+(txsize/2))-tx)*cos((angt*pi)/180) - ((ty+(tysize/2))-ty)*sin((angt*pi)/180));
				tmpy=ty+(((tx+(txsize/2))-tx)*sin((angt*pi)/180) + ((ty+(tysize/2))-ty)*cos((angt*pi)/180));
				if(tmpx>xtb) xtb=tmpx;
				else if(tmpx<xtl) xtl=tmpx;

				if(tmpy>ytb) ytb=tmpy;
				else if(tmpy<ytl) ytl=tmpy;
			}
			else
			if(i==7)
			{
				tmpx=tx+(((tx-(txsize/2))-tx)*cos((angt*pi)/180) - ((ty+(tysize/2))-ty)*sin((angt*pi)/180));
				tmpy=ty+(((tx-(txsize/2))-tx)*sin((angt*pi)/180) + ((ty+(tysize/2))-ty)*cos((angt*pi)/180));
				if(tmpx>xtb) xtb=tmpx;
				else if(tmpx<xtl) xtl=tmpx;

				if(tmpy>ytb) ytb=tmpy;
				else if(tmpy<ytl) ytl=tmpy;
			}
		}
	}

	if((xtb>xl && xtb<xb && ytb>yl && ytb<yb) || (xtl>xl && xtl<xb && ytl>yl && ytl<yb))
		return 1; //Collided
	else
		return 0; //No collision
	
}

uint8 CheckColisionMouse(double x, double y, double xsize, double ysize, float ang)
{

	double xb, xl, yb, yl, xtb, xtl, ytb, ytl, tmpx, tmpy;

	for(register uint8 i=0;i<4;i++)
	{
			if(i==0)
			{
				tmpx=x+(((x-(xsize/2))-x)*cos((ang*pi)/180) - ((y-(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x-(xsize/2))-x)*sin((ang*pi)/180) + ((y-(ysize/2))-y)*cos((ang*pi)/180));
				xb=xl=tmpx;
				yb=yl=tmpy;
			}
			else
			if(i==1)
			{
				tmpx=x+(((x+(xsize/2))-x)*cos((ang*pi)/180) - ((y-(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x+(xsize/2))-x)*sin((ang*pi)/180) + ((y-(ysize/2))-y)*cos((ang*pi)/180));
				if(tmpx>xb) xb=tmpx;
				else if(tmpx<xl) xl=tmpx;

				if(tmpy>yb) yb=tmpy;
				else if(tmpy<yl) yl=tmpy;
			}
			else
			if(i==2)
			{
				tmpx=x+(((x+(xsize/2))-x)*cos((ang*pi)/180) - ((y+(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x+(xsize/2))-x)*sin((ang*pi)/180) + ((y+(ysize/2))-y)*cos((ang*pi)/180));
				if(tmpx>xb) xb=tmpx;
				else if(tmpx<xl) xl=tmpx;

				if(tmpy>yb) yb=tmpy;
				else if(tmpy<yl) yl=tmpy;
			}
			else
			if(i==3)
			{
				tmpx=x+(((x-(xsize/2))-x)*cos((ang*pi)/180) - ((y+(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x-(xsize/2))-x)*sin((ang*pi)/180) + ((y+(ysize/2))-y)*cos((ang*pi)/180));
				if(tmpx>xb) xb=tmpx;
				else if(tmpx<xl) xl=tmpx;

				if(tmpy>yb) yb=tmpy;
				else if(tmpy<yl) yl=tmpy;
			}
	}

	if(st.mouse.x>xl && st.mouse.x<xb && st.mouse.y>yl && st.mouse.y<yb)
		return 1; //Collided
	else
		return 0; //No collision
	
}

uint8 CheckColisionMouseWorld(double x, double y, double xsize, double ysize, float ang)
{

	double xb, xl, yb, yl, xtb, xtl, ytb, ytl, tmpx, tmpy;

	x-=st.Camera.position.x;
	y-=st.Camera.position.y;

	x=((x*st.screenx)/16384)*st.Camera.dimension.x;
	y=((y*st.screeny)/8192)*st.Camera.dimension.y;

	xsize=((xsize*st.screenx)/16384)*st.Camera.dimension.x;
	ysize=((ysize*st.screeny)/8192)*st.Camera.dimension.y;

	for(register uint8 i=0;i<4;i++)
	{
			if(i==0)
			{
				tmpx=x+(((x-(xsize/2))-x)*cos((ang*pi)/180) - ((y-(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x-(xsize/2))-x)*sin((ang*pi)/180) + ((y-(ysize/2))-y)*cos((ang*pi)/180));
				xb=xl=tmpx;
				yb=yl=tmpy;
			}
			else
			if(i==1)
			{
				tmpx=x+(((x+(xsize/2))-x)*cos((ang*pi)/180) - ((y-(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x+(xsize/2))-x)*sin((ang*pi)/180) + ((y-(ysize/2))-y)*cos((ang*pi)/180));
				if(tmpx>xb) xb=tmpx;
				else if(tmpx<xl) xl=tmpx;

				if(tmpy>yb) yb=tmpy;
				else if(tmpy<yl) yl=tmpy;
			}
			else
			if(i==2)
			{
				tmpx=x+(((x+(xsize/2))-x)*cos((ang*pi)/180) - ((y+(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x+(xsize/2))-x)*sin((ang*pi)/180) + ((y+(ysize/2))-y)*cos((ang*pi)/180));
				if(tmpx>xb) xb=tmpx;
				else if(tmpx<xl) xl=tmpx;

				if(tmpy>yb) yb=tmpy;
				else if(tmpy<yl) yl=tmpy;
			}
			else
			if(i==3)
			{
				tmpx=x+(((x-(xsize/2))-x)*cos((ang*pi)/180) - ((y+(ysize/2))-y)*sin((ang*pi)/180));
				tmpy=y+(((x-(xsize/2))-x)*sin((ang*pi)/180) + ((y+(ysize/2))-y)*cos((ang*pi)/180));
				if(tmpx>xb) xb=tmpx;
				else if(tmpx<xl) xl=tmpx;

				if(tmpy>yb) yb=tmpy;
				else if(tmpy<yl) yl=tmpy;
			}
	}

	if(st.mouse.x>xl && st.mouse.x<xb && st.mouse.y>yl && st.mouse.y<yb)
		return 1; //Collided
	else
		return 0; //No collision
	
}

int8 DrawSprite(double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, GLuint data, float a)
{
	double tmp;

	uint8 val=0;

	Pos dim=st.Camera.dimension;

	x-=st.Camera.position.x;
	y-=st.Camera.position.y;

	if(dim.x<0) dim.x*=-1;
	if(dim.y<0) dim.y*=-1;

	if(dim.x<1) dim.x=16384/dim.x;
	else dim.x*=16384;
	if(dim.y<1) dim.y=8192/dim.y;
	else dim.y*=8192;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>dim.x) val++;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>dim.y) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>dim.x) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>dim.y) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>dim.x) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>dim.y) val++;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>dim.x) val++;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>dim.y) val++;

	if(val==8) return 1;

	for(register uint32 i=0;i<MAX_GRAPHICS+1;i++)
	{
		if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
			return 2;

		if(ent[i].stat==DEAD)
		{
			ent[i].stat=USED;
			ent[i].ang=ang;
			ent[i].pos.x=(st.screenx*x)/16384;
			ent[i].pos.y=(st.screeny*y)/8192;
			ent[i].size.x=(sizex*st.screenx)/16384;
			ent[i].size.y=(sizey*st.screeny)/8192;
			ent[i].type=SPRITE;
			ent[i].data=data;
			ent[i].color.r=(float)r/255;
			ent[i].color.g=(float)g/255;
			ent[i].color.b=(float)b/255;
			ent[i].color.a=a;
			st.num_tex++;
			st.num_entities++;
			break;
		}
	}

	return 0;
}

int8 DrawLight(double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, GLuint data, float a)
{
	double tmp;

	uint8 val=0;

	Pos dim=st.Camera.dimension;

	x-=st.Camera.position.x;
	y-=st.Camera.position.y;

	if(dim.x<0) dim.x*=-1;
	if(dim.y<0) dim.y*=-1;

	if(dim.x<1) dim.x=16384/dim.x;
	else dim.x*=16384;
	if(dim.y<1) dim.y=8192/dim.y;
	else dim.y*=8192;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>dim.x) val++;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>dim.y) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>dim.x) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>dim.y) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>dim.x) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>dim.y) val++;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>dim.x) val++;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>dim.y) val++;

	if(val==8) return 1;

	for(register uint32 i=0;i<MAX_GRAPHICS+1;i++)
	{
		if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
			return 2;

		if(ent[i].stat==DEAD)
		{
			ent[i].stat=USED;
			ent[i].ang=ang;
			ent[i].pos.x=(st.screenx*x)/16384;
			ent[i].pos.y=(st.screeny*y)/8192;
			ent[i].size.x=(sizex*st.screenx)/16384;
			ent[i].size.y=(sizey*st.screeny)/8192;
			ent[i].type=LIGHT_MAP;
			ent[i].data=data;
			ent[i].color.r=(float)r/255;
			ent[i].color.g=(float)g/255;
			ent[i].color.b=(float)b/255;
			ent[i].color.a=a;
			st.num_tex++;
			st.num_entities++;
			break;
		}
	}

	return 0;
}

int8 DrawGraphic(double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, GLuint data, float a, float texpanX, float texpanY, float texsizeX, float texsizeY)
{
	double tmp;

	Pos dim=st.Camera.dimension;

	uint8 val=0;

	x-=st.Camera.position.x;
	y-=st.Camera.position.y;

	if(dim.x<0) dim.x*=-1;
	if(dim.y<0) dim.y*=-1;

	if(dim.x<1) dim.x=16384/dim.x;
	else dim.x*=16384;
	if(dim.y<1) dim.y=8192/dim.y;
	else dim.y*=8192;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>dim.x) val++;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>dim.y) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>dim.x) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>dim.y) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>dim.x) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>dim.y) val++;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>dim.x) val++;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>dim.y) val++;

	if(val==8) return 1;

	for(register uint32 i=0;i<MAX_GRAPHICS+1;i++)
	{
		if(ent[i].stat==DEAD)
		{
			if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
				return 2;

			ent[i].stat=USED;
			ent[i].ang=ang;
			ent[i].pos.x=(st.screenx*x)/16384;
			ent[i].pos.y=(st.screeny*y)/8192;
			ent[i].size.x=(sizex*st.screenx)/16384;
			ent[i].size.y=(sizey*st.screeny)/8192;
			ent[i].type=TEXTURE;
			ent[i].data=data;
			ent[i].color.r=(float)r/255;
			ent[i].color.g=(float)g/255;
			ent[i].color.b=(float)b/255;
			ent[i].color.a=a;
			ent[i].x1y1.x=texsizeX+texpanX;
			ent[i].x1y1.y=texsizeY+texpanY;
			ent[i].x2y2.x=texsizeX;
			ent[i].x2y2.y=texsizeY;
			st.num_tex++;
			st.num_entities++;
			break;
		}
	}

	return 0;
}

int8 DrawHud(double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, double x1, double y1, double x2, double y2, GLuint data, float a)
{
	double tmp;
	uint8 val=0;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	if(val==8) return 1;

	for(register uint32 i=0;i<MAX_GRAPHICS+1;i++)
	{
		if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
			return 2;

		if(ent[i].stat==DEAD)
		{
			ent[i].stat=USED;
			ent[i].ang=ang;
			ent[i].pos.x=(st.screenx*x)/800;
			ent[i].pos.y=(st.screeny*y)/600;
			ent[i].size.x=(sizex*st.screenx)/800;
			ent[i].size.y=(sizey*st.screeny)/600;
			ent[i].type=HUD;
			ent[i].data=data;
			ent[i].x1y1.x=x1;
			ent[i].x1y1.y=y1;
			ent[i].x2y2.x=x2;
			ent[i].x2y2.y=y2;
			ent[i].color.r=(float)r/255;
			ent[i].color.g=(float)g/255;
			ent[i].color.b=(float)b/255;
			ent[i].color.a=a;
			st.num_hud++;
			st.num_entities++;
			break;
		}
	}

	return 0;
}

int8 DrawUI(double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, double x1, double y1, double x2, double y2, GLuint data, float a)
{
	double tmp;
	uint8 val=0;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	if(val==8) return 1;

	for(register uint32 i=0;i<MAX_GRAPHICS+1;i++)
	{
		if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
			return 2;

		if(ent[i].stat==DEAD)
		{
			ent[i].stat=USED;
			ent[i].ang=ang;
			ent[i].pos.x=(st.screenx*x)/800;
			ent[i].pos.y=(st.screeny*y)/600;
			ent[i].size.x=(sizex*st.screenx)/800;
			ent[i].size.y=(sizey*st.screeny)/600;
			ent[i].type=UI;
			ent[i].data=data;
			ent[i].x1y1.x=x1;
			ent[i].x1y1.y=y1;
			ent[i].x2y2.x=x2;
			ent[i].x2y2.y=y2;
			ent[i].color.r=(float)r/255;
			ent[i].color.g=(float)g/255;
			ent[i].color.b=(float)b/255;
			ent[i].color.a=a;
			st.num_ui++;
			st.num_entities++;
			break;
		}
	}

	return 0;
}

int8 DrawLine(double x, double y, double x2, double y2, uint8 r, uint8 g, uint8 b, float a, double linewidth)
{
	uint8 val=0;

	Pos dim=st.Camera.dimension;

	if(dim.x<0) dim.x*=-1;
	if(dim.y<0) dim.y*=-1;

	if(dim.x<1) dim.x=16384/dim.x;
	else dim.x*=16384;
	if(dim.y<1) dim.y=8192/dim.y;
	else dim.y*=8192;

	x-=st.Camera.position.x;
	y-=st.Camera.position.y;

	x2-=st.Camera.position.x;
	y2-=st.Camera.position.y;

	if(x>dim.x) val++;
	if(y>dim.y) val++;

	if(x2>dim.x) val++;
	if(y2>dim.y) val++;

	if(val==4) return 1;

	for(register uint32 i=0;i<MAX_GRAPHICS+1;i++)
	{
		if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
			return 2;

		if(ent[i].stat==DEAD)
		{
			ent[i].stat=USED;
			ent[i].ang=0;
			ent[i].pos.x=(st.screenx*x)/16384;
			ent[i].pos.y=(st.screeny*y)/8192;
			ent[i].size.x=(x2*st.screenx)/16384;
			ent[i].size.y=(y2*st.screeny)/8192;
			ent[i].type=LINE;
			ent[i].color.r=(float)r/255;
			ent[i].color.g=(float)g/255;
			ent[i].color.b=(float)b/255;
			ent[i].color.a=a;
			ent[i].data=linewidth;
			st.num_tex++;
			st.num_entities++;
			break;
		}
	}

	return 0;
}

int32 MAnim(double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, _MGG *mgf, uint16 id, float speed, float a)
{
	uint16 curf=0;
	if(mgf->anim[id].current_frame>mgf->anim[id].endID) mgf->anim[id].current_frame=mgf->anim[id].startID; else
	if(mgf->anim[id].current_frame==0) curf=mgf->anim[id].startID; else
	if((mgf->anim[id].current_frame>0) && (mgf->anim[id].current_frame<mgf->anim[id].endID)) curf=curf=mgf->anim[id].current_frame;
	DrawSprite(x,y,sizex,sizey,ang,r,g,b, mgf->frames[curf],a);
	mgf->anim[id].current_frame+=speed;
	if(mgf->anim[id].current_frame>mgf->anim[id].endID) mgf->anim[id].current_frame=mgf->anim[id].startID;
	return mgf->anim[id].current_frame;
}

int8 DrawString(const char *text, double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, float a, TTF_Font *f)
{	
	SDL_Color co;
	co.r=255;
	co.g=255;
	co.b=255;
	co.a=255;
	uint16 formatt;

	double tmp;

	uint8 val=0;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	if(val==8) return 1;

	
	SDL_Surface *msg=TTF_RenderUTF8_Blended(f,text,co);
	
	if(msg->format->BytesPerPixel==4)
	{
		if(msg->format->Rmask==0x000000ff) formatt=GL_RGBA;
		else formatt=GL_BGRA_EXT;
	} else
	{
		if(msg->format->Rmask==0x000000ff) formatt=GL_RGB;
		else formatt=GL_BGR_EXT;
	}

	for(register uint32 i=0;i<MAX_GRAPHICS+1;i++)
	{
		if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
			return 2;

		if(ent[i].stat==DEAD)
		{
			glGenTextures(1,&ent[i].data);
			glBindTexture(GL_TEXTURE_2D,ent[i].data);
			glTexImage2D(GL_TEXTURE_2D,0,msg->format->BytesPerPixel,msg->w,msg->h,0,formatt,GL_UNSIGNED_BYTE,msg->pixels);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			ent[i].ang=ang;
			ent[i].stat=USED;
			ent[i].type=TEXT;
			ent[i].pos.x=(st.screenx*x)/800;
			ent[i].pos.y=(st.screeny*y)/600;
			ent[i].size.x=(sizex*st.screenx)/800;
			ent[i].size.y=(sizey*st.screeny)/600;
			ent[i].x1y1.x=0;
			ent[i].x1y1.y=0;
			ent[i].x2y2.x=1;
			ent[i].x2y2.y=1;
			ent[i].color.r=(float)r/255;
			ent[i].color.g=(float)g/255;
			ent[i].color.b=(float)b/255;
			ent[i].color.a=a;

			SDL_FreeSurface(msg);
			st.num_hud++;
			st.num_entities++;
			
			break;
		}
	}

	return 0;

}

int8 DrawStringUI(const char *text, double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, float a, TTF_Font *f)
{	
	SDL_Color co;
	co.r=255;
	co.g=255;
	co.b=255;
	co.a=255;
	uint16 formatt;

	double tmp;

	uint8 val=0;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y-(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y-(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	tmp=x+(((x+(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x+(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	tmp=x+(((x-(sizex/2))-x)*cos((ang*pi)/180) - ((y+(sizey/2))-y)*sin((ang*pi)/180));
	if(tmp>800) val++;

	tmp=y+(((x-(sizex/2))-x)*sin((ang*pi)/180) - ((y+(sizey/2))-y)*cos((ang*pi)/180));
	if(tmp>600) val++;

	if(val==8) return 1;

	
	SDL_Surface *msg=TTF_RenderUTF8_Blended(f,text,co);
	
	if(msg->format->BytesPerPixel==4)
	{
		if(msg->format->Rmask==0x000000ff) formatt=GL_RGBA;
		else formatt=GL_BGRA_EXT;
	} else
	{
		if(msg->format->Rmask==0x000000ff) formatt=GL_RGB;
		else formatt=GL_BGR_EXT;
	}

	for(register uint32 i=0;i<MAX_GRAPHICS+1;i++)
	{
		if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
			return 2;

		if(ent[i].stat==DEAD)
		{
			glGenTextures(1,&ent[i].data);
			glBindTexture(GL_TEXTURE_2D,ent[i].data);
			glTexImage2D(GL_TEXTURE_2D,0,msg->format->BytesPerPixel,msg->w,msg->h,0,formatt,GL_UNSIGNED_BYTE,msg->pixels);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			ent[i].ang=ang;
			ent[i].stat=USED;
			ent[i].type=TEXT_UI;
			ent[i].pos.x=(st.screenx*x)/800;
			ent[i].pos.y=(st.screeny*y)/600;
			ent[i].size.x=(sizex*st.screenx)/800;
			ent[i].size.y=(sizey*st.screeny)/600;
			ent[i].x1y1.x=0;
			ent[i].x1y1.y=0;
			ent[i].x2y2.x=1;
			ent[i].x2y2.y=1;
			ent[i].color.r=(float)r/255;
			ent[i].color.g=(float)g/255;
			ent[i].color.b=(float)b/255;
			ent[i].color.a=a;

			SDL_FreeSurface(msg);
			st.num_hud++;
			st.num_entities++;
			
			break;
		}
	}

	return 0;

}

int8 DrawString2UI(const char *text, double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, float a, TTF_Font *f)
{	
	SDL_Color co;
	co.r=255;
	co.g=255;
	co.b=255;
	co.a=255;
	uint16 formatt;

	SDL_Surface *msg=TTF_RenderUTF8_Blended(f,text,co);
	
	if(msg->format->BytesPerPixel==4)
	{
		if(msg->format->Rmask==0x000000ff) formatt=GL_RGBA;
		else formatt=GL_BGRA_EXT;
	} else
	{
		if(msg->format->Rmask==0x000000ff) formatt=GL_RGB;
		else formatt=GL_BGR_EXT;
	}

	for(register uint32 i=0;i<MAX_GRAPHICS+1;i++)
	{
		if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
			return 2;

		if(ent[i].stat==DEAD)
		{
			glGenTextures(1,&ent[i].data);
			glBindTexture(GL_TEXTURE_2D,ent[i].data);
			glTexImage2D(GL_TEXTURE_2D,0,msg->format->BytesPerPixel,msg->w,msg->h,0,formatt,GL_UNSIGNED_BYTE,msg->pixels);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			ent[i].ang=ang;
			ent[i].stat=USED;
			ent[i].type=TEXT_UI;
			ent[i].pos.x=(st.screenx*x)/800;
			ent[i].pos.y=(st.screeny*y)/600;
			ent[i].size.x=((msg->w*sizex)*st.screenx)/800;
			ent[i].size.y=((msg->h*sizey)*st.screeny)/600;
			ent[i].x1y1.x=0;
			ent[i].x1y1.y=0;
			ent[i].x2y2.x=1;
			ent[i].x2y2.y=1;
			ent[i].color.r=(float)r/255;
			ent[i].color.g=(float)g/255;
			ent[i].color.b=(float)b/255;
			ent[i].color.a=a;

			SDL_FreeSurface(msg);
			st.num_ui++;
			st.num_entities++;
			
			break;
		}
	}

	return 0;

}

uint32 PlayMovie(const char *name)
{
	SDL_Thread *t;
	int ReturnVal, ids, ReturnVal2, ReturnVal3;
	unsigned int ms, ms1, ms2;
	int ms3;
	register uint32 i=0;
	char header[21];

	FMOD_RESULT y;

	uint8 id;

	FMOD_BOOL p;
	
	_MGVFORMAT mgvt;
	_MGV *mgv;

	mgv=(_MGV*) malloc(sizeof(_MGV));

	if((mgv->file=fopen(name,"rb"))==NULL)
	{
		LogApp("Error opening MGV file %s",name);
				return false;
	}

	rewind(mgv->file);
	fread(header,21,1,mgv->file);

	if(strcmp(header,"MGV File Version 1.0")!=NULL)
	{
		LogApp("Invalid MGV file header %s",name);
		fclose(mgv->file);
		return false;
	}

	rewind(mgv->file);
	fseek(mgv->file,21,SEEK_SET);

	fread(&mgvt,sizeof(_MGVFORMAT),1,mgv->file);

	mgv->fps=mgvt.fps;
	mgv->num_frames=mgvt.num_frames;
	
	mgv->framesize=(uint32*) malloc(mgv->num_frames*sizeof(uint32));

	fseek(mgv->file,(sizeof(_MGVFORMAT)+512)+21,SEEK_SET);

	fread(mgv->framesize,sizeof(uint32),mgv->num_frames,mgv->file);

	mgv->totalsize=((sizeof(_MGVFORMAT)+512)+((mgv->num_frames*sizeof(uint32))+512))+21;

	mgv->frames=(_MGVTEX*) malloc(mgv->num_frames*sizeof(_MGVTEX)); 
	mgv->seeker=(uint32*) malloc(mgv->num_frames*sizeof(uint32));

	for(register uint32 o=0;o<mgv->num_frames;o++)
	{
		
		if(o==0)
			mgv->seeker[0]=mgv->totalsize;
		else
		{
			mgv->seeker[o]=mgv->seeker[o-1];
			mgv->seeker[o]+=mgv->framesize[o-1]+2048;
		}

		mgv->totalsize+=mgv->framesize[o]+2048;
	}

	rewind(mgv->file);
	fseek(mgv->file,mgv->totalsize+512,SEEK_CUR);

	void *buffer=(void*) malloc(mgvt.sound_buffer_lenght);
	fread(buffer,mgvt.sound_buffer_lenght,1,mgv->file);
	
	FMOD_CREATESOUNDEXINFO info;
	memset(&info,0,sizeof(info));
	info.length=mgvt.sound_buffer_lenght;
	info.cbsize=sizeof(info);

	y=FMOD_System_CreateSound(st.sound_sys.Sound_System,(const char*) buffer,FMOD_HARDWARE | FMOD_OPENMEMORY,&info,&mgv->sound);

	if(y!=FMOD_OK)
	{
		LogApp("Error while creating sound: %s",FMOD_ErrorString(y));
		return false;
	}
	
	FMOD_CHANNEL *ch;

	StopAllSounds();
	
	mgv->totalsize=((sizeof(_MGVFORMAT)+512)+((mgv->num_frames*sizeof(uint32))+512))+21;


	y=FMOD_System_PlaySound(st.sound_sys.Sound_System,FMOD_CHANNEL_FREE,mgv->sound,0,&ch);

	if(y!=FMOD_OK)
	{
		LogApp("Error while playing sound: %s",FMOD_ErrorString(y));
		return false;
	}
	
	//Here's the video part!!
	i=0;

	st.PlayingVideo=1;

	while(st.PlayingVideo)
	{
		InputProcess();

		if(st.quit) break;
		glClearColor(0.0,0.0,0.0,0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();

		glPushMatrix();

		glColor4f(1.0,1.0,1.0,1.0f);
		
		FMOD_System_Update(st.sound_sys.Sound_System);
		
		FMOD_Channel_IsPlaying(ch,&p);

		if(!p) break;
		
		FMOD_Channel_GetPosition(ch,&ms,FMOD_TIMEUNIT_MS);

		if((ms/mgv->fps)>i+1000) FMOD_Channel_SetPosition(ch,i*mgv->fps,FMOD_TIMEUNIT_MS);

		if(i>mgv->num_frames-1) break;

		ms1=GetTicks();

		rewind(mgv->file);
		fseek(mgv->file,mgv->seeker[i],SEEK_SET);

			mgv->frames[i].buffer=(void*) malloc(mgv->framesize[i]);

			if(mgv->frames[i].buffer==NULL)
			{
				LogApp("Unable to allocate memory for MGV frame %d",i);
				continue;
			}

			fread(mgv->frames[i].buffer,mgv->framesize[i],1,mgv->file);

			mgv->frames[i].rw=SDL_RWFromMem(mgv->frames[i].buffer,mgv->framesize[i]);

			mgv->frames[i].data=IMG_LoadJPG_RW(mgv->frames[i].rw);

			glEnable(GL_TEXTURE_RECTANGLE);

			glGenTextures(1,&mgv->frames[i].ID);
			glBindTexture(GL_TEXTURE_RECTANGLE,mgv->frames[i].ID);
			glTexImage2D(GL_TEXTURE_RECTANGLE,0,3,mgv->frames[i].data->w,mgv->frames[i].data->h,0,GL_RGB,GL_UNSIGNED_BYTE,mgv->frames[i].data->pixels);
			glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
			glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_RECTANGLE,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_RECTANGLE,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

			glBegin(GL_TRIANGLES);
				glTexCoord2i(0,0);
				glVertex2d(0,0);
				glTexCoord2i(mgv->frames[i].data->w,0);
				glVertex2d(st.screenx,0);
				glTexCoord2i(mgv->frames[i].data->w,mgv->frames[i].data->h);
				glVertex2d(st.screenx, st.screeny);

				glTexCoord2i(mgv->frames[i].data->w,mgv->frames[i].data->h);
				glVertex2d(st.screenx, st.screeny);
				glTexCoord2i(0,mgv->frames[i].data->h);
				glVertex2d(0,st.screeny);
				glTexCoord2i(0,0);
				glVertex2d(0,0);
			glEnd();

			glPopMatrix();

			SDL_GL_SwapWindow(wn);
			
			
			free(mgv->frames[i].buffer);
			SDL_FreeRW(mgv->frames[i].rw);
			SDL_FreeSurface(mgv->frames[i].data);
			glDeleteTextures(1,&mgv->frames[i].ID);

			i++;

			ms2=GetTicks();

			ms3=ms2-ms1;

			ms3=(1000/mgv->fps)-ms3;

			if(ms3<0) ms3=0;

			SDL_Delay(ms3);

			if(st.FPSYes)
				FPSCounter();
		
	}

	glDisable(GL_TEXTURE_RECTANGLE);

	free(mgv->framesize);
	free(mgv->frames);
	free(buffer);
	FMOD_Channel_Stop(ch);
	FMOD_Sound_Release(mgv->sound);
	st.PlayingVideo=0;

	return true;
	
}

#ifdef ENGINEER
uint32 SaveMap(const char *name)
{
	FILE *file;
	char header[13];
	_MGMFORMAT map;
	_MGMOBJ *obj;
	_MGMSPRITE *sprites;
	_MGMLIGHT *lights;

	if((file=fopen(name,"wb"))==NULL)
	{
		LogApp("Could not save file");
				return false;
	}

	strcpy(header,"V1 mGear-1");

	fwrite(header,13,1,file);

	obj=(_MGMOBJ*) malloc(st.Current_Map.num_obj*sizeof(_MGMOBJ));
	sprites=(_MGMSPRITE*) malloc(st.Current_Map.num_sprites*sizeof(_MGMSPRITE));
	lights=(_MGMLIGHT*) malloc(st.Current_Map.num_lights*sizeof(_MGMLIGHT));

	memcpy(&obj,&st.Current_Map.obj,sizeof(st.Current_Map.num_obj*sizeof(_MGMOBJ)));
	memcpy(&sprites,&st.Current_Map.sprites,sizeof(st.Current_Map.num_sprites*sizeof(_MGMSPRITE)));
	memcpy(&lights,&st.Current_Map.light,sizeof(st.Current_Map.num_lights*sizeof(_MGMLIGHT)));

	map.num_mgg=st.Current_Map.num_mgg;
	map.num_obj=st.Current_Map.num_obj;
	map.num_lights=st.Current_Map.num_lights;
	map.num_sprites=st.Current_Map.num_sprites;
	strcpy(map.name,st.Current_Map.name);
	memcpy(map.MGG_FILES,st.Current_Map.MGG_FILES,sizeof(st.Current_Map.MGG_FILES));

	fwrite(&map,sizeof(_MGMFORMAT),1,file);
	fwrite(obj,sizeof(_MGMOBJ),map.num_obj,file);
	fwrite(sprites,sizeof(_MGMSPRITE),map.num_sprites,file);
	fwrite(lights,sizeof(_MGMLIGHT),map.num_lights,file);

	fclose(file);

	free(obj);
	free(sprites);
	free(lights);

	return 1;
}
#endif

uint32 LoadMap(const char *name)
{
	FILE *file;
	char header[13];
	_MGMFORMAT map;

	if((file=fopen(name,"rb"))==NULL)
	{
		LogApp("Could not open file %s",name);
				return false;
	}

	//Tries to read 13 byte header
	fread(header,13,1,file);

	//Checks if it's the same version

	if(strcmp(header,"V1 mGear-1")!=NULL)
	{
		LogApp("Invalid map format or version: %s", header);
				return false;
	}

					if(st.Current_Map.obj)
						free(st.Current_Map.obj);

					if(st.Current_Map.sprites)
						free(st.Current_Map.sprites);

					if(st.Current_Map.sector)
						free(st.Current_Map.sector);

	//loads the map

	fread(&map,sizeof(_MGMFORMAT),1,file);

	st.Current_Map.num_mgg=map.num_mgg;
	st.Current_Map.num_obj=map.num_obj;
	//st.Current_Map.num_lights=map.num_lights;
	st.Current_Map.num_sprites=map.num_sprites;
	st.Current_Map.num_sector=map.num_sector;
	strcpy(st.Current_Map.name,map.name);
	memcpy(&st.Current_Map.MGG_FILES,&map.MGG_FILES,sizeof(map.MGG_FILES));

#ifdef ENGINEER
	st.Current_Map.obj=(_MGMOBJ*) malloc(MAX_OBJS*sizeof(_MGMOBJ));
	st.Current_Map.sprites=(_MGMSPRITE*) malloc(MAX_SPRITES*sizeof(_MGMSPRITE));
	//st.Current_Map.light=(_MGMLIGHT*) malloc(MAX_LIGHT*sizeof(_MGMLIGHT));
	st.Current_Map.sector=(_SECTOR*) malloc(MAX_SECTORS*sizeof(_SECTOR));
#else
	st.Current_Map.obj=(_MGMOBJ*) malloc(st.Current_Map.num_obj*sizeof(_MGMOBJ));
	st.Current_Map.sprites=(_MGMSPRITE*) malloc(st.Current_Map.num_sprites*sizeof(_MGMSPRITE));
	//st.Current_Map.light=(_MGMLIGHT*) malloc(st.Current_Map.num_lights*sizeof(_MGMLIGHT));
	st.Current_Map.sector=(_SECTOR*) malloc(st.Current_Map.num_sector*sizeof(_SECTOR));
#endif

	fread(st.Current_Map.obj,sizeof(_MGMOBJ),st.Current_Map.num_obj,file);
	fread(st.Current_Map.sprites,sizeof(_MGMSPRITE),st.Current_Map.num_sprites,file);
	//fread(st.Current_Map.light,sizeof(_MGMLIGHT),st.Current_Map.num_lights,file);
	fread(st.Current_Map.sector,sizeof(_SECTOR),st.Current_Map.num_sector,file);

	fclose(file);

	return 1;
}

void FreeMap()
{
	if(st.Current_Map.num_mgg>0)
	{
		free(st.Current_Map.obj);
		free(st.Current_Map.sprites);

		register uint8 j=MGG_MAP_START;

		for(register uint8 i=0;i<st.Current_Map.num_mgg;i++)
		{
			FreeMGG(&mgg[j]);
			j++;
		}

		memset(&st.Current_Map,0,sizeof(_MGM));
	}
}

void DrawMap()
{
	
	//Draw the objects first

	double x, y, sizex, sizey, ang, size;
	

	for(register uint16 i=0;i<st.Current_Map.num_obj;i++)
		if(st.Current_Map.obj[i].type==FOREGROUND)
			DrawGraphic(st.Current_Map.obj[i].position.x,st.Current_Map.obj[i].position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,
			st.Current_Map.obj[i].angle,st.Current_Map.obj[i].color.r,st.Current_Map.obj[i].color.g,st.Current_Map.obj[i].color.b,mgg[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID],st.Current_Map.obj[i].color.a,st.Current_Map.obj[i].texsize.x,st.Current_Map.obj[i].texsize.y,st.Current_Map.obj[i].texpan.x,st.Current_Map.obj[i].texpan.y);
	for(register uint16 i=0;i<st.Current_Map.num_obj;i++)
		if(st.Current_Map.obj[i].type==MIDGROUND)
				DrawGraphic(st.Current_Map.obj[i].position.x,st.Current_Map.obj[i].position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,
				st.Current_Map.obj[i].angle,st.Current_Map.obj[i].color.r*st.Current_Map.obj[i].amblight,st.Current_Map.obj[i].color.g*st.Current_Map.obj[i].amblight,st.Current_Map.obj[i].color.b*st.Current_Map.obj[i].amblight,mgg[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID],st.Current_Map.obj[i].color.a,st.Current_Map.obj[i].texsize.x,st.Current_Map.obj[i].texsize.y,st.Current_Map.obj[i].texpan.x,st.Current_Map.obj[i].texpan.y);

	//for(register uint16 i=0;i<st.Current_Map.num_lights;i++)
		//	DrawLight(st.Current_Map.light[i].position.x-st.Camera.position.x,st.Current_Map.light[i].position.y-st.Camera.position.y,st.Current_Map.light[i].size.x,st.Current_Map.light[i].size.y,
			//st.Current_Map.light[i].angle,st.Current_Map.light[i].color.r,st.Current_Map.light[i].color.g,st.Current_Map.light[i].color.b,st.MapTex[st.Current_Map.light[i].TextureID].ID,st.Current_Map.light[i].color.a);
	for(register uint16 i=0;i<st.Current_Map.num_obj;i++)
		if(st.Current_Map.obj[i].type==BACKGROUND3)
				DrawGraphic(st.Current_Map.obj[i].position.x-st.Camera.position.x,st.Current_Map.obj[i].position.y-st.Camera.position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,
					st.Current_Map.obj[i].angle,st.Current_Map.obj[i].color.r,st.Current_Map.obj[i].color.g,st.Current_Map.obj[i].color.b,mgg[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID],st.Current_Map.obj[i].color.a,st.Current_Map.obj[i].texsize.x,st.Current_Map.obj[i].texsize.y,st.Current_Map.obj[i].texpan.x,st.Current_Map.obj[i].texpan.y);
	for(register uint16 i=0;i<st.Current_Map.num_obj;i++)
		if(st.Current_Map.obj[i].type==BACKGROUND2)
				DrawGraphic(st.Current_Map.obj[i].position.x-st.Camera.position.x,st.Current_Map.obj[i].position.y-st.Camera.position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,
					st.Current_Map.obj[i].angle,st.Current_Map.obj[i].color.r,st.Current_Map.obj[i].color.g,st.Current_Map.obj[i].color.b,mgg[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID],st.Current_Map.obj[i].color.a,st.Current_Map.obj[i].texsize.x,st.Current_Map.obj[i].texsize.y,st.Current_Map.obj[i].texpan.x,st.Current_Map.obj[i].texpan.y);
	for(register uint16 i=0;i<st.Current_Map.num_obj;i++)
		if(st.Current_Map.obj[i].type==BACKGROUND1)
				DrawGraphic(st.Current_Map.obj[i].position.x-st.Camera.position.x,st.Current_Map.obj[i].position.y-st.Camera.position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,
					st.Current_Map.obj[i].angle,st.Current_Map.obj[i].color.r,st.Current_Map.obj[i].color.g,st.Current_Map.obj[i].color.b,mgg[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID],st.Current_Map.obj[i].color.a,st.Current_Map.obj[i].texsize.x,st.Current_Map.obj[i].texsize.y,st.Current_Map.obj[i].texpan.x,st.Current_Map.obj[i].texpan.y);
	if(st.Developer_Mode)
	{
		for(register uint16 i=0;i<st.Current_Map.num_sector;i++)
			if(st.Current_Map.sector[i].id>-1)
			{
				DrawLine(st.Current_Map.sector[i].vertex[0].x,st.Current_Map.sector[i].vertex[0].y,st.Current_Map.sector[i].vertex[1].x,st.Current_Map.sector[i].vertex[1].y,255,255,255,1,2);
				DrawLine(st.Current_Map.sector[i].vertex[1].x,st.Current_Map.sector[i].vertex[1].y,st.Current_Map.sector[i].vertex[2].x,st.Current_Map.sector[i].vertex[2].y,255,255,255,1,2);
				DrawLine(st.Current_Map.sector[i].vertex[2].x,st.Current_Map.sector[i].vertex[2].y,st.Current_Map.sector[i].vertex[3].x,st.Current_Map.sector[i].vertex[3].y,255,255,255,1,2);
				DrawLine(st.Current_Map.sector[i].vertex[3].x,st.Current_Map.sector[i].vertex[3].y,st.Current_Map.sector[i].vertex[0].x,st.Current_Map.sector[i].vertex[0].y,255,255,255,1,2);

				DrawLine(st.Current_Map.sector[i].vertex[0].x,st.Current_Map.sector[i].vertex[0].y,st.Current_Map.sector[i].position.x,st.Current_Map.sector[i].position.y,255,255,255,1,1);
				DrawLine(st.Current_Map.sector[i].vertex[1].x,st.Current_Map.sector[i].vertex[1].y,st.Current_Map.sector[i].position.x,st.Current_Map.sector[i].position.y,255,255,255,1,1);
				DrawLine(st.Current_Map.sector[i].vertex[2].x,st.Current_Map.sector[i].vertex[2].y,st.Current_Map.sector[i].position.x,st.Current_Map.sector[i].position.y,255,255,255,1,1);
				DrawLine(st.Current_Map.sector[i].vertex[3].x,st.Current_Map.sector[i].vertex[3].y,st.Current_Map.sector[i].position.x,st.Current_Map.sector[i].position.y,255,255,255,1,1);

				DrawGraphic(st.Current_Map.sector[i].vertex[0].x,st.Current_Map.sector[i].vertex[0].y,256,256,0,255,255,255,mgg[0].frames[4],1,1,1,0,0);
				DrawGraphic(st.Current_Map.sector[i].vertex[1].x,st.Current_Map.sector[i].vertex[1].y,256,256,0,255,255,255,mgg[0].frames[4],1,1,1,0,0);
				DrawGraphic(st.Current_Map.sector[i].vertex[2].x,st.Current_Map.sector[i].vertex[2].y,256,256,0,255,255,255,mgg[0].frames[4],1,1,1,0,0);
				DrawGraphic(st.Current_Map.sector[i].vertex[3].x,st.Current_Map.sector[i].vertex[3].y,256,256,0,255,255,255,mgg[0].frames[4],1,1,1,0,0);

				DrawGraphic(st.Current_Map.sector[i].position.x,st.Current_Map.sector[i].position.y,484,484,0,255,255,255,mgg[0].frames[0],1,1,1,0,0);
			}
	}
}

void Renderer()
{
	_ENTITIES tmp[MAX_GRAPHICS];
	uint32 j=0, k=0, l=0;

	memset(&tmp,0,MAX_GRAPHICS*sizeof(_ENTITIES));

	for(register uint32 i=0;i<st.num_entities;i++)
	{
		if(ent[i].type==TEXTURE || ent[i].type==LINE || ent[i].type==SPRITE)
		{
			tmp[j]=ent[i];
			j++;
		}
		else 
		if(ent[i].type==HUD || ent[i].type==TEXT)
		{
			tmp[st.num_tex+k]=ent[i];
			k++;
		}
		else 
		if(ent[i].type==UI || ent[i].type==TEXT_UI)
		{
			tmp[st.num_tex+st.num_hud+l]=ent[i];
			l++;
		}
	}

	memcpy(&ent,&tmp,MAX_GRAPHICS*sizeof(_ENTITIES));

	glClearColor(0.0,0.0,0.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();

	//glTranslated(-((st.Camera.position.x*st.screenx)/16384),-((st.Camera.position.y*st.screeny)/8192),0);

	for(register uint32 i=0;i<MAX_GRAPHICS;i++)
	{	
		if(ent[i].stat==USED)
		{
			if(ent[i].type==TEXTURE || ent[i].type==LINE)
			{
				//glLoadIdentity();

				glPushMatrix();

				//glTranslated(-((st.Camera.position.x*st.screenx)/16384),-((st.Camera.position.y*st.screeny)/8192),0);

				glColor4f(ent[i].color.r,ent[i].color.g,ent[i].color.b,ent[i].color.a);
				
				glTranslated(ent[i].pos.x,ent[i].pos.y,0);
				glRotatef(ent[i].ang,0.0,0.0,1.0);
				glTranslated(-ent[i].pos.x,-ent[i].pos.y,0);
				glScalef(st.Camera.dimension.x,st.Camera.dimension.y,0);
				
				if(ent[i].type==LINE)
				{
					glDisable(GL_TEXTURE_2D);
					glLineWidth(ent[i].data);
					glBegin(GL_LINES);
						glVertex2d(ent[i].pos.x, ent[i].pos.y);
						glVertex2d(ent[i].size.x, ent[i].size.y);
					glEnd();
					glEnable(GL_TEXTURE_2D);
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D,ent[i].data);
						glBegin(GL_TRIANGLES);
							glTexCoord2f(ent[i].x1y1.x,ent[i].x1y1.y);
							glVertex2d(ent[i].pos.x-(ent[i].size.x/2),ent[i].pos.y-(ent[i].size.y/2));
							glTexCoord2f(ent[i].x2y2.x,ent[i].x1y1.y);
							glVertex2d(ent[i].pos.x+(ent[i].size.x/2),ent[i].pos.y-(ent[i].size.y/2));
							glTexCoord2f(ent[i].x2y2.x,ent[i].x2y2.y);
							glVertex2d(ent[i].pos.x+(ent[i].size.x/2),ent[i].pos.y+(ent[i].size.y/2));

							glTexCoord2f(ent[i].x2y2.x,ent[i].x2y2.y);
							glVertex2d(ent[i].pos.x+(ent[i].size.x/2),ent[i].pos.y+(ent[i].size.y/2));
							glTexCoord2f(ent[i].x1y1.x,ent[i].x2y2.y);
							glVertex2d(ent[i].pos.x-(ent[i].size.x/2),ent[i].pos.y+(ent[i].size.y/2));
							glTexCoord2f(ent[i].x1y1.x,ent[i].x1y1.y);
							glVertex2d(ent[i].pos.x-(ent[i].size.x/2),ent[i].pos.y-(ent[i].size.y/2));
						glEnd();
				}
				glPopMatrix();

				ent[i].data=-1;
				ent[i].stat=DEAD;
				st.num_entities--;
			}
			else
			if(ent[i].type==SPRITE)
			{
				//glLoadIdentity();

				glPushMatrix();

				//glBlendFunc(GL_DST_COLOR, GL_MAX);
				//glBlendEquation(GL_FUNC_ADD);
				
				glColor4f(ent[i].color.r,ent[i].color.g,ent[i].color.b,ent[i].color.a);
				glTranslated(ent[i].pos.x,ent[i].pos.y,0);
				glRotatef(ent[i].ang,0.0,0.0,1.0);
				glTranslated(-ent[i].pos.x,-ent[i].pos.y,0);

				//glTranslated(-((st.Camera.position.x*st.screenx)/16384),-((st.Camera.position.y*st.screeny)/8192),0);
				
				glBindTexture(GL_TEXTURE_2D,ent[i].data);
					glBegin(GL_TRIANGLES);
						glTexCoord2i(0,0);
						glVertex2d(ent[i].pos.x-(ent[i].size.x/2),ent[i].pos.y-(ent[i].size.y/2));
						glTexCoord2i(1,0);
						glVertex2d(ent[i].pos.x+(ent[i].size.x/2),ent[i].pos.y-(ent[i].size.y/2));
						glTexCoord2i(1,1);
						glVertex2d(ent[i].pos.x+(ent[i].size.x/2),ent[i].pos.y+(ent[i].size.y/2));

						glTexCoord2i(1,1);
						glVertex2d(ent[i].pos.x+(ent[i].size.x/2),ent[i].pos.y+(ent[i].size.y/2));
						glTexCoord2i(0,1);
						glVertex2d(ent[i].pos.x-(ent[i].size.x/2),ent[i].pos.y+(ent[i].size.y/2));
						glTexCoord2i(0,0);
						glVertex2d(ent[i].pos.x-(ent[i].size.x/2),ent[i].pos.y-(ent[i].size.y/2));
					glEnd();
					
				glPopMatrix();
				ent[i].data=-1;
				ent[i].stat=DEAD;
				st.num_entities--;
				
			}
			else
			if(ent[i].type==HUD || ent[i].type==TEXT)
			{
				//glLoadIdentity();

				glPushMatrix();

				glColor4f(ent[i].color.r,ent[i].color.g,ent[i].color.b,ent[i].color.a);
				glTranslated(ent[i].pos.x,ent[i].pos.y,0);
				glRotatef(ent[i].ang,0.0,0.0,1.0);
				glTranslated(-ent[i].pos.x,-ent[i].pos.y,0);
				//glScalef(st.Camera.dimension.x,st.Camera.dimension.y,0);
				
				glBindTexture(GL_TEXTURE_2D,ent[i].data);
					glBegin(GL_TRIANGLES);
						glTexCoord2d(ent[i].x1y1.x,ent[i].x1y1.y);
						glVertex2d(ent[i].pos.x-(ent[i].size.x/2),ent[i].pos.y-(ent[i].size.y/2));
						glTexCoord2d(ent[i].x2y2.x,ent[i].x1y1.y);
						glVertex2d(ent[i].pos.x+(ent[i].size.x/2),ent[i].pos.y-(ent[i].size.y/2));
						glTexCoord2d(ent[i].x2y2.x,ent[i].x2y2.y);
						glVertex2d(ent[i].pos.x+(ent[i].size.x/2),ent[i].pos.y+(ent[i].size.y/2));

						glTexCoord2d(ent[i].x2y2.x,ent[i].x2y2.y);
						glVertex2d(ent[i].pos.x+(ent[i].size.x/2),ent[i].pos.y+(ent[i].size.y/2));
						glTexCoord2d(ent[i].x1y1.x,ent[i].x2y2.y);
						glVertex2d(ent[i].pos.x-(ent[i].size.x/2),ent[i].pos.y+(ent[i].size.y/2));
						glTexCoord2d(ent[i].x1y1.x,ent[i].x1y1.y);
						glVertex2d(ent[i].pos.x-(ent[i].size.x/2),ent[i].pos.y-(ent[i].size.y/2));
					glEnd();

				glPopMatrix();

				

				if(ent[i].type==TEXT) glDeleteTextures(1,&ent[i].data);
				ent[i].data=-1;
				ent[i].stat=DEAD;
				st.num_entities--;
				
			}
			else
			if(ent[i].type==UI || ent[i].type==TEXT_UI)
			{
				//glLoadIdentity();

				glPushMatrix();

				glColor4f(ent[i].color.r,ent[i].color.g,ent[i].color.b,ent[i].color.a);

				glTranslated(ent[i].pos.x,ent[i].pos.y,0);
				glRotatef(ent[i].ang,0.0,0.0,1.0);
				glTranslated(-ent[i].pos.x,-ent[i].pos.y,0);
				//glScalef(st.Camera.dimension.x,st.Camera.dimension.y,0);
				
				glBindTexture(GL_TEXTURE_2D,ent[i].data);
					glBegin(GL_TRIANGLES);
						glTexCoord2i(0,0);
						glVertex2d(ent[i].pos.x-(ent[i].size.x/2),ent[i].pos.y-(ent[i].size.y/2));
						glTexCoord2i(1,0);
						glVertex2d(ent[i].pos.x+(ent[i].size.x/2),ent[i].pos.y-(ent[i].size.y/2));
						glTexCoord2i(1,1);
						glVertex2d(ent[i].pos.x+(ent[i].size.x/2),ent[i].pos.y+(ent[i].size.y/2));

						glTexCoord2i(1,1);
						glVertex2d(ent[i].pos.x+(ent[i].size.x/2),ent[i].pos.y+(ent[i].size.y/2));
						glTexCoord2i(0,1);
						glVertex2d(ent[i].pos.x-(ent[i].size.x/2),ent[i].pos.y+(ent[i].size.y/2));
						glTexCoord2i(0,0);
						glVertex2d(ent[i].pos.x-(ent[i].size.x/2),ent[i].pos.y-(ent[i].size.y/2));
					glEnd();
				glPopMatrix();

				if(ent[i].type==TEXT_UI) glDeleteTextures(1,&ent[i].data);
				ent[i].data=-1;
				ent[i].stat=DEAD;
				st.num_entities--;
			}
		}
	}

	st.num_hud=0;
	st.num_tex=0;
	st.num_ui=0;

	SDL_GL_SwapWindow(wn);
}

void PlayMusic(const char *filename, uint8 loop)
{
	if(st.sound_sys.slot_ID[MUSIC_SLOT]==-1 && st.sound_sys.slotch_ID[MUSIC_CHANNEL]==-1)
	{
		FMOD_System_CreateSound(st.sound_sys.Sound_System,filename,FMOD_HARDWARE | loop==1 ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF,0,&st.sound_sys.slots[MUSIC_SLOT]);
		st.sound_sys.slot_ID[MUSIC_SLOT]=1;
		st.sound_sys.slotch_ID[MUSIC_SLOT]=MUSIC_SLOT;
	}
	
	if(st.sound_sys.slot_ID[MUSIC_SLOT]==1 || st.sound_sys.slotch_ID[MUSIC_CHANNEL]!=-1)
		FMOD_System_PlaySound(st.sound_sys.Sound_System,FMOD_CHANNEL_FREE,st.sound_sys.slots[MUSIC_SLOT],0,&st.sound_sys.channels[MUSIC_CHANNEL]);
}

void PlaySound(const char *filename, uint8 loop)
{
	int16 id=0, idc=0;
	for(register uint16 i=0;i<MAX_SOUNDS-1;i++)
	{
		if(i==MAX_SOUNDS-1 && st.sound_sys.slot_ID[i]==1)
		{
			id=666; //Magic number
			break;
		}
		else
		if(st.sound_sys.slot_ID[i]==-1)
		{
			FMOD_System_CreateSound(st.sound_sys.Sound_System,filename,FMOD_HARDWARE | loop==1 ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF,0,&st.sound_sys.slots[i]);
			st.sound_sys.slot_ID[i]=1;
			id=i;
			break;
		}
	}

	for(register uint16 i=0;i<MAX_CHANNELS-1;i++)
	{
		if(i==MAX_CHANNELS-1 && st.sound_sys.slotch_ID[i]!=-1)
		{
			id=666; //Magic number
			break;
		}
		else
		if(st.sound_sys.slotch_ID[i]==-1)
		{
			idc=i;
			st.sound_sys.slotch_ID[i]=id;
			break;
		}
	}

	if(id!=666)
		FMOD_System_PlaySound(st.sound_sys.Sound_System,FMOD_CHANNEL_FREE,st.sound_sys.slots[id],0,&st.sound_sys.channels[idc]);
}
/*
void PlayMusic(const char *filename)
{
	StopMusic();
	FreeMusic(st.music);
	st.music=Mix_LoadMUS(filename);
	Mix_PlayMusic(st.music,-1);
}
*/
/*
static void FinishMusic()
{
	StopMusic();
	FreeMusic(st.music);
}
*/
void MainSound()
{
	FMOD_System_Update(st.sound_sys.Sound_System);

	int p;

	for(register uint32 i=0;i<MAX_CHANNELS;i++)
	{
		if(st.sound_sys.slotch_ID[i]>-1)
		{
			FMOD_Channel_IsPlaying(st.sound_sys.channels[i],&p);
			if(!p)
			{
				FMOD_Sound_Release(st.sound_sys.slots[st.sound_sys.slotch_ID[i]]);
				st.sound_sys.slot_ID[st.sound_sys.slotch_ID[i]]=-1;
				st.sound_sys.slotch_ID[i]=-1;
			}
		}
	}

}

void StopAllSounds()
{
	for(register uint32 i=0;i<MAX_CHANNELS-1;i++)
	{
		if(st.sound_sys.slotch_ID[i]>-1)
		{
			FMOD_Channel_Stop(st.sound_sys.channels[i]);
			FMOD_Sound_Release(st.sound_sys.slots[st.sound_sys.slotch_ID[i]]);
			st.sound_sys.slot_ID[st.sound_sys.slotch_ID[i]]=-1;
			st.sound_sys.slotch_ID[i]=-1;
		}
	}
}

void StopMusic()
{
	if(st.sound_sys.slotch_ID[MUSIC_CHANNEL]>-1)
	{
		FMOD_Channel_Stop(st.sound_sys.channels[MUSIC_CHANNEL]);
		FMOD_Sound_Release(st.sound_sys.slots[st.sound_sys.slotch_ID[MUSIC_CHANNEL]]);
		st.sound_sys.slot_ID[st.sound_sys.slotch_ID[MUSIC_CHANNEL]]=-1;
		st.sound_sys.slotch_ID[MUSIC_CHANNEL]=-1;
	}
}
