#include "engine.h"
#include "quicklz.h"
#include "input.h"
#include <math.h>
#include <SDL_image.h>

#ifdef _VBO_RENDER
	#include "shader110.h"
#endif

#ifdef _VAO_RENDER
	#include "shader130.h"
#endif

_SETTINGS st;
_ENTITIES ent[MAX_GRAPHICS];
SDL_Event events;
_LIGHTS game_lights[MAX_LIGHTS];
_ENTITIES lmp[MAX_LIGHTMAPS];
unsigned char *DataN;
GLuint DataNT, DATA_TEST;

//Foreground + Background + Midground
//Each layer (z position) has 8 sub-layers (slots)
//The allocation is done during initialization
//z_slots keeps track of the current number of sub-layers
//DO NOT ALTER Z_BUFFER WITHOUT ALTERING Z_SLOTS

int16 z_buffer[(7*8)+1][2048];
int16 z_slot[(7*8)+1];
int8 z_used;

GLuint lm;

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)
	VB_DATAT vbd;
	VB_DATAT *vbdt;
	uint16 vbdt_num=0;
	uint16 texone_num=0;
	uint16 texone_ids[MAX_GRAPHICS];
#endif

_MGG movie;

_MGG mgg_sys[3];
_MGG mgg_map[MAX_MAP_MGG];
_MGG mgg_game[MAX_GAME_MGG];

SDL_Window *wn;

#define _ENGINE_VERSION 0.01

#define timer SDL_Delay

//Faster square root
float _fastcall mSqrt(float x)
{
	_asm fld x
	_asm fsqrt
}

void LogIn(void *userdata, int category, SDL_LogPriority log, const char *message)
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

	fprintf(file,"%s %.2f\n",st.WindowTitle,version);
	fprintf(file,"Engine started\n");
	fprintf(file,"Log created\n");

	fclose(file);
}
/*
void StartTimer()
{
	GetTicks();
}
*/
void _fastcall SetTimerM(unsigned long long int x)
{
	st.time+=x;
}

unsigned long long _fastcall GetTimerM()
{
	return st.time;
}

void FPSCounter()
{
	if((SDL_GetTicks() - st.FPSTime)!=0)
	{
		st.FPS=SDL_GetTicks()-st.FPSTime;
		st.FPS=1000/st.FPS;
		sprintf(st.WINDOW_NAME,"%s fps: %.2f",st.WindowTitle,st.FPS);
		//if(st.FPS<50) printf("%d\n",st.time);
		//st.FPS=0;
		st.FPSTime=SDL_GetTicks();
		SDL_SetWindowTitle(wn,st.WINDOW_NAME);
	}
}

void _fastcall STW(int32 *x, int32 *y)
{
	*x=((((*x*16384)/st.screenx)/st.Camera.dimension.x)+st.Camera.position.x);
	*y=((((*y*8192)/st.screeny)/st.Camera.dimension.y)+st.Camera.position.y);
}

void _fastcall STWci(int32 *x, int32 *y)
{
	*x=((((*x*16384)/st.screenx)));
	*y=((((*y*8192)/st.screeny)));
}

void _fastcall STWf(float *x, float *y)
{
	*x=(float) ((((*x*16384)/st.screenx)/st.Camera.dimension.x)+st.Camera.position.x);
	*y=(float) ((((*y*8192)/st.screeny)/st.Camera.dimension.y)+st.Camera.position.y);
}

void _fastcall STWcf(float *x, float *y)
{
	*x=(float) ((((*x*16384)/st.screenx)/st.Camera.dimension.x));
	*y=(float) ((((*y*8192)/st.screeny)/st.Camera.dimension.y));
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

void _fastcall WTS(int32 *x, int32 *y)
{
	*x-=st.Camera.position.x;
	*y-=st.Camera.position.y;

	*x=((*x*st.screenx)/16384)*st.Camera.dimension.x;
	*y=((*y*st.screeny)/8192)*st.Camera.dimension.y;
}

void _fastcall WTSci(int32 *x, int32 *y)
{
	*x=((*x*st.screenx)/16384)*st.Camera.dimension.x;
	*y=((*y*st.screeny)/8192)*st.Camera.dimension.y;
}

void _fastcall WTSf(float *x, float *y)
{
	*x-=st.Camera.position.x;
	*y-=st.Camera.position.y;

	*x=(float) ((*x*st.screenx)/16384)*st.Camera.dimension.x;
	*y=(float) ((*y*st.screeny)/8192)*st.Camera.dimension.y;
}

void _fastcall WTScf(float *x, float *y)
{
	*x=(float) ((*x*st.screenx)/16384)*st.Camera.dimension.x;
	*y=(float) ((*y*st.screeny)/8192)*st.Camera.dimension.y;
}

float mCos(int16 ang)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	return st.CosTable[ang];
}

float mSin(int16 ang)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	return st.SinTable[ang];
}

float mTan(int16 ang)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	return st.TanTable[ang];
}


//Signed
void CalCos16s(int16 ang, int16 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	*val*=(int16)(st.CosTable[ang]*100);
	*val/=100;
}

void CalSin16s(int16 ang, int16 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	*val*=(int16)(st.SinTable[ang]*100);
	*val/=100;
}

void CalTan16s(int16 ang, int16 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	*val*=(int16)(st.TanTable[ang]*100);
	*val/=100;
}

void CalCos32s(int16 ang, int32 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	*val*=(int32)(st.CosTable[ang]*100);
	*val/=100;
}

void CalSin32s(int16 ang, int32 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	*val*=(int32)(st.SinTable[ang]*100);
	*val/=100;
}

void CalTan32s(int16 ang, int32 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	*val*=(int32)(st.TanTable[ang]*100);
	*val/=100;
}


//Unsigned
void CalCos16u(int16 ang, uint16 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	*val*=(uint16)(st.CosTable[ang]*100);
	*val/=100;
}

void CalSin16u(int16 ang, uint16 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	*val*=(uint16)(st.SinTable[ang]*100);
	*val/=100;
}

void CalTan16u(int16 ang, uint16 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	*val*=(uint16)(st.TanTable[ang]*100);
	*val/=100;
}

void CalCos32u(int16 ang, uint32 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	*val*=(uint32)(st.CosTable[ang]*100);
	*val/=100;
}

void CalSin32u(int16 ang, uint32 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	*val*=(uint32)(st.SinTable[ang]*100);
	*val/=100;
}

void CalTan32u(int16 ang, uint32 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang+=3600;

	*val*=(uint32)(st.TanTable[ang]*100);
	*val/=100;
}

int8 CheckBounds(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, int8 z)
{
	int32 vertex[8];
	/*
	if(sizex>16384 || sizey>8192 || z>47)
		return 1;
	else
	{
		if(z>
	} */

}

void Quit()
{
	//ResetVB();
	InputClose();
	SDL_DestroyWindow(wn);
	SDL_Quit();
	FMOD_System_Close(st.sound_sys.Sound_System);
	FMOD_System_Release(st.sound_sys.Sound_System);
	TTF_Quit();
	exit(1);
}

unsigned char *GenerateAlphaLight(uint16 w, uint16 h)
{
	unsigned char *data;

	data=(unsigned char*) calloc(w*h*4,sizeof(unsigned char));

	if(!data)
		return NULL;

	return data;
}

uint8 FillAlphaLight(unsigned char *data, uint8 r, uint8 g, uint8 b, uint16 w, uint16 h)
{
	uint16 i, j;

	if(!data)
		return NULL;

	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			data[(i*w*4)+(j*4)]=r;
			data[(i*w*4)+(j*4)+1]=g;
			data[(i*w*4)+(j*4)+2]=b;
			data[(i*w*4)+(j*4)+3]=0;
		}
	}

	return 1;
}

uint32 AddLightToAlphaLight(unsigned char *data, uint16 w, uint16 h, uint8 r, uint8 g, uint8 b, float falloff, uint16 x, uint16 y, uint16 z, uint16 intensity, LIGHT_TYPE type)
{
	register uint16 i, j;
	float d, att;
	register uint16 col;

	if(!data)
		return 0;
	
	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			d=((x-j)*(x-j)) + ((y-i)*(y-i)) + (z*z);
			d=mSqrt(d);

			if(d==0.0f)
				d=1.0f;

			if(type==POINT_LIGHT_STRONG)
				att=1.0f/(d*d*falloff);
			else
			if(type==POINT_LIGHT_NORMAL)
				att=1.0f/(1.0f + falloff * (d*d));
			else
				att=1.0f/(d*falloff);

			col=255*att*intensity;

			if(col>255)
				col=255;
			
			if((data[(i*w*4)+(j*4)+3]+col)>255)
				data[(i*w*4)+(j*4)+3]=255;
			else
				data[(i*w*4)+(j*4)+3]+=(unsigned char)col;
			/*
			col=b*att*intensity;

			if(col>255)
				col=255;
			
			if((data[(i*w*4)+(j*4)+1]+col)>255)
				data[(i*w*4)+(j*4)+1]=255;
			else
				data[(i*w*4)+(j*4)]+=(unsigned char)col;

			col=g*att*intensity;

			if(col>255)
				col=255;
			
			if((data[(i*w*4)+(j*4)+2]+col)>255)
				data[(i*w*4)+(j*4)+2]=255;
			else
				data[(i*w*4)+(j*4)+1]+=(unsigned char)col;
				
			col=r*att*intensity;

			if(col>255)
				col=255;
			
			if((data[(i*w*4)+(j*4)+3]+col)>255)
				data[(i*w*4)+(j*4)+3]=255;
			else
				data[(i*w*4)+(j*4)+2]+=(unsigned char)col;
				*/
		}
	}

	return 1;
}

uint32 AddSpotlightToAlphaLight(unsigned char *data, uint16 w, uint16 h, uint8 r, uint8 g, uint8 b, float falloff, uint16 x, uint16 y, uint16 z, uint16 intensity, LIGHT_TYPE type, uint16 x2, uint16 y2, uint16 ang)
{
	register uint16 i, j;
	float d, att;
	float angle, angle2;
	register uint16 col;

	if(!data)
		return 0;
	
	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			d=((x-j)*(x-j)) + ((y-i)*(y-i)) + (z*z);
			d=mSqrt(d);

			if(d==0)
				d=1;

			if(type==SPOTLIGHT_STRONG)
				att=1.0f/(d*d*falloff);
			else
			if(type==SPOTLIGHT_NORMAL)
				att=1.0f/(1.0f + falloff * (d*d));
			else
				att=1.0f/(d*falloff);


			angle=atan2(x2-x,y2-y);
			angle2=atan2(j-x,i-y);

			angle2-=angle;

			angle2=(angle2*180)/pi;

			if(angle2<0) angle2*=-1;

			if(angle2>ang)
				continue;
		
			col=255*att*intensity;
			if(col>255)
				col=255;
			
			if((data[(i*w*4)+(j*4)]+col)>255)
				data[(i*w*4)+(j*4)]=255;
			else
				data[(i*w*4)+(j*4)]+=(unsigned char)col;

			col=b*att*intensity;
			if(col>255)
				col=255;
			
			if((data[(i*w*4)+(j*4)+1]+col)>255)
				data[(i*w*4)+(j*4)+1]=255;
			else
				data[(i*w*4)+(j*4)+1]+=(unsigned char)col;

			col=g*att*intensity;
			if(col>255)
				col=255;
			
			if((data[(i*w*4)+(j*4)+2]+col)>255)
				data[(i*w*4)+(j*4)+2]=255;
			else
				data[(i*w*4)+(j*4)+2]+=(unsigned char)col;
				
			col=r*att*intensity;
			if(col>255)
				col=255;
			
			if((data[(i*w*4)+(j*4)+3]+col)>255)
				data[(i*w*4)+(j*4)+3]=255;
			else
				data[(i*w*4)+(j*4)+3]+=(unsigned char)col;
				
		}
	}

	return 1;
}


GLuint GenerateAlphaLightTexture(unsigned char* data, uint16 w, uint16 h)
{
	GLuint tex;

	if(!data)
		return 0;

	glGenTextures(1,&tex);
	glBindTexture(GL_TEXTURE_2D,tex);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,data);

	glGenerateMipmap(GL_TEXTURE_2D);

	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	return tex;
}

uint8 AddLightToAlphaTexture(GLuint *tex, unsigned char* data, uint16 w, uint16 h)
{
	if(!data)
		return 0;

	glBindTexture(GL_TEXTURE_2D,*tex);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,data);

	glGenerateMipmap(GL_TEXTURE_2D);

	return 1;
}


unsigned char *GenerateLightmap(uint16 w, uint16 h)
{
	unsigned char *data;

	data=(unsigned char*) calloc(w*h*3,sizeof(unsigned char));

	if(!data)
		return NULL;

	return data;
}

uint8 FillLightmap(unsigned char *data, uint8 r, uint8 g, uint8 b, uint16 w, uint16 h)
{
	register uint16 i, j;

	if(!data)
		return NULL;

	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			data[(i*w*3)+(j*3)]=b;
			data[(i*w*3)+(j*3)+1]=g;
			data[(i*w*3)+(j*3)+2]=r;
		}
	}

	return 1;
}

uint32 AddLightToLightmap(unsigned char *data, uint16 w, uint16 h, uint8 r, uint8 g, uint8 b, float falloff, uint16 x, uint16 y, uint16 z, uint16 intensity, LIGHT_TYPE type)
{
	register uint16 i, j;
	float d, att;
	register uint16 col;
	uint8 max;

	if(!data)
		return 0;
	
	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			att=1.0f;

			d=((x-j)*(x-j)) + ((y-i)*(y-i)) + (z*z);
			d=mSqrt(d);

			if(d==0.0f)
				d=1.0f;

			//att/=(d*falloff);

			if(type==POINT_LIGHT_STRONG)
			{
				att/=(d*d*falloff);
				max=255;
			}
			else
			if(type==POINT_LIGHT_NORMAL)
			{
				att/=(1.0f + falloff * (d*d));
				max=64;
			}
			else
			{
				att/=(d*falloff);
				max=128;
			}

			col=b*att*intensity;
			
			if((data[(i*w*3)+(j*3)]+col)>max)
				data[(i*w*3)+(j*3)]=max;
			else
				data[(i*w*3)+(j*3)]+=(unsigned char)col;

			col=g*att*intensity;
			
			if((data[(i*w*3)+(j*3)+1]+col)>max)
				data[(i*w*3)+(j*3)+1]=max;
			else
				data[(i*w*3)+(j*3)+1]+=(unsigned char)col;
				
			col=r*att*intensity;
			
			if((data[(i*w*3)+(j*3)+2]+col)>max)
				data[(i*w*3)+(j*3)+2]=max;
			else
				data[(i*w*3)+(j*3)+2]+=(unsigned char)col;
				
		}
	}

	return 1;
}

uint32 AddSpotlightToLightmap(unsigned char *data, uint16 w, uint16 h, uint8 r, uint8 g, uint8 b, float falloff, uint16 x, uint16 y, uint16 z, uint16 intensity, LIGHT_TYPE type, uint16 x2, uint16 y2, uint16 ang)
{
	register  i, j;
	float d, att;
	float angle, angle2;
	register uint16 col;
	uint8 max;

	if(!data)
		return 0;
	
	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			att=1.0f;

			d=((x-j)*(x-j)) + ((y-i)*(y-i)) + (z*z);
			d=mSqrt(d);

			if(d==0)
				d=1;

			if(type==SPOTLIGHT_STRONG)
			{
				att/=(d*d*falloff);
				max=255;
			}
			else
			if(type==SPOTLIGHT_NORMAL)
			{
				att/=(1.0f + falloff * (d*d));
				max=64;
			}
			else
			{
				att/=(d*falloff);
				max=128;
			}


			angle=atan2(x2-x,y2-y);
			angle2=atan2(j-x,i-y);

			angle2-=angle;

			angle2=(angle2*180)/pi;

			if(angle2<0) angle2*=-1;

			if(angle2>ang)
				continue;
		
			col=b*att*intensity;
			
			if((data[(i*w*3)+(j*3)]+col)>max)
				data[(i*w*3)+(j*3)]=max;
			else
				data[(i*w*3)+(j*3)]+=(unsigned char)col;

			col=g*att*intensity;
			
			if((data[(i*w*3)+(j*3)+1]+col)>max)
				data[(i*w*3)+(j*3)+1]=max;
			else
				data[(i*w*3)+(j*3)+1]+=(unsigned char)col;
				
			col=r*att*intensity;
			
			if((data[(i*w*3)+(j*3)+2]+col)>max)
				data[(i*w*3)+(j*3)+2]=max;
			else
				data[(i*w*3)+(j*3)+2]+=(unsigned char)col;
				
		}
	}

	return 1;
}

GLuint GenerateLightmapTexture(unsigned char* data, uint16 w, uint16 h)
{
	GLuint tex;

	if(!data)
		return 0;

	glGenTextures(1,&tex);
	glBindTexture(GL_TEXTURE_2D,tex);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,w,h,0,GL_BGR,GL_UNSIGNED_BYTE,data);

	glGenerateMipmap(GL_TEXTURE_2D);

	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	return tex;
}

uint8 AddLightToTexture(GLuint *tex, unsigned char* data, uint16 w, uint16 h)
{
	if(!data)
		return 0;

	glBindTexture(GL_TEXTURE_2D,*tex);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,w,h,0,GL_BGR,GL_UNSIGNED_BYTE,data);

	glGenerateMipmap(GL_TEXTURE_2D);

	return 1;
}

#ifdef _VAO_RENDER
static void CreateVAO(VB_DATAT *data, uint8 type, uint8 pr)
{
	GLint pos, texc, col, texl, texr;

	if(type==1)
	{
		data->buffer_elements=1;
		data->num_elements=1;
	}

	glGenVertexArrays(1,&data->vao_id);
	glBindVertexArray(data->vao_id);

	col=glGetAttribLocation(st.renderer.Program[pr],"Color");
	pos=glGetAttribLocation(st.renderer.Program[pr],"Position");
	texc=glGetAttribLocation(st.renderer.Program[pr],"TexCoord");
	texl=glGetAttribLocation(st.renderer.Program[pr],"TexLight");
	//texr=glGetAttribLocation(st.renderer.Program[pr],"TexRepeat");


	glEnableVertexAttribArray(pos);
	glEnableVertexAttribArray(texc);
	glEnableVertexAttribArray(col);
	glEnableVertexAttribArray(texl);
	//glEnableVertexAttribArray(texr);

	glGenBuffers(1,&data->vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER,data->vbo_id);

	glBufferData(GL_ARRAY_BUFFER,((data->buffer_elements*12)*sizeof(GLfloat))+((data->buffer_elements*8)*sizeof(GLfloat))+((data->buffer_elements*16)*sizeof(GLubyte))+((data->buffer_elements*8)*sizeof(GLfloat))+((data->buffer_elements*4)*sizeof(GLfloat)),NULL,GL_STREAM_DRAW);
	/*
	glBufferSubData(GL_ARRAY_BUFFER,0,(12*data->num_elements)*sizeof(GLfloat),data->vertex);
	glBufferSubData(GL_ARRAY_BUFFER,(12*data->num_elements)*sizeof(GLfloat),(8*data->num_elements)*sizeof(GLfloat),data->texcoord);
	glBufferSubData(GL_ARRAY_BUFFER,(12*data->num_elements)*sizeof(GLfloat)+(8*data->num_elements)*sizeof(GLfloat),(((data->num_elements*16)*sizeof(GLubyte))),data->color);
	*/

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);
	glVertexAttribPointer(texc,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) ((12*data->buffer_elements)*sizeof(GLfloat)));
	glVertexAttribPointer(col,4,GL_UNSIGNED_BYTE,GL_TRUE,0,(GLvoid*) ((12*data->buffer_elements)*sizeof(GLfloat)+(8*data->buffer_elements)*sizeof(GLfloat)));
	glVertexAttribPointer(texl,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) (((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))+((16*data->buffer_elements)*sizeof(GLubyte))));
	//glVertexAttribPointer(texr,4,GL_FLOAT,GL_FALSE,0,(GLvoid*) (((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))+((16*data->buffer_elements)*sizeof(GLubyte))+((data->buffer_elements*8)*sizeof(GLfloat))));


	glGenBuffers(1,&data->ibo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,data->ibo_id);

	if(type==1)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*data->buffer_elements)*sizeof(GLushort),data->index,GL_STATIC_DRAW);
	else
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*data->buffer_elements)*sizeof(GLushort),NULL,GL_DYNAMIC_DRAW);
	
	glBindVertexArray(0);

	glDisableVertexAttribArray(pos);
	glDisableVertexAttribArray(texc);
	glDisableVertexAttribArray(col);
	glDisableVertexAttribArray(texl);
	//glDisableVertexAttribArray(texr);

	data->num_elements2=0;
	
	/*
	free(data->texcoord);
	free(data->index);
	free(data->color);
	free(data->vertex);
	*/

}

static int16 UpdateVAO(VB_DATAT *data, uint8 upd_buff, uint8 upd_index, uint8 pr)
{
	GLint pos, texc, col, texl, texr;
	GLenum error;

	//glUseProgram(st.renderer.Program[pr]);

	if(!upd_buff)
	{
		if(data->num_elements>data->buffer_elements) return 1;
		else
		{
			
			glBindVertexArray(data->vao_id);
			glBindBuffer(GL_ARRAY_BUFFER,data->vbo_id);

			glBufferSubData(GL_ARRAY_BUFFER,0,(12*data->num_elements)*sizeof(GLfloat),data->vertex);
			glBufferSubData(GL_ARRAY_BUFFER,(12*data->buffer_elements)*sizeof(GLfloat),(8*data->num_elements)*sizeof(GLfloat),data->texcoord);
			glBufferSubData(GL_ARRAY_BUFFER,(((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))),(((data->num_elements*16)*sizeof(GLubyte))),data->color);
			glBufferSubData(GL_ARRAY_BUFFER,(((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))+((data->buffer_elements*16)*sizeof(GLubyte))),(data->num_elements*8)*sizeof(float),data->texcoordlight);
		//	glBufferSubData(GL_ARRAY_BUFFER,(((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))+((data->buffer_elements*16)*sizeof(GLubyte))+((data->num_elements*8)*sizeof(float))),(data->num_elements*4)*sizeof(float),data->texrepeat);

			if(upd_index)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,data->ibo_id);
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,0,(6*data->num_elements)*sizeof(GLushort),data->index);
				//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
			}

			glBindVertexArray(0);
			
			
			free(data->texcoord);
			free(data->index);
			free(data->color);
			free(data->vertex);
			free(data->texcoordlight);
			//free(data->texrepeat);
			
		}
	}
	else
	if(upd_buff==1)
	{
		
		glBindVertexArray(data->vao_id);

		col=glGetAttribLocation(st.renderer.Program[pr],"Color");
		pos=glGetAttribLocation(st.renderer.Program[pr],"Position");
		texc=glGetAttribLocation(st.renderer.Program[pr],"TexCoord");
		texl=glGetAttribLocation(st.renderer.Program[pr],"TexLight");
		//texr=glGetAttribLocation(st.renderer.Program[pr],"TexRepeat");


		glEnableVertexAttribArray(pos);
		glEnableVertexAttribArray(texc);
		glEnableVertexAttribArray(col);
		glEnableVertexAttribArray(texl);
		//glEnableVertexAttribArray(texr);

		glBindBuffer(GL_ARRAY_BUFFER,data->vbo_id);

		glBufferData(GL_ARRAY_BUFFER,((data->buffer_elements*12)*sizeof(GLfloat))+((data->buffer_elements*8)*sizeof(GLfloat))+((data->buffer_elements*16)*sizeof(GLubyte))+((data->buffer_elements*8)*sizeof(GLfloat))+((data->buffer_elements*4)*sizeof(GLfloat)),NULL,GL_STREAM_DRAW);

		glBufferSubData(GL_ARRAY_BUFFER,0,(12*data->num_elements)*sizeof(GLfloat),data->vertex);
		glBufferSubData(GL_ARRAY_BUFFER,(12*data->buffer_elements)*sizeof(GLfloat),(8*data->num_elements)*sizeof(GLfloat),data->texcoord);
		glBufferSubData(GL_ARRAY_BUFFER,(((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))),(((data->num_elements*16)*sizeof(GLubyte))),data->color);
		glBufferSubData(GL_ARRAY_BUFFER,(((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))+((data->buffer_elements*16)*sizeof(GLubyte))),(data->num_elements*8)*sizeof(float),data->texcoordlight);
		//glBufferSubData(GL_ARRAY_BUFFER,(((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))+((data->buffer_elements*16)*sizeof(GLubyte))+((data->num_elements*8)*sizeof(float))),(data->num_elements*4)*sizeof(float),data->texrepeat);


		glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);
		glVertexAttribPointer(texc,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) ((12*data->buffer_elements)*sizeof(GLfloat)));
		glVertexAttribPointer(col,4,GL_UNSIGNED_BYTE,GL_TRUE,0,(GLvoid*) (((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))));
		glVertexAttribPointer(texl,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) (((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))+((16*data->buffer_elements)*sizeof(GLubyte))));
		//glVertexAttribPointer(texr,4,GL_FLOAT,GL_FALSE,0,(GLvoid*) (((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))+((16*data->buffer_elements)*sizeof(GLubyte))+((data->buffer_elements*8)*sizeof(GLfloat))));


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,data->ibo_id);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*data->buffer_elements)*sizeof(GLushort),NULL,GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,0,(6*data->num_elements)*sizeof(GLushort),data->index);

		glBindVertexArray(0);

		glDisableVertexAttribArray(pos);
		glDisableVertexAttribArray(texc);
		glDisableVertexAttribArray(col);
		glDisableVertexAttribArray(texl);
		//glDisableVertexAttribArray(texr);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
		//glBindBuffer(GL_ARRAY_BUFFER,0);

		free(data->texcoord);
		free(data->index);
		free(data->color);
		free(data->vertex);
		free(data->texcoordlight);
		//free(data->texrepeat);
	}
	else
	if(upd_buff==2)
	{
		glBindVertexArray(data->vao_id);
		glBindBuffer(GL_ARRAY_BUFFER,data->vbo_id);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,data->ibo_id);

		col=glGetAttribLocation(st.renderer.Program[pr],"Color");
		pos=glGetAttribLocation(st.renderer.Program[pr],"Position");
		texc=glGetAttribLocation(st.renderer.Program[pr],"TexCoord");
		texl=glGetAttribLocation(st.renderer.Program[pr],"TexLight");
		//texr=glGetAttribLocation(st.renderer.Program[pr],"TexRepeat");

		glEnableVertexAttribArray(pos);
		glEnableVertexAttribArray(texc);
		glEnableVertexAttribArray(col);
		glEnableVertexAttribArray(texl);
		//glEnableVertexAttribArray(texr);

		glBufferData(GL_ARRAY_BUFFER,((data->buffer_elements*12)*sizeof(GLfloat))+((data->buffer_elements*8)*sizeof(GLfloat))+((data->buffer_elements*16)*sizeof(GLubyte))+((data->buffer_elements*8)*sizeof(GLfloat))+((data->buffer_elements*4)*sizeof(GLfloat)),NULL,GL_STREAM_DRAW);

		glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);
		glVertexAttribPointer(texc,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) ((12*data->buffer_elements)*sizeof(GLfloat)));
		glVertexAttribPointer(col,4,GL_UNSIGNED_BYTE,GL_TRUE,0,(GLvoid*) (((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))));
		glVertexAttribPointer(texl,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) (((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))+((16*data->buffer_elements)*sizeof(GLubyte))));
		//glVertexAttribPointer(texr,4,GL_FLOAT,GL_FALSE,0,(GLvoid*) (((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))+((16*data->buffer_elements)*sizeof(GLubyte))+((data->buffer_elements*8)*sizeof(GLfloat))));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,data->ibo_id);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*data->buffer_elements)*sizeof(GLushort),NULL,GL_DYNAMIC_DRAW);

		glDisableVertexAttribArray(pos);
		glDisableVertexAttribArray(texc);
		glDisableVertexAttribArray(col);
		glDisableVertexAttribArray(texl);
		//glDisableVertexAttribArray(texr);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
		//glBindBuffer(GL_ARRAY_BUFFER,0);
		glBindVertexArray(0);
	}

	//glUseProgram(0);

	return 0;
}

void ResetVB()
{
	uint16 i=0;

	for(i=0;i<vbdt_num;i++)
	{
		if(vbdt[i].color)
			free(vbdt[i].color);

		if(vbdt[i].index)
			free(vbdt[i].index);

		if(vbdt[i].texcoord)
			free(vbdt[i].texcoord);

		if(vbdt[i].vertex)
			free(vbdt[i].vertex);
	}

	free(vbdt);

	vbdt_num=0;
}

#endif

#ifdef _VBO_RENDER
static void CreateVBO(VB_DATAT *data, uint8 type)
{
	glGenBuffers(1,&data->vbo_id);
	glGenBuffers(1,&data->ibo_id);

	glBindBuffer(GL_ARRAY_BUFFER,data->vbo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,data->ibo_id);

	glBufferData(GL_ARRAY_BUFFER,(((data->num_elements*12)*sizeof(GLfloat)))+(((data->num_elements*8)*sizeof(GLfloat)))+(((data->num_elements*16)*sizeof(GLubyte))),NULL,GL_STREAM_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER,0,(12*data->num_elements)*sizeof(GLfloat),data->vertex);
	//glBufferSubData(GL_ARRAY_BUFFER,(12*data->num_elements)*sizeof(GLfloat),(8*data->num_elements)*sizeof(GLfloat),data->texcoord);
	//glBufferSubData(GL_ARRAY_BUFFER,(12*data->num_elements)*sizeof(GLfloat)+(8*data->num_elements)*sizeof(GLfloat),(((data->num_elements*16)*sizeof(GLubyte))),data->color);

	if(type==1)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*data->buffer_elements)*sizeof(GLushort),data->index,GL_STATIC_DRAW);
	else
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*data->buffer_elements)*sizeof(GLushort),NULL,GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	/*
	free(data->texcoord);
	free(data->index);
	free(data->color);
	free(data->vertex);
	*/
}

static int16 UpdateVBO(VB_DATAT *data, uint8 upd_buff, uint8 upd_index, uint8 pr)
{
	GLint pos, texc, col;
	GLenum error;

	//glUseProgram(st.renderer.Program[pr]);

	if(!upd_buff)
	{
		if(data->num_elements>data->buffer_elements) return 1;
		else
		{
			
			//glBindVertexArray(data->vao_id);
			glBindBuffer(GL_ARRAY_BUFFER,data->vbo_id);

			glBufferSubData(GL_ARRAY_BUFFER,0,(12*data->num_elements)*sizeof(GLfloat),data->vertex);
			glBufferSubData(GL_ARRAY_BUFFER,(12*data->buffer_elements)*sizeof(GLfloat),(8*data->num_elements)*sizeof(GLfloat),data->texcoord);
			glBufferSubData(GL_ARRAY_BUFFER,(((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))),(((data->num_elements*16)*sizeof(GLubyte))),data->color);

			if(upd_index)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,data->ibo_id);
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,0,(6*data->num_elements)*sizeof(GLushort),data->index);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
			}

			//glBindVertexArray(0);

			glBindBuffer(GL_ARRAY_BUFFER,0);
			
			free(data->texcoord);
			free(data->index);
			free(data->color);
			free(data->vertex);
			
		}
	}
	else
	if(upd_buff==1)
	{
		
		//glBindVertexArray(data->vao_id);

		pos=glGetAttribLocation(st.renderer.Program[pr],"Position");
		texc=glGetAttribLocation(st.renderer.Program[pr],"TexCoord");
		col=glGetAttribLocation(st.renderer.Program[pr],"Color");

		glEnableVertexAttribArray(pos);
		glEnableVertexAttribArray(texc);
		glEnableVertexAttribArray(col);

		glBindBuffer(GL_ARRAY_BUFFER,data->vbo_id);

		glBufferData(GL_ARRAY_BUFFER,(((data->buffer_elements*12)*sizeof(GLfloat)))+(((data->buffer_elements*8)*sizeof(GLfloat)))+(((data->buffer_elements*16)*sizeof(GLubyte))),NULL,GL_STATIC_DRAW);

		glBufferSubData(GL_ARRAY_BUFFER,0,(12*data->num_elements)*sizeof(GLfloat),data->vertex);
		glBufferSubData(GL_ARRAY_BUFFER,(12*data->buffer_elements)*sizeof(GLfloat),(8*data->num_elements)*sizeof(GLfloat),data->texcoord);
		glBufferSubData(GL_ARRAY_BUFFER,(((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))),(((data->num_elements*16)*sizeof(GLubyte))),data->color);

		glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);
		glVertexAttribPointer(texc,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) ((12*data->buffer_elements)*sizeof(GLfloat)));
		glVertexAttribPointer(col,4,GL_UNSIGNED_BYTE,GL_TRUE,0,(GLvoid*) (((12*data->buffer_elements)*sizeof(GLfloat))+((8*data->buffer_elements)*sizeof(GLfloat))));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,data->ibo_id);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*data->buffer_elements)*sizeof(GLushort),NULL,GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,0,(6*data->num_elements)*sizeof(GLushort),data->index);

		//glBindVertexArray(0);

		glDisableVertexAttribArray(pos);
		glDisableVertexAttribArray(texc);
		glDisableVertexAttribArray(col);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
		glBindBuffer(GL_ARRAY_BUFFER,0);

		free(data->texcoord);
		free(data->index);
		free(data->color);
		free(data->vertex);
	}
	else
	if(upd_buff==2)
	{
		//glBindVertexArray(data->vao_id);
		glBindBuffer(GL_ARRAY_BUFFER,data->vbo_id);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,data->ibo_id);

		pos=glGetAttribLocation(st.renderer.Program[pr],"Position");
		texc=glGetAttribLocation(st.renderer.Program[pr],"TexCoord");
		col=glGetAttribLocation(st.renderer.Program[pr],"Color");

		glEnableVertexAttribArray(pos);
		glEnableVertexAttribArray(texc);
		glEnableVertexAttribArray(col);

		glBufferData(GL_ARRAY_BUFFER,(((data->buffer_elements*12)*sizeof(GLfloat)))+(((data->buffer_elements*8)*sizeof(GLfloat)))+(((data->buffer_elements*16)*sizeof(GLubyte))),NULL,GL_STREAM_DRAW);

		glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);
		glVertexAttribPointer(texc,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) ((12*data->buffer_elements)*sizeof(GLfloat)));
		glVertexAttribPointer(col,4,GL_UNSIGNED_BYTE,GL_TRUE,0,(GLvoid*) ((12*data->buffer_elements)*sizeof(GLfloat)+(8*data->buffer_elements)*sizeof(GLfloat)));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,data->ibo_id);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*data->buffer_elements)*sizeof(GLushort),NULL,GL_DYNAMIC_DRAW);

		glDisableVertexAttribArray(pos);
		glDisableVertexAttribArray(texc);
		glDisableVertexAttribArray(col);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
		glBindBuffer(GL_ARRAY_BUFFER,0);
		//glBindVertexArray(0);
	}

	//glUseProgram(0);

	return 0;
}


#endif
/*
static int VBBuffProcess(void *data)
{
	 uint16 i=0;
	uint16 current_ent=0;
	uint8 done=0;

	while(!done)
	{
		if(st.num_entities>current_ent)
		{
			i=current_ent+1;

			
		}
	}

}
*/

void Init()
{	
	 uint16 i=0, j=0, l=0;
	 float k=0;

	int check;
	FMOD_RESULT result;

	GLenum checkfb;

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

#if defined (_VAO_RENDER) || defined (_VBO_RENDER)
	GLint statusCM[32], statusLK[32];
	GLchar logs[32][1024];

#ifdef _DEBUG
	printf("Waiting...\n");
	system("pause");
#endif

	CreateLog();

	vbd.vertex=(float*) malloc(12*sizeof(float));
	vbd.texcoord=(float*) malloc(8*sizeof(float));
#endif
	vbd.color=(GLubyte*) malloc(16*sizeof(GLubyte));
	vbd.index=(GLushort*) malloc(6*sizeof(GLushort));
	
#if defined (_VAO_RENDER) || defined (_VBO_RENDER)

	vbd.vertex[0]=-1.0f;
	vbd.vertex[1]=-1.0f;
	vbd.vertex[2]=0.0f;
	vbd.vertex[3]=1.0f;
	vbd.vertex[4]=-1.0f;
	vbd.vertex[5]=0.0f;
	vbd.vertex[6]=1.0f;
	vbd.vertex[7]=1.0f;
	vbd.vertex[8]=0.0f;
	vbd.vertex[9]=-1.0f;
	vbd.vertex[10]=1.0f;
	vbd.vertex[11]=0.0f;

	vbd.texcoord[0]=0.0f;
	vbd.texcoord[1]=0.0f;
	vbd.texcoord[2]=1.0f;
	vbd.texcoord[3]=0.0f;
	vbd.texcoord[4]=1.0f;
	vbd.texcoord[5]=1.0f;
	vbd.texcoord[6]=0.0f;
	vbd.texcoord[7]=1.0f;

#endif

	vbd.color[0]=255;
	vbd.color[1]=255;
	vbd.color[2]=255;
	vbd.color[3]=255;
	vbd.color[4]=255;
	vbd.color[5]=255;
	vbd.color[6]=255;
	vbd.color[7]=255;
	vbd.color[8]=255;
	vbd.color[9]=255;
	vbd.color[10]=255;
	vbd.color[11]=255;
	vbd.color[12]=255;
	vbd.color[13]=255;
	vbd.color[14]=255;
	vbd.color[15]=255;

	vbd.index[0]=0;
	vbd.index[1]=1;
	vbd.index[2]=2;
	vbd.index[3]=2;
	vbd.index[4]=3;
	vbd.index[5]=0;
	
	vbdt_num=0;

#endif

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
	/*
	for(i=0;i<MAX_SOUNDS;i++)
		st.sound_sys.slot_ID[i]=-1;

	for(i=0;i<MAX_CHANNELS;i++)
		st.sound_sys.slotch_ID[i]=-1;
		*/
	if(TTF_Init()==-1)
	{
		LogApp("Error while initializing SDL TTF : %s",TTF_GetError());
			Quit();
	}

	LogApp("SDL TTF initialized");
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL,1);

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

#ifdef _VAO_RENDER
	st.renderer.VAO_ON=0;
#endif

#ifdef _VBO_RENDER
	st.renderer.VBO_ON=0;
#endif

#ifdef _VA_RENDER
	st.renderer.VA_ON=0;
#endif

#ifdef _VAO_RENDER
	if(!GLEE_VERSION_3_0 || strstr((char const*) glGetString(GL_EXTENSIONS),"GL_ARB_vertex_array_object")==NULL)
	{
		LogApp("VAO not supported, check your video's card driver for updates... Using VBO instead");
		st.renderer.VAO_ON=0;

#ifdef _VBO_RENDER

		if(strstr((char const*) glGetString(GL_EXTENSIONS),"GL_ARB_vertex_buffer_object")==NULL)
		{
#ifdef _VA_RENDER
			st.renderer.VA_ON=1;
			LogApp("VBOs not supported, check your video's card driver for updates... Using VA instead!!");
#else 
			LogApp("Your video card is not adequate to play this game... Goodbye!!");
			Quit();
#endif
		}
		else
			st.renderer.VBO_ON=1;
#endif

#ifdef _VA_RENDER
		if(strstr((char const*) glGetString(GL_EXTENSIONS),"GL_EXT_vertex_array")==NULL)
		{
			LogApp("Your video card is not adequate to play this game... Goodbye!!");
			Quit();
		}
#endif
	}
	else
		st.renderer.VAO_ON=1;
#endif

#if !defined (_VAO_RENDER) && defined (_VBO_RENDER)
	if(strstr((char const*) glGetString(GL_EXTENSIONS),"GL_ARB_vertex_buffer_object")==NULL)
	{
#ifdef _VA_RENDER
		st.renderer.VA_ON=1;
		LogApp("VBOs not supported, check your video's card driver for updates... Using VA instead!!");
#else
		LogApp("Your video card is not adequate to play this game... Goodbye!!");
		Quit();
#endif

	}
	else
		st.renderer.VBO_ON=1;
#endif

#if !defined (_VAO_RENDER) && !defined (_VBO_RENDER) && defined (_VA_RENDER)
	if(strstr((char const*) glGetString(GL_EXTENSIONS),"GL_EXT_vertex_array")==NULL)
	{
		LogApp("Your video card is not adequate to play this game... Goodbye!!");
		Quit();
	}
	else
		st.renderer.VA_ON=1;
#endif

#if defined (_VAO_RENDER) || defined (_VBO_RENDER)
	if(strstr((char const*) glGetString(GL_EXTENSIONS),"GL_EXT_framebuffer")==NULL)
	{
#ifdef _VA_RENDER
		st.renderer.VA_ON=1;
		LogApp("Framebuffer object not support, check your video's card driver for updates... Usign VA instead!");
#else
		LogApp("Your video card is not adequate to play this game... Goodbye!!");
		Quit();
#endif
	}
#endif

	//Initialize OpenGL
	glClearColor(0,0,0,0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0,0,st.screenx,st.screeny);
	glEnable(GL_TEXTURE_2D);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	SDL_GL_SetSwapInterval(st.vsync);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

	//glHint(GL_GENERATE_MIPMAP_HINT,GL_FASTEST);

	if(st.renderer.VAO_ON)
		st.renderer.shader_version=130;
	else
	if(st.renderer.VBO_ON)
		st.renderer.shader_version=110;
	else
		st.renderer.shader_version=0;

	//Initialize with 32 slots
	//Reallocation is done if necessary
	//z_slots keeps track of the current number of slots available

	//z_buffer=(int16**) calloc((3*8)*32,sizeof(int16));
	//z_slots=32;

	memset(z_buffer,0,((7*8)*2048)*sizeof(int16));
	memset(z_slot,0,(7*8)*sizeof(int16));

	z_used=0;

#if defined (_VAO_RENDER) || defined (_VBO_RENDER)
	if(st.renderer.VAO_ON || st.renderer.VBO_ON)
	{
		glGenFramebuffers(1,&st.renderer.FBO[0]);
		glBindFramebuffer(GL_FRAMEBUFFER,st.renderer.FBO[0]);

		glGenRenderbuffers(1,&st.renderer.RBO[0]);
		glBindRenderbuffer(GL_RENDERBUFFER,st.renderer.RBO[0]);
		glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT,st.screenx,st.screeny);

		glBindRenderbuffer(GL_RENDERBUFFER,0);

		glGenTextures(1,&st.renderer.FBTex[0]);
		glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[0]);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,st.screenx,st.screeny,0,GL_BGRA,GL_UNSIGNED_BYTE,NULL);

		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,st.renderer.FBTex[0],0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,st.renderer.RBO[0]);

		glGenTextures(1,&st.renderer.FBTex[1]);
		glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[1]);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,st.screenx,st.screeny,0,GL_BGRA,GL_UNSIGNED_BYTE,NULL);

		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,st.renderer.FBTex[1],0);

		glGenTextures(1,&st.renderer.FBTex[2]);
		glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[2]);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,st.screenx,st.screeny,0,GL_BGRA,GL_UNSIGNED_BYTE,NULL);

		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT2,GL_TEXTURE_2D,st.renderer.FBTex[2],0);

		glGenTextures(1,&st.renderer.FBTex[3]);
		glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[3]);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,st.screenx,st.screeny,0,GL_BGRA,GL_UNSIGNED_BYTE,NULL);

		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT3,GL_TEXTURE_2D,st.renderer.FBTex[3],0);

		glBindFramebuffer(GL_FRAMEBUFFER,0);
		
		st.renderer.Buffers[0]=GL_COLOR_ATTACHMENT0;
		st.renderer.Buffers[1]=GL_COLOR_ATTACHMENT1;
		st.renderer.Buffers[2]=GL_COLOR_ATTACHMENT2;
		st.renderer.Buffers[3]=GL_COLOR_ATTACHMENT3;

SHADER_CREATION:

		st.renderer.VShader[0]=glCreateShader(GL_VERTEX_SHADER);
		//st.renderer.FShader[0]=glCreateShader(GL_FRAGMENT_SHADER);
		//st.renderer.FShader[1]=glCreateShader(GL_FRAGMENT_SHADER);
		st.renderer.FShader[2]=glCreateShader(GL_FRAGMENT_SHADER);
		st.renderer.FShader[3]=glCreateShader(GL_FRAGMENT_SHADER);
		st.renderer.FShader[4]=glCreateShader(GL_FRAGMENT_SHADER);
		st.renderer.FShader[5]=glCreateShader(GL_FRAGMENT_SHADER);
		st.renderer.FShader[6]=glCreateShader(GL_FRAGMENT_SHADER);

#ifdef _VAO_RENDER

		if(st.renderer.shader_version==130)
		{
			glShaderSource(st.renderer.VShader[0],1,(const GLchar**) Texture_VShader,0);
			//glShaderSource(st.renderer.FShader[0],1,(const GLchar**) Lighting_FShader,0);
			//glShaderSource(st.renderer.FShader[1],1,(const GLchar**) Normal_FShader,0);
			glShaderSource(st.renderer.FShader[2],1,(const GLchar**) Texture_FShader,0);
			glShaderSource(st.renderer.FShader[3],1,(const GLchar**) Lightmap_FShader,0);
			glShaderSource(st.renderer.FShader[4],1,(const GLchar**) TextureNoT_FShader,0);
			glShaderSource(st.renderer.FShader[5],1,(const GLchar**) TextureT_FShader,0);
			glShaderSource(st.renderer.FShader[6],1,(const GLchar**) Blend_FShader,0);
		}
#endif
		//else
#ifdef _VBO_RENDER
		if(st.renderer.shader_version==110)
		{
			
			glShaderSource(st.renderer.VShader[0],1,(const GLchar**) Texture_VShader110,0);
			glShaderSource(st.renderer.FShader[2],1,(const GLchar**) Texture_FShader110,0);
			glShaderSource(st.renderer.FShader[3],1,(const GLchar**) Lightmap_FShader110,0);
			glShaderSource(st.renderer.FShader[4],1,(const GLchar**) TextureNoT_FShader110,0);
			glShaderSource(st.renderer.FShader[5],1,(const GLchar**) TextureT_FShader110,0);
			glShaderSource(st.renderer.FShader[6],1,(const GLchar**) Blend_FShader110,0);
		}
#endif

		glCompileShader(st.renderer.VShader[0]);
		//glCompileShader(st.renderer.FShader[0]);
		//glCompileShader(st.renderer.FShader[1]);
		glCompileShader(st.renderer.FShader[2]);
		glCompileShader(st.renderer.FShader[3]);
		glCompileShader(st.renderer.FShader[4]);
		glCompileShader(st.renderer.FShader[5]);
		glCompileShader(st.renderer.FShader[6]);

		glGetShaderiv(st.renderer.VShader[0],GL_COMPILE_STATUS,&statusCM[0]);
		//glGetShaderiv(st.renderer.FShader[0],GL_COMPILE_STATUS,&statusCM[1]);
		//glGetShaderiv(st.renderer.FShader[1],GL_COMPILE_STATUS,&statusCM[2]);
		glGetShaderiv(st.renderer.FShader[2],GL_COMPILE_STATUS,&statusCM[3]);
		glGetShaderiv(st.renderer.FShader[3],GL_COMPILE_STATUS,&statusCM[4]);
		glGetShaderiv(st.renderer.FShader[4],GL_COMPILE_STATUS,&statusCM[5]);
		glGetShaderiv(st.renderer.FShader[5],GL_COMPILE_STATUS,&statusCM[6]);
		glGetShaderiv(st.renderer.FShader[6],GL_COMPILE_STATUS,&statusCM[7]);

		if(!statusCM[0] || !statusCM[3] || !statusCM[4] || !statusCM[5] || !statusCM[6] || !statusCM[7])
		{
			
			for(i=0;i<8;i++)
			{
				if(i==1 || i==2) continue;

				if(!statusCM[i])
				{
					if(i==1)
						glGetShaderInfoLog(st.renderer.FShader[0],1024,NULL,logs[0]);
					else
					if(i==0)
						glGetShaderInfoLog(st.renderer.VShader[0],1024,NULL,logs[0]);
					else
						glGetShaderInfoLog(st.renderer.FShader[i-1],1024,NULL,logs[0]);

					LogApp("Shader %d: %s",i ,logs[0]);
					LogApp("Counld not compile shader");
				}
			}

#ifdef _VBO_RENDER
			if(st.renderer.shader_version==130)
			{
				LogApp("Changing to VBO...");

				if(strstr((char const*) glGetString(GL_EXTENSIONS),"GL_ARB_vertex_buffer_object")==NULL)
				{
#ifdef _VA_RENDER
					st.renderer.VA_ON=1;
					LogApp("VBOs not supported, check your video's card driver for updates... Using VA instead!!");
#else
					LogApp("Your video card is not adequate to play this game... Goodbye!!");
					Quit();
#endif
				}
				else
				{
					st.renderer.VBO_ON=1;
					st.renderer.shader_version=110;

					glDeleteShader(st.renderer.VShader[0]);
					glDeleteShader(st.renderer.VShader[3]);
					glDeleteShader(st.renderer.VShader[4]);
					glDeleteShader(st.renderer.VShader[5]);
					glDeleteShader(st.renderer.VShader[6]);
					glDeleteShader(st.renderer.VShader[7]);

					goto SHADER_CREATION;
				}
			}
			else
			if(st.renderer.shader_version==110)
			{

#ifdef _VA_RENDER

				LogApp("Changing to VA...");

				if(strstr((char const*) glGetString(GL_EXTENSIONS),"GL_EXT_vertex_array")==NULL)
				{
					LogApp("Your video card is not adequate to play this game... Goodbye!!");
					Quit();
				}
				else
				{
					st.renderer.VA_ON=1;

					glDeleteShader(st.renderer.VShader[0]);
					glDeleteShader(st.renderer.VShader[3]);
					glDeleteShader(st.renderer.VShader[4]);
					glDeleteShader(st.renderer.VShader[5]);
					glDeleteShader(st.renderer.VShader[6]);
					glDeleteShader(st.renderer.VShader[7]);
				}
			}
#endif

#endif
		}
		else
		if(statusCM[0] && statusCM[3] && statusCM[4] && statusCM[5] && statusCM[6] && statusCM[7])
		{
			//st.renderer.Program[0]=glCreateProgram();
			//st.renderer.Program[1]=glCreateProgram();
			st.renderer.Program[2]=glCreateProgram();
			st.renderer.Program[3]=glCreateProgram();
			st.renderer.Program[4]=glCreateProgram();
			st.renderer.Program[5]=glCreateProgram();
			st.renderer.Program[6]=glCreateProgram();

			//glAttachShader(st.renderer.Program[0],st.renderer.VShader[0]);
			//glAttachShader(st.renderer.Program[0],st.renderer.FShader[0]);
			
			//glAttachShader(st.renderer.Program[1],st.renderer.VShader[0]);
			//glAttachShader(st.renderer.Program[1],st.renderer.FShader[1]);

			glAttachShader(st.renderer.Program[2],st.renderer.VShader[0]);
			glAttachShader(st.renderer.Program[2],st.renderer.FShader[2]);
			
			glAttachShader(st.renderer.Program[3],st.renderer.VShader[0]);
			glAttachShader(st.renderer.Program[3],st.renderer.FShader[3]);

			glAttachShader(st.renderer.Program[4],st.renderer.VShader[0]);
			glAttachShader(st.renderer.Program[4],st.renderer.FShader[4]);

			glAttachShader(st.renderer.Program[5],st.renderer.VShader[0]);
			glAttachShader(st.renderer.Program[5],st.renderer.FShader[5]);

			glAttachShader(st.renderer.Program[6],st.renderer.VShader[0]);
			glAttachShader(st.renderer.Program[6],st.renderer.FShader[6]);

			//glLinkProgram(st.renderer.Program[0]);
			//glLinkProgram(st.renderer.Program[1]);
			glLinkProgram(st.renderer.Program[2]);
			glLinkProgram(st.renderer.Program[3]);
			glLinkProgram(st.renderer.Program[4]);
			glLinkProgram(st.renderer.Program[5]);
			glLinkProgram(st.renderer.Program[6]);

			//glGetProgramiv(st.renderer.Program[0],GL_LINK_STATUS,&statusLK[0]);
			//glGetProgramiv(st.renderer.Program[1],GL_LINK_STATUS,&statusLK[1]);
			glGetProgramiv(st.renderer.Program[2],GL_LINK_STATUS,&statusLK[2]);
			glGetProgramiv(st.renderer.Program[3],GL_LINK_STATUS,&statusLK[3]);
			glGetProgramiv(st.renderer.Program[4],GL_LINK_STATUS,&statusLK[4]);
			glGetProgramiv(st.renderer.Program[5],GL_LINK_STATUS,&statusLK[5]);
			glGetProgramiv(st.renderer.Program[6],GL_LINK_STATUS,&statusLK[6]);
			/*
			if(!statusLK[0])
			{
				glDeleteProgram(st.renderer.Program[0]);
				glDeleteShader(st.renderer.VShader[0]);
				glDeleteShader(st.renderer.FShader[0]);
			}
			else
			{
				glDetachShader(st.renderer.Program[0],st.renderer.VShader[0]);
				glDetachShader(st.renderer.Program[0],st.renderer.FShader[0]);
			}
			
			if(!statusLK[1])
			{
				glDeleteProgram(st.renderer.Program[1]);
				glDeleteShader(st.renderer.VShader[0]);
				glDeleteShader(st.renderer.FShader[1]);
			}
			else
			{
				glDetachShader(st.renderer.Program[1],st.renderer.VShader[0]);
				glDetachShader(st.renderer.Program[1],st.renderer.FShader[1]);
			}
			*/
			if(!statusLK[2])
			{
				glDeleteProgram(st.renderer.Program[2]);
				glDeleteShader(st.renderer.VShader[0]);
				glDeleteShader(st.renderer.FShader[2]);
			}
			else
			{
				glDetachShader(st.renderer.Program[2],st.renderer.VShader[0]);
				glDetachShader(st.renderer.Program[2],st.renderer.FShader[2]);
			}
			
			if(!statusLK[3])
			{
				glGetProgramInfoLog(st.renderer.Program[3],1024,NULL,logs[0]);
				glDeleteProgram(st.renderer.Program[3]);
				glDeleteShader(st.renderer.VShader[0]);
				glDeleteShader(st.renderer.FShader[3]);
			}
			else
			{
				glDetachShader(st.renderer.Program[3],st.renderer.VShader[0]);
				glDetachShader(st.renderer.Program[3],st.renderer.FShader[3]);
			}

			if(!statusLK[4])
			{
				glGetProgramInfoLog(st.renderer.Program[4],1024,NULL,logs[0]);
				glDeleteProgram(st.renderer.Program[4]);
				glDeleteShader(st.renderer.VShader[0]);
				glDeleteShader(st.renderer.FShader[4]);
			}
			else
			{
				glDetachShader(st.renderer.Program[4],st.renderer.VShader[0]);
				glDetachShader(st.renderer.Program[4],st.renderer.FShader[4]);
			}

			if(!statusLK[5])
			{
				glGetProgramInfoLog(st.renderer.Program[5],1024,NULL,logs[0]);
				glDeleteProgram(st.renderer.Program[5]);
				glDeleteShader(st.renderer.VShader[0]);
				glDeleteShader(st.renderer.FShader[5]);
			}
			else
			{
				glDetachShader(st.renderer.Program[5],st.renderer.VShader[0]);
				glDetachShader(st.renderer.Program[5],st.renderer.FShader[5]);
			}

			if(!statusLK[6])
			{
				glGetProgramInfoLog(st.renderer.Program[6],1024,NULL,logs[0]);
				glDeleteProgram(st.renderer.Program[6]);
				glDeleteShader(st.renderer.VShader[0]);
				glDeleteShader(st.renderer.FShader[6]);
			}
			else
			{
				glDetachShader(st.renderer.Program[6],st.renderer.VShader[0]);
				glDetachShader(st.renderer.Program[6],st.renderer.FShader[6]);
			}

			if(!statusLK[2] || !statusLK[3] || !statusLK[4] || !statusLK[5] || !statusLK[6])
			{
#ifdef _VA_RENDER

				LogApp("Changing to VA...");

				if(strstr((char const*) glGetString(GL_EXTENSIONS),"GL_EXT_vertex_array")==NULL)
				{
					LogApp("Your video card is not adequate to play this game... Goodbye!!");
					Quit();
				}
				else
					st.renderer.VA_ON=1;
#endif
			}

			//This is the main VAO/VBO, used for 1 Quad only objects

			if(statusLK[0] || statusLK[3] || statusLK[4] || statusLK[5] || statusLK[6])
			{
				glUseProgram(st.renderer.Program[2]);
				st.renderer.unifs[0]=glGetUniformLocation(st.renderer.Program[2],"texu");
				glUseProgram(st.renderer.Program[3]);
				st.renderer.unifs[1]=glGetUniformLocation(st.renderer.Program[3],"texu");
				st.renderer.unifs[2]=glGetUniformLocation(st.renderer.Program[3],"texu2");
				st.renderer.unifs[3]=glGetUniformLocation(st.renderer.Program[3],"texu3");
				st.renderer.unifs[4]=glGetUniformLocation(st.renderer.Program[3],"normal");
				st.renderer.unifs[5]=glGetUniformLocation(st.renderer.Program[2],"light_type");
				//st.renderer.unifs[6]=glGetUniformLocation(st.renderer.Program[3],"Tile");
				//st.renderer.unifs[7]=glGetUniformLocation(st.renderer.Program[3],"Tiles");

#ifdef _VAO_RENDER
			CreateVAO(&vbd,1,3);
#elif _VBO_RENDER
			CreateVBO(&vbd, 1);
#endif 
			}
		}
	}

#endif

	LogApp("Opengl initialized");

	if(strstr((char const*) glGetString(GL_EXTENSIONS),"GL_ARB_texture_non_power_of_two")==NULL)
	{
		st.LOWRES=1;
		LogApp("Non power of two textures not supported, loading times might increase and video's fps might decrease");
	}

	if(strstr((char const*) glGetString(GL_EXTENSIONS),"GL_ARB_texture_rectangle")==NULL && strstr((char const*) glGetString(GL_EXTENSIONS),"GL_NV_texture_rectangle")==NULL && strstr((char const*) glGetString(GL_EXTENSIONS),"GL_NV_texture_rectangle")==NULL)
		LogApp("Rectangle textures not supported, your video card is not supported or try updating your driver");

	st.quit=0;

	st.time=0;

	st.PlayingVideo=0;

	InputInit();

	LogApp("Input initialized");

	st.Camera.position.x=0;
	st.Camera.position.y=0;
	st.Camera.dimension.x=1.0f;
	st.Camera.dimension.y=1.0f;
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

	st.num_uiwindow=0;

	st.cursor_type=0;

	memset(&ent,0,MAX_GRAPHICS*sizeof(_ENTITIES));
	memset(&lmp,0,MAX_LIGHTMAPS*sizeof(_ENTITIES));
	memset(&st.Game_Sprites,0,MAX_SPRITES*sizeof(_SPRITES));

	memset(&st.strings,0,MAX_STRINGS*sizeof(StringsE));

	//Calculates Cos, Sin and Tan tables
	for(k=0.0f;k<360.1f;k+=0.1f)
	{
		i=k*10;
		st.CosTable[i]=cos((k*pi)/180);
		st.SinTable[i]=sin((k*pi)/180);
		st.TanTable[i]=tan((k*pi)/180);
	}

	memset(&st.game_lightmaps,0,MAX_LIGHTMAPS*sizeof(_GAME_LIGHTMAPS));

	st.game_lightmaps[0].stat=1;

	st.game_lightmaps[0].W_w=16384;
	st.game_lightmaps[0].W_h=8192;

	st.game_lightmaps[0].T_w=4;
	st.game_lightmaps[0].T_h=2;

	st.game_lightmaps[0].num_lights=1;
	st.game_lightmaps[0].w_pos.x=8192;
	st.game_lightmaps[0].w_pos.y=4096;
	st.game_lightmaps[0].w_pos.z=0;

	st.game_lightmaps[0].t_pos[0].x=2;
	st.game_lightmaps[0].t_pos[0].y=1;
	st.game_lightmaps[0].t_pos[0].z=0;

	st.game_lightmaps[0].t_pos[1].x=0;
	st.game_lightmaps[0].t_pos[1].y=0;
	st.game_lightmaps[0].t_pos[1].z=0;

	st.game_lightmaps[0].t_pos[2].x=200;
	st.game_lightmaps[0].t_pos[2].y=95;
	st.game_lightmaps[0].t_pos[2].z=0;

	st.game_lightmaps[0].data=GenerateLightmap(st.game_lightmaps[0].T_w, st.game_lightmaps[0].T_h);
	AddLightToLightmap(st.game_lightmaps[0].data,st.game_lightmaps[0].T_w,st.game_lightmaps[0].T_h,255,255,255,0.1,st.game_lightmaps[0].t_pos[0].x,st.game_lightmaps[0].t_pos[0].y,st.game_lightmaps[0].t_pos[0].z,255,POINT_LIGHT_NORMAL);
	//AddLightToLightmap(st.game_lightmaps[0].data,st.game_lightmaps[0].T_w,st.game_lightmaps[0].T_h,255,255,255,16,st.game_lightmaps[0].t_pos[1].x,st.game_lightmaps[0].t_pos[1].y,st.game_lightmaps[0].t_pos[1].z,128);
	//AddLightToLightmap(st.game_lightmaps[0].data,st.game_lightmaps[0].T_w,st.game_lightmaps[0].T_h,255,255,255,16,st.game_lightmaps[0].t_pos[2].x,st.game_lightmaps[0].t_pos[2].y,st.game_lightmaps[0].t_pos[0].z,255);

	st.game_lightmaps[0].tex=GenerateLightmapTexture(st.game_lightmaps[0].data,st.game_lightmaps[0].T_w,st.game_lightmaps[0].T_h);

	DataN=(unsigned char*) calloc(64*64*3,sizeof(unsigned char));

	for(i=0;i<64;i++)
	{
		for(j=0;j<64;j++)
		{
			DataN[(i*64*3)+(j*3)]=255;
			DataN[(i*64*3)+(j*3)+1]=255;
			DataN[(i*64*3)+(j*3)+2]=255;
		}
	}

	glGenTextures(1,&DataNT);
	glBindTexture(GL_TEXTURE_2D,DataNT);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,64,64,0,GL_BGR,GL_UNSIGNED_BYTE,DataN);

	glGenerateMipmap(GL_TEXTURE_2D);

	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
}

uint8 OpenFont(const char *file,const char *name, uint8 index, size_t font_size)
{
	if((st.fonts[index].font=TTF_OpenFont(file,font_size))==NULL)
	{
		LogApp("Error while opening TTF font : %s",TTF_GetError());
		return 0;
	}

	st.fonts[index].size_h_px=font_size;
	st.fonts[index].size_w_px=font_size/2;
	st.fonts[index].size_w_gm=font_size;
	st.fonts[index].size_h_gm=font_size;
	
	strcpy(st.fonts[index].name,name);

	return 1;
}

void RestartVideo()
{
	
	LogApp("Video restarted");

	SDL_DestroyWindow(wn);

//	wn=SDL_CreateWindow(st.WINDOW_NAME,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,st.screenx,st.screeny, st.fullscreen==1 ? SDL_WINDOW_FULLSCREEN : NULL | SDL_WINDOW_OPENGL);

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
	SDL_GL_SetSwapInterval(st.vsync);
	
	if(SDL_GetRelativeMouseMode())
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	
}

static FILE *DecompressFile(const char *name)
{
	FILE *f;

	FILE *file;
	size_t len;

	char *buf, *buf2;

	qlz_state_decompress *decomp;

	if((f=fopen(name,"rb"))==NULL)
		return NULL;

	file=tmpfile();

	decomp=(qlz_state_decompress*) malloc(sizeof(qlz_state_decompress));

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

int32 CheckMGGInSystem(const char *name)
{
	FILE *file;
	char header[21];
	int8 i;

	_MGGFORMAT mggf;
	/*
	if((file=DecompressFile(name))==NULL)
	{
		LogApp("Error reading MGG file %s",name);
			return -2;
	}
	*/

	if((file=fopen(name,"rb"))==NULL)
	{
		LogApp("Error reading MGG file %s",name);
			return -2;
	}

	rewind(file);

	fread(header,21,1,file);

	if(strcmp(header,"MGG File Version 1")!=NULL)
	{
		LogApp("Invalid MGG file header %s",header);
		fclose(file);
		return -2;
	}

	fread(&mggf,sizeof(_MGGFORMAT),1,file);

	for(i=0;i<st.num_mgg_basic;i++)
	{
		if(strcmp(mggf.name,mgg_sys[i].name)==NULL)
		{
			fclose(file);
			return i+1000;
		}
	}

	for(i=0;i<st.num_mgg;i++)
	{
		if(strcmp(mggf.name,mgg_game[i].name)==NULL)
		{
			fclose(file);
			return i+10000;
		}
	}

	for(i=0;i<st.Current_Map.num_mgg;i++)
	{
		if(strcmp(mggf.name,mgg_map[i].name)==NULL)
		{
			fclose(file);
			return i+100000;
		}
	}

	fclose(file);

	return -1;
}

uint32 CheckMGGFile(const char *name)
{
	FILE *file;
	char header[21];

	_MGGFORMAT mggf;

	/*
	if((file=DecompressFile(name))==NULL)
	{
		LogApp("Error reading MGG file %s",name);
			return 0;
	}
	*/

	
	if((file=fopen(name,"rb"))==NULL)
	{
		LogApp("Error reading MGG file %s",name);
			return -2;
	}

	rewind(file);

	fread(header,21,1,file);

	if(strcmp(header,"MGG File Version 1")!=NULL)
	{
		LogApp("Invalid MGG file header %s",name);
		fclose(file);
		return 0;
	}

	rewind(file);

	fread(&mggf,sizeof(_MGGFORMAT),1,file);

	if((mggf.type!=SPRITEM && mggf.type!=TEXTUREM && mggf.type!=NONE) || (mggf.num_animations<0 || mggf.num_animations>MAX_ANIMATIONS) || (mggf.num_frames<0 || mggf.num_frames>MAX_FRAMES))
	{
		fclose(file);
		LogApp("Invalid MGG file %s",name);
		return 0;
	}

	fclose(file);
	return 1;
}

uint32 LoadMGG(_MGG *mgg, const char *name)
{
	FILE *file, *file2;
	void *data;
	_MGGFORMAT mggf;
	char header[21];
	 uint16 i=0, j=0, k=0, l=0, m=0, n=0, o=0;
	uint32 framesize[MAX_FRAMES], frameoffset[MAX_FRAMES], *framealone;
	uint16 *posx, *posy, *sizex, *sizey, *dimx, *dimy, channel2;
	uint8 *imgatlas;
	int16 *w, *h, *currh, *offx, *offy;
	int width, height, channel;
	unsigned char *imgdata;
	uint8 normals[MAX_FRAMES];
	uint32 normalsize[MAX_FRAMES];
	_MGGANIM *mga;
	int32 checkmgg;

	checkmgg=CheckMGGInSystem(name);

	if(checkmgg==-2)
		return 0;
	else
	if(checkmgg>0)
	//{
		//LogApp("This MGG is already loaded
		return 0;
	//}

	memset(&normals,0,MAX_FRAMES*sizeof(uint8));

	/*
	if((file=DecompressFile(name))==NULL)
	{
		LogApp("Error reading MGG file %s",name);
			return 0;
	}
	*/

	
	if((file=fopen(name,"rb"))==NULL)
	{
		LogApp("Error reading MGG file %s",name);
			return -2;
	}

	rewind(file);

	fread(header,21,1,file);

	if(strcmp(header,"MGG File Version 1")!=NULL)
	{
		LogApp("Invalid MGG file header %s",header);
		fclose(file);
		return 0;
	}

	fread(&mggf,sizeof(_MGGFORMAT),1,file);

	if((mggf.type!=SPRITEM && mggf.type!=TEXTUREM && mggf.type!=NONE) || (mggf.num_animations<0 || mggf.num_animations>MAX_ANIMATIONS) || (mggf.num_frames<0 || mggf.num_frames>MAX_FRAMES))
	{
		fclose(file);
		LogApp("Invalid MGG file info %s",name);
		return 0;
	}

	strcpy(mgg->name,mggf.name);

	mgg->num_frames=mggf.num_frames;

	mgg->type=mggf.type;
	
	mgg->num_anims=mggf.num_animations;

	mgg->frames=(TEX_DATA*) calloc(mgg->num_frames,sizeof(TEX_DATA));

	framealone=(uint32*)calloc(mgg->num_frames,sizeof(uint32));

	mga=(_MGGANIM*) malloc(mgg->num_anims*sizeof(_MGGANIM));
	mgg->anim=(_MGGANIM*) malloc(mgg->num_anims*sizeof(_MGGANIM));

	fread(mga,sizeof(_MGGANIM),mgg->num_anims,file);

	for(i=0;i<mgg->num_anims;i++)
	{
		mgg->anim[i]=mga[i];
		mgg->anim[i].current_frame*=10;
	}

	rewind(file);
	fseek(file,mggf.framesize_offset,SEEK_CUR);
	fread(framesize,sizeof(uint32),mggf.num_singletex+mggf.num_atlas,file);
	fread(frameoffset,sizeof(uint32),mggf.num_singletex+mggf.num_atlas,file);
	fread(normals,sizeof(uint8),mggf.num_singletex+mggf.num_atlas,file);
	fread(normalsize,sizeof(uint32),mggf.num_singletex+mggf.num_atlas,file);

	mgg->size=(Pos*) malloc(mgg->num_frames*sizeof(Pos));
	mgg->atlas=(GLint*) malloc(mggf.num_atlas*sizeof(GLint));

	for(i=0, j=0;i<mggf.num_singletex+mggf.num_atlas;i++)
	{
		
		if(i==0) fseek(file,mggf.textures_offset+1,SEEK_SET);
		else
			fseek(file,frameoffset[i-1]+1,SEEK_SET);
		
		data=malloc(framesize[i]);

		if(data==NULL)
		{
			LogApp("Error allocating memory for texture %d, size %d, file %s",i,framesize[i],name);
			continue;
		}

		fread(data,framesize[i],1,file);

		if(j<mggf.num_atlas)
		{
			imgdata=SOIL_load_image_from_memory((unsigned char*)data,framesize[i],&width,&height,&channel,SOIL_LOAD_AUTO);

			mgg->atlas[i]=SOIL_create_OGL_texture(imgdata,width,height,channel,0,SOIL_FLAG_TEXTURE_REPEATS); //mgg->atlas[i]=SOIL_load_OGL_texture_from_memory((unsigned char*)data,framesize[i],SOIL_LOAD_AUTO,0,SOIL_FLAG_TEXTURE_REPEATS);

			if(mggf.mipmap)
			{
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			}
			else
				glGenerateMipmap(GL_TEXTURE_2D);

			mgg->frames[i].channel=channel;
			mgg->frames[i].w=width;
			mgg->frames[i].h=height;

			if(mgg->atlas[i]==NULL)
				LogApp("Error loading texture from memory");

			if (data)						
				free(data);
			if(imgdata)
				SOIL_free_image_data(imgdata);

			if(normals[i])
			{
				fseek(file,frameoffset[i]-normalsize[i],SEEK_SET);
		
				data=malloc(normalsize[i]);

				if(data==NULL)
				{
					LogApp("Error allocating memory for normal mapping texture %d, size %d, file %s",i,normalsize[i],name);
					continue;
				}

				fread(data,normalsize[i],1,file);

				imgdata=SOIL_load_image_from_memory((unsigned char*)data,normalsize[i],&width,&height,&channel,SOIL_LOAD_AUTO);
				mgg->frames[i].Ndata=SOIL_create_OGL_texture(imgdata,width,height,channel,0,SOIL_FLAG_TEXTURE_REPEATS);

				if(mggf.mipmap)
				{
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				}
				else
					glGenerateMipmap(GL_TEXTURE_2D);

				mgg->frames[i].normal=1;

				if(mgg->frames[i].Ndata==NULL)
					LogApp("Error loading normal mapping texture from memory");

					if (data)						
						free(data);
					if(imgdata)
						SOIL_free_image_data(imgdata);
			}
			else
				mgg->frames[i].normal=0;

			j++;
		}
		else
		{
			i-=mggf.num_atlas;
			imgdata=SOIL_load_image_from_memory((unsigned char*)data,framesize[i],&width,&height,&channel,SOIL_LOAD_AUTO);
			mgg->frames[i+(mggf.num_frames-mggf.num_singletex)].data=SOIL_create_OGL_texture(imgdata,width,height,channel,0,SOIL_FLAG_TEXTURE_REPEATS);//SOIL_load_OGL_texture_from_memory((unsigned char*)data,framesize[i],SOIL_LOAD_AUTO,0,SOIL_FLAG_TEXTURE_REPEATS);

			//glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			if(mggf.mipmap)
			{
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			}
			else
				glGenerateMipmap(GL_TEXTURE_2D);

			mgg->frames[i+(mggf.num_texinatlas)].w=width;
			mgg->frames[i+(mggf.num_texinatlas)].h=height;
			mgg->frames[i+(mggf.num_texinatlas)].channel=channel;

			mgg->frames[i+(mggf.num_texinatlas)].posx=0;
			mgg->frames[i+(mggf.num_texinatlas)].posy=0;
			mgg->frames[i+mggf.num_texinatlas].vb_id=-1;

			if(mgg->frames[i+(mggf.num_texinatlas)].data==NULL)
				LogApp("Error loading texture from memory");

			if (data)						
				free(data);
			if(imgdata)
				SOIL_free_image_data(imgdata);

			if(normals[i])
			{
				fseek(file,frameoffset[i]-normalsize[i],SEEK_SET);
		
				data=malloc(normalsize[i]);

				if(data==NULL)
				{
					LogApp("Error allocating memory for normal mapping texture %d, size %d, file %s",i,normalsize[i],name);
					continue;
				}

				fread(data,normalsize[i],1,file);

				imgdata=SOIL_load_image_from_memory((unsigned char*)data,normalsize[i],&width,&height,&channel,SOIL_LOAD_AUTO);
				mgg->frames[i+(mggf.num_texinatlas)].Ndata=SOIL_create_OGL_texture(imgdata,width,height,channel,0,SOIL_FLAG_TEXTURE_REPEATS || SOIL_FLAG_MIPMAPS);

				if(mggf.mipmap)
				{
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				}

				mgg->frames[i+(mggf.num_texinatlas)].normal=1;

				if(mgg->frames[i+(mggf.num_texinatlas)].Ndata==NULL)
					LogApp("Error loading normal mapping texture from memory");

				if(data)						
					free(data);
				if(imgdata)
					SOIL_free_image_data(imgdata);

			}
			else
				mgg->frames[i+(mggf.num_texinatlas)].normal=0;

			i+=mggf.num_atlas;
		}
	}
		
#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)
	if(mggf.num_atlas>0)
	{
		for(i=0;i<mggf.num_atlas;i++)
		{
			if(!vbdt_num)
			{
				vbdt_num++;
				vbdt=(VB_DATAT*) malloc(sizeof(VB_DATAT));
				if(!vbdt)
					LogApp("Error could not allocate memory for the Vertex Buffer");
			}
			else
			{
				vbdt_num++;
				vbdt=(VB_DATAT*) realloc(vbdt,vbdt_num*sizeof(VB_DATAT));
			}

			vbdt[vbdt_num-1].num_elements=0;
			vbdt[vbdt_num-1].texture=mgg->atlas[i];

			vbdt[vbdt_num-1].normal=mgg->frames[i].normal;

			
			if(mgg->frames[i].normal)
				vbdt[vbdt_num-1].Ntexture=mgg->frames[i].Ndata;

			mgg->frames[i].vb_id=vbdt_num-1;

			vbdt[vbdt_num-1].buffer_elements=8;
			
#ifdef _VAO_RENDER
			if(st.renderer.VAO_ON) CreateVAO(&vbdt[vbdt_num-1],0,3);
#endif
#ifdef _VBO_RENDER
			if(st.renderer.VBO_ON) CreateVBO(&vbdt[vbdt_num-1], 0);
#endif
		}

	}
#endif

	fseek(file,mggf.possize_offset,SEEK_SET);

	posx=(uint16*) malloc((mggf.num_texinatlas)*sizeof(uint16));
	posy=(uint16*) malloc((mggf.num_texinatlas)*sizeof(uint16));
	sizex=(uint16*) malloc((mggf.num_texinatlas)*sizeof(uint16));
	sizey=(uint16*) malloc((mggf.num_texinatlas)*sizeof(uint16));
	imgatlas=(uint8*) malloc((mggf.num_texinatlas)*sizeof(uint8));
	offx=malloc(mggf.num_frames*sizeof(int16));
	offy=malloc(mggf.num_frames*sizeof(int16));

	fread(posx,sizeof(uint16),(mggf.num_texinatlas),file);
	fread(posy,sizeof(uint16),(mggf.num_texinatlas),file);
	fread(sizex,sizeof(uint16),(mggf.num_texinatlas),file);
	fread(sizey,sizeof(uint16),(mggf.num_texinatlas),file);
	fread(imgatlas,sizeof(uint8),(mggf.num_texinatlas),file);

	for(i=mggf.num_atlas-1;i<mggf.num_texinatlas;i++)
	{
		mgg->frames[i].data=mgg->atlas[imgatlas[i]];
		mgg->frames[i].posx=posx[i];
		mgg->frames[i].posy=posy[i];
		mgg->frames[i].sizex=sizex[i];
		mgg->frames[i].sizey=sizey[i];
		mgg->frames[i].vb_id=mgg->frames[imgatlas[i]].vb_id;
		mgg->frames[i].w=mgg->frames[imgatlas[i]].w;
		mgg->frames[i].h=mgg->frames[imgatlas[i]].h;
	}
	
	fseek(file,mggf.framealone_offset,SEEK_SET);

	fread(framealone,sizeof(uint32),mgg->num_frames,file);

	fseek(file,mggf.frameoffset_offset,SEEK_SET);

	fread(offx,sizeof(int16),mgg->num_frames,file);
	fread(offy,sizeof(int16),mgg->num_frames,file);

	for(i=0;i<mgg->num_frames;i++)
	{
		mgg->frames[i].x_offset=(offx[i]*16384)/st.screenx;
		mgg->frames[i].y_offset=(offy[i]*8192)/st.screeny;
	}


#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

	k=vbdt_num;
	
	if(mggf.num_frames>1)
	{
		for(i=(mggf.num_texinatlas), j=mggf.num_atlas;i<mggf.num_frames;i++, j++)
		{	
			if(framealone[i] && i==mggf.num_frames-1) 
				break;

			if(framealone[i]) 
				continue;

			if(i==(mggf.num_texinatlas) && (mgg->frames[i].w<1024 && mgg->frames[i].h<1024) && !mgg->frames[i].normal)
			{
				if(!vbdt_num)
				{
					vbdt_num++;
					l=vbdt_num-1;
					vbdt=(VB_DATAT*) malloc(sizeof(VB_DATAT));

					vbdt[l].normal=0;

					vbdt[l].buffer_elements=8;

			#ifdef _VAO_RENDER
			if(st.renderer.VAO_ON) CreateVAO(&vbdt[l],0,3);
			#endif
			#ifdef _VBO_RENDER
			if(st.renderer.VBO_ON) CreateVBO(&vbdt[l], 0);
			#endif

					w=(int16*) calloc(vbdt_num,sizeof(int16));
					h=(int16*) calloc(vbdt_num,sizeof(int16));
					currh=(int16*) calloc(vbdt_num,sizeof(int16));

					if(!vbdt)
						LogApp("Error could not allocate memory for the Vertex Buffer");
				}
				else
				{
					vbdt_num++;
					l=vbdt_num-1;
					vbdt=(VB_DATAT*) realloc(vbdt,vbdt_num*sizeof(VB_DATAT));

					vbdt[l].normal=0;

					vbdt[l].buffer_elements=8;

					#ifdef _VAO_RENDER
					if(st.renderer.VAO_ON) CreateVAO(&vbdt[l],0,3);
					#endif
					#ifdef _VBO_RENDER
					if(st.renderer.VBO_ON) CreateVBO(&vbdt[l], 0);
					#endif

					w=(int16*) calloc(vbdt_num,sizeof(int16));
					h=(int16*) calloc(vbdt_num,sizeof(int16));
					currh=(int16*) calloc(vbdt_num,sizeof(int16));

					if(vbdt_num>1)
					{
						for(n=0;n<vbdt_num-2;n++)
							currh[n]=w[n]=h[n]=-1;
					}

					currh[l]=w[l]=h[l]=0;
				}

				vbdt[l].num_elements=0;

				glGenTextures(1,&vbdt[vbdt_num-1].texture);

				glBindTexture(GL_TEXTURE_2D,vbdt[l].texture);

				glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,2048,2048,0,GL_BGRA,GL_UNSIGNED_BYTE,NULL);

				glBindTexture(GL_TEXTURE_2D,mgg->frames[i].data);

				data=malloc(framesize[j]);

				if(data==NULL)
				{
					LogApp("Error allocating memory for texture %d, size %d, file %s during atlas creating",i,framesize[i],name);
					continue;
				}

				glBindTexture(GL_TEXTURE_2D,vbdt[l].texture);
			
				if(i==0) fseek(file,mggf.textures_offset+1,SEEK_SET);
				else fseek(file,frameoffset[j-1]+1,SEEK_SET);

				fread(data,framesize[j],1,file);

				imgdata=SOIL_load_image_from_memory((unsigned char*) data,framesize[j],&width,&height,&channel,0);

				if(mgg->frames[i].channel==4)
					channel2=GL_RGBA;
				else
				if(mgg->frames[i].channel==3)
					channel2=GL_RGB;

				glTexSubImage2D(GL_TEXTURE_2D,0,w[l],currh[l],mgg->frames[i].w,mgg->frames[i].h,channel2,GL_UNSIGNED_BYTE,imgdata);

				mgg->frames[i].posx=(float)(w[l]*32768)/2048;
				mgg->frames[i].posy=(float)(currh[l]*32768)/2048;

				w[l]=mgg->frames[i].w;
				h[l]=mgg->frames[i].h;

				mgg->frames[i].sizex=(float)(mgg->frames[i].w*32768)/2048;
				mgg->frames[i].sizey=(float)(mgg->frames[i].h*32768)/2048;

				glDeleteTextures(1,&mgg->frames[i].data);

				mgg->frames[i].data=vbdt[l].texture;

				mgg->frames[i].vb_id=l;

				free(data);

				SOIL_free_image_data(imgdata);
			}
			else
			{
				if((mgg->frames[i].w+w[l]<2048 && mgg->frames[i].h+currh[l]<2048) && (mgg->frames[i].w<1024 && mgg->frames[i].h<1024) && !mgg->frames[i].normal)
				{
					glBindTexture(GL_TEXTURE_2D,mgg->frames[i].data);

					data=malloc(framesize[j]);

					if(data==NULL)
					{
						LogApp("Error allocating memory for texture %d, size %d, file %s during atlas creating",i,framesize[i],name);
						continue;
					}

					fseek(file,frameoffset[j-1]+1,SEEK_SET);

					fread(data,framesize[j],1,file);

					imgdata=SOIL_load_image_from_memory((unsigned char*) data,framesize[j],&width,&height,&channel,0);

					if(mgg->frames[i].channel==4)
						channel2=GL_RGBA;
					else
					if(mgg->frames[i].channel==3)
						channel2=GL_RGB;

					glBindTexture(GL_TEXTURE_2D,vbdt[l].texture);

					glTexSubImage2D(GL_TEXTURE_2D,0,w[l],currh[l],mgg->frames[i].w,mgg->frames[i].h,channel2,GL_UNSIGNED_BYTE,imgdata);

					mgg->frames[i].posx=(w[l]*32768)/2048;
					mgg->frames[i].posy=(currh[l]*32768)/2048;

					w[l]+=mgg->frames[i].w;

					if(currh[l]+mgg->frames[i].h>h[l])
						h[l]+=mgg->frames[i].h;

					mgg->frames[i].sizex=(mgg->frames[i].w*32768)/2048;
					mgg->frames[i].sizey=(mgg->frames[i].h*32768)/2048;

					glDeleteTextures(1,&mgg->frames[i].data);

					mgg->frames[i].data=vbdt[l].texture;

					mgg->frames[i].vb_id=l;

					free(data);

					SOIL_free_image_data(imgdata);

					if(2048-w[l]<128 && 2048-currh[l]>128)
					{
						w[l]=0;
						currh[l]=h[l];
					}
					else
					if(2048-w[l]<128 && 2048-currh[l]<128)
					{
						if(l==vbdt_num-1)
						{
							vbdt_num++;
							vbdt=(VB_DATAT*) realloc(vbdt,vbdt_num*sizeof(VB_DATAT));

							vbdt[vbdt_num-1].normal=0;

							w=(int16*) realloc(w,vbdt_num*sizeof(int16));
							h=(int16*) realloc(h,vbdt_num*sizeof(int16));

							vbdt[vbdt_num-1].num_elements=0;

							currh=(int16*) realloc(currh,vbdt_num*sizeof(int16));

							currh[vbdt_num-1]=w[vbdt_num-1]=h[vbdt_num-1]=0;

							l=vbdt_num-1;
						}
						else
						{
							for(n=0;n<vbdt_num;n++)
							{
								if(2048-w[n]>128 || 2048-currh[n]>128)
								{
									l=n;
									break;
								}
							}
						}
					}
				}
				else
				if((mgg->frames[i].w<1024 && mgg->frames[i].h<1024) && (mgg->frames[i].w+w[l]>2048 && mgg->frames[i].h+currh[l]>2048) && !mgg->frames[i].normal)
				{
					if(l==vbdt_num-1)
					{
						vbdt_num++;
						vbdt=(VB_DATAT*) realloc(vbdt,vbdt_num*sizeof(VB_DATAT));

						vbdt[vbdt_num-1].normal=0;

						w=(int16*) realloc(w,vbdt_num*sizeof(int16));
						h=(int16*) realloc(h,vbdt_num*sizeof(int16));

						currh=(int16*) realloc(currh,vbdt_num*sizeof(int16));

						currh[vbdt_num-1]=w[vbdt_num-1]=h[vbdt_num-1]=0;

						vbdt[vbdt_num-1].num_elements=0;

						glGenTextures(1,&vbdt[vbdt_num-1].texture);

						glBindTexture(GL_TEXTURE_2D,vbdt[vbdt_num-1].texture);

						glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,2048,2048,0,GL_BGRA,GL_UNSIGNED_BYTE,NULL);

						glBindTexture(GL_TEXTURE_2D,mgg->frames[i].data);

						data=malloc(framesize[j]);

						if(data==NULL)
						{
							LogApp("Error allocating memory for texture %d, size %d, file %s during atlas creating",i,framesize[i],name);
							continue;
						}

						fseek(file,frameoffset[j-1]+1,SEEK_SET);

						fread(data,framesize[j],1,file);

						imgdata=SOIL_load_image_from_memory((unsigned char*) data,framesize[j],&width,&height,&channel,0);

						if(mgg->frames[i].channel==4)
							channel2=GL_RGBA;
						else
						if(mgg->frames[i].channel==3)
							channel2=GL_RGB;

						glBindTexture(GL_TEXTURE_2D,vbdt[vbdt_num-1].texture);
				
						glTexSubImage2D(GL_TEXTURE_2D,0,w[vbdt_num-1],currh[vbdt_num-1],mgg->frames[i].w,mgg->frames[i].h,channel2,GL_UNSIGNED_BYTE,imgdata);

						mgg->frames[i].posx=(w[vbdt_num-1]*32768)/2048;
						mgg->frames[i].posy=(currh[vbdt_num-1]*32768)/2048;

						w[vbdt_num-1]=mgg->frames[i].w;
						h[vbdt_num-1]=mgg->frames[i].h;

						mgg->frames[i].sizex=(mgg->frames[i].w*32768)/2048;
						mgg->frames[i].sizey=(mgg->frames[i].h*32768)/2048;

						glDeleteTextures(1,&mgg->frames[i].data);

						mgg->frames[i].data=vbdt[vbdt_num-1].texture;

						mgg->frames[i].vb_id=vbdt_num-1;

						free(data);

						SOIL_free_image_data(imgdata);
					}
					else
					{
						for(n=l;n<vbdt_num;n++)
						{
							if(mgg->frames[i].w+w[n]<2048 || mgg->frames[i].h+currh[n]<2048)
							{
								glBindTexture(GL_TEXTURE_2D,mgg->frames[i].data);

								data=malloc(framesize[j]);

								if(data==NULL)
								{
									LogApp("Error allocating memory for texture %d, size %d, file %s during atlas creating",i,framesize[i],name);
									continue;
								}

								fseek(file,frameoffset[j-1]+1,SEEK_SET);

								fread(data,framesize[j],1,file);

								imgdata=SOIL_load_image_from_memory((unsigned char*) data,framesize[j],&width,&height,&channel,0);

								if(mgg->frames[i].channel==4)
									channel2=GL_RGBA;
								else
								if(mgg->frames[i].channel==3)
									channel2=GL_RGB;

								glBindTexture(GL_TEXTURE_2D,vbdt[n].texture);
				
								glTexSubImage2D(GL_TEXTURE_2D,0,w[n],h[n],mgg->frames[i].w,mgg->frames[i].h,channel2,GL_UNSIGNED_BYTE,imgdata);
	
								mgg->frames[i].posx=(w[n]*32768)/2048;
								mgg->frames[i].posy=(currh[n]*32768)/2048;

								w[n]+=mgg->frames[i].w;

								if(currh[n]+mgg->frames[i].h>h[n])
									h[n]+=mgg->frames[i].h;

								mgg->frames[i].sizex=(mgg->frames[i].w*32768)/2048;
								mgg->frames[i].sizey=(mgg->frames[i].h*32768)/2048;

								glDeleteTextures(1,&mgg->frames[i].data);

								mgg->frames[i].data=vbdt[n].texture;

								mgg->frames[i].vb_id=n;

								free(data);

								SOIL_free_image_data(imgdata);

								if(2048-w[n]<128 && 2048-currh[n]>128)
								{
									w[n]=0;
									currh[n]=h[n];
								}

								free(w);
								free(h);
								free(currh);

								break;
							}
							else
							if(n==vbdt_num-1)
							{
								vbdt_num++;
								vbdt=(VB_DATAT*) realloc(vbdt,vbdt_num*sizeof(VB_DATAT));

								w=(int16*) realloc(w,vbdt_num*sizeof(int16));
								h=(int16*) realloc(h,vbdt_num*sizeof(int16));

								vbdt[vbdt_num-1].normal=0;

								currh=(int16*) realloc(currh,vbdt_num*sizeof(int16));

								currh[vbdt_num-1]=w[vbdt_num-1]=h[vbdt_num-1]=0;

								vbdt[vbdt_num-1].num_elements=0;

								glGenTextures(1,&vbdt[vbdt_num-1].texture);

								glBindTexture(GL_TEXTURE_2D,vbdt[vbdt_num-1].texture);

								glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,2048,2048,0,GL_BGRA,GL_UNSIGNED_BYTE,NULL);

								glBindTexture(GL_TEXTURE_2D,mgg->frames[i].data);

								data=malloc(framesize[j]);

								if(data==NULL)
								{
									LogApp("Error allocating memory for texture %d, size %d, file %s during atlas creating",i,framesize[i],name);
									continue;
								}

								fseek(file,frameoffset[j-1]+1,SEEK_SET);

								fread(data,framesize[j],1,file);

								imgdata=SOIL_load_image_from_memory((unsigned char*) data,framesize[j],&width,&height,&channel,0);

								if(mgg->frames[i].channel==4)
									channel2=GL_RGBA;
								else
								if(mgg->frames[i].channel==3)
									channel2=GL_RGB;

								glBindTexture(GL_TEXTURE_2D,vbdt[vbdt_num-1].texture);
				
								glTexSubImage2D(GL_TEXTURE_2D,0,w[vbdt_num-1],h[vbdt_num-1],mgg->frames[i].w,mgg->frames[i].h,channel2,GL_UNSIGNED_BYTE,imgdata);

								mgg->frames[i].posx=(w[vbdt_num-1]*32768)/2048;
								mgg->frames[i].posy=(currh[vbdt_num-1]*32768)/2048;

								w[vbdt_num-1]=mgg->frames[i].w;
								h[vbdt_num-1]=mgg->frames[i].h;

								mgg->frames[i].sizex=(mgg->frames[i].w*32768)/2048;
								mgg->frames[i].sizey=(mgg->frames[i].h*32768)/2048;

								glDeleteTextures(1,&mgg->frames[i].data);

								mgg->frames[i].data=vbdt[vbdt_num-1].texture;

								mgg->frames[i].vb_id=vbdt_num-1;

								free(data);

								SOIL_free_image_data(imgdata);

								free(w);
								free(h);
								free(currh);

								break;
							}
						}
					}
				}
			}

			if(i==mggf.num_frames-1)
			{
				for(n=k;n<vbdt_num;n++)
				{
					glBindTexture(GL_TEXTURE_2D,vbdt[n].texture);
					//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

					if(mggf.mipmap)
					{
						glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
						glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
					}
					else
						glGenerateMipmap(GL_TEXTURE_2D);
				}
			}
		}
	}

#endif
	
	free(posx);
	free(posy);
	free(sizex);
	free(sizey);
	
	//if(imgatlas!=0)
		free(imgatlas);

	//if(framealone!=0)
		free(framealone);

	fclose(file);

	return 1;
		
}

void FreeMGG(_MGG *file)
{
	uint32 i;

	file->type=NONE;

	for(i=0; i<file->num_frames; i++)
	{
		glDeleteTextures(1, &file->frames[i].data);

		if(file->frames[i].normal)
			glDeleteTextures(1, &file->frames[i].Ndata);

		file->size[i].x=NULL;
		file->size[i].y=NULL;
	}

	free(file->frames);
	free(file->anim);
	free(file->size);

	file->num_frames=NULL;
	
	file->num_anims=NULL;

	memset(file->name,0,32);

	memset(file,0,sizeof(_MGG));

	file->type=NONE;
}

void InitMGG()
{
	uint16 i;

	for(i=0; i<MAX_MAP_MGG; i++)
	{
		memset(&mgg_map[i],0,sizeof(_MGG));
		mgg_map[i].type=NONE;
	}

	for(i=0; i<MAX_GAME_MGG; i++)
	{
		memset(&mgg_game[i],0,sizeof(_MGG));
		mgg_game[i].type=NONE;
	}

	for(i=0; i<3; i++)
	{
		memset(&mgg_sys[i],0,sizeof(_MGG));
		mgg_sys[i].type=NONE;
	}
}

int8 LoadLightmapFromFile(const char *file)
{
	FILE *f;
	unsigned char *buffer, *imgdata;
	size_t size;
	int w, h, channel, i;

	if((f=fopen(file,"rb"))==NULL)
	{
		LogApp("Error: invalid %s file", file);
		return NULL;
	}

	fseek(f,0,SEEK_END);

	size=ftell(f);

	rewind(f);

	i=st.num_lights+1;

	buffer=malloc(size);

	fread(buffer,size,1,f);

	st.game_lightmaps[i].data=SOIL_load_image_from_memory(buffer,size,&w,&h,&channel,0);

	if(st.game_lightmaps[i].data)
		st.game_lightmaps[i].tex=SOIL_create_OGL_texture(st.game_lightmaps[i].data,w,h,channel,NULL,SOIL_FLAG_MIPMAPS);

	if(st.game_lightmaps[i].tex)
	{
		st.game_lightmaps[i].T_w=w;
		st.game_lightmaps[i].T_h=h;
		st.game_lightmaps[i].type[0]=TGA_FILE;
		st.num_lights++;
		st.game_lightmaps[i].num_lights=1;
		if(channel==4)
			st.game_lightmaps[i].alpha=1;
		else
			st.game_lightmaps[i].alpha=0;

		st.game_lightmaps[i].W_w=(w*16384)/st.screenx;
		st.game_lightmaps[i].W_h=(h*8192)/st.screeny;
	}
	else
	{
		free(buffer);
		LogApp("Error: could not create lightmap texture from file %s", file);
		return NULL;
	}

	free(buffer);

	return 1;
}

int16 CheckCollisionSector(int32 x, int32 y, int32 xsize, int32 ysize, int16 ang)
{
	register uint16 i;
	int32 ydist;
	int16 sector=-1;

	for(i=0;i<st.Current_Map.num_sector;i++)
	{
		if((x-(xsize/2)>st.Current_Map.sector[i].vertex[0].x && x-(xsize/2)<st.Current_Map.sector[i].vertex[1].x) || (x+(xsize/2)>st.Current_Map.sector[i].vertex[0].x && x+(xsize/2)<st.Current_Map.sector[i].vertex[1].x))
		{
			if(!st.Current_Map.sector[i].sloped)
			{
				if((y-(ysize/2))<st.Current_Map.sector[i].base_y)
				{
					if(sector==-1)
					{
						ydist=abs((y-(ysize/2))-st.Current_Map.sector[i].base_y);
						sector=i;
					}
					else
					{
						if(((y-(ysize/2)))-st.Current_Map.sector[i].base_y<ydist)
						{
							ydist=abs(((y+(ysize/2)))-st.Current_Map.sector[i].base_y);
							sector=i;
						}
					}
				}
			}
		}
	}

	if(sector!=-1)
		return sector;
	else
		return -1;
}

uint8 CheckColision(float x, float y, float xsize, float ysize, float tx, float ty, float txsize, float tysize, float ang, float angt)
{
	uint8 i;

	float xb, xl, yb, yl, xtb, xtl, ytb, ytl, tmpx, tmpy;

	x-=st.Camera.position.x;
	y-=st.Camera.position.y;

	tx-=st.Camera.position.x;
	ty-=st.Camera.position.y;

	for(i=0;i<8;i++)
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

uint8 CheckColisionMouse(float x, float y, float xsize, float ysize, float ang)
{
	uint8 i;

	float xb, xl, yb, yl, xtb, xtl, ytb, ytl, tmpx, tmpy;

	int32 mx, my;

	for(i=0;i<4;i++)
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

	mx=st.mouse.x;
	my=st.mouse.y;

	//STW(&mx, &my);

	mx=((((mx*16384)/st.screenx)));
	my=((((my*8192)/st.screeny)));

	if(mx>xl && mx<xb && my>yl && my<yb)
		return 1; //Collided
	else
		return 0; //No collision
	
}

uint8 CheckColisionMouseWorld(float x, float y, float xsize, float ysize, float ang, int8 z)
{
	uint8 i;

	float xb, xl, yb, yl, xtb, xtl, ytb, ytl, tmpx, tmpy;

	int32 mx, my;

	/*
	x=((x*st.screenx)/16384)*st.Camera.dimension.x;
	y=((y*st.screeny)/8192)*st.Camera.dimension.y;

	xsize=((xsize*st.screenx)/16384)*st.Camera.dimension.x;
	ysize=((ysize*st.screeny)/8192)*st.Camera.dimension.y;
	*/

	//x*=st.Camera.dimension.x;
	//y*=st.Camera.dimension.y;

	//xsize*=st.Camera.dimension.x;
	//ysize*=st.Camera.dimension.y;

	for(i=0;i<4;i++)
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

	mx=st.mouse.x;
	my=st.mouse.y;

	STWci(&mx, &my);

			if(z>39 && z<48)
			{
				mx/=(float) st.Camera.dimension.x;
				my/=(float) st.Camera.dimension.y;

				mx+=(float) st.Camera.position.x*st.Current_Map.bck2_v;
				my+=(float) st.Camera.position.y*st.Current_Map.bck2_v;

				//mx*=st.Camera.dimension.x;
				//my*=st.Camera.dimension.y;
			}
			else
			if(z>31 && z<40)
			{
				
				mx/=(float) st.Camera.dimension.x;
				my/=(float) st.Camera.dimension.y;

				mx+=(float) st.Camera.position.x*st.Current_Map.bck1_v;
				my+=(float) st.Camera.position.y*st.Current_Map.bck1_v;

			}
			else
			if((z>23 && z<32) || z>=0 && z<16)
			{

				mx/=(float) st.Camera.dimension.x;
				my/=(float) st.Camera.dimension.y;

				mx+=st.Camera.position.x;
				my+=st.Camera.position.y;
			}
			if(z>15 && z<24)
			{
				mx/=(float) st.Camera.dimension.x;
				my/=(float) st.Camera.dimension.y;

				mx+=(float) st.Camera.position.x*st.Current_Map.fr_v;
				my+=(float) st.Camera.position.y*st.Current_Map.fr_v;

				//mx*=st.Camera.dimension.x;
				//my*=st.Camera.dimension.y;
			}

	if(mx>xl && mx<xb && my>yl && my<yb)
		return 1; //Collided
	else
		return 0; //No collision
	
}

int8 DrawPolygon(Pos vertex_s[4], uint8 r, uint8 g, uint8 b, uint8 a, int32 z)
{
	uint8 valx=0, valy=0;

	uint16 i=0, j=0, k=0;

	int32 x3, y3;

	uint32 a1;

	int16 ang;

	Pos vertex[4];

	float ax=1/(16384/2), ay=1/(8192/2), az=1/(4096/2), ang2, tx1, ty1, tx2, ty2;

	memcpy(vertex,vertex_s,4*sizeof(Pos));

	i=st.num_entities;

	if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
		return 2;

		ent[i].stat=USED;
		st.num_entities++;
		ent[i].data.data=DataNT;
		ent[i].data.vb_id=-1;
		ent[i].data.channel=0;
		ent[i].data.normal=0;
		ent[i].lightmapid=-2;

		if(z>56) z=56;
		else if(z<16) z+=16;

		z_buffer[z][z_slot[z]]=i;
		z_slot[z]++;

		if(z>z_used) z_used=z;

		if(z>15)
		{
			for(j=0;j<4;j++)
			{

				vertex[j].x-=st.Camera.position.x;
				vertex[j].y-=st.Camera.position.y;

				vertex[j].x*=st.Camera.dimension.x;
				vertex[j].y*=st.Camera.dimension.y;

			}
		}

		ent[i].vertex[0]=vertex[0].x;
		ent[i].vertex[1]=vertex[0].y;
		ent[i].vertex[2]=z;

		ent[i].vertex[3]=vertex[1].x;
		ent[i].vertex[4]=vertex[1].y;
		ent[i].vertex[5]=z;

		ent[i].vertex[6]=vertex[2].x;
		ent[i].vertex[7]=vertex[2].y;
		ent[i].vertex[8]=z;

		ent[i].vertex[9]=vertex[3].x;
		ent[i].vertex[10]=vertex[3].y;
		ent[i].vertex[11]=z;

		ent[i].texcor[0]=0;
		ent[i].texcor[1]=0;
		ent[i].texcor[2]=0;
		ent[i].texcor[3]=0;
		ent[i].texcor[4]=0;
		ent[i].texcor[5]=0;
		ent[i].texcor[6]=0;
		ent[i].texcor[7]=0;


		ax=(float) 1/(16384/2);
		ay=(float) 1/(8192/2);

		ay*=-1.0f;

		az=(float) 1/(4096/2);

		for(j=0;j<12;j+=3)
		{
			ent[i].vertex[j]*=ax;
			ent[i].vertex[j]-=1;

			ent[i].vertex[j+1]*=ay;
			ent[i].vertex[j+1]+=1;
				
			ent[i].vertex[j+2]*=az;
			ent[i].vertex[j+2]-=1;
				
		}

		for(j=0;j<16;j+=4)
		{
			ent[i].color[j]=r;
			ent[i].color[j+1]=g;
			ent[i].color[j+2]=b;
			ent[i].color[j+3]=a;
		}

		texone_ids[texone_num]=i;
		texone_num++;

	return 0;
}


int8 DrawSprite(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, TEX_DATA data, uint8 a, int32 z, int16 flags, int32 sizeax, int32 sizeay, int32 sizemx, int32 sizemy)
{
	float tmp, ax, ay, az, tx1, ty1, tx2, ty2, tw, th;

	uint8 valx=0, valy=0;
	uint32 i=0, j=0, k=0;
	int32 t1, t2, t3, t4, timej, timel;
	
	PosF dim=st.Camera.dimension;
	
	if(dim.x<0) dim.x*=-1;
	if(dim.y<0) dim.y*=-1;

	if(dim.x<10) dim.x=16384/dim.x;
	else dim.x*=16384;
	if(dim.y<10) dim.y=8192/dim.y;
	else dim.y*=8192;

	t3=(int32) dim.x;
	t4=(int32) dim.y;
	
	//Checkbounds(x,y,sizex,sizey,ang,t3,t4)) return 1;

	if(flags & 4)
	{
		if(data.vb_id!=-1)
		{
			if(sizemx!=NULL && sizemy!=NULL)
			{
				sizex=(data.sizex*sizemx)+sizeax;
				sizey=(data.sizey*sizemy)+sizeay;
			}
			else
			{
				sizex=data.sizex+sizeax;
				sizey=data.sizey+sizeay;
			}
		}
		else
		{
			if(sizemx!=NULL && sizemy!=NULL)
			{
				sizex=(((data.w*16384)/st.screenx)*sizemx)+sizeax;
				sizey=(((data.h*8192)/st.screeny)*sizemy)+sizeay;
			}
			else
			{
				sizex=((data.w*16384)/st.screenx)+sizeax;
				sizey=((data.h*8192)/st.screeny)+sizeay;
			}
		}
	}

	x+=data.x_offset;
	y+=data.y_offset;
			
	if(st.num_entities==MAX_GRAPHICS-1)
		return 2;
	else
		i=st.num_entities;

			if(data.vb_id!=-1)
			{
				ent[i].texrepeat[0]=(float) data.posx/32768;
				ent[i].texrepeat[1]=(float) data.posy/32768;
				ent[i].texrepeat[2]=(float) data.sizex/32768;
				ent[i].texrepeat[3]=(float) data.sizey/32768;
			}
			else
			{
				ent[i].texrepeat[0]=0.0f;
				ent[i].texrepeat[1]=0.0f;
				ent[i].texrepeat[2]=1.0f;
				ent[i].texrepeat[3]=1.0f;
			}

	ent[i].lightmapid=-1;
	
			ent[i].data=data;

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

			tx1=x;
			ty1=y;
			tx2=sizex;
			ty2=sizey;

			if(r==0 && g==0 && b==0)
				r=g=b=1;

			if(z>56) z=56;
			else if(z<16) z+=16;

			z_buffer[z][z_slot[z]]=i;
			z_slot[z]++;

			if(z>z_used) z_used=z;

			if(z>39 && z<48)
			{
				
				x-=(float) st.Camera.position.x*st.Current_Map.bck2_v;
				y-=(float) st.Camera.position.y*st.Current_Map.bck2_v;

				ent[i].lightmapid=-2;
			}
			else
			if(z>31 && z<40)
			{
				
				x-=(float) st.Camera.position.x*st.Current_Map.bck1_v;
				y-=(float) st.Camera.position.y*st.Current_Map.bck1_v;

				sizex*=st.Camera.dimension.x;
				sizey*=st.Camera.dimension.y;

				x*=st.Camera.dimension.x;
				y*=st.Camera.dimension.y;

				ent[i].lightmapid=-2;
			}
			else
			if(z>23 && z<32)
			{
				x-=st.Camera.position.x;
				y-=st.Camera.position.y;

				sizex*=st.Camera.dimension.x;
				sizey*=st.Camera.dimension.y;

				x*=st.Camera.dimension.x;
				y*=st.Camera.dimension.y;
			}
			if(z>15 && z<24)
			{
				x-=(float) st.Camera.position.x*st.Current_Map.fr_v;
				y-=(float) st.Camera.position.y*st.Current_Map.fr_v;

				sizex*=st.Camera.dimension.x;
				sizey*=st.Camera.dimension.y;

				x*=st.Camera.dimension.x;
				y*=st.Camera.dimension.y;

				ent[i].lightmapid=-2;
			}

			//timej=GetTicks();

			ent[i].vertex[0]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[1]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[2]=z;

			ent[i].vertex[3]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[4]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[5]=z;

			ent[i].vertex[6]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[7]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[8]=z;

			ent[i].vertex[9]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[10]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[11]=z;

			//timel=GetTicks() - timej;

			//ang=1000;

			//ang/=10;
	
			ax=(float) 1/(16384/2);
			ay=(float) 1/(8192/2);

			ay*=-1.0f;

			az=(float) 1/(4096/2);

			ent[i].texcorlight[0]=(float)tx1+(((tx1-(tx2/2))-tx1)*mCos(ang) - ((ty1-(ty2/2))-ty1)*mSin(ang));
			ent[i].texcorlight[1]=(float)ty1+(((tx1-(tx2/2))-tx1)*mSin(ang) + ((ty1-(ty2/2))-ty1)*mCos(ang));

			ent[i].texcorlight[2]=(float)tx1+(((tx1+(tx2/2))-tx1)*mCos(ang) - ((ty1-(ty2/2))-ty1)*mSin(ang));
			ent[i].texcorlight[3]=(float)ty1+(((tx1+(tx2/2))-tx1)*mSin(ang) + ((ty1-(ty2/2))-ty1)*mCos(ang));

			ent[i].texcorlight[4]=(float)tx1+(((tx1+(tx2/2))-tx1)*mCos(ang) - ((ty1+(ty2/2))-ty1)*mSin(ang));
			ent[i].texcorlight[5]=(float)ty1+(((tx1+(tx2/2))-tx1)*mSin(ang) + ((ty1+(ty2/2))-ty1)*mCos(ang));

			ent[i].texcorlight[6]=(float)tx1+(((tx1-(tx2/2))-tx1)*mCos(ang) - ((ty1+(ty2/2))-ty1)*mSin(ang));
			ent[i].texcorlight[7]=(float)ty1+(((tx1-(tx2/2))-tx1)*mSin(ang) + ((ty1+(ty2/2))-ty1)*mCos(ang));

			WTSf(&ent[i].texcorlight[0],&ent[i].texcorlight[1]);
			WTSf(&ent[i].texcorlight[2],&ent[i].texcorlight[3]);
			WTSf(&ent[i].texcorlight[4],&ent[i].texcorlight[5]);
			WTSf(&ent[i].texcorlight[6],&ent[i].texcorlight[7]);
			
			ent[i].texcorlight[0]/=(float) st.screenx;
			ent[i].texcorlight[1]/=(float) st.screeny;
			ent[i].texcorlight[2]/=(float) st.screenx;
			ent[i].texcorlight[3]/=(float) st.screeny;
			ent[i].texcorlight[4]/=(float) st.screenx;
			ent[i].texcorlight[5]/=(float) st.screeny;
			ent[i].texcorlight[6]/=(float) st.screenx;
			ent[i].texcorlight[7]/=(float) st.screeny;

			ent[i].texcorlight[1]*=-1;
			ent[i].texcorlight[3]*=-1;
			ent[i].texcorlight[5]*=-1;
			ent[i].texcorlight[7]*=-1;

			if(data.vb_id==-1)
			{
				ent[i].texcor[0]=0;
				ent[i].texcor[1]=0;
				ent[i].texcor[2]=32768;
				ent[i].texcor[3]=0;
				ent[i].texcor[4]=32768;
				ent[i].texcor[5]=32768;
				ent[i].texcor[6]=0;
				ent[i].texcor[7]=32768;
			}
			else
			{
				ent[i].texcor[0]=data.posx;
				ent[i].texcor[1]=data.posy;

				ent[i].texcor[2]=data.posx+data.sizex;
				ent[i].texcor[3]=data.posy;

				ent[i].texcor[4]=data.posx+data.sizex;
				ent[i].texcor[5]=data.posy+data.sizey;

				ent[i].texcor[6]=data.posx;
				ent[i].texcor[7]=data.posy+data.sizey;
			}

			//timej=GetTicks();

			for(j=0;j<12;j+=3)
			{
				ent[i].vertex[j]*=ax;
				ent[i].vertex[j]-=1;

				ent[i].vertex[j+1]*=ay;
				ent[i].vertex[j+1]+=1;
				
				ent[i].vertex[j+2]*=az;
				ent[i].vertex[j+2]-=1;
				
				if(j<7)
				{
					ent[i].texcor[j]/=(float)32768;
					ent[i].texcor[j+1]/=(float)32768;

					if((j+2)<7)
						ent[i].texcor[j+2]/=(float)32768;
				}
			}

			for(j=0;j<16;j+=4)
			{
				ent[i].color[j]=r;
				ent[i].color[j+1]=g;
				ent[i].color[j+2]=b;
				ent[i].color[j+3]=a;
			}

			if(data.vb_id!=-1)
			{
				vbdt[data.vb_id].num_elements++;
				ent[i].data.loc=vbdt[data.vb_id].num_elements-1;
			}
			else
			{
				texone_ids[texone_num]=i;
				texone_num++;
			}

#endif

			//timel=GetTicks() - timej;

			st.num_entities++;

	return 0;
}

int8 DrawLight(int32 x, int32 y, int32 z, int16 ang, uint8 r, uint8 g, uint8 b, LIGHT_TYPE type, uint8 intensity, float falloff, int32 radius)
{
	 uint16 i=0;

	uint8 val=0;
	/*
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
	*/
	//for( uint32 i=0;i<MAX_GRAPHICS+1;i++)
	//{

	i=st.num_lights;

	if(i==MAX_LIGHTS-1)
		return 2;

	game_lights[i].pos.x=(float) x/16384;
	game_lights[i].pos.y=(float) y/8192;
	game_lights[i].pos.y*=-1.0f;
	game_lights[i].pos.y+=1.0f;
	game_lights[i].pos.z=(float) z/4096;

	game_lights[i].radius=(float) radius/8192;
	
	game_lights[i].color.r=(float) r/255;
	game_lights[i].color.g=(float) g/255;
	game_lights[i].color.b=(float) b/255;

	game_lights[i].color.a=(float) intensity/255;

	game_lights[i].falloff=falloff;

	st.num_lights++;

	return 0;
}

int8 DrawLightmap(int32 x, int32 y, int32 z, int32 sizex, int32 sizey, GLuint data, LIGHT_TYPE type, int16 ang)
{
	float tmp, ax, ay, az;

	uint8 val=0;

	//int16 ang=0;

	uint32 i=0, j=0, k=0;
	
	PosF dim=st.Camera.dimension;
	
	x-=st.Camera.position.x;
	y-=st.Camera.position.y;

	i=st.num_lightmap;

	if(i==MAX_LIGHTMAPS)
		return 2;

	sizex*=st.Camera.dimension.x;
	sizey*=st.Camera.dimension.y;

	x*=st.Camera.dimension.x;
	y*=st.Camera.dimension.y;

	if(dim.x<0) dim.x*=-1;
	if(dim.y<0) dim.y*=-1;

	if(dim.x<10) dim.x=16384/dim.x;
	else dim.x*=16384;
	if(dim.y<10) dim.y=8192/dim.y;
	else dim.y*=8192;

	lmp[i].data.data=data;

	if(type==AMBIENT_LIGHT)
		lmp[i].ang=0;
	else
	if(type==POINT_LIGHT_MEDIUM)
		lmp[i].ang=1;
	else
	if(type==POINT_LIGHT_STRONG)
		lmp[i].ang=2;

			lmp[i].vertex[0]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			lmp[i].vertex[1]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			lmp[i].vertex[2]=0;

			lmp[i].vertex[3]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			lmp[i].vertex[4]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			lmp[i].vertex[5]=0;

			lmp[i].vertex[6]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			lmp[i].vertex[7]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			lmp[i].vertex[8]=0;

			lmp[i].vertex[9]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			lmp[i].vertex[10]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			lmp[i].vertex[11]=0;


			ax=(float) 1/(16384/2);
			ay=(float) 1/(8192/2);

			ay*=-1.0f;

			az=(float) 1/(4096/2);

			lmp[i].texcor[0]=0;
			lmp[i].texcor[1]=0;
			lmp[i].texcor[2]=1;
			lmp[i].texcor[3]=0;
			lmp[i].texcor[4]=1;
			lmp[i].texcor[5]=1;
			lmp[i].texcor[6]=0;
			lmp[i].texcor[7]=1;

			for(j=0;j<12;j+=3)
			{
				lmp[i].vertex[j]*=ax;
				lmp[i].vertex[j]-=1;

				lmp[i].vertex[j+1]*=ay;
				lmp[i].vertex[j+1]+=1;
				
				lmp[i].vertex[j+2]*=az;
				lmp[i].vertex[j+2]-=1;
			}

			for(j=0;j<16;j+=4)
			{
				lmp[i].color[j]=255;
				lmp[i].color[j+1]=255;
				lmp[i].color[j+2]=255;
				lmp[i].color[j+3]=255;
			}

			st.num_lightmap++;

	return 0;
}

void BASICBKD(uint8 r, uint8 g, uint8 b)
{
	float tmp, ax, ay, az;

	uint32 i=0, j=0, k=0;

	i=0;

	lmp[i].data.data=st.game_lightmaps[0].tex;
	lmp[i].ang=0;

			lmp[i].vertex[0]=0.0f;
			lmp[i].vertex[1]=0.0f;
			lmp[i].vertex[2]=0.0f;

			lmp[i].vertex[3]=16384.0f;
			lmp[i].vertex[4]=0.0f;
			lmp[i].vertex[5]=0.0f;

			lmp[i].vertex[6]=16384.0f;
			lmp[i].vertex[7]=8192.0f;
			lmp[i].vertex[8]=0.0f;

			lmp[i].vertex[9]=0.0f;
			lmp[i].vertex[10]=8192.0f;
			lmp[i].vertex[11]=0.0f;

			ax=(float) 1/(16384/2);
			ay=(float) 1/(8192/2);

			ay*=-1.0f;

			az=(float) 1/(4096/2);

			lmp[i].texcor[0]=0;
			lmp[i].texcor[1]=0;
			lmp[i].texcor[2]=1;
			lmp[i].texcor[3]=0;
			lmp[i].texcor[4]=1;
			lmp[i].texcor[5]=1;
			lmp[i].texcor[6]=0;
			lmp[i].texcor[7]=1;

			for(j=0;j<12;j+=3)
			{
				lmp[i].vertex[j]*=ax;
				lmp[i].vertex[j]-=1;

				lmp[i].vertex[j+1]*=ay;
				lmp[i].vertex[j+1]+=1;
				
				lmp[i].vertex[j+2]*=az;
				lmp[i].vertex[j+2]-=1;
			}

			for(j=0;j<16;j+=4)
			{
				lmp[i].color[j]=r;
				lmp[i].color[j+1]=g;
				lmp[i].color[j+2]=b;
				lmp[i].color[j+3]=255;
			}

			st.num_lightmap++;

}

int8 DrawObj(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, TEX_DATA data, uint8 a, int32 x1, int32 y1, int32 x2, int32 y2, int8 z, int16 lightmap_id)
{
	float tmp, ax, ay, az, tx1, ty1, tx2, ty2;

	uint8 valx=0, valy=0;
	uint32 i=0, j=0, k=0;
	int32 t1, t2, t3, t4, timej, timel;

	PosF dim=st.Camera.dimension;
	
	x-=st.Camera.position.x;
	y-=st.Camera.position.y;
	
	if(dim.x<0) dim.x*=-1;
	if(dim.y<0) dim.y*=-1;

	if(dim.x<10) dim.x=16384/dim.x;
	else dim.x*=16384;
	if(dim.y<10) dim.y=8192/dim.y;
	else dim.y*=8192;

	t3=(int32) dim.x;
	t4=(int32) dim.y;

	//Checkbounds(x,y,sizex,sizey,ang,t3,t4)) return 1;
			
	if(st.num_entities==MAX_GRAPHICS-1)
		return 2;
	else
		i=st.num_entities;

	ent[i].lightmapid=lightmap_id;
	
			ent[i].data=data;

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

			if(r==0 && g==0 && b==0)
				r=g=b=1;

			sizex*=st.Camera.dimension.x;
			sizey*=st.Camera.dimension.y;

			if(z>56) 
				z=56;
			else 
				if(z<16) 
					z+=16;

			z_buffer[z][z_slot[z]]=i;
			z_slot[z]++;

			if(z>z_used) z_used=z;

			if(z>47)
			{
				x+=st.Camera.position.x;
				y+=st.Camera.position.y;
			}
			else
			if(z>39 && z<48)
			{
				x+=st.Camera.position.x;
				y+=st.Camera.position.y;
			}
			else
			if(z>31 && z<40)
			{
				x+=st.Camera.position.x;
				y+=st.Camera.position.y;
			}
			else
			if(z>15 && z<24)
			{
				x-=st.Camera.position.x;
				y-=st.Camera.position.y;
			}
			
			x*=st.Camera.dimension.x;
			y*=st.Camera.dimension.y;

			//timej=GetTicks();

			ent[i].vertex[0]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[1]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[2]=z;

			ent[i].vertex[3]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[4]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[5]=z;

			ent[i].vertex[6]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[7]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[8]=z;

			ent[i].vertex[9]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[10]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[11]=z;

			//timel=GetTicks() - timej;

			//ang=1000;

			ang/=10;

			tmp=cos((ang*pi)/180);
			tmp=mCos(ang*10);
	
			ax=(float) 1/(16384/2);
			ay=(float) 1/(8192/2);

			ay*=-1.0f;

			az=(float) 1/(4096/2);

			if(data.vb_id==-1)
			{
				ent[i].texcor[0]=x1;
				ent[i].texcor[1]=y1;
				ent[i].texcor[2]=x2;
				ent[i].texcor[3]=y1;
				ent[i].texcor[4]=x2;
				ent[i].texcor[5]=y2;
				ent[i].texcor[6]=x1;
				ent[i].texcor[7]=y2;
			}
			else
			{
				tx1=(float) x1/32768;
				ty1=(float) y1/32768;
				tx2=(float) x2/32768;
				ty2=(float) y2/32768;

				ent[i].texcor[0]=data.posx+(tx1*data.sizex);
				ent[i].texcor[1]=data.posy+(ty1*data.sizey);

				ent[i].texcor[2]=data.posx+(tx2*data.sizex);
				ent[i].texcor[3]=data.posy+(ty1*data.sizey);

				ent[i].texcor[4]=data.posx+(tx2*data.sizex);
				ent[i].texcor[5]=data.posy+(ty2*data.sizey);

				ent[i].texcor[6]=data.posx+(tx1*data.sizex);
				ent[i].texcor[7]=data.posy+(ty2*data.sizey);
			}

			for(j=0;j<12;j+=3)
			{
				ent[i].vertex[j]*=ax;
				ent[i].vertex[j]-=1;

		 		ent[i].vertex[j+1]*=ay;
				ent[i].vertex[j+1]+=1;
				
				ent[i].vertex[j+2]*=az;
				ent[i].vertex[j+2]-=1;
				
				if(j<8)
				{
					ent[i].texcor[j]/=(float)32768;
					ent[i].texcor[j+1]/=(float)32768;
					ent[i].texcor[j+2]/=(float)32768;
				}
				
			}

			/*
			ent[i].texcor[0]/=tx1;
			ent[i].texcor[1]/=ty1;
			ent[i].texcor[2]*=tx2;
			ent[i].texcor[3]/=ty1;
			ent[i].texcor[4]*=tx2;
			ent[i].texcor[5]*=ty2;
			ent[i].texcor[6]/=tx1;
			ent[i].texcor[7]*=ty2;
			*/

			for(j=0;j<16;j+=4)
			{
				ent[i].color[j]=r;
				ent[i].color[j+1]=g;
				ent[i].color[j+2]=b;
				ent[i].color[j+3]=a;
			}

			if(data.vb_id!=-1)
			{
				vbdt[data.vb_id].num_elements++;
				ent[i].data.loc=vbdt[data.vb_id].num_elements-1;
			}
			else
			{
				texone_ids[texone_num]=i;
				texone_num++;
			}

#endif

			st.num_entities++;

	return 0;
}


int8 DrawGraphic(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, TEX_DATA data, uint8 a, int32 x1, int32 y1, int32 x2, int32 y2, int8 z, uint16 flag)
{
	float tmp, ax, ay, az, tx1, ty1, tx2, ty2, tw, th;

	uint8 valx=0, valy=0;
	uint32 i=0, j=0, k=0;
	int32 t1, t2, t3, t4, timej, timel, xp1, yp1, xs2, ys2;

	PosF dim=st.Camera.dimension;
	
	//if((!(st.viewmode & 1) && z<24) || (!(st.viewmode & 2) && z>23 && z<31) || (!(st.viewmode & 4) && z>31 && z<40) || (!(st.viewmode & 8) && z>39 && z<48) || (!(st.viewmode & 16) && z>47))
		//return 3;
	
	if(dim.x<0) dim.x*=-1;
	if(dim.y<0) dim.y*=-1;

	if(dim.x<10) dim.x=16384/dim.x;
	else dim.x*=16384;
	if(dim.y<10) dim.y=8192/dim.y;
	else dim.y*=8192;

	t3=(int32) dim.x;
	t4=(int32) dim.y;

	//Checkbounds(x,y,sizex,sizey,ang,t3,t4)) return 1;
			
	if(st.num_entities==MAX_GRAPHICS-1)
		return 2;
	else
		i=st.num_entities;

	ent[i].lightmapid=-1;
	
			ent[i].data=data;

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

			if(r==0 && g==0 && b==0)
				r=g=b=1;

			tx1=x;
			ty1=y;
			tx2=sizex;
			ty2=sizey;

			if(z>56) 
				z=56;
			else 
			if(z<16) 
				z+=16;

			z_buffer[z][z_slot[z]]=i;
			z_slot[z]++;

			if(z>z_used) z_used=z;

			if(z>47)
				ent[i].lightmapid=-2;
			else
			if(z>39 && z<48)
			{
				if(flag & 1)
				{
					x1-=(float) st.Camera.position.x*st.Current_Map.bck2_v;
					y1-=(float) st.Camera.position.y*st.Current_Map.bck2_v;
					x2-=(float) st.Camera.position.x*st.Current_Map.bck2_v;
					y2-=(float) st.Camera.position.y*st.Current_Map.bck2_v;
				}
				else
				{
					x-=(float) st.Camera.position.x*st.Current_Map.bck2_v;
					y-=(float) st.Camera.position.y*st.Current_Map.bck2_v;
				}

				ent[i].lightmapid=-2;
			}
			else
			if(z>31 && z<40)
			{
				if(flag & 1)
				{
					x1-=(float) st.Camera.position.x*st.Current_Map.bck1_v;
					y1-=(float) st.Camera.position.y*st.Current_Map.bck1_v;
					x2-=(float) st.Camera.position.x*st.Current_Map.bck1_v;
					y2-=(float) st.Camera.position.y*st.Current_Map.bck1_v;
				}
				else
				{
					x-=(float) st.Camera.position.x*st.Current_Map.bck1_v;
					y-=(float) st.Camera.position.y*st.Current_Map.bck1_v;
				}

				sizex*=st.Camera.dimension.x;
				sizey*=st.Camera.dimension.y;

				x*=st.Camera.dimension.x;
				y*=st.Camera.dimension.y;

				ent[i].lightmapid=-2;
			}
			else
			if(z>23 && z<32)
			{
				if( (~flag & 2) )
				{
					x-=st.Camera.position.x;
					y-=st.Camera.position.y;

					sizex*=st.Camera.dimension.x;
					sizey*=st.Camera.dimension.y;

					x*=st.Camera.dimension.x;
					y*=st.Camera.dimension.y;
				}
			}
			else
			if(z>15 && z<24)
			{
				if(flag & 1)
				{
					x1-=(float) st.Camera.position.x*st.Current_Map.fr_v;
					y1-=(float) st.Camera.position.y*st.Current_Map.fr_v;
					x2-=(float) st.Camera.position.x*st.Current_Map.fr_v;
					y2-=(float) st.Camera.position.y*st.Current_Map.fr_v;
				}
				else
				if(flag & 2)
				{
					x-=st.Camera.position.x;
					y-=st.Camera.position.y;
				}
				else
				{
					x-=(float) st.Camera.position.x*st.Current_Map.fr_v;
					y-=(float) st.Camera.position.y*st.Current_Map.fr_v;
				}

				sizex*=st.Camera.dimension.x;
				sizey*=st.Camera.dimension.y;

				x*=st.Camera.dimension.x;
				y*=st.Camera.dimension.y;

				ent[i].lightmapid=-2;
			}

			//timej=GetTicks();

			ent[i].vertex[0]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[1]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[2]=z;

			ent[i].vertex[3]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[4]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[5]=z;

			ent[i].vertex[6]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[7]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[8]=z;

			ent[i].vertex[9]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[10]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[11]=z;
	
			ax=(float) 1/(16384/2);
			ay=(float) 1/(8192/2);

			ay*=-1.0f;

			az=(float) 1/(4096/2);

			tw=st.Camera.position.x;
			th=st.Camera.position.y;

			//WTSf(&tx1,&ty1);
			//WTScf(&tx2,&ty2);
			
			if(ent[i].lightmapid==-2)
			{
				ent[i].texcorlight[0]=0;
				ent[i].texcorlight[1]=0;
				ent[i].texcorlight[2]=0;
				ent[i].texcorlight[3]=0;
				ent[i].texcorlight[4]=0;
				ent[i].texcorlight[5]=0;
				ent[i].texcorlight[6]=0;
				ent[i].texcorlight[7]=0;
			}
			else
			{
			
				ent[i].texcorlight[0]=(float)tx1+(((tx1-(tx2/2))-tx1)*mCos(ang) - ((ty1-(ty2/2))-ty1)*mSin(ang));
				ent[i].texcorlight[1]=(float)ty1+(((tx1-(tx2/2))-tx1)*mSin(ang) + ((ty1-(ty2/2))-ty1)*mCos(ang));

				ent[i].texcorlight[2]=(float)tx1+(((tx1+(tx2/2))-tx1)*mCos(ang) - ((ty1-(ty2/2))-ty1)*mSin(ang));
				ent[i].texcorlight[3]=(float)ty1+(((tx1+(tx2/2))-tx1)*mSin(ang) + ((ty1-(ty2/2))-ty1)*mCos(ang));

				ent[i].texcorlight[4]=(float)tx1+(((tx1+(tx2/2))-tx1)*mCos(ang) - ((ty1+(ty2/2))-ty1)*mSin(ang));
				ent[i].texcorlight[5]=(float)ty1+(((tx1+(tx2/2))-tx1)*mSin(ang) + ((ty1+(ty2/2))-ty1)*mCos(ang));

				ent[i].texcorlight[6]=(float)tx1+(((tx1-(tx2/2))-tx1)*mCos(ang) - ((ty1+(ty2/2))-ty1)*mSin(ang));
				ent[i].texcorlight[7]=(float)ty1+(((tx1-(tx2/2))-tx1)*mSin(ang) + ((ty1+(ty2/2))-ty1)*mCos(ang));

				WTSf(&ent[i].texcorlight[0],&ent[i].texcorlight[1]);
				WTSf(&ent[i].texcorlight[2],&ent[i].texcorlight[3]);
				WTSf(&ent[i].texcorlight[4],&ent[i].texcorlight[5]);
				WTSf(&ent[i].texcorlight[6],&ent[i].texcorlight[7]);
			
				ent[i].texcorlight[0]/=(float) st.screenx;
				ent[i].texcorlight[1]/=(float) st.screeny;
				ent[i].texcorlight[2]/=(float) st.screenx;
				ent[i].texcorlight[3]/=(float) st.screeny;
				ent[i].texcorlight[4]/=(float) st.screenx;
				ent[i].texcorlight[5]/=(float) st.screeny;
				ent[i].texcorlight[6]/=(float) st.screenx;
				ent[i].texcorlight[7]/=(float) st.screeny;

				ent[i].texcorlight[1]*=-1;
				ent[i].texcorlight[3]*=-1;
				ent[i].texcorlight[5]*=-1;
				ent[i].texcorlight[7]*=-1;
			}
			
			if(data.vb_id==-1)
			{
				ent[i].texcor[0]=x1;
				ent[i].texcor[1]=y1;
				ent[i].texcor[2]=x2;
				ent[i].texcor[3]=y1;
				ent[i].texcor[4]=x2;
				ent[i].texcor[5]=y2;
				ent[i].texcor[6]=x1;
				ent[i].texcor[7]=y2;
			}
			else
			{
				tx1=(float) x1/32768;
				ty1=(float) y1/32768;
				tx2=(float) x2/32768;
				ty2=(float) y2/32768;

				ent[i].texcor[0]=data.posx+(tx1*data.sizex);
				ent[i].texcor[1]=data.posy+(ty1*data.sizey);

				ent[i].texcor[2]=data.posx+(tx2*data.sizex);
				ent[i].texcor[3]=data.posy+(ty1*data.sizey);

				ent[i].texcor[4]=data.posx+(tx2*data.sizex);
				ent[i].texcor[5]=data.posy+(ty2*data.sizey);

				ent[i].texcor[6]=data.posx+(tx1*data.sizex);
				ent[i].texcor[7]=data.posy+(ty2*data.sizey);
			}
			
			for(j=0;j<12;j+=3)
			{
				ent[i].vertex[j]*=ax;
				ent[i].vertex[j]-=1;

		 		ent[i].vertex[j+1]*=ay;
				ent[i].vertex[j+1]+=1;
				
				ent[i].vertex[j+2]*=az;
				ent[i].vertex[j+2]-=1;
				
				if(j<7)
				{
					ent[i].texcor[j]/=(float)32768;
					ent[i].texcor[j+1]/=(float)32768;

					if((j+2)<7)
						ent[i].texcor[j+2]/=(float)32768;
				}
				
			}

			/*
			ent[i].texcor[0]/=tx1;
			ent[i].texcor[1]/=ty1;
			ent[i].texcor[2]*=tx2;
			ent[i].texcor[3]/=ty1;
			ent[i].texcor[4]*=tx2;
			ent[i].texcor[5]*=ty2;
			ent[i].texcor[6]/=tx1;
			ent[i].texcor[7]*=ty2;
			*/

			for(j=0;j<16;j+=4)
			{
				ent[i].color[j]=r;
				ent[i].color[j+1]=g;
				ent[i].color[j+2]=b;
				ent[i].color[j+3]=a;
			}

			if(data.vb_id!=-1)
			{
				vbdt[data.vb_id].num_elements++;
				ent[i].data.loc=vbdt[data.vb_id].num_elements-1;
			}
			else
			{
				texone_ids[texone_num]=i;
				texone_num++;
			}

#endif

			st.num_entities++;

	return 0;
}

int8 DrawHud(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, int32 x1, int32 y1, int32 x2, int32 y2, TEX_DATA data, uint8 a, int8 layer)
{
	float tmp, ax, ay, az, tx1, ty1, tx2, ty2;

	uint8 valx=0, valy=0;
	uint32 i=0, j=0, k=0;
	int32 t1, t2, t3, t4, timej, timel;

	t3=16384;
	t4=8192;

	//Checkbounds(x,y,sizex,sizey,ang,t3,t4)) return 1;
			
	if(st.num_entities==MAX_GRAPHICS-1)
		return 2;
	else
		i=st.num_entities;

	ent[i].lightmapid=-1;
	
			ent[i].data=data;

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

			if(data.vb_id!=-1)
			{
				ent[i].texrepeat[0]=(float) data.posx/32768;
				ent[i].texrepeat[1]=(float) data.posy/32768;
				ent[i].texrepeat[2]=(float) data.sizex/32768;
				ent[i].texrepeat[3]=(float) data.sizey/32768;
			}
			else
			{
				ent[i].texrepeat[0]=0.0f;
				ent[i].texrepeat[1]=0.0f;
				ent[i].texrepeat[2]=1.0f;
				ent[i].texrepeat[3]=1.0f;
			}

			if(r==0 && g==0 && b==0)
				r=g=b=1;

			if(layer>15) layer=15;
			else if(layer<8) layer+=8;

			z_buffer[layer][z_slot[layer]]=i;
			z_slot[layer]++;

			if(layer>z_used) z_used=layer;

			//timej=GetTicks();

			ent[i].vertex[0]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[1]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[2]=layer;

			ent[i].vertex[3]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[4]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[5]=layer;

			ent[i].vertex[6]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[7]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[8]=layer;

			ent[i].vertex[9]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[10]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[11]=layer;

			//timel=GetTicks() - timej;

			//ang=1000;

			ang/=10;

			tmp=cos((ang*pi)/180);
			tmp=mCos(ang*10);
	
			ax=(float) 1/(16384/2);
			ay=(float) 1/(8192/2);

			ay*=-1.0f;

			az=(float) 1/(4096/2);

			if(data.vb_id==-1)
			{
				ent[i].texcor[0]=x1;
				ent[i].texcor[1]=y1;
				ent[i].texcor[2]=x2;
				ent[i].texcor[3]=y1;
				ent[i].texcor[4]=x2;
				ent[i].texcor[5]=y2;
				ent[i].texcor[6]=x1;
				ent[i].texcor[7]=y2;
			}
			else
			{
				tx1=(float) x1/32768;
				ty1=(float) y1/32768;
				tx2=(float) x2/32768;
				ty2=(float) y2/32768;

				ent[i].texcor[0]=data.posx+(tx1*data.sizex);
				ent[i].texcor[1]=data.posy+(ty1*data.sizey);

				ent[i].texcor[2]=data.posx+(tx2*data.sizex);
				ent[i].texcor[3]=data.posy+(ty1*data.sizey);

				ent[i].texcor[4]=data.posx+(tx2*data.sizex);
				ent[i].texcor[5]=data.posy+(ty2*data.sizey);

				ent[i].texcor[6]=data.posx+(tx1*data.sizex);
				ent[i].texcor[7]=data.posy+(ty2*data.sizey);
			}

			for(j=0;j<12;j+=3)
			{
				ent[i].vertex[j]*=ax;
				ent[i].vertex[j]-=1;

		 		ent[i].vertex[j+1]*=ay;
				ent[i].vertex[j+1]+=1;
				
				ent[i].vertex[j+2]*=az;
				ent[i].vertex[j+2]-=1;
				
				if(j<8)
				{
					ent[i].texcor[j]/=(float)32768;
					ent[i].texcor[j+1]/=(float)32768;
					ent[i].texcor[j+2]/=(float)32768;
				}
				
			}

			for(j=0;j<16;j+=4)
			{
				ent[i].color[j]=r;
				ent[i].color[j+1]=g;
				ent[i].color[j+2]=b;
				ent[i].color[j+3]=a;
			}

			if(data.vb_id!=-1)
			{
				vbdt[data.vb_id].num_elements++;
				ent[i].data.loc=vbdt[data.vb_id].num_elements-1;
			}
			else
			{
				texone_ids[texone_num]=i;
				texone_num++;
			}

#endif

			st.num_entities++;

	return 0;
}

int8 DrawUI(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, int32 x1, int32 y1, int32 x2, int32 y2, TEX_DATA data, uint8 a, int8 layer)
{
	float tmp, ax, ay, az, tx1, ty1, tx2, ty2;

	uint8 valx=0, valy=0;
	uint32 i=0, j=0, k=0;
	int32 t1, t2, t3, t4, timej, timel;

	t3=16384;
	t4=8192;

	//Checkbounds(x,y,sizex,sizey,ang,t3,t4)) return 1;
			
	if(st.num_entities==MAX_GRAPHICS-1)
		return 2;
	else
		i=st.num_entities;

	ent[i].lightmapid=-1;

			if(data.vb_id!=-1)
			{
				ent[i].texrepeat[0]=(float) data.posx/32768;
				ent[i].texrepeat[1]=(float) data.posy/32768;
				ent[i].texrepeat[2]=(float) data.sizex/32768;
				ent[i].texrepeat[3]=(float) data.sizey/32768;
			}
			else
			{
				ent[i].texrepeat[0]=0.0f;
				ent[i].texrepeat[1]=0.0f;
				ent[i].texrepeat[2]=1.0f;
				ent[i].texrepeat[3]=1.0f;
			}
	
			ent[i].data=data;

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

			if(r==0 && g==0 && b==0)
				r=g=b=1;

			if(layer>7) layer=7;
			//else if(layer<0) layer+=8;

			z_buffer[layer][z_slot[layer]]=i;
			z_slot[layer]++;

			if(layer>z_used) z_used=layer;

			//timej=GetTicks();

			ent[i].vertex[0]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[1]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[2]=layer;

			ent[i].vertex[3]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[4]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[5]=layer;

			ent[i].vertex[6]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[7]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[8]=layer;

			ent[i].vertex[9]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[10]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[11]=layer;

			//timel=GetTicks() - timej;

			//ang=1000;
			/*
			ang/=10;

			tmp=cos((ang*pi)/180);
			tmp=mCos(ang*10);
	*/
			ax=(float) 1/(16384/2);
			ay=(float) 1/(8192/2);

			ay*=-1.0f;

			az=(float) 1/(4096/2);

			if(data.vb_id==-1)
			{
				ent[i].texcor[0]=x1;
				ent[i].texcor[1]=y1;
				ent[i].texcor[2]=x2;
				ent[i].texcor[3]=y1;
				ent[i].texcor[4]=x2;
				ent[i].texcor[5]=y2;
				ent[i].texcor[6]=x1;
				ent[i].texcor[7]=y2;
			}
			else
			{
				tx1=(float) x1/32768;
				ty1=(float) y1/32768;
				tx2=(float) x2/32768;
				ty2=(float) y2/32768;

				ent[i].texcor[0]=data.posx+(tx1*data.sizex);
				ent[i].texcor[1]=data.posy+(ty1*data.sizey);

				ent[i].texcor[2]=data.posx+(tx2*data.sizex);
				ent[i].texcor[3]=data.posy+(ty1*data.sizey);

				ent[i].texcor[4]=data.posx+(tx2*data.sizex);
				ent[i].texcor[5]=data.posy+(ty2*data.sizey);

				ent[i].texcor[6]=data.posx+(tx1*data.sizex);
				ent[i].texcor[7]=data.posy+(ty2*data.sizey);
			}

			for(j=0;j<12;j+=3)
			{
				ent[i].vertex[j]*=ax;
				ent[i].vertex[j]-=1;

		 		ent[i].vertex[j+1]*=ay;
				ent[i].vertex[j+1]+=1;
				
				ent[i].vertex[j+2]*=az;
				ent[i].vertex[j+2]-=1;
				
				if(j<8)
				{
					ent[i].texcor[j]/=(float)32768;
					ent[i].texcor[j+1]/=(float)32768;
					ent[i].texcor[j+2]/=(float)32768;
				}
				
			}

			for(j=0;j<16;j+=4)
			{
				ent[i].color[j]=r;
				ent[i].color[j+1]=g;
				ent[i].color[j+2]=b;
				ent[i].color[j+3]=a;
			}

			if(data.vb_id!=-1)
			{
				vbdt[data.vb_id].num_elements++;
				ent[i].data.loc=vbdt[data.vb_id].num_elements-1;
			}
			else
			{
				texone_ids[texone_num]=i;
				texone_num++;
			}

#endif

			st.num_entities++;

	return 0;
}

int8 DrawUI2(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, int32 x1, int32 y1, int32 x2, int32 y2, TEX_DATA data, uint8 a, int8 layer)
{
	float tmp, ax, ay, az, tx1, ty1, tx2, ty2;

	uint8 valx=0, valy=0;
	uint32 i=0, j=0, k=0;
	int32 t1, t2, t3, t4, timej, timel;

	t3=16384;
	t4=8192;

	//Checkbounds(x,y,sizex,sizey,ang,t3,t4)) return 1;
			
	if(st.num_entities==MAX_GRAPHICS-1)
		return 2;
	else
		i=st.num_entities;

	ent[i].lightmapid=-1;
	
			ent[i].data=data;

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

			if(r==0 && g==0 && b==0)
				r=g=b=1;

			x-=st.Camera.position.x;
			y-=st.Camera.position.y;

			sizex*=st.Camera.dimension.x;
			sizey*=st.Camera.dimension.y;

			if(layer>7) layer=7;
			//else if(layer<0) layer+=8;

			z_buffer[layer][z_slot[layer]]=i;
			z_slot[layer]++;

			if(layer>z_used) z_used=layer;

			//timej=GetTicks();

			ent[i].vertex[0]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[1]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[2]=layer;

			ent[i].vertex[3]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[4]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[5]=layer;

			ent[i].vertex[6]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[7]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[8]=layer;

			ent[i].vertex[9]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[10]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[11]=layer;

			//timel=GetTicks() - timej;

			//ang=1000;
			/*
			ang/=10;

			tmp=cos((ang*pi)/180);
			tmp=mCos(ang*10);
	*/
			ax=(float) 1/(16384/2);
			ay=(float) 1/(8192/2);

			ay*=-1.0f;

			az=(float) 1/(4096/2);

			if(data.vb_id==-1)
			{
				ent[i].texcor[0]=x1;
				ent[i].texcor[1]=y1;
				ent[i].texcor[2]=x2;
				ent[i].texcor[3]=y1;
				ent[i].texcor[4]=x2;
				ent[i].texcor[5]=y2;
				ent[i].texcor[6]=x1;
				ent[i].texcor[7]=y2;
			}
			else
			{
				tx1=(float) x1/32768;
				ty1=(float) y1/32768;
				tx2=(float) x2/32768;
				ty2=(float) y2/32768;

				ent[i].texcor[0]=data.posx+(tx1*data.sizex);
				ent[i].texcor[1]=data.posy+(ty1*data.sizey);

				ent[i].texcor[2]=data.posx+(tx2*data.sizex);
				ent[i].texcor[3]=data.posy+(ty1*data.sizey);

				ent[i].texcor[4]=data.posx+(tx2*data.sizex);
				ent[i].texcor[5]=data.posy+(ty2*data.sizey);

				ent[i].texcor[6]=data.posx+(tx1*data.sizex);
				ent[i].texcor[7]=data.posy+(ty2*data.sizey);
			}

			for(j=0;j<12;j+=3)
			{
				ent[i].vertex[j]*=ax;
				ent[i].vertex[j]-=1;

		 		ent[i].vertex[j+1]*=ay;
				ent[i].vertex[j+1]+=1;
				
				ent[i].vertex[j+2]*=az;
				ent[i].vertex[j+2]-=1;
				
				if(j<8)
				{
					ent[i].texcor[j]/=(float)32768;
					ent[i].texcor[j+1]/=(float)32768;
					ent[i].texcor[j+2]/=(float)32768;
				}
				
			}

			for(j=0;j<16;j+=4)
			{
				ent[i].color[j]=r;
				ent[i].color[j+1]=g;
				ent[i].color[j+2]=b;
				ent[i].color[j+3]=a;
			}

			if(data.vb_id!=-1)
			{
				vbdt[data.vb_id].num_elements++;
				ent[i].data.loc=vbdt[data.vb_id].num_elements-1;
			}
			else
			{
				texone_ids[texone_num]=i;
				texone_num++;
			}

#endif

			st.num_entities++;

	return 0;
}

void UIData(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, int32 x1, int32 y1, int32 x2, int32 y2, TEX_DATA data, uint8 a, int8 layer)
{
	uint16 i=st.num_calls;

	st.renderer.ppline[i].pos.x=x;
	st.renderer.ppline[i].pos.y=y;
	st.renderer.ppline[i].pos.z=layer;
	st.renderer.ppline[i].size.x=sizex;
	st.renderer.ppline[i].size.y=sizey;
	st.renderer.ppline[i].ang=ang;
	st.renderer.ppline[i].color.r=r;
	st.renderer.ppline[i].color.g=g;
	st.renderer.ppline[i].color.b=b;
	st.renderer.ppline[i].color.a=a;
	st.renderer.ppline[i].tex_panx=x1;
	st.renderer.ppline[i].tex_pany=y1;
	st.renderer.ppline[i].tex_sizex=x2;
	st.renderer.ppline[i].tex_sizey=y2;
	st.renderer.ppline[i].data=data;

	st.renderer.ppline[i].type=UI_CALL;

	st.num_calls++;
}

void GraphicData(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, TEX_DATA data, uint8 a, int32 x1, int32 y1, int32 x2, int32 y2, int8 z)
{
	uint16 i=st.num_calls;

	st.renderer.ppline[i].pos.x=x;
	st.renderer.ppline[i].pos.y=y;
	st.renderer.ppline[i].pos.z=z;
	st.renderer.ppline[i].size.x=sizex;
	st.renderer.ppline[i].size.y=sizey;
	st.renderer.ppline[i].ang=ang;
	st.renderer.ppline[i].color.r=r;
	st.renderer.ppline[i].color.g=g;
	st.renderer.ppline[i].color.b=b;
	st.renderer.ppline[i].color.a=a;
	st.renderer.ppline[i].tex_panx=x1;
	st.renderer.ppline[i].tex_pany=y1;
	st.renderer.ppline[i].tex_sizex=x2;
	st.renderer.ppline[i].tex_sizey=y2;
	st.renderer.ppline[i].data=data;

	st.renderer.ppline[i].type=GRAPHICS_CALL;

	st.num_calls++;
}

void String2Data(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z)
{
	uint16 i=st.num_calls;

	st.renderer.ppline[i].pos.x=x;
	st.renderer.ppline[i].pos.y=y;
	st.renderer.ppline[i].pos.z=z;
	st.renderer.ppline[i].size.x=sizex;
	st.renderer.ppline[i].size.y=sizey;
	st.renderer.ppline[i].size2.x=override_sizex;
	st.renderer.ppline[i].size2.y=override_sizey;
	st.renderer.ppline[i].ang=ang;
	st.renderer.ppline[i].color.r=r;
	st.renderer.ppline[i].color.g=g;
	st.renderer.ppline[i].color.b=b;
	st.renderer.ppline[i].color.a=a;
	//st.renderer.ppline[i].text=malloc(strlen(text));
	strcpy(st.renderer.ppline[i].text,text);
	st.renderer.ppline[i].font=font;

	st.renderer.ppline[i].type=STRING2_CALL;

	st.num_calls++;
}

void StringData(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z)
{
	uint16 i=st.num_calls;

	st.renderer.ppline[i].pos.x=x;
	st.renderer.ppline[i].pos.y=y;
	st.renderer.ppline[i].pos.z=z;
	st.renderer.ppline[i].size.x=sizex;
	st.renderer.ppline[i].size.y=sizey;
	st.renderer.ppline[i].size2.x=override_sizex;
	st.renderer.ppline[i].size2.y=override_sizey;
	st.renderer.ppline[i].ang=ang;
	st.renderer.ppline[i].color.r=r;
	st.renderer.ppline[i].color.g=g;
	st.renderer.ppline[i].color.b=b;
	st.renderer.ppline[i].color.a=a;
	//st.renderer.ppline[i].text=malloc(strlen(text));
	strcpy(st.renderer.ppline[i].text,text);
	st.renderer.ppline[i].font=font;

	st.renderer.ppline[i].type=STRING_CALL;

	st.num_calls++;
}

void StringUIvData(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z)
{
	uint16 i=st.num_calls;

	st.renderer.ppline[i].pos.x=x;
	st.renderer.ppline[i].pos.y=y;
	st.renderer.ppline[i].pos.z=z;
	st.renderer.ppline[i].size.x=sizex;
	st.renderer.ppline[i].size.y=sizey;
	st.renderer.ppline[i].size2.x=override_sizex;
	st.renderer.ppline[i].size2.y=override_sizey;
	st.renderer.ppline[i].ang=ang;
	st.renderer.ppline[i].color.r=r;
	st.renderer.ppline[i].color.g=g;
	st.renderer.ppline[i].color.b=b;
	st.renderer.ppline[i].color.a=a;
	//st.renderer.ppline[i].text=malloc(strlen(text));
	strcpy(st.renderer.ppline[i].text,text);
	st.renderer.ppline[i].font=font;

	st.renderer.ppline[i].type=STRINGUIV_CALL;

	st.num_calls++;
}

void StringUIData(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z)
{
	uint16 i=st.num_calls;

	st.renderer.ppline[i].pos.x=x;
	st.renderer.ppline[i].pos.y=y;
	st.renderer.ppline[i].pos.z=z;
	st.renderer.ppline[i].size.x=sizex;
	st.renderer.ppline[i].size.y=sizey;
	st.renderer.ppline[i].size2.x=override_sizex;
	st.renderer.ppline[i].size2.y=override_sizey;
	st.renderer.ppline[i].ang=ang;
	st.renderer.ppline[i].color.r=r;
	st.renderer.ppline[i].color.g=g;
	st.renderer.ppline[i].color.b=b;
	st.renderer.ppline[i].color.a=a;
	//st.renderer.ppline[i].text=malloc(strlen(text));
	strcpy(st.renderer.ppline[i].text,text);
	st.renderer.ppline[i].font=font;

	st.renderer.ppline[i].type=STRINGUI_CALL;

	st.num_calls++;
}

void StringUI2Data(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z)
{
	uint16 i=st.num_calls;

	st.renderer.ppline[i].pos.x=x;
	st.renderer.ppline[i].pos.y=y;
	st.renderer.ppline[i].pos.z=z;
	st.renderer.ppline[i].size.x=sizex;
	st.renderer.ppline[i].size.y=sizey;
	st.renderer.ppline[i].size2.x=override_sizex;
	st.renderer.ppline[i].size2.y=override_sizey;
	st.renderer.ppline[i].ang=ang;
	st.renderer.ppline[i].color.r=r;
	st.renderer.ppline[i].color.g=g;
	st.renderer.ppline[i].color.b=b;
	st.renderer.ppline[i].color.a=a;
	//st.renderer.ppline[i].text=malloc(strlen(text));
	strcpy(st.renderer.ppline[i].text,text);
	st.renderer.ppline[i].font=font;

	st.renderer.ppline[i].type=STRINGUI2_CALL;

	st.num_calls++;
}

void HudData(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, int32 x1, int32 y1, int32 x2, int32 y2, TEX_DATA data, uint8 a, int8 layer)
{
	uint16 i=st.num_calls;

	st.renderer.ppline[i].pos.x=x;
	st.renderer.ppline[i].pos.y=y;
	st.renderer.ppline[i].pos.z=layer;
	st.renderer.ppline[i].size.x=sizex;
	st.renderer.ppline[i].size.y=sizey;
	st.renderer.ppline[i].ang=ang;
	st.renderer.ppline[i].color.r=r;
	st.renderer.ppline[i].color.g=g;
	st.renderer.ppline[i].color.b=b;
	st.renderer.ppline[i].color.a=a;
	st.renderer.ppline[i].tex_panx=x1;
	st.renderer.ppline[i].tex_pany=y1;
	st.renderer.ppline[i].tex_sizex=x2;
	st.renderer.ppline[i].tex_sizey=y2;
	st.renderer.ppline[i].data=data;

	st.renderer.ppline[i].type=HUD_CALL;

	st.num_calls++;
}

void LineData(int32 x, int32 y, int32 x2, int32 y2, uint8 r, uint8 g, uint8 b, uint8 a, int16 linewidth, int32 z)
{
	uint16 i=st.num_calls;

	st.renderer.ppline[i].pos.x=x;
	st.renderer.ppline[i].pos.y=y;
	st.renderer.ppline[i].pos.z=z;
	st.renderer.ppline[i].pos2.x=x2;
	st.renderer.ppline[i].pos2.y=y2;
	st.renderer.ppline[i].size.x=linewidth;
	st.renderer.ppline[i].color.r=r;
	st.renderer.ppline[i].color.g=g;
	st.renderer.ppline[i].color.b=b;
	st.renderer.ppline[i].color.a=a;

	st.renderer.ppline[i].type=LINE_CALL;

	st.num_calls++;
}

int8 DrawLine(int32 x, int32 y, int32 x2, int32 y2, uint8 r, uint8 g, uint8 b, uint8 a, int16 linewidth, int32 z)
{
	uint8 valx=0, valy=0;

	uint16 i=0, j=0, k=0;

	int32 x3, y3;

	uint32 a1;

	int16 ang;

	float ax=1/(16384/2), ay=1/(8192/2), az=1/(4096/2), ang2, tx1, ty1, tx2, ty2;

	i=st.num_entities;

	if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
		return 2;

	//if(ent[i].stat==DEAD)
	//{
		ent[i].stat=USED;
		st.num_entities++;
		ent[i].data.data=DataNT;
		ent[i].data.vb_id=-1;
		ent[i].data.channel=0;
		ent[i].data.normal=0;
		ent[i].lightmapid=-1;

		if(z>56) z=56;
		//else if(z<16) z+=16;

		z_buffer[z][z_slot[z]]=i;
		z_slot[z]++;

		if(z>z_used) z_used=z;
		
		if(z>15)
		{
			tx1=x;
			ty1=y;
			tx2=x2;
			ty2=y2;

			x-=st.Camera.position.x;
			y-=st.Camera.position.y;

			x2-=st.Camera.position.x;
			y2-=st.Camera.position.y;

			x*=st.Camera.dimension.x;
			y*=st.Camera.dimension.y;

			x2*=st.Camera.dimension.x;
			y2*=st.Camera.dimension.y;

			linewidth*=st.Camera.dimension.x;
		}
		
		x3=x2-x;
		y3=y2-y;

		ang2=atan2((float)y3,(float)x3);
		if(ang2==pi)
			ang2=0;
		ang2+=pi;
		ang2=(180/pi)*ang2;
		ang=ang2;
		ang*=10;

		linewidth/=2;
		
		ent[i].vertex[0]=(float) x-(linewidth*mSin(ang));
		ent[i].vertex[1]=(float) y+(linewidth*mCos(ang));
		ent[i].vertex[2]=z;

		ent[i].vertex[3]=(float) x2-(linewidth*mSin(ang));
		ent[i].vertex[4]=(float) y2+(linewidth*mCos(ang));
		ent[i].vertex[5]=z;

		ent[i].vertex[6]=(float) x2+(linewidth*mSin(ang));
		ent[i].vertex[7]=(float) y2-(linewidth*mCos(ang));
		ent[i].vertex[8]=z;

		ent[i].vertex[9]=(float) x+(linewidth*mSin(ang));
		ent[i].vertex[10]=(float) y-(linewidth*mCos(ang));
		ent[i].vertex[11]=z;

		ax=(float) 1/(16384/2);
		ay=(float) 1/(8192/2);

		ay*=-1.0f;

		az=(float) 1/(4096/2);
		
		if(z>15)
		{
			/*
			ent[i].texcorlight[0]=(float)tx1+(((tx1-(tx2/2))-tx1)*mCos(ang) - ((ty1-(ty2/2))-ty1)*mSin(ang));
			ent[i].texcorlight[1]=(float)ty1+(((tx1-(tx2/2))-tx1)*mSin(ang) + ((ty1-(ty2/2))-ty1)*mCos(ang));

			ent[i].texcorlight[2]=(float)tx1+(((tx1+(tx2/2))-tx1)*mCos(ang) - ((ty1-(ty2/2))-ty1)*mSin(ang));
			ent[i].texcorlight[3]=(float)ty1+(((tx1+(tx2/2))-tx1)*mSin(ang) + ((ty1-(ty2/2))-ty1)*mCos(ang));

			ent[i].texcorlight[4]=(float)tx1+(((tx1+(tx2/2))-tx1)*mCos(ang) - ((ty1+(ty2/2))-ty1)*mSin(ang));
			ent[i].texcorlight[5]=(float)ty1+(((tx1+(tx2/2))-tx1)*mSin(ang) + ((ty1+(ty2/2))-ty1)*mCos(ang));

			ent[i].texcorlight[6]=(float)tx1+(((tx1-(tx2/2))-tx1)*mCos(ang) - ((ty1+(ty2/2))-ty1)*mSin(ang));
			ent[i].texcorlight[7]=(float)ty1+(((tx1-(tx2/2))-tx1)*mSin(ang) + ((ty1+(ty2/2))-ty1)*mCos(ang));
			*/
		
			ent[i].texcorlight[0]=(float) tx1-(linewidth*mSin(ang));
			ent[i].texcorlight[1]=(float) ty1+(linewidth*mCos(ang));

			ent[i].texcorlight[2]=(float) tx2-(linewidth*mSin(ang));
			ent[i].texcorlight[3]=(float) ty2+(linewidth*mCos(ang));

			ent[i].texcorlight[4]=(float) tx2+(linewidth*mSin(ang));
			ent[i].texcorlight[5]=(float) ty2-(linewidth*mCos(ang));

			ent[i].texcorlight[6]=(float) tx1+(linewidth*mSin(ang));
			ent[i].texcorlight[7]=(float) ty1-(linewidth*mCos(ang));

			WTSf(&ent[i].texcorlight[0],&ent[i].texcorlight[1]);
			WTSf(&ent[i].texcorlight[2],&ent[i].texcorlight[3]);
			WTSf(&ent[i].texcorlight[4],&ent[i].texcorlight[5]);
			WTSf(&ent[i].texcorlight[6],&ent[i].texcorlight[7]);
			
			ent[i].texcorlight[0]/=(float) st.screenx;
			ent[i].texcorlight[1]/=(float) st.screeny;
			ent[i].texcorlight[2]/=(float) st.screenx;
			ent[i].texcorlight[3]/=(float) st.screeny;
			ent[i].texcorlight[4]/=(float) st.screenx;
			ent[i].texcorlight[5]/=(float) st.screeny;
			ent[i].texcorlight[6]/=(float) st.screenx;
			ent[i].texcorlight[7]/=(float) st.screeny;

			ent[i].texcorlight[1]*=-1;
			ent[i].texcorlight[3]*=-1;
			ent[i].texcorlight[5]*=-1;
			ent[i].texcorlight[7]*=-1;
		}

		ent[i].texcor[0]=0.0f;
		ent[i].texcor[1]=0.0f;
		ent[i].texcor[2]=1.0f;
		ent[i].texcor[3]=0.0f;
		ent[i].texcor[4]=1.0f;
		ent[i].texcor[5]=1.0f;
		ent[i].texcor[6]=0.0f;
		ent[i].texcor[7]=1.0f;

		for(j=0;j<12;j+=3)
		{
			ent[i].vertex[j]*=ax;
			ent[i].vertex[j]-=1;

			ent[i].vertex[j+1]*=ay;
			ent[i].vertex[j+1]+=1;
				
			ent[i].vertex[j+2]*=az;
			ent[i].vertex[j+2]-=1;
				
		}

		for(j=0;j<16;j+=4)
		{
			ent[i].color[j]=r;
			ent[i].color[j+1]=g;
			ent[i].color[j+2]=b;
			ent[i].color[j+3]=a;
		}

		texone_ids[texone_num]=i;
		texone_num++;
	//}

	return 0;
}

void SetAnim(int16 id, int16 sprite_id)
{
	if(mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].startID<mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].endID)
	{
		if(st.Current_Map.sprites[sprite_id].current_frame<mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].startID*100 || 
			st.Current_Map.sprites[sprite_id].current_frame>=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].endID*100)
		{
			st.Current_Map.sprites[sprite_id].frame_ID=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].startID;
			st.Current_Map.sprites[sprite_id].current_frame=st.Current_Map.sprites[sprite_id].frame_ID*100;
		}
	}
	else
	{
		if(st.Current_Map.sprites[sprite_id].current_frame>mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].startID*100 || 
			st.Current_Map.sprites[sprite_id].current_frame<=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].endID*100)
		{
			st.Current_Map.sprites[sprite_id].frame_ID=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].startID;
			st.Current_Map.sprites[sprite_id].current_frame=st.Current_Map.sprites[sprite_id].frame_ID*100;
		}
	}
}

int8 CheckAnim(int16 sprite_id, int16 anim)
{
	if(st.Current_Map.sprites[sprite_id].frame_ID>=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[anim].startID && 
		st.Current_Map.sprites[sprite_id].frame_ID<mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[anim].endID)
		return 1;
	else
		return 0;
}

int8 MAnim(int16 id, float speed_mul, int16 sprite_id, int8 loop)
{
	uint16 curf=0;
	int16 speed=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].speed;

	if(speed_mul==0.00)
		speed_mul=1.0;

	speed*=speed_mul;

	if(speed==0)
		speed=1;

	if(speed>0)
	{
		if(st.Current_Map.sprites[sprite_id].current_frame<mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].startID*100)
			st.Current_Map.sprites[sprite_id].current_frame=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].startID*100;
	}
	else
	if(speed<0)
	{
		if(st.Current_Map.sprites[sprite_id].current_frame>mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].startID*100)
			st.Current_Map.sprites[sprite_id].current_frame=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].startID*100;
	}


	st.Current_Map.sprites[sprite_id].current_frame+=speed;

	if(loop)
	{
		if(speed<0)
		{
			if(st.Current_Map.sprites[sprite_id].current_frame<=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].endID*100)
				st.Current_Map.sprites[sprite_id].current_frame=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].startID*100;
		}
		else
		{
			if(st.Current_Map.sprites[sprite_id].current_frame>=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].endID*100)
				st.Current_Map.sprites[sprite_id].current_frame=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].startID*100;
		}
	}
	else
	{
		if(speed<0)
		{
			if(st.Current_Map.sprites[sprite_id].current_frame<=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].endID*100)
				return 1;
		}
		else
		{
			if(st.Current_Map.sprites[sprite_id].current_frame>=mgg_game[st.Current_Map.sprites[sprite_id].MGG_ID].anim[id].endID*100)
				return 1;
		}
	}

	st.Current_Map.sprites[sprite_id].frame_ID=st.Current_Map.sprites[sprite_id].current_frame/100;

	return 0;
}

void GetSpriteBodySize(int16 id, int16 gameid)
{
	if(st.Game_Sprites[gameid].flags & 4)
	{
		if(mgg_game[st.Current_Map.sprites[id].MGG_ID].frames[st.Current_Map.sprites[id].frame_ID].vb_id!=-1)
		{
			if(st.Game_Sprites[gameid].size_m.x!=NULL && st.Game_Sprites[gameid].size_m.y!=NULL)
			{
				st.Current_Map.sprites[id].body.size.x=(mgg_game[st.Current_Map.sprites[id].MGG_ID].frames[st.Current_Map.sprites[id].frame_ID].sizex*st.Game_Sprites[gameid].size_m.x)+st.Game_Sprites[gameid].size_a.x;
				st.Current_Map.sprites[id].body.size.y=(mgg_game[st.Current_Map.sprites[id].MGG_ID].frames[st.Current_Map.sprites[id].frame_ID].sizey*st.Game_Sprites[gameid].size_m.y)+st.Game_Sprites[gameid].size_a.y;
			}
			else
			{
				st.Current_Map.sprites[id].body.size.x=mgg_game[st.Current_Map.sprites[id].MGG_ID].frames[st.Current_Map.sprites[id].frame_ID].sizex+st.Game_Sprites[gameid].size_a.x;
				st.Current_Map.sprites[id].body.size.y=mgg_game[st.Current_Map.sprites[id].MGG_ID].frames[st.Current_Map.sprites[id].frame_ID].sizey+st.Game_Sprites[gameid].size_a.y;
			}
		}
		else
		{
			if(st.Game_Sprites[gameid].size_m.x!=NULL && st.Game_Sprites[gameid].size_m.y!=NULL)
			{
				st.Current_Map.sprites[id].body.size.x=(((mgg_game[st.Current_Map.sprites[id].MGG_ID].frames[st.Current_Map.sprites[id].frame_ID].w*16384)/st.screenx)*
					st.Game_Sprites[gameid].size_m.x)+st.Game_Sprites[gameid].size_a.x;
				st.Current_Map.sprites[id].body.size.y=(((mgg_game[st.Current_Map.sprites[id].MGG_ID].frames[st.Current_Map.sprites[id].frame_ID].h*8192)/st.screeny)*
					st.Game_Sprites[gameid].size_m.y)+st.Game_Sprites[gameid].size_a.y;
			}
			else
			{
				st.Current_Map.sprites[id].body.size.x=((mgg_game[st.Current_Map.sprites[id].MGG_ID].frames[st.Current_Map.sprites[id].frame_ID].w*16384)/st.screenx)+st.Game_Sprites[gameid].size_a.x;
				st.Current_Map.sprites[id].body.size.y=((mgg_game[st.Current_Map.sprites[id].MGG_ID].frames[st.Current_Map.sprites[id].frame_ID].h*8192)/st.screeny)+st.Game_Sprites[gameid].size_a.y;
			}
		}
	}
}

int8 DrawString(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z)
{	
	uint8 valx=0, valy=0;
	uint32 i=0, j=0, k=0, checked=0;
	int32 t1, t2, t3, t4, timej, timel, id=-1;
	float tx1, ty1, tx2, ty2;

	PosF dim=st.Camera.dimension;

	SDL_Color co;
	uint16 formatt;

	float tmp, ax, ay, az;

	uint8 val=0;
	
	SDL_Surface *msg;

	if(st.num_entities==MAX_GRAPHICS-1)
		return 2;

	//Checkbounds(x,y,sizex,sizey,ang,16384,8192)) return 1;

	for(i=0;i<MAX_STRINGS;i++)
	{
		if(st.strings[i].stat==2)
		{
			if(strcmp(text,st.strings[i].string)==NULL)
			{
				st.strings[i].stat=1;
				j=st.num_entities;
				st.strings[i].data.posx=i;
				memcpy(&ent[j].data,&st.strings[i].data,sizeof(TEX_DATA));
				id=i;
				checked=1;
				break;
			}
		}
		else
		if(st.strings[i].stat==0 && checked==0)
		{
			checked=3;
			id=i;
		}

		if(i==MAX_STRINGS-1)
		{
			if(id!=-1 && checked==3)
			{
				st.strings[id].stat=1;
				strcpy(st.strings[id].string,text);
				checked=2;
			}
			else
			if(checked==0 && id==-1)
			{
				LogApp("Warning: max number os strings reached");
				return 2;
			}
		}
	}

	i=st.num_entities;

	if(checked==2)
	{

		co.r=255;
		co.g=255;
		co.b=255;
		co.a=255;
	
		if(strlen(text)==0) 
			msg=TTF_RenderUTF8_Blended(st.fonts[font].font," ",co);
		else
			msg=TTF_RenderUTF8_Blended(st.fonts[font].font,text,co);
	
		if(msg->format->BytesPerPixel==4)
		{
			if(msg->format->Rmask==0x000000ff) formatt=GL_RGBA;
			else formatt=GL_BGRA_EXT;
		} else
		{
			if(msg->format->Rmask==0x000000ff) formatt=GL_RGB;
			else formatt=GL_BGR_EXT;
		}

		glGenTextures(1,&ent[i].data.data);
		glBindTexture(GL_TEXTURE_2D,ent[i].data.data);
		glTexImage2D(GL_TEXTURE_2D,0,msg->format->BytesPerPixel,msg->w,msg->h,0,formatt,GL_UNSIGNED_BYTE,msg->pixels);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		ent[i].data.channel=63; //magical number only used for rendering

		ent[i].data.posx=id;

		ent[i].data.normal=0;
		ent[i].data.vb_id=-1;

		ent[i].data.w=msg->w;
		ent[i].data.h=msg->h;

		memcpy(&st.strings[id].data,&ent[i].data,sizeof(TEX_DATA));

		SDL_FreeSurface(msg);
	}

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

	ent[i].lightmapid=-1;

			
			if(override_sizex!=0)
				sizex=st.strings[id].data.w*(float)(override_sizex/1024.0f);

			if(override_sizey!=0)
				sizey=st.strings[id].data.h*(float)(override_sizey/1024.0f);

			ent[i].texrepeat[0]=0.0f;
			ent[i].texrepeat[1]=0.0f;
			ent[i].texrepeat[2]=1.0f;
			ent[i].texrepeat[3]=1.0f;

			tx1=x;
			ty1=y;
			tx2=sizex;
			ty2=sizey;

			if(r==0 && g==0 && b==0)
				r=g=b=1;

			if(z>56) z=56;
			else if(z<16) z+=16;

			z_buffer[z][z_slot[z]]=i;
			z_slot[z]++;

			if(z>z_used) z_used=z;

			if(z>39 && z<48)
			{
				
				x-=(float) st.Camera.position.x*st.Current_Map.bck2_v;
				y-=(float) st.Camera.position.y*st.Current_Map.bck2_v;
			}
			else
			if(z>31 && z<40)
			{
				
				x-=(float) st.Camera.position.x*st.Current_Map.bck1_v;
				y-=(float) st.Camera.position.y*st.Current_Map.bck1_v;

				sizex*=st.Camera.dimension.x;
				sizey*=st.Camera.dimension.y;

				x*=st.Camera.dimension.x;
				y*=st.Camera.dimension.y;
			}
			else
			if(z>23 && z<32)
			{
				x-=st.Camera.position.x;
				y-=st.Camera.position.y;

				sizex*=st.Camera.dimension.x;
				sizey*=st.Camera.dimension.y;

				x*=st.Camera.dimension.x;
				y*=st.Camera.dimension.y;
			}
			if(z>15 && z<24)
			{
				x-=(float) st.Camera.position.x*st.Current_Map.fr_v;
				y-=(float) st.Camera.position.y*st.Current_Map.fr_v;

				sizex*=st.Camera.dimension.x;
				sizey*=st.Camera.dimension.y;

				x*=st.Camera.dimension.x;
				y*=st.Camera.dimension.y;
			}

			//timej=GetTicks();

			ent[i].vertex[0]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[1]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[2]=z;

			ent[i].vertex[3]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[4]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[5]=z;

			ent[i].vertex[6]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[7]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[8]=z;

			ent[i].vertex[9]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[10]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[11]=z;

			ang/=10;

			tmp=cos((ang*pi)/180);
			tmp=mCos(ang*10);
	
			ax=(float) 1/(16384/2);
			ay=(float) 1/(8192/2);

			ay*=-1.0f;

			az=(float) 1/(4096/2);

			
			ent[i].texcorlight[0]=(float)tx1+(((tx1-(tx2/2))-tx1)*mCos(ang) - ((ty1-(ty2/2))-ty1)*mSin(ang));
			ent[i].texcorlight[1]=(float)ty1+(((tx1-(tx2/2))-tx1)*mSin(ang) + ((ty1-(ty2/2))-ty1)*mCos(ang));

			ent[i].texcorlight[2]=(float)tx1+(((tx1+(tx2/2))-tx1)*mCos(ang) - ((ty1-(ty2/2))-ty1)*mSin(ang));
			ent[i].texcorlight[3]=(float)ty1+(((tx1+(tx2/2))-tx1)*mSin(ang) + ((ty1-(ty2/2))-ty1)*mCos(ang));

			ent[i].texcorlight[4]=(float)tx1+(((tx1+(tx2/2))-tx1)*mCos(ang) - ((ty1+(ty2/2))-ty1)*mSin(ang));
			ent[i].texcorlight[5]=(float)ty1+(((tx1+(tx2/2))-tx1)*mSin(ang) + ((ty1+(ty2/2))-ty1)*mCos(ang));

			ent[i].texcorlight[6]=(float)tx1+(((tx1-(tx2/2))-tx1)*mCos(ang) - ((ty1+(ty2/2))-ty1)*mSin(ang));
			ent[i].texcorlight[7]=(float)ty1+(((tx1-(tx2/2))-tx1)*mSin(ang) + ((ty1+(ty2/2))-ty1)*mCos(ang));

			WTSf(&ent[i].texcorlight[0],&ent[i].texcorlight[1]);
			WTSf(&ent[i].texcorlight[2],&ent[i].texcorlight[3]);
			WTSf(&ent[i].texcorlight[4],&ent[i].texcorlight[5]);
			WTSf(&ent[i].texcorlight[6],&ent[i].texcorlight[7]);
			
			ent[i].texcorlight[0]/=(float) st.screenx;
			ent[i].texcorlight[1]/=(float) st.screeny;
			ent[i].texcorlight[2]/=(float) st.screenx;
			ent[i].texcorlight[3]/=(float) st.screeny;
			ent[i].texcorlight[4]/=(float) st.screenx;
			ent[i].texcorlight[5]/=(float) st.screeny;
			ent[i].texcorlight[6]/=(float) st.screenx;
			ent[i].texcorlight[7]/=(float) st.screeny;

			ent[i].texcorlight[1]*=-1;
			ent[i].texcorlight[3]*=-1;
			ent[i].texcorlight[5]*=-1;
			ent[i].texcorlight[7]*=-1;

			ent[i].texcor[0]=0;
			ent[i].texcor[1]=0;
			ent[i].texcor[2]=1;
			ent[i].texcor[3]=0;
			ent[i].texcor[4]=1;
			ent[i].texcor[5]=1;
			ent[i].texcor[6]=0;
			ent[i].texcor[7]=1;

			for(j=0;j<12;j+=3)
			{
				ent[i].vertex[j]*=ax;
				ent[i].vertex[j]-=1;

				ent[i].vertex[j+1]*=ay;
				ent[i].vertex[j+1]+=1;
				
				ent[i].vertex[j+2]*=az;
				ent[i].vertex[j+2]-=1;
			}

			for(j=0;j<16;j+=4)
			{
				ent[i].color[j]=r;
				ent[i].color[j+1]=g;
				ent[i].color[j+2]=b;
				ent[i].color[j+3]=a;
			}

			
				texone_ids[texone_num]=i;
				texone_num++;

#endif

			st.num_entities++;

	return 0;

}

int8 DrawString2(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z)
{	
	uint8 valx=0, valy=0;
	uint32 i=0, j=0, k=0, checked=0;
	int32 t1, t2, t3, t4, timej, timel, id=-1;
	
	PosF dim=st.Camera.dimension;

	SDL_Color co;
	uint16 formatt;

	float tmp, ax, ay, az;

	uint8 val=0;
	
	SDL_Surface *msg;

	if(st.num_entities==MAX_GRAPHICS-1)
		return 2;

	//Checkbounds(x,y,sizex,sizey,ang,16384,8192)) return 1;

	for(i=0;i<MAX_STRINGS;i++)
	{
		if(st.strings[i].stat==2)
		{
			if(strcmp(text,st.strings[i].string)==NULL)
			{
				st.strings[i].stat=1;
				j=st.num_entities;
				st.strings[i].data.posx=i;
				memcpy(&ent[j].data,&st.strings[i].data,sizeof(TEX_DATA));
				id=i;
				checked=1;
				break;
			}
		}
		else
		if(st.strings[i].stat==0 && checked==0)
		{
			checked=3;
			id=i;
		}

		if(i==MAX_STRINGS-1)
		{
			if(id!=-1 && checked==3)
			{
				st.strings[id].stat=1;
				strcpy(st.strings[id].string,text);
				checked=2;
			}
			else
			if(checked==0 && id==-1)
			{
				LogApp("Warning: max number os strings reached");
				return 2;
			}
		}
	}

	i=st.num_entities;

	if(checked==2)
	{

		co.r=255;
		co.g=255;
		co.b=255;
		co.a=255;
	
		if(strlen(text)==0) 
			msg=TTF_RenderUTF8_Blended(st.fonts[font].font," ",co);
		else
			msg=TTF_RenderUTF8_Blended(st.fonts[font].font,text,co);
	
		if(msg->format->BytesPerPixel==4)
		{
			if(msg->format->Rmask==0x000000ff) formatt=GL_RGBA;
			else formatt=GL_BGRA_EXT;
		} else
		{
			if(msg->format->Rmask==0x000000ff) formatt=GL_RGB;
			else formatt=GL_BGR_EXT;
		}

		glGenTextures(1,&ent[i].data.data);
		glBindTexture(GL_TEXTURE_2D,ent[i].data.data);
		glTexImage2D(GL_TEXTURE_2D,0,msg->format->BytesPerPixel,msg->w,msg->h,0,formatt,GL_UNSIGNED_BYTE,msg->pixels);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		ent[i].data.channel=63; //magical number only used for rendering

		ent[i].data.posx=id;

		ent[i].data.normal=0;
		ent[i].data.vb_id=-1;

		ent[i].data.w=msg->w;
		ent[i].data.h=msg->h;

		ent[i].lightmapid=-1;

		memcpy(&st.strings[id].data,&ent[i].data,sizeof(TEX_DATA));

		SDL_FreeSurface(msg);
	}

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

	ent[i].lightmapid=-1;

			if(override_sizex!=0)
				sizex=st.strings[id].data.w*(float)(override_sizex/1024.0f);

			if(override_sizey!=0)
				sizey=st.strings[id].data.h*(float)(override_sizey/1024.0f);

			ent[i].texrepeat[0]=0.0f;
			ent[i].texrepeat[1]=0.0f;
			ent[i].texrepeat[2]=1.0f;
			ent[i].texrepeat[3]=1.0f;

			if(r==0 && g==0 && b==0)
				r=g=b=1;

			if(z>39 && z<48)
			{
				
				x-=(float) st.Camera.position.x*st.Current_Map.bck2_v;
				y-=(float) st.Camera.position.y*st.Current_Map.bck2_v;
			}
			else
			if(z>31 && z<40)
			{
				
				x-=(float) st.Camera.position.x*st.Current_Map.bck1_v;
				y-=(float) st.Camera.position.y*st.Current_Map.bck1_v;

				sizex*=st.Camera.dimension.x;
				sizey*=st.Camera.dimension.y;

				x*=st.Camera.dimension.x;
				y*=st.Camera.dimension.y;
			}
			else
			if(z>23 && z<32)
			{
				x-=st.Camera.position.x;
				y-=st.Camera.position.y;

				sizex*=st.Camera.dimension.x;
				sizey*=st.Camera.dimension.y;

				x*=st.Camera.dimension.x;
				y*=st.Camera.dimension.y;
			}
			if(z>15 && z<24)
			{
				x-=(float) st.Camera.position.x*st.Current_Map.fr_v;
				y-=(float) st.Camera.position.y*st.Current_Map.fr_v;

				sizex*=st.Camera.dimension.x;
				sizey*=st.Camera.dimension.y;

				x*=st.Camera.dimension.x;
				y*=st.Camera.dimension.y;
			}

			if(z>15) z=15;
			if(z<8) z=8;

			z_buffer[z][z_slot[z]]=i;
			z_slot[z]++;

			if(z>z_used) z_used=z;

			//timej=GetTicks();

			ent[i].vertex[0]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[1]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[2]=z;

			ent[i].vertex[3]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[4]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[5]=z;

			ent[i].vertex[6]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[7]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[8]=z;

			ent[i].vertex[9]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[10]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[11]=z;

			ang/=10;

			tmp=cos((ang*pi)/180);
			tmp=mCos(ang*10);
	
			ax=(float) 1/(16384/2);
			ay=(float) 1/(8192/2);

			ay*=-1.0f;

			az=(float) 1/(4096/2);

			
				ent[i].texcor[0]=0;
				ent[i].texcor[1]=0;
				ent[i].texcor[2]=1;
				ent[i].texcor[3]=0;
				ent[i].texcor[4]=1;
				ent[i].texcor[5]=1;
				ent[i].texcor[6]=0;
				ent[i].texcor[7]=1;

			for(j=0;j<12;j+=3)
			{
				ent[i].vertex[j]*=ax;
				ent[i].vertex[j]-=1;

				ent[i].vertex[j+1]*=ay;
				ent[i].vertex[j+1]+=1;
				
				ent[i].vertex[j+2]*=az;
				ent[i].vertex[j+2]-=1;
			}

			for(j=0;j<16;j+=4)
			{
				ent[i].color[j]=r;
				ent[i].color[j+1]=g;
				ent[i].color[j+2]=b;
				ent[i].color[j+3]=a;
			}

			
				texone_ids[texone_num]=i;
				texone_num++;

#endif

			st.num_entities++;

	return 0;

}

int8 DrawStringUIv(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z)
{	
	uint8 valx=0, valy=0;
	uint32 i=0, j=0, k=0, checked=0;
	int32 t1, t2, t3, t4, timej, timel, id=-1;

	SDL_Color co;
	uint16 formatt;

	float tmp, ax, ay, az;

	uint8 val=0;
	
	SDL_Surface *msg;
if(st.num_entities==MAX_GRAPHICS-1)
		return 2;

	//Checkbounds(x,y,sizex,sizey,ang,16384,8192)) return 1;

	for(i=0;i<MAX_STRINGS;i++)
	{
		if(st.strings[i].stat==2)
		{
			if(strcmp(text,st.strings[i].string)==NULL)
			{
				st.strings[i].stat=1;
				j=st.num_entities;
				st.strings[i].data.posx=i;
				memcpy(&ent[j].data,&st.strings[i].data,sizeof(TEX_DATA));
				id=i;
				checked=1;
				break;
			}
		}
		else
		if(st.strings[i].stat==0 && checked==0)
		{
			checked=3;
			id=i;
		}

		if(i==MAX_STRINGS-1)
		{
			if(id!=-1 && checked==3)
			{
				st.strings[id].stat=1;
				strcpy(st.strings[id].string,text);
				checked=2;
			}
			else
			if(checked==0 && id==-1)
			{
				LogApp("Warning: max number os strings reached");
				return 2;
			}
		}
	}

	i=st.num_entities;

	if(checked==2)
	{

		co.r=255;
		co.g=255;
		co.b=255;
		co.a=255;

		if(strlen(text)==0) 
			msg=TTF_RenderUTF8_Blended(st.fonts[font].font," ",co);
		else
			msg=TTF_RenderUTF8_Blended(st.fonts[font].font,text,co);
	
		if(msg->format->BytesPerPixel==4)
		{
			if(msg->format->Rmask==0x000000ff) formatt=GL_RGBA;
			else formatt=GL_BGRA_EXT;
		} else
		{
			if(msg->format->Rmask==0x000000ff) formatt=GL_RGB;
			else formatt=GL_BGR_EXT;
		}

		glGenTextures(1,&ent[i].data.data);
		glBindTexture(GL_TEXTURE_2D,ent[i].data.data);
		glTexImage2D(GL_TEXTURE_2D,0,msg->format->BytesPerPixel,msg->w,msg->h,0,formatt,GL_UNSIGNED_BYTE,msg->pixels);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		ent[i].data.channel=63; //magical number only used for rendering

		ent[i].data.posx=id;

		ent[i].data.normal=0;
		ent[i].data.vb_id=-1;

		ent[i].data.w=msg->w;
		ent[i].data.h=msg->h;

		memcpy(&st.strings[id].data,&ent[i].data,sizeof(TEX_DATA));

		SDL_FreeSurface(msg);
	}

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

	ent[i].lightmapid=-1;

			
			if(override_sizex!=0)
				sizex=st.strings[id].data.w*(float)(override_sizex/1024.0f);

			if(override_sizey!=0)
				sizey=st.strings[id].data.h*(float)(override_sizey/1024.0f);

			if(r==0 && g==0 && b==0)
				r=g=b=1;

			if(z>7) z=7;
			//else if(z<0) z+=8;

			z_buffer[z][z_slot[z]]=i;
			z_slot[z]++;

			if(z>z_used) z_used=z;

			x+=sizex/2;
			//y-=sizey/2;

			//timej=GetTicks();

			ent[i].vertex[0]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[1]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[2]=z;

			ent[i].vertex[3]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[4]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[5]=z;

			ent[i].vertex[6]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[7]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[8]=z;

			ent[i].vertex[9]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[10]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[11]=z;

			/*
			ang/=10;

			tmp=cos((ang*pi)/180);
			tmp=mCos(ang*10);
	*/
			ax=(float) 1/(16384/2);
			ay=(float) 1/(8192/2);

			ay*=-1.0f;

			az=(float) 1/(4096/2);

			
				ent[i].texcor[0]=0;
				ent[i].texcor[1]=0;
				ent[i].texcor[2]=1;
				ent[i].texcor[3]=0;
				ent[i].texcor[4]=1;
				ent[i].texcor[5]=1;
				ent[i].texcor[6]=0;
				ent[i].texcor[7]=1;

			for(j=0;j<12;j+=3)
			{
				ent[i].vertex[j]*=ax;
				ent[i].vertex[j]-=1;

				ent[i].vertex[j+1]*=ay;
				ent[i].vertex[j+1]+=1;
				
				ent[i].vertex[j+2]*=az;
				ent[i].vertex[j+2]-=1;
			}

			for(j=0;j<16;j+=4)
			{
				ent[i].color[j]=r;
				ent[i].color[j+1]=g;
				ent[i].color[j+2]=b;
				ent[i].color[j+3]=a;
			}

			
				texone_ids[texone_num]=i;
				texone_num++;

#endif

			st.num_entities++;

	return 0;

}

int8 DrawStringUI(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z)
{	
	uint8 valx=0, valy=0;
	uint32 i=0, j=0, k=0, checked=0;
	int32 t1, t2, t3, t4, timej, timel, id=-1;

	SDL_Color co;
	uint16 formatt;

	float tmp, ax, ay, az;

	uint8 val=0;
	
	SDL_Surface *msg;
if(st.num_entities==MAX_GRAPHICS-1)
		return 2;

	//Checkbounds(x,y,sizex,sizey,ang,16384,8192)) return 1;

	for(i=0;i<MAX_STRINGS;i++)
	{
		if(st.strings[i].stat==2)
		{
			if(strcmp(text,st.strings[i].string)==NULL)
			{
				st.strings[i].stat=1;
				j=st.num_entities;
				st.strings[i].data.posx=i;
				memcpy(&ent[j].data,&st.strings[i].data,sizeof(TEX_DATA));
				id=i;
				checked=1;
				break;
			}
		}
		else
		if(st.strings[i].stat==0 && checked==0)
		{
			checked=3;
			id=i;
		}

		if(i==MAX_STRINGS-1)
		{
			if(id!=-1 && checked==3)
			{
				st.strings[id].stat=1;
				strcpy(st.strings[id].string,text);
				checked=2;
			}
			else
			if(checked==0 && id==-1)
			{
				LogApp("Warning: max number os strings reached");
				return 2;
			}
		}
	}

	i=st.num_entities;

	if(checked==2)
	{

		co.r=255;
		co.g=255;
		co.b=255;
		co.a=255;

		if(strlen(text)==0) 
			msg=TTF_RenderUTF8_Blended(st.fonts[font].font," ",co);
		else
			msg=TTF_RenderUTF8_Blended(st.fonts[font].font,text,co);
	
		if(msg->format->BytesPerPixel==4)
		{
			if(msg->format->Rmask==0x000000ff) formatt=GL_RGBA;
			else formatt=GL_BGRA_EXT;
		} else
		{
			if(msg->format->Rmask==0x000000ff) formatt=GL_RGB;
			else formatt=GL_BGR_EXT;
		}

		glGenTextures(1,&ent[i].data.data);
		glBindTexture(GL_TEXTURE_2D,ent[i].data.data);
		glTexImage2D(GL_TEXTURE_2D,0,msg->format->BytesPerPixel,msg->w,msg->h,0,formatt,GL_UNSIGNED_BYTE,msg->pixels);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		ent[i].data.channel=63; //magical number only used for rendering

		ent[i].data.posx=id;

		ent[i].data.normal=0;
		ent[i].data.vb_id=-1;

		ent[i].data.w=msg->w;
		ent[i].data.h=msg->h;

		memcpy(&st.strings[id].data,&ent[i].data,sizeof(TEX_DATA));

		SDL_FreeSurface(msg);
	}

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

	ent[i].lightmapid=-1;

			
			if(override_sizex!=0)
				sizex=st.strings[id].data.w*(float)(override_sizex/1024.0f);

			if(override_sizey!=0)
				sizey=st.strings[id].data.h*(float)(override_sizey/1024.0f);

			ent[i].texrepeat[0]=0.0f;
			ent[i].texrepeat[1]=0.0f;
			ent[i].texrepeat[2]=1.0f;
			ent[i].texrepeat[3]=1.0f;

			if(r==0 && g==0 && b==0)
				r=g=b=1;

			if(z>7) z=7;
			//else if(z<0) z+=8;

			z_buffer[z][z_slot[z]]=i;
			z_slot[z]++;

			if(z>z_used) z_used=z;

			//timej=GetTicks();

			ent[i].vertex[0]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[1]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[2]=z;

			ent[i].vertex[3]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[4]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[5]=z;

			ent[i].vertex[6]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[7]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[8]=z;

			ent[i].vertex[9]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[10]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[11]=z;

			/*
			ang/=10;

			tmp=cos((ang*pi)/180);
			tmp=mCos(ang*10);
	*/
			ax=(float) 1/(16384/2);
			ay=(float) 1/(8192/2);

			ay*=-1.0f;

			az=(float) 1/(4096/2);

			
				ent[i].texcor[0]=0;
				ent[i].texcor[1]=0;
				ent[i].texcor[2]=1;
				ent[i].texcor[3]=0;
				ent[i].texcor[4]=1;
				ent[i].texcor[5]=1;
				ent[i].texcor[6]=0;
				ent[i].texcor[7]=1;

			for(j=0;j<12;j+=3)
			{
				ent[i].vertex[j]*=ax;
				ent[i].vertex[j]-=1;

				ent[i].vertex[j+1]*=ay;
				ent[i].vertex[j+1]+=1;
				
				ent[i].vertex[j+2]*=az;
				ent[i].vertex[j+2]-=1;
			}

			for(j=0;j<16;j+=4)
			{
				ent[i].color[j]=r;
				ent[i].color[j+1]=g;
				ent[i].color[j+2]=b;
				ent[i].color[j+3]=a;
			}

			
				texone_ids[texone_num]=i;
				texone_num++;

#endif

			st.num_entities++;

	return 0;

}

int8 DrawString2UI(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z)
{	
	uint8 valx=0, valy=0;
	uint32 i=0, j=0, k=0, checked=0;
	int32 t1, t2, t3, t4, timej, timel, id=-1;

	SDL_Color co;
	uint16 formatt;

	float tmp, ax, ay, az;

	uint8 val=0;
	
	SDL_Surface *msg;

	if(st.num_entities==MAX_GRAPHICS-1)
		return 2;

	//Checkbounds(x,y,sizex,sizey,ang,16384,8192)) return 1;

	for(i=0;i<MAX_STRINGS;i++)
	{
		if(st.strings[i].stat==2)
		{
			if(strcmp(text,st.strings[i].string)==NULL)
			{
				st.strings[i].stat=1;
				j=st.num_entities;
				st.strings[i].data.posx=i;
				memcpy(&ent[j].data,&st.strings[i].data,sizeof(TEX_DATA));
				id=i;
				checked=1;
				break;
			}
		}
		else
		if(st.strings[i].stat==0 && checked==0)
		{
			checked=3;
			id=i;
		}

		if(i==MAX_STRINGS-1)
		{
			if(id!=-1 && checked==3)
			{
				st.strings[id].stat=1;
				strcpy(st.strings[id].string,text);
				checked=2;
			}
			else
			if(checked==0 && id==-1)
			{
				LogApp("Warning: max number os strings reached");
				return 2;
			}
		}
	}

	i=st.num_entities;

	if(checked==2)
	{

		co.r=255;
		co.g=255;
		co.b=255;
		co.a=255;
	
		if(strlen(text)==0) 
			msg=TTF_RenderUTF8_Blended(st.fonts[font].font," ",co);
		else
			msg=TTF_RenderUTF8_Blended(st.fonts[font].font,text,co);
	
		if(msg->format->BytesPerPixel==4)
		{
			if(msg->format->Rmask==0x000000ff) formatt=GL_RGBA;
			else formatt=GL_BGRA_EXT;
		} else
		{
			if(msg->format->Rmask==0x000000ff) formatt=GL_RGB;
			else formatt=GL_BGR_EXT;
		}

		glGenTextures(1,&ent[i].data.data);
		glBindTexture(GL_TEXTURE_2D,ent[i].data.data);
		glTexImage2D(GL_TEXTURE_2D,0,msg->format->BytesPerPixel,msg->w,msg->h,0,formatt,GL_UNSIGNED_BYTE,msg->pixels);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		ent[i].data.channel=63; //magical number only used for rendering

		ent[i].data.posx=id;

		ent[i].data.normal=0;
		ent[i].data.vb_id=-1;

		ent[i].data.w=msg->w;
		ent[i].data.h=msg->h;

		memcpy(&st.strings[id].data,&ent[i].data,sizeof(TEX_DATA));

		SDL_FreeSurface(msg);
	}

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

	ent[i].lightmapid=-1;

			
			if(override_sizex!=0)
				sizex=st.strings[id].data.w*(float)(override_sizex/1024.0f);

			if(override_sizey!=0)
				sizey=st.strings[id].data.h*(float)(override_sizey/1024.0f);

			ent[i].texrepeat[0]=0.0f;
			ent[i].texrepeat[1]=0.0f;
			ent[i].texrepeat[2]=1.0f;
			ent[i].texrepeat[3]=1.0f;

			if(r==0 && g==0 && b==0)
				r=g=b=1;

			if(z>15) z=15;
			else if(z<8) z+=8;

			z_buffer[z][z_slot[z]]=i;
			z_slot[z]++;

			if(z>z_used) z_used=z;

			//timej=GetTicks();

			ent[i].vertex[0]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[1]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[2]=z;

			ent[i].vertex[3]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y-(sizey/2))-y)*mSin(ang));
			ent[i].vertex[4]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			ent[i].vertex[5]=z;

			ent[i].vertex[6]=(float)x+(((x+(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[7]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[8]=z;

			ent[i].vertex[9]=(float)x+(((x-(sizex/2))-x)*mCos(ang) - ((y+(sizey/2))-y)*mSin(ang));
			ent[i].vertex[10]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			ent[i].vertex[11]=z;
			/*
			ang/=10;

			tmp=cos((ang*pi)/180);
			tmp=mCos(ang*10);
	*/
			ax=(float) 1/(16384/2);
			ay=(float) 1/(8192/2);

			ay*=-1.0f;

			az=(float) 1/(4096/2);

			
				ent[i].texcor[0]=0;
				ent[i].texcor[1]=0;
				ent[i].texcor[2]=1;
				ent[i].texcor[3]=0;
				ent[i].texcor[4]=1;
				ent[i].texcor[5]=1;
				ent[i].texcor[6]=0;
				ent[i].texcor[7]=1;

			for(j=0;j<12;j+=3)
			{
				ent[i].vertex[j]*=ax;
				ent[i].vertex[j]-=1;

				ent[i].vertex[j+1]*=ay;
				ent[i].vertex[j+1]+=1;
				
				ent[i].vertex[j+2]*=az;
				ent[i].vertex[j+2]-=1;
			}

			for(j=0;j<16;j+=4)
			{
				ent[i].color[j]=r;
				ent[i].color[j+1]=g;
				ent[i].color[j+2]=b;
				ent[i].color[j+3]=a;
			}

			
				texone_ids[texone_num]=i;
				texone_num++;

#endif

			st.num_entities++;

	return 0;

}

uint32 PlayMovie(const char *name)
{
	SDL_Thread *t;
	int ReturnVal, ids, ReturnVal2, ReturnVal3;
	unsigned int ms, ms1, ms2, o;
	int ms3;
	 uint32 i=0, j=0;
	char header[21];
	GLint unif;

	void *buffer;

	FMOD_RESULT y;
	FMOD_CREATESOUNDEXINFO info;
	FMOD_CHANNEL *ch;

	uint8 id;

	FMOD_BOOL p;
	
	_MGVFORMAT mgvt;
	_MGV *mgv;

	float vertex[12]={
		-1,-1,0, 1,-1,0,
		1,1,0, -1,1,0 };

	float texcoord[8]={
		0,1, 1,1,
		1,0, 0,0 };

	mgv=(_MGV*) malloc(sizeof(_MGV));

	if((mgv->file=fopen(name,"rb"))==NULL)
	{
		LogApp("Error opening MGV file %s",name);
				return 0;
	}

	rewind(mgv->file);
	fread(header,21,1,mgv->file);

	if(strcmp(header,"MGV File Version 1.0")!=NULL)
	{
		LogApp("Invalid MGV file header %s",name);
		fclose(mgv->file);
		return 0;
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

	for(o=0;o<mgv->num_frames;o++)
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

	buffer=(void*) malloc(mgvt.sound_buffer_lenght);
	fread(buffer,mgvt.sound_buffer_lenght,1,mgv->file);
	
	memset(&info,0,sizeof(info));
	info.length=mgvt.sound_buffer_lenght;
	info.cbsize=sizeof(info);

	y=FMOD_System_CreateSound(st.sound_sys.Sound_System,(const char*) buffer,FMOD_HARDWARE | FMOD_OPENMEMORY,&info,&mgv->sound);

	if(y!=FMOD_OK)
	{
		LogApp("Error while creating sound: %s",FMOD_ErrorString(y));
		return 0;
	}

	StopAllSounds();
	
	mgv->totalsize=((sizeof(_MGVFORMAT)+512)+((mgv->num_frames*sizeof(uint32))+512))+21;


	y=FMOD_System_PlaySound(st.sound_sys.Sound_System,FMOD_CHANNEL_FREE,mgv->sound,0,&ch);

	if(y!=FMOD_OK)
	{
		LogApp("Error while playing sound: %s",FMOD_ErrorString(y));
		return 0;
	}
	
	//Here's the video part!!
	i=0;

	st.PlayingVideo=1;

	glDisable(GL_BLEND);

	glBindFramebuffer(GL_FRAMEBUFFER,0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(st.renderer.Program[2]);
	glBindVertexArray(vbd.vao_id);

	unif=glGetUniformLocation(st.renderer.Program[2],"texu");
	glUniform1i(unif,0);

	unif=glGetUniformLocation(st.renderer.Program[2],"normal");
	glUniform1f(unif,0);

	glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);

	glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),vertex);
	glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),texcoord);
	glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),vbd.color);

	glActiveTexture(GL_TEXTURE0);

	while(st.PlayingVideo)
	{
		InputProcess();

		if(st.quit) break;
		
		glClear(GL_COLOR_BUFFER_BIT);
		
		ms1=GetTicks();
		
		FMOD_System_Update(st.sound_sys.Sound_System);
		
		FMOD_Channel_IsPlaying(ch,&p);

		if(!p) break;
		
		FMOD_Channel_GetPosition(ch,&ms,FMOD_TIMEUNIT_MS);

		ms2=ms-(j*(1000/mgv->fps));
		ms3=(j*(1000/mgv->fps))-ms;

		if(ms2 > 30)
		{
			free(mgv->frames[i].buffer);
			SDL_FreeRW(mgv->frames[i].rw);
			SDL_FreeSurface(mgv->frames[i].data);
			glDeleteTextures(1,&mgv->frames[i].ID);

			i=j;

			if(i>mgv->num_frames-1) break;

			rewind(mgv->file);
			fseek(mgv->file,mgv->seeker[i],SEEK_SET);

			mgv->frames[i].buffer=malloc(mgv->framesize[i]);

			if(mgv->frames[i].buffer==NULL)
			{
				LogApp("Unable to allocate memory for MGV frame %d",i);
				continue;
			}

			fread(mgv->frames[i].buffer,mgv->framesize[i],1,mgv->file);

			mgv->frames[i].rw=SDL_RWFromMem(mgv->frames[i].buffer,mgv->framesize[i]);

			mgv->frames[i].data=IMG_LoadJPG_RW(mgv->frames[i].rw);

			glGenTextures(1,&mgv->frames[i].ID);
			glBindTexture(GL_TEXTURE_2D,mgv->frames[i].ID);
			glTexImage2D(GL_TEXTURE_2D,0,3,mgv->frames[i].data->w,mgv->frames[i].data->h,0,GL_RGB,GL_UNSIGNED_BYTE,mgv->frames[i].data->pixels);

			glGenerateMipmap(GL_TEXTURE_2D);
		}
		
		if(ms3 < 30 && ms2 < 30)
		{
			if(j>0)
			{
				free(mgv->frames[i].buffer);
				SDL_FreeRW(mgv->frames[i].rw);
				SDL_FreeSurface(mgv->frames[i].data);
				glDeleteTextures(1,&mgv->frames[i].ID);

				i++;
			}

			if(i>mgv->num_frames-1) break;

			rewind(mgv->file);
			fseek(mgv->file,mgv->seeker[i],SEEK_SET);

			mgv->frames[i].buffer=malloc(mgv->framesize[i]);

			if(mgv->frames[i].buffer==NULL)
			{
				LogApp("Unable to allocate memory for MGV frame %d",i);
				continue;
			}

			fread(mgv->frames[i].buffer,mgv->framesize[i],1,mgv->file);

			mgv->frames[i].rw=SDL_RWFromMem(mgv->frames[i].buffer,mgv->framesize[i]);

			mgv->frames[i].data=IMG_LoadJPG_RW(mgv->frames[i].rw);

			glGenTextures(1,&mgv->frames[i].ID);
			glBindTexture(GL_TEXTURE_2D,mgv->frames[i].ID);
			glTexImage2D(GL_TEXTURE_2D,0,3,mgv->frames[i].data->w,mgv->frames[i].data->h,0,GL_RGB,GL_UNSIGNED_BYTE,mgv->frames[i].data->pixels);

			glGenerateMipmap(GL_TEXTURE_2D);
		}

		glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,0);

			
		j++;

		SDL_GL_SwapWindow(wn);

		ms2=GetTicks();

		ms3=ms2-ms1;

		ms3=(1000/(mgv->fps+1))-ms3;

		if(ms3<0) ms3=0;

		SDL_Delay(ms3);

		if(st.FPSYes)
			FPSCounter();
		
	}

	glEnable(GL_BLEND);

	glUseProgram(0);
	glBindVertexArray(0);

	free(mgv->framesize);
	free(mgv->frames);
	free(buffer);
	FMOD_Channel_Stop(ch);
	FMOD_Sound_Release(mgv->sound);
	st.PlayingVideo=0;

	return 1;
	
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
	_SECTOR *sectors;
	register int16 i=0;

	if((file=fopen(name,"wb"))==NULL)
	{
		LogApp("Could not save file");
				return 0;
	}

	strcpy(header,"V0.1 mGear-1");

	fwrite(header,13,1,file);

	obj=malloc(st.Current_Map.num_obj*sizeof(_MGMOBJ));
	sprites=malloc(st.Current_Map.num_sprites*sizeof(_MGMSPRITE));
	sectors=malloc(st.Current_Map.num_sector*sizeof(_SECTOR));
	lights=malloc(st.num_lights*sizeof(_MGMLIGHT));

	memcpy(obj,st.Current_Map.obj,st.Current_Map.num_obj*sizeof(_MGMOBJ));
	memcpy(sprites,st.Current_Map.sprites,st.Current_Map.num_sprites*sizeof(_MGMSPRITE));

	for(i=0;i<st.num_lights;i++)
	{
		lights[i].alpha=st.game_lightmaps[i+1].alpha;
		lights[i].ambient_color=st.game_lightmaps[i+1].ambient_color;
		lights[i].ang=st.game_lightmaps[i+1].ang;

		memcpy(&lights[i].color,&st.game_lightmaps[i+1].color,16*sizeof(ColorA16));

		lights[i].num_lights=st.game_lightmaps[i+1].num_lights;
		lights[i].obj_id=st.game_lightmaps[i+1].obj_id;
		lights[i].T_h=st.game_lightmaps[i+1].T_h;
		lights[i].T_w=st.game_lightmaps[i+1].T_w;
		lights[i].W_h=st.game_lightmaps[i+1].W_h;
		lights[i].W_w=st.game_lightmaps[i+1].W_w;
		lights[i].w_pos=st.game_lightmaps[i+1].w_pos;

		memcpy(&lights[i].falloff,st.game_lightmaps[i+1].falloff,16*sizeof(float));
		memcpy(&lights[i].spot_ang,st.game_lightmaps[i+1].spot_ang,16*sizeof(int16));
		memcpy(&lights[i].type,st.game_lightmaps[i+1].type,16*sizeof(LIGHT_TYPE));
		memcpy(&lights[i].t_pos,st.game_lightmaps[i+1].t_pos,16*sizeof(uPos16));
		memcpy(&lights[i].t_pos2,st.game_lightmaps[i+1].t_pos2,16*sizeof(uPos16));
	}

	memcpy(sectors,st.Current_Map.sector,st.Current_Map.num_sector*sizeof(_SECTOR));

	map.num_mgg=st.Current_Map.num_mgg;
	map.num_obj=st.Current_Map.num_obj;
	map.num_lights=st.num_lights;
	map.num_sprites=st.Current_Map.num_sprites;
	map.num_sector=st.Current_Map.num_sector;
	map.bck1_v=st.Current_Map.bck1_v;
	map.bck2_v=st.Current_Map.bck2_v;
	map.fr_v=st.Current_Map.fr_v;
	map.bcktex_id=st.Current_Map.bcktex_id;
	map.bcktex_mgg=st.Current_Map.bcktex_mgg;
	strcpy(map.name,st.Current_Map.name);
	memcpy(map.MGG_FILES,st.Current_Map.MGG_FILES,sizeof(st.Current_Map.MGG_FILES));
	map.amb_color=st.Current_Map.amb_color;
	map.bck3_pan=st.Current_Map.bck3_pan;
	map.bck3_size=st.Current_Map.bck3_size;

	memcpy(&map.cam_area,&st.Current_Map.cam_area,sizeof(map.cam_area));

	fwrite(&map,sizeof(_MGMFORMAT),1,file);
	fwrite(obj,sizeof(_MGMOBJ),map.num_obj,file);
	fwrite(sprites,sizeof(_MGMSPRITE),map.num_sprites,file);
	fwrite(sectors,sizeof(_SECTOR),map.num_sector,file);
	fwrite(lights,sizeof(_MGMLIGHT),map.num_lights,file);

	fseek(file,4,SEEK_CUR);

	for(i=1;i<=st.num_lights;i++)
	{
		if(st.game_lightmaps[i].alpha)
			fwrite(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4,1,file);
		else
			fwrite(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3,1,file);
	}

	fclose(file);

	free(obj);
	free(sprites);
	free(lights);
	free(sectors);

	LogApp("Map saved");

	return 1;
}

#endif

int32 LoadSpriteCFG(char *filename, int id)
{
	FILE *file;
	int value, value2, value3;
	uint16 i, id2, skip, num_frames, gameid;
	char buf[1024], str[16], str2[16], str3[16], str4[8][16], *tok;

	for(i=0;i<MAX_GAME_MGG;i++)
	{
		if(i==MAX_GAME_MGG-1 && mgg_game[i].type!=NONE)
		{
			LogApp("Cannot load MGG, reached max number of map MGGs loaded");
			return 0;
		}

		if(mgg_game[i].type==NONE)
		{
			id2=i;
			break;
		}
	}

	if((file=fopen(filename,"r"))==NULL)
	{
		LogApp("error reading sprite cfg file: %s",filename);
		return 0;
	}

	while(!feof(file))
	{
		memset(str,0,16);
		memset(buf,0,1024);
		fgets(buf,1024,file);

		sscanf(buf,"%s %s",str, str2);

		if(strcmp(str,"\0")==NULL)
			continue;

		if(strcmp(str,"NAME")==NULL)
		{
			strcpy(st.Game_Sprites[id].name,str2);
			continue;
		}
		else
		if(strcmp(str,"MGG")==NULL)
		{
			sscanf(buf,"%s %s %s",str, str2, str3);

			if(strcmp(str2,"NONE")==NULL)
			{
				st.Game_Sprites[id].MGG_ID=-1;
				continue;
			}

			if(CheckMGGFile(str2))
			{
				value=CheckMGGInSystem(str2);

				if(value>999 && value<1003)
				{
					st.Game_Sprites[id].MGG_ID=value-1000;
					id2=value-1000;
				}
				else
				if(value>9999 && value<100000)
				{
					st.Game_Sprites[id].MGG_ID=value-10000;
					id2=value-10000;
				}
				else
				if(value>99999 && value<1000000)
				{
					st.Game_Sprites[id].MGG_ID=value-100000;
					id2=value-100000;
				}
				else
				if(value==-1)
				{
					if(!LoadMGG(&mgg_game[id2],str2))
					{
						//fclose(file);
						//return 0;
						LogApp("Error: failed to load sprite's MGG");
						st.Game_Sprites[id].MGG_ID = -1;
					}

					st.num_mgg++;
					st.Game_Sprites[id].MGG_ID=id2;
				}
				else
				if(value==-2)
				{
					fclose(file);
					return 0;
				}
			}
			else
			{
				LogApp("Error loading sprite MGG: %s",str2);
				return 0;
			}

			if(strcmp(str3,"ALL")==NULL)
			{
				st.Game_Sprites[id].num_frames=mgg_game[id2].num_frames;

				skip=2;
			}
			else
			if(strcmp(str3,"PART")==NULL)
			{
				skip=1;
			}

			continue;
		}
		else
		if(strcmp(str,"NUM_FRAMES")==NULL)
		{
			num_frames=atoi(str2);

			if(skip==1)
				st.Game_Sprites[id].num_start_frames=st.Game_Sprites[id].num_frames=num_frames;
			else
			if(skip==2)
				st.Game_Sprites[id].num_start_frames=num_frames;
			else
			if(skip==3 || skip==4)
			{
				LogApp("Warning: number os frames already declared: %s",filename);
				num_frames=st.Game_Sprites[id].num_start_frames;
				continue;
			}

			skip=3;

			continue;
		}
		else
		if(strcmp(str,"FRAMES")==NULL)
		{
			if(skip==1 || skip==2)
			{
				LogApp("Error: number of start frames not declared: %s", filename);
				//FreeMGG(&mgg_game[id2]);
				//st.num_mgg--;
				return 0;
			}
			else
			if(skip==4)
			{
				LogApp("Warning: start frames already declared: %s", filename);
				continue;
			}
			else
			if(skip==3)
			{
				st.Game_Sprites[id].frame=(int32*) malloc(num_frames*sizeof(int32));
				tok=strtok(buf," ");

				for(i=0;i<num_frames;i++)
				{
					tok=strtok(NULL," ");

					if(tok==NULL) break;

					st.Game_Sprites[id].frame[i]=atoi(tok);
				}

				skip=4;

				continue;
			}
		}
		else
		if(strcmp(str,"DEFAULT")==NULL)
		{
			st.Game_Sprites[id].health=0;
			st.Game_Sprites[id].body.mass=0;
			st.Game_Sprites[id].body.flamable=0;
			st.Game_Sprites[id].body.explosive=0;
			st.Game_Sprites[id].body.material=MATERIAL_END;
			st.Game_Sprites[id].body.size.x=st.Game_Sprites[id].body.size.y=512;

			continue;
		}
		else
		if(strcmp(str,"HEALTH")==NULL)
		{
			st.Game_Sprites[id].health=atoi(str2);

			continue;
		}
		else
		if(strcmp(str,"MASS")==NULL)
		{
			st.Game_Sprites[id].body.mass=atoi(str2);

			continue;
		}
		else
		if(strcmp(str,"RESIZEABLE")==NULL)
		{
			st.Game_Sprites[id].flags+=1;
			
			continue;
		}
		else
		if(strcmp(str,"ORIGINAL_SIZE")==NULL)
		{
			st.Game_Sprites[id].flags+=4;
			
			continue;
		}
		else
		if(strcmp(str,"SIZE_MUL")==NULL)
		{
			tok=strtok(buf," ");

			tok=strtok(NULL," ");

			st.Game_Sprites[id].size_m.x=atoi(tok);

			tok=strtok(NULL," ");

			st.Game_Sprites[id].size_m.y=atoi(tok);
			
			continue;
		}
		else
		if(strcmp(str,"SIZE_ADD")==NULL)
		{
			tok=strtok(buf," ");

			tok=strtok(NULL," ");

			st.Game_Sprites[id].size_a.x=atoi(tok);

			tok=strtok(NULL," ");

			st.Game_Sprites[id].size_a.y=atoi(tok);
			
			continue;
		}
		else
		if(strcmp(str,"HIDDEN")==NULL)
		{
			st.Game_Sprites[id].flags+=2;
			
			continue;
		}
		else
		if(strcmp(str,"MATERIAL")==NULL)
		{
			if(strcmp(str2,"METAL")==NULL)
				st.Game_Sprites[id].body.material=METAL;
			else
			if(strcmp(str2,"CONCRETE")==NULL)
				st.Game_Sprites[id].body.material=CONCRETE;
			else
			if(strcmp(str2,"PLASTIC")==NULL)
				st.Game_Sprites[id].body.material=PLASTIC;
			else
			if(strcmp(str2,"ORGANIC")==NULL)
				st.Game_Sprites[id].body.material=ORGANIC;
			else
			if(strcmp(str2,"NONE")==NULL)
				st.Game_Sprites[id].body.material=MATERIAL_END;
			else
			if(strcmp(str2,"WOOD")==NULL)
				st.Game_Sprites[id].body.material=WOOD;
			else
			{
				LogApp("Error: MATERIAL declaration undefined: %s", filename);
				return 0;
			}

			continue;
		}
		else
		if(strcmp(str,"SIZE")==NULL)
		{
			sscanf(buf,"%s %d %d",str, &value, &value2);

			st.Game_Sprites[id].body.size.x=value;
			st.Game_Sprites[id].body.size.y=value2;

			continue;
		}
		else
		if(strcmp(str,"FLAMABLE")==NULL)
		{
			st.Game_Sprites[id].body.flamable=atoi(str2);

			continue;
		}
		else
		if(strcmp(str,"EXPLOSIVE")==NULL)
		{
			st.Game_Sprites[id].body.explosive=atoi(str2);

			continue;
		}
		else
		if(strcmp(str,"TYPE")==NULL)
		{
			if(strcmp(str2,"GAME_LOGICAL")==NULL)
				st.Game_Sprites[id].type=GAME_LOGICAL;
			else
			if(strcmp(str2,"ENEMY")==NULL)
				st.Game_Sprites[id].type=ENEMY;
			else
			if(strcmp(str2,"FRIEND")==NULL)
				st.Game_Sprites[id].type=FRIEND;
			else
			if(strcmp(str2,"NORMAL")==NULL)
				st.Game_Sprites[id].type=NORMAL;
			else
			{
				LogApp("Error: TYPE declaration undefined: %s", filename);
				return 0;
			}
		}
	}

	fclose(file);

	gameid=id;

	if(st.Game_Sprites[gameid].flags & 4)
	{
		if(mgg_game[st.Game_Sprites[gameid].MGG_ID].frames[st.Game_Sprites[gameid].frame[0]].vb_id!=-1)
		{
			if(st.Game_Sprites[gameid].size_m.x!=NULL && st.Game_Sprites[gameid].size_m.y!=NULL)
			{
				st.Game_Sprites[gameid].body.size.x=(mgg_game[st.Game_Sprites[gameid].MGG_ID].frames[st.Game_Sprites[gameid].frame[0]].sizex*st.Game_Sprites[gameid].size_m.x)+st.Game_Sprites[gameid].size_a.x;
				st.Game_Sprites[gameid].body.size.y=(mgg_game[st.Game_Sprites[gameid].MGG_ID].frames[st.Game_Sprites[gameid].frame[0]].sizey*st.Game_Sprites[gameid].size_m.y)+st.Game_Sprites[gameid].size_a.y;
			}
			else
			{
				st.Game_Sprites[gameid].body.size.x=mgg_game[st.Game_Sprites[gameid].MGG_ID].frames[st.Game_Sprites[gameid].frame[0]].sizex+st.Game_Sprites[gameid].size_a.x;
				st.Game_Sprites[gameid].body.size.y=mgg_game[st.Game_Sprites[gameid].MGG_ID].frames[st.Game_Sprites[gameid].frame[0]].sizey+st.Game_Sprites[gameid].size_a.y;
			}
		}
		else
		{
			if(st.Game_Sprites[gameid].size_m.x!=NULL && st.Game_Sprites[gameid].size_m.y!=NULL)
			{
				st.Game_Sprites[gameid].body.size.x=(((mgg_game[st.Game_Sprites[gameid].MGG_ID].frames[st.Game_Sprites[gameid].frame[0]].w*16384)/st.screenx)*
					st.Game_Sprites[gameid].size_m.x)+st.Game_Sprites[gameid].size_a.x;
				st.Game_Sprites[gameid].body.size.y=(((mgg_game[st.Game_Sprites[gameid].MGG_ID].frames[st.Game_Sprites[gameid].frame[0]].h*8192)/st.screeny)*
					st.Game_Sprites[gameid].size_m.y)+st.Game_Sprites[gameid].size_a.y;
			}
			else
			{
				st.Game_Sprites[gameid].body.size.x=((mgg_game[st.Game_Sprites[gameid].MGG_ID].frames[st.Game_Sprites[gameid].frame[0]].w*16384)/st.screenx)+st.Game_Sprites[gameid].size_a.x;
				st.Game_Sprites[gameid].body.size.y=((mgg_game[st.Game_Sprites[gameid].MGG_ID].frames[st.Game_Sprites[gameid].frame[0]].h*8192)/st.screeny)+st.Game_Sprites[gameid].size_a.y;
			}
		}
	}

	st.sprite_id_list[st.num_sprites] = id;

	st.num_sprites++;

	return 1;
}

int32 LoadSpriteList(char *filename)
{
	FILE *file;
	int value, value2, value3;
	int16 i, j, k, l;
	int lenght;
	char buf[1024], str[32], str2[32], str3[64], str4[8][16], *tok;

	if((file=fopen(filename,"r"))==NULL)
	{
		LogApp("error reading sprite list file: %s",filename);
		return 0;
	}

	while(!feof(file))
	{
		memset(str,0,16);
		memset(buf,0,1024);
		memset(str2,0,32);
		memset(str3,0,64);
		memset(str4,0,8*16);

		fgets(buf,1024,file);

		if(buf[0]=='/0')
			continue;

		if(buf[0]=='/' && buf[1]=='/')
			continue;

		sscanf(buf,"%s %s %d %s %d %s %s %s %s %s %s %s %s",str, str2, &value, str3, &value2, str4[0], str4[1], str4[2], str4[3], str4[4], str4[5], str4[6], str4[7]);

		if(strcmp(str,"\0")==NULL)
			continue;

		if(strcmp(str,"SPRITE")==NULL)
		{
			strcpy(st.Game_Sprites[value].name,str2);

			if(strcmp(str3,"NONE")!=NULL)
			{
				if(!LoadSpriteCFG(str3,value))
				{
					LogApp("failed loading sprite cfg: %s",str3);

					//st.num_sprites++;

					continue;
				}
			}

			if(value2>0)
			{
				st.Game_Sprites[value].num_tags=value2;

				for(i=0;i<value2;i++)
					strcpy(st.Game_Sprites[value].tag_names[i], str4[i]);
			}

			continue;
		}
		else
		if(strcmp(str,"SPRITE_S_TAG")==NULL)
		{
			tok=strtok(buf,", \"");
			tok=strtok(NULL,", \"");

			j=atoi(tok);

			tok=strtok(NULL,", \"");
			strcpy(str3,tok);

			tok=strtok(NULL,", \"");

			k=atoi(tok);

			for(i=0;i<k;i++)
			{
				tok=strtok(NULL,", \"");
				strcpy(str4[i],tok);
			}

			for(i=0;i<st.Game_Sprites[j].num_tags;i++)
			{
				if(strcmp(st.Game_Sprites[j].tag_names[i],str3)==NULL)
				{
					st.Game_Sprites[j].num_ext=k;

					for(l=0;l<k;l++)
						strcpy(st.Game_Sprites[j].tags_ext[l],str4[l]);
				}
			}
		}
		else
		if(strcmp(str,"SPRITE_TAG_LIST")==NULL)
		{
			tok=strtok(buf," ");
			tok=strtok(NULL," ");

			j=atoi(tok);

			tok=strtok(NULL," ");
			strcpy(str3,tok);

			tok=strtok(NULL," ");

			k=atoi(tok);

			for(i=0;i<k;i++)
			{
				tok=strtok(NULL," ");
				strcpy(str4[i],tok);
			}

			for(i=0;i<st.Game_Sprites[j].num_tags;i++)
			{
				if(strcmp(st.Game_Sprites[j].tag_names[i],str3)==NULL)
				{
					st.Game_Sprites[j].num_list_tag=k;

					for(l=0;l<k;l++)
						strcpy(st.Game_Sprites[j].tag_mod_list[l],str4[l]);
				}
			}
		}
	}

	fclose(file);

	memset(str,0,16);
	memset(buf,0,1024);
	memset(str2,0,32);
	memset(str3,0,64);
	memset(str4,0,8*16);

	return 1;
}

void LockCamera()
{
	if(st.Current_Map.cam_area.horiz_lim)
	{
		if((float) st.Camera.position.x+(16384/st.Camera.dimension.x)>st.Current_Map.cam_area.limit[1].x)
			st.Camera.position.x=(float) st.Current_Map.cam_area.limit[1].x-(16384/st.Camera.dimension.x);

		if(st.Camera.position.x<st.Current_Map.cam_area.limit[0].x)
			st.Camera.position.x=st.Current_Map.cam_area.limit[0].x;
	}

	if(st.Current_Map.cam_area.vert_lim)
	{
		if((float) st.Camera.position.y+(8192/st.Camera.dimension.y)>st.Current_Map.cam_area.limit[1].y)
			st.Camera.position.y=(float) st.Current_Map.cam_area.limit[1].y-(8192/st.Camera.dimension.y);

		if(st.Camera.position.y<st.Current_Map.cam_area.limit[0].y)
			st.Camera.position.y=st.Current_Map.cam_area.limit[0].y;
	}

	if(st.Camera.dimension.x>st.Current_Map.cam_area.max_dim.x)
		st.Camera.dimension.x=st.Current_Map.cam_area.max_dim.x;

	if(st.Camera.dimension.y>st.Current_Map.cam_area.max_dim.y)
		st.Camera.dimension.y=st.Current_Map.cam_area.max_dim.y;
}

uint32 LoadMap(const char *name)
{
	FILE *file;
	char header[13];
	_MGMFORMAT map;
	_MGMLIGHT *lights;
	register int16 i=0;

	if((file=fopen(name,"rb"))==NULL)
	{
		LogApp("Could not open file %s",name);
				return 0;
	}

	//Tries to read 13 byte header
	fread(header,13,1,file);

	//Checks if it's the same version

	if(strcmp(header,"V0.1 mGear-1")!=NULL)
	{
		LogApp("Invalid map format or version: %s", header);
				return 0;
	}

					if(st.Current_Map.obj)
						free(st.Current_Map.obj);

					if(st.Current_Map.sprites)
						free(st.Current_Map.sprites);

					if(st.Current_Map.sector)
						free(st.Current_Map.sector);

	//loads the map

	if(st.Current_Map.num_mgg>0)
	{
		for(i=0;i<st.Current_Map.num_mgg;i++)
			FreeMGG(&mgg_map[i]);
	}

	memset(st.Current_Map.MGG_FILES,0,32*256);

	if(st.num_lights>0)
	{
		for(i=1;i<=st.num_lights;i++)
		{
			free(st.game_lightmaps[i].data);
			st.game_lightmaps[i].obj_id=-1;
			st.game_lightmaps[i].stat=0;
			glDeleteTextures(1,&st.game_lightmaps[i].tex);
		}
	}

	fread(&map,sizeof(_MGMFORMAT),1,file);

	st.Current_Map.num_mgg=map.num_mgg;
	st.Current_Map.num_obj=map.num_obj;
	st.num_lights=st.Current_Map.num_lights=map.num_lights;
	st.Current_Map.num_sprites=map.num_sprites;
	st.Current_Map.num_sector=map.num_sector;
	strcpy(st.Current_Map.name,map.name);
	st.Current_Map.bck1_v=map.bck1_v;
	st.Current_Map.bck2_v=map.bck2_v;
	st.Current_Map.fr_v=map.fr_v;
	st.Current_Map.bcktex_id=map.bcktex_id;
	st.Current_Map.bcktex_mgg=map.bcktex_mgg;
	st.Current_Map.amb_color=map.amb_color;
	st.Current_Map.bck3_pan=map.bck3_pan;
	st.Current_Map.bck3_size=map.bck3_size;
	memcpy(&st.Current_Map.MGG_FILES,&map.MGG_FILES,sizeof(map.MGG_FILES));
	memcpy(&st.Current_Map.cam_area,&map.cam_area,sizeof(map.cam_area));

#ifdef ENGINEER
	st.Current_Map.obj=(_MGMOBJ*) malloc(MAX_OBJS*sizeof(_MGMOBJ));
	st.Current_Map.sprites=(_MGMSPRITE*) malloc(MAX_SPRITES*sizeof(_MGMSPRITE));
	lights=malloc(MAX_LIGHTMAPS*sizeof(_MGMLIGHT));
	st.Current_Map.sector=malloc(MAX_SECTORS*sizeof(_SECTOR));
#else
	st.Current_Map.obj=(_MGMOBJ*) malloc(st.Current_Map.num_obj*sizeof(_MGMOBJ));
	st.Current_Map.sprites=(_MGMSPRITE*) malloc(MAX_SPRITES*sizeof(_MGMSPRITE));
	lights=(_MGMLIGHT*) malloc(st.num_lights*sizeof(_MGMLIGHT));
	st.Current_Map.sector=(_SECTOR*) malloc(st.Current_Map.num_sector*sizeof(_SECTOR));
#endif

	for(i=0;i<MAX_SECTORS;i++)
	{
		st.Current_Map.sector[i].id=-1;
		//st.Current_Map.sector[i].layers=1;
		st.Current_Map.sector[i].material=CONCRETE;
		st.Current_Map.sector[i].tag=0;
	}

	for(i=0;i<MAX_OBJS;i++)
	{
		st.Current_Map.obj[i].type=BLANK;
		st.Current_Map.obj[i].lightmapid=-1;
	}

	for(i=0;i<MAX_SPRITES;i++)
		st.Current_Map.sprites[i].stat=0;

	fread(st.Current_Map.obj,sizeof(_MGMOBJ),st.Current_Map.num_obj,file);
	fread(st.Current_Map.sprites,sizeof(_MGMSPRITE),st.Current_Map.num_sprites,file);
	fread(st.Current_Map.sector,sizeof(_SECTOR),st.Current_Map.num_sector,file);
	fread(lights,sizeof(_MGMLIGHT),st.Current_Map.num_lights,file);
	

	for(i=0;i<st.Current_Map.num_lights;i++)
	{
		st.game_lightmaps[i+1].alpha=lights[i].alpha;
		st.game_lightmaps[i+1].ambient_color=lights[i].ambient_color;
		st.game_lightmaps[i+1].ang=lights[i].ang;

		memcpy(&st.game_lightmaps[i+1].color,&lights[i].color,16*sizeof(ColorA16));

		st.game_lightmaps[i+1].num_lights=lights[i].num_lights;
		st.game_lightmaps[i+1].obj_id=lights[i].obj_id;
		st.game_lightmaps[i+1].T_h=lights[i].T_h;
		st.game_lightmaps[i+1].T_w=lights[i].T_w;
		st.game_lightmaps[i+1].W_h=lights[i].W_h;
		st.game_lightmaps[i+1].W_w=lights[i].W_w;
		st.game_lightmaps[i+1].w_pos=lights[i].w_pos;

		memcpy(&st.game_lightmaps[i+1].falloff,lights[i].falloff,16*sizeof(float));
		memcpy(&st.game_lightmaps[i+1].spot_ang,lights[i].spot_ang,16*sizeof(int16));
		memcpy(&st.game_lightmaps[i+1].type,lights[i].type,16*sizeof(LIGHT_TYPE));
		memcpy(&st.game_lightmaps[i+1].t_pos,lights[i].t_pos,16*sizeof(uPos16));
		memcpy(&st.game_lightmaps[i+1].t_pos2,lights[i].t_pos2,16*sizeof(uPos16));

		st.game_lightmaps[i+1].stat=1;

		if(st.game_lightmaps[i+1].alpha)
			st.game_lightmaps[i+1].data=malloc(st.game_lightmaps[i+1].T_w*st.game_lightmaps[i+1].T_h*4);
		else
			st.game_lightmaps[i+1].data=malloc(st.game_lightmaps[i+1].T_w*st.game_lightmaps[i+1].T_h*3);
	}

	fseek(file,4,SEEK_CUR);

	for(i=1;i<=st.Current_Map.num_lights;i++)
	{
		if(st.game_lightmaps[i].alpha)
		{
			fread(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4,1,file);
			st.game_lightmaps[i].tex=GenerateAlphaLightTexture(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
		}
		else
		{
			fread(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3,1,file);
			st.game_lightmaps[i].tex=GenerateLightmapTexture(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
		}
	}

	LogApp("Map %s loaded",name);

	fclose(file);

	free(lights);

	return 1;
}

void FreeMap()
{
	uint8 i, j;

	if(st.Current_Map.num_mgg>0)
	{
		free(st.Current_Map.obj);
		free(st.Current_Map.sprites);
		free(st.Current_Map.sector);

		for(i=0;i<st.Current_Map.num_lights;i++)
		{
			free(st.game_lightmaps[i].data);
			st.game_lightmaps[i].obj_id=-1;
			st.game_lightmaps[i].stat=0;
		}

		for(i=0;i<st.Current_Map.num_mgg;i++)
			FreeMGG(&mgg_map[i]);

		memset(&st.Current_Map,0,sizeof(_MGM));
		st.Current_Map.bcktex_id=-1;
	}
}

void DrawMap()
{
	
	//Draw the objects first

	float x, y, sizex, sizey, ang, size;
	uint32 i;
	int16 curf, max_f;
	int32 x_f;
	Pos tmp;
	
	if(st.Current_Map.bcktex_id>-1)
		DrawGraphic(8192,4096,16384,8192,0,255,255,255,mgg_map[st.Current_Map.bcktex_mgg].frames[st.Current_Map.bcktex_id],255,st.Current_Map.bck3_pan.x,st.Current_Map.bck3_pan.y,
		st.Current_Map.bck3_size.x,st.Current_Map.bck3_size.y,55,0);

			for(i=0;i<st.Current_Map.num_obj;i++)
			{

				if((st.viewmode & 1 && st.Current_Map.obj[i].position.z<24) || (st.viewmode & 2 && st.Current_Map.obj[i].position.z>23 && st.Current_Map.obj[i].position.z<32)
					 || (st.viewmode & 4 && st.Current_Map.obj[i].position.z>31 && st.Current_Map.obj[i].position.z<40) || (st.viewmode & 8 && st.Current_Map.obj[i].position.z>39 && st.Current_Map.obj[i].position.z<48)
					  || (st.viewmode & 16 && st.Current_Map.obj[i].position.z>47 && st.Current_Map.obj[i].position.z<57))
				{
					if(st.Current_Map.obj[i].flag & 2)
					{
						max_f=mgg_map[st.Current_Map.obj[i].tex.MGG_ID].anim[0].num_frames;
						curf=mgg_map[st.Current_Map.obj[i].tex.MGG_ID].anim[0].startID;

						st.Current_Map.obj[i].current_frame=curf+(st.Camera.position.x/512);

						if(st.Current_Map.obj[i].current_frame>max_f-1)
							st.Current_Map.obj[i].current_frame=max_f-1;
						else
						if(st.Current_Map.obj[i].current_frame<0)
							st.Current_Map.obj[i].current_frame=0;

						DrawGraphic(st.Current_Map.obj[i].position.x,st.Current_Map.obj[i].position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,
							st.Current_Map.obj[i].angle,st.Current_Map.obj[i].color.r,st.Current_Map.obj[i].color.g,st.Current_Map.obj[i].color.b,
							mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].current_frame],st.Current_Map.obj[i].color.a,st.Current_Map.obj[i].texpan.x,st.Current_Map.obj[i].texpan.y,
							st.Current_Map.obj[i].texsize.x,st.Current_Map.obj[i].texsize.y,st.Current_Map.obj[i].position.z,st.Current_Map.obj[i].flag);

					}
					else
						DrawGraphic(st.Current_Map.obj[i].position.x,st.Current_Map.obj[i].position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,
							st.Current_Map.obj[i].angle,st.Current_Map.obj[i].color.r,st.Current_Map.obj[i].color.g,st.Current_Map.obj[i].color.b,
							mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID],st.Current_Map.obj[i].color.a,st.Current_Map.obj[i].texpan.x,st.Current_Map.obj[i].texpan.y,
							st.Current_Map.obj[i].texsize.x,st.Current_Map.obj[i].texsize.y,st.Current_Map.obj[i].position.z,st.Current_Map.obj[i].flag);
				}
			}

			for(i=0;i<st.Current_Map.num_sprites;i++)
			{
				if((st.viewmode & 1 && st.Current_Map.sprites[i].position.z<24) || (st.viewmode & 2 && st.Current_Map.sprites[i].position.z>23 && st.Current_Map.sprites[i].position.z<32)
					 || (st.viewmode & 4 && st.Current_Map.sprites[i].position.z>31 && st.Current_Map.sprites[i].position.z<40)
					 || (st.viewmode & 8 && st.Current_Map.sprites[i].position.z>39 && st.Current_Map.sprites[i].position.z<48)
					 || (st.viewmode & 16 && st.Current_Map.sprites[i].position.z>47 && st.Current_Map.sprites[i].position.z<57))
				{
					if((st.viewmode & 32 && (~st.Current_Map.sprites[i].flags & 2)) || ((~st.viewmode & 32) && st.Current_Map.sprites[i].flags & 2) || ((~st.viewmode & 32) && (~st.Current_Map.sprites[i].flags & 2)))
						DrawSprite(st.Current_Map.sprites[i].position.x,st.Current_Map.sprites[i].position.y,st.Current_Map.sprites[i].body.size.x,st.Current_Map.sprites[i].body.size.y,st.Current_Map.sprites[i].angle,
							st.Current_Map.sprites[i].color.r,st.Current_Map.sprites[i].color.g,st.Current_Map.sprites[i].color.b,
							mgg_game[st.Current_Map.sprites[i].MGG_ID].frames[st.Current_Map.sprites[i].frame_ID],
							st.Current_Map.sprites[i].color.a,st.Current_Map.sprites[i].position.z,st.Current_Map.sprites[i].flags,
							st.Current_Map.sprites[i].size_a.x,st.Current_Map.sprites[i].size_a.y,st.Current_Map.sprites[i].size_m.x,st.Current_Map.sprites[i].size_m.y);
				}
			}

			for(i=1;i<=st.num_lights;i++)
				if(st.viewmode & LIGHT_VIEW || st.viewmode & INGAME_VIEW)
					DrawLightmap(st.game_lightmaps[i].w_pos.x,st.game_lightmaps[i].w_pos.y,st.game_lightmaps[i].w_pos.z,st.game_lightmaps[i].W_w,st.game_lightmaps[i].W_h,st.game_lightmaps[i].tex,0,st.game_lightmaps[i].ang);

	
	/*
	for( uint16 i=0;i<st.Current_Map.num_obj;i++)
		if(st.Current_Map.obj[i].type==MIDGROUND)
				DrawGraphic(st.Current_Map.obj[i].position.x,st.Current_Map.obj[i].position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,
				st.Current_Map.obj[i].angle,st.Current_Map.obj[i].color.r*st.Current_Map.obj[i].amblight,st.Current_Map.obj[i].color.g*st.Current_Map.obj[i].amblight,st.Current_Map.obj[i].color.b*st.Current_Map.obj[i].amblight,mgg[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID],st.Current_Map.obj[i].color.a,st.Current_Map.obj[i].texsize.x,st.Current_Map.obj[i].texsize.y,st.Current_Map.obj[i].texpan.x,st.Current_Map.obj[i].texpan.y);

	//for( uint16 i=0;i<st.Current_Map.num_lights;i++)
		//	DrawLight(st.Current_Map.light[i].position.x-st.Camera.position.x,st.Current_Map.light[i].position.y-st.Camera.position.y,st.Current_Map.light[i].size.x,st.Current_Map.light[i].size.y,
			//st.Current_Map.light[i].angle,st.Current_Map.light[i].color.r,st.Current_Map.light[i].color.g,st.Current_Map.light[i].color.b,st.MapTex[st.Current_Map.light[i].TextureID].ID,st.Current_Map.light[i].color.a);
	for( uint16 i=0;i<st.Current_Map.num_obj;i++)
		if(st.Current_Map.obj[i].type==BACKGROUND3)
				DrawGraphic(st.Current_Map.obj[i].position.x-st.Camera.position.x,st.Current_Map.obj[i].position.y-st.Camera.position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,
					st.Current_Map.obj[i].angle,st.Current_Map.obj[i].color.r,st.Current_Map.obj[i].color.g,st.Current_Map.obj[i].color.b,mgg[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID],st.Current_Map.obj[i].color.a,st.Current_Map.obj[i].texsize.x,st.Current_Map.obj[i].texsize.y,st.Current_Map.obj[i].texpan.x,st.Current_Map.obj[i].texpan.y);
	for( uint16 i=0;i<st.Current_Map.num_obj;i++)
		if(st.Current_Map.obj[i].type==BACKGROUND2)
				DrawGraphic(st.Current_Map.obj[i].position.x-st.Camera.position.x,st.Current_Map.obj[i].position.y-st.Camera.position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,
					st.Current_Map.obj[i].angle,st.Current_Map.obj[i].color.r,st.Current_Map.obj[i].color.g,st.Current_Map.obj[i].color.b,mgg[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID],st.Current_Map.obj[i].color.a,st.Current_Map.obj[i].texsize.x,st.Current_Map.obj[i].texsize.y,st.Current_Map.obj[i].texpan.x,st.Current_Map.obj[i].texpan.y);
	for( uint16 i=0;i<st.Current_Map.num_obj;i++)
		if(st.Current_Map.obj[i].type==BACKGROUND1)
				DrawGraphic(st.Current_Map.obj[i].position.x-st.Camera.position.x,st.Current_Map.obj[i].position.y-st.Camera.position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,
					st.Current_Map.obj[i].angle,st.Current_Map.obj[i].color.r,st.Current_Map.obj[i].color.g,st.Current_Map.obj[i].color.b,mgg[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID],st.Current_Map.obj[i].color.a,st.Current_Map.obj[i].texsize.x,st.Current_Map.obj[i].texsize.y,st.Current_Map.obj[i].texpan.x,st.Current_Map.obj[i].texpan.y);
	*/
	if(st.Developer_Mode && (~st.viewmode & 32))
	{
		for(i=0;i<st.Current_Map.num_sector;i++)
		{
			if(st.Current_Map.sector[i].id>-1 && st.Current_Map.sector[i].num_vertexadded==1)
			{
				if(st.keys[LSHIFT_KEY].state)
				{
					tmp=st.mouse;
					STW(&tmp.x,&tmp.y);
				}
				else
				{
					tmp.x=st.mouse.x;
					STW(&tmp.x,&tmp.y);

					tmp.y=st.Current_Map.sector[i].vertex[0].y;
				}

				DrawLine(st.Current_Map.sector[i].vertex[0].x,st.Current_Map.sector[i].vertex[0].y,tmp.x,tmp.y,255,0,0,255,64,16);
				DrawGraphic(st.Current_Map.sector[i].vertex[0].x,st.Current_Map.sector[i].vertex[0].y,256,256,0,255,0,0,mgg_sys[0].frames[4],255,0,0,32768,32768,16,2);
				DrawGraphic(tmp.x,tmp.y,256,256,0,255,0,0,mgg_sys[0].frames[4],255,0,0,32768,32768,16,2);
			}
			else
			if(st.Current_Map.sector[i].id>-1 && st.Current_Map.sector[i].num_vertexadded==2)
			{
				DrawLine(st.Current_Map.sector[i].vertex[0].x,st.Current_Map.sector[i].vertex[0].y,st.Current_Map.sector[i].vertex[1].x,st.Current_Map.sector[i].vertex[1].y,255,0,0,255,64,16);
				DrawGraphic(st.Current_Map.sector[i].vertex[0].x,st.Current_Map.sector[i].vertex[0].y,256,256,0,255,0,0,mgg_sys[0].frames[4],255,0,0,32768,32768,16,2);
				DrawGraphic(st.Current_Map.sector[i].vertex[1].x,st.Current_Map.sector[i].vertex[1].y,256,256,0,255,0,0,mgg_sys[0].frames[4],255,0,0,32768,32768,16,2);
			}
		}
	}


	
}

void DrawSys()
{
	register uint16 i=0;

	for(i=0;i<st.num_calls;i++)
	{
		switch(st.renderer.ppline[i].type)
		{	
			case GRAPHICS_CALL: DrawGraphic(st.renderer.ppline[i].pos.x,st.renderer.ppline[i].pos.y,st.renderer.ppline[i].size.x,st.renderer.ppline[i].size.y,st.renderer.ppline[i].ang,st.renderer.ppline[i].color.r,
									st.renderer.ppline[i].color.g,st.renderer.ppline[i].color.b,st.renderer.ppline[i].data,st.renderer.ppline[i].color.a,st.renderer.ppline[i].tex_panx,st.renderer.ppline[i].tex_pany,
									st.renderer.ppline[i].tex_sizex,st.renderer.ppline[i].tex_sizey,st.renderer.ppline[i].pos.z,0); break;

			case HUD_CALL: DrawHud(st.renderer.ppline[i].pos.x,st.renderer.ppline[i].pos.y,st.renderer.ppline[i].size.x,st.renderer.ppline[i].size.y,st.renderer.ppline[i].ang,st.renderer.ppline[i].color.r,
							  st.renderer.ppline[i].color.g,st.renderer.ppline[i].color.b,st.renderer.ppline[i].tex_panx,st.renderer.ppline[i].tex_pany,st.renderer.ppline[i].tex_sizex,
							  st.renderer.ppline[i].tex_sizey,st.renderer.ppline[i].data,st.renderer.ppline[i].color.a,st.renderer.ppline[i].pos.z); break;

			case UI_CALL: DrawUI(st.renderer.ppline[i].pos.x,st.renderer.ppline[i].pos.y,st.renderer.ppline[i].size.x,st.renderer.ppline[i].size.y,st.renderer.ppline[i].ang,st.renderer.ppline[i].color.r,
							  st.renderer.ppline[i].color.g,st.renderer.ppline[i].color.b,st.renderer.ppline[i].tex_panx,st.renderer.ppline[i].tex_pany,st.renderer.ppline[i].tex_sizex,
							  st.renderer.ppline[i].tex_sizey,st.renderer.ppline[i].data,st.renderer.ppline[i].color.a,st.renderer.ppline[i].pos.z); break;

			case STRING_CALL: DrawString(st.renderer.ppline[i].text,st.renderer.ppline[i].pos.x,st.renderer.ppline[i].pos.y,st.renderer.ppline[i].size.x,st.renderer.ppline[i].size.y,st.renderer.ppline[i].ang,
								  st.renderer.ppline[i].color.r,st.renderer.ppline[i].color.g,st.renderer.ppline[i].color.b,st.renderer.ppline[i].color.a,st.renderer.ppline[i].font,st.renderer.ppline[i].size2.x,
								  st.renderer.ppline[i].size2.y,st.renderer.ppline[i].pos.z); break;

			case STRING2_CALL: DrawString2(st.renderer.ppline[i].text,st.renderer.ppline[i].pos.x,st.renderer.ppline[i].pos.y,st.renderer.ppline[i].size.x,st.renderer.ppline[i].size.y,st.renderer.ppline[i].ang,
								  st.renderer.ppline[i].color.r,st.renderer.ppline[i].color.g,st.renderer.ppline[i].color.b,st.renderer.ppline[i].color.a,st.renderer.ppline[i].font,st.renderer.ppline[i].size2.x,
								  st.renderer.ppline[i].size2.y,st.renderer.ppline[i].pos.z); break;

			case STRINGUI_CALL: DrawStringUI(st.renderer.ppline[i].text,st.renderer.ppline[i].pos.x,st.renderer.ppline[i].pos.y,st.renderer.ppline[i].size.x,st.renderer.ppline[i].size.y,st.renderer.ppline[i].ang,
								  st.renderer.ppline[i].color.r,st.renderer.ppline[i].color.g,st.renderer.ppline[i].color.b,st.renderer.ppline[i].color.a,st.renderer.ppline[i].font,st.renderer.ppline[i].size2.x,
								  st.renderer.ppline[i].size2.y,st.renderer.ppline[i].pos.z); break;

			case STRINGUI2_CALL: DrawString2UI(st.renderer.ppline[i].text,st.renderer.ppline[i].pos.x,st.renderer.ppline[i].pos.y,st.renderer.ppline[i].size.x,st.renderer.ppline[i].size.y,st.renderer.ppline[i].ang,
								  st.renderer.ppline[i].color.r,st.renderer.ppline[i].color.g,st.renderer.ppline[i].color.b,st.renderer.ppline[i].color.a,st.renderer.ppline[i].font,st.renderer.ppline[i].size2.x,
								  st.renderer.ppline[i].size2.y,st.renderer.ppline[i].pos.z); break;

			case LINE_CALL: DrawLine(st.renderer.ppline[i].pos.x,st.renderer.ppline[i].pos.y,st.renderer.ppline[i].pos2.x,st.renderer.ppline[i].pos2.y,st.renderer.ppline[i].color.r,st.renderer.ppline[i].color.g,
								st.renderer.ppline[i].color.b,st.renderer.ppline[i].color.a,st.renderer.ppline[i].size.x,st.renderer.ppline[i].pos.z); break;

			case STRINGUIV_CALL: DrawStringUIv(st.renderer.ppline[i].text,st.renderer.ppline[i].pos.x,st.renderer.ppline[i].pos.y,st.renderer.ppline[i].size.x,st.renderer.ppline[i].size.y,st.renderer.ppline[i].ang,
								  st.renderer.ppline[i].color.r,st.renderer.ppline[i].color.g,st.renderer.ppline[i].color.b,st.renderer.ppline[i].color.a,st.renderer.ppline[i].font,st.renderer.ppline[i].size2.x,
								  st.renderer.ppline[i].size2.y,st.renderer.ppline[i].pos.z); break;
		}
	}

	//Finish();
}

void Renderer(uint8 type)
{

	GLint unif, texcat, vertat, pos, col, texc, tex_bound[2]= { -1, -1 };
	GLenum error;

	uint32 num_targets=0;
	 int32 i=0, j=0, m=0, timej, timel, n=0;
	uint16 *k, l=0;

	static uint32 tesg=0;

	int8 mrt=0;

	float m1, m2;

	float vertex[12]={
		-1,-1,0, 1,-1,0,
		1,1,0, -1,1,0 };

	float texcoord[8]={
		0,0, 1,0,
		1,1, 0,1 };

	float tile[2];

	GLuint fbo, fbr, txo=0, txr, cat[1]={ GL_COLOR_ATTACHMENT0 };

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

//if(tesg==0)
//{
	for(i=0;i<vbdt_num;i++)
	{
		if(vbdt[i].num_elements>0)
		{
			vbdt[i].vertex=(float*) calloc(vbdt[i].num_elements*12,sizeof(float));
			vbdt[i].texcoord=(float*) calloc(vbdt[i].num_elements*8,sizeof(float));
			vbdt[i].index=(GLushort*) calloc(vbdt[i].num_elements*6,sizeof(GLushort));
			vbdt[i].color=(GLubyte*) calloc(vbdt[i].num_elements*16,sizeof(GLubyte));
			vbdt[i].texcoordlight=(float*) calloc(vbdt[i].num_elements*8,sizeof(float));
			//vbdt[i].texrepeat=(float*) calloc(vbdt[i].num_elements*4,sizeof(float));
		}
	}

	k=(uint16*) calloc(vbdt_num,sizeof(uint16));

	for(m=z_used;m>-1;m--)
	{
		if(!z_slot[m]) continue;
		else
		for(n=0;n<z_slot[m];n++)
		{
			i=z_buffer[m][n];
			if(ent[i].data.vb_id!=-1)
			{
				for(j=0;j<16;j++)
				{
					//if(j<4)
						//vbdt[ent[i].data.vb_id].texrepeat[(k[ent[i].data.vb_id])+j]=ent[i].texrepeat[j];

					vbdt[ent[i].data.vb_id].color[(k[ent[i].data.vb_id]*16)+j]=ent[i].color[j];

					if(j<12)
						vbdt[ent[i].data.vb_id].vertex[(k[ent[i].data.vb_id]*12)+j]=ent[i].vertex[j];

					if(j<8)
					{
						vbdt[ent[i].data.vb_id].texcoord[(k[ent[i].data.vb_id]*8)+j]=ent[i].texcor[j];

						vbdt[ent[i].data.vb_id].texcoordlight[(k[ent[i].data.vb_id]*8)+j]=ent[i].texcorlight[j];
					}
					
					if(j<=2)
						vbdt[ent[i].data.vb_id].index[(k[ent[i].data.vb_id]*6)+j]=((k[ent[i].data.vb_id]*6)-(k[ent[i].data.vb_id]*2))+j;

					if(j==3 || j==4)
						vbdt[ent[i].data.vb_id].index[(k[ent[i].data.vb_id]*6)+j]=((k[ent[i].data.vb_id]*6)-(k[ent[i].data.vb_id]*2))+(j-1);

					if(j==5)
						vbdt[ent[i].data.vb_id].index[(k[ent[i].data.vb_id]*6)+j]=((k[ent[i].data.vb_id]*6)-(k[ent[i].data.vb_id]*2));
				}

				ent[i].data.loc=k[ent[i].data.vb_id];

				k[ent[i].data.vb_id]++;
			}
			else
				ent[i].data.loc=-1;
		}
	}

	free(k);

#ifdef _VAO_RENDER

	if(st.renderer.VAO_ON)
	{
		for(i=0;i<vbdt_num;i++)
		{
			if(vbdt[i].num_elements>0)
			{
				if(vbdt[i].num_elements<vbdt[i].buffer_elements)
				{
					if(vbdt[i].num_elements!=vbdt[i].num_elements2)
					{
						UpdateVAO(&vbdt[i],0,1,3);
						vbdt[i].num_elements2=vbdt[i].num_elements;
					}
					else
						UpdateVAO(&vbdt[i],0,0,3);
				}
				else
				if(vbdt[i].num_elements<vbdt[i].buffer_elements-8)
				{
					vbdt[i].buffer_elements=vbdt[i].num_elements+8;
					UpdateVAO(&vbdt[i],1,1,3);
				}
				else
				if(vbdt[i].num_elements>=vbdt[i].buffer_elements)
				{
					vbdt[i].buffer_elements=vbdt[i].num_elements+8;
					UpdateVAO(&vbdt[i],1,1,3);
				}
			}
		}
	}

#endif
		/*
		else
		if(vbdt[i].num_elements==0 && vbdt[i].buffer_elements>8)
		{
			vbdt[i].buffer_elements=8;
			UpdateVAO(&vbdt[i],2,0,2);
		}
		*/

#endif

	if(type)
		st.num_lightmap=1; //Make sure BASICBKD() is being used

	num_targets=st.num_entities;

#ifdef _VAO_RENDER
	if(st.renderer.VAO_ON)
	{
		glUseProgram(st.renderer.Program[3]);

		glUniform1i(st.renderer.unifs[1],0);

		glUniform1i(st.renderer.unifs[2],2);

		glUniform1i(st.renderer.unifs[3],1);

		glUniform1f(st.renderer.unifs[4],0);

		glBindFramebuffer(GL_FRAMEBUFFER,st.renderer.FBO[0]);

		glDrawBuffers(1,&st.renderer.Buffers[0]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);

		glBindVertexArray(vbd.vao_id);

		glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);

		if(z_used>15)
		{
			for(i=0;i<st.num_lightmap;i++)
			{
				glBindTexture(GL_TEXTURE_2D,lmp[i].data.data);

				glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),lmp[i].vertex);
				glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),vbd.texcoord);
				glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),lmp[i].color);

				glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,0);
			}
		}

		glActiveTexture(GL_TEXTURE2);

		glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[0]);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glBindFramebuffer(GL_FRAMEBUFFER,0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for(i=z_used;i>-1;i--)
		{
			for(j=0;j<z_slot[i];j++)
			{
				if(ent[z_buffer[i][j]].data.vb_id!=-1)
				{
					m=ent[z_buffer[i][j]].data.vb_id;

					glActiveTexture(GL_TEXTURE0);

					if(tex_bound[0]!=vbdt[m].texture)
					{
						glBindTexture(GL_TEXTURE_2D,vbdt[m].texture);
						tex_bound[0]=vbdt[m].texture;
					}

					//glUniform2f(st.renderer.unifs[6],(float) ent[z_buffer[i][j]].data.sizex/32768,(float) ent[z_buffer[i][j]].data.sizey/32768);
					//glUniform2f(st.renderer.unifs[7],(float) ent[z_buffer[i][j]].data.posx/32768,(float) ent[z_buffer[i][j]].data.posy/32768);

					if(i<16  || ent[z_buffer[i][j]].lightmapid==-2)
						glUniform1f(st.renderer.unifs[4],3);
					else
					{
						glActiveTexture(GL_TEXTURE1);

						if(vbdt[m].normal)
						{
							if(tex_bound[1]!=vbdt[m].texture)
							{
								glBindTexture(GL_TEXTURE_2D,vbdt[m].Ntexture);
								tex_bound[1]=vbdt[m].texture;
							}
							glUniform1f(st.renderer.unifs[4],1);
						}
						else
						{
							//if(tex_bound[1]!=vbdt[m].texture)
							//{
							//	glBindTexture(GL_TEXTURE_2D,vbdt[m].texture);
								//tex_bound[1]=vbdt[m].texture;
							//}

							glUniform1f(st.renderer.unifs[4],2);
						}

					}

					glBindVertexArray(vbdt[m].vao_id);

					l=0;
					if(j<z_slot[i]-2)
					{
						for(m=j+1;m<z_slot[i];m++)
						{
							if(ent[z_buffer[i][m]].data.vb_id==ent[z_buffer[i][j]].data.vb_id && ent[z_buffer[i][m]].lightmapid==ent[z_buffer[i][j]].lightmapid)
								l++;
							else
								break;
						}
					}

					if(!l)
						glDrawRangeElements(GL_TRIANGLES,0,(ent[z_buffer[i][j]].data.loc*6)+6,6,GL_UNSIGNED_SHORT,(void*) ((ent[z_buffer[i][j]].data.loc*6)*2));
					else
						glDrawRangeElements(GL_TRIANGLES,0,((ent[z_buffer[i][j]].data.loc+l)*6)+6,(l*6)+6,GL_UNSIGNED_SHORT,(void*) ((ent[z_buffer[i][j]].data.loc*6)*2));

					glBindVertexArray(0);

					if(l)
						j+=l;
				}
				else
				{
					glActiveTexture(GL_TEXTURE0);

					if(tex_bound[0]!=ent[z_buffer[i][j]].data.data)
					{
						glBindTexture(GL_TEXTURE_2D,ent[z_buffer[i][j]].data.data);
						tex_bound[0]=ent[z_buffer[i][j]].data.data;
					}

					//glUniform2f(st.renderer.unifs[6],(float) ent[z_buffer[i][j]].data.sizex/32768,(float) ent[z_buffer[i][j]].data.sizey/32768);
					//glUniform2f(st.renderer.unifs[7],(float) ent[z_buffer[i][j]].data.posx/32768,(float) ent[z_buffer[i][j]].data.posy/32768);

					if(i<16 || ent[z_buffer[i][j]].lightmapid==-2)
						glUniform1f(st.renderer.unifs[4],3);
					else
					{
						glActiveTexture(GL_TEXTURE1);

						if(ent[z_buffer[i][j]].data.normal)
						{
							glUniform1f(st.renderer.unifs[4],1);

							if(tex_bound[1]!=ent[z_buffer[i][j]].data.data)
							{
								glBindTexture(GL_TEXTURE_2D,ent[z_buffer[i][j]].data.Ndata);
								tex_bound[1]=ent[z_buffer[i][j]].data.data;
							}
						}
						else
						{
							if(tex_bound[1]!=ent[z_buffer[i][j]].data.data)
							{
								glBindTexture(GL_TEXTURE_2D,ent[z_buffer[i][j]].data.data);
								tex_bound[1]=ent[z_buffer[i][j]].data.data;
							}

							glUniform1f(st.renderer.unifs[4],2);
						}
					}

					glBindVertexArray(vbd.vao_id);

					glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);

					glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),ent[z_buffer[i][j]].vertex);
					glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),ent[z_buffer[i][j]].texcor);
					glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),ent[z_buffer[i][j]].color);
					glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),8*sizeof(float),ent[z_buffer[i][j]].texcorlight);
					//glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte))+(8*sizeof(GLfloat)),4*sizeof(float),ent[z_buffer[i][j]].texrepeat);

					glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,0);

					glBindVertexArray(0);
				}
			}

			if(i==0) break;
		}

		memset(z_buffer,0,((7*8)+1)*(2048)*sizeof(int16));
		memset(z_slot,0,((7*8)+1)*sizeof(int16));
		z_used=0;

		for(i=0;i<MAX_STRINGS;i++)
		{
			if(st.strings[i].stat==1)
			{
				st.strings[i].stat=2;
				continue;
			}
			else
			if(st.strings[i].stat==2)
			{
				glDeleteTextures(1,&st.strings[i].data.data);
				st.strings[i].data.channel=0;
				st.strings[i].stat=0;
			}
		}
		
		for(i=0;i<vbdt_num;i++)
		{
			if(vbdt[i].num_elements>0)
			{
				vbdt[i].num_elements=0;
			}
		}
		
	}
	
#endif
	
#ifdef _VBO_RENDER
	if(st.renderer.VBO_ON)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,st.renderer.FBO[0]);

		if(st.num_lightmap>0)
		{
			glDrawBuffers(1,&st.renderer.Buffers[2]);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for(i=0;i<st.num_lightmap;i++)
			{
				glUseProgram(st.renderer.Program[2]);
				unif=glGetUniformLocation(st.renderer.Program[2],"texu");
				glUniform1i(unif,0);

				unif=glGetUniformLocation(st.renderer.Program[2],"normal");
				glUniform1f(unif,0);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D,lmp[i].data.data);

				glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbd.ibo_id);

				glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),lmp[i].vertex);
				glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),texcoord);
				glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),vbd.color);

				glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*sizeof(GLushort)),vbd.index,GL_STREAM_DRAW);
				
				pos=glGetAttribLocation(st.renderer.Program[2],"Position");
				texc=glGetAttribLocation(st.renderer.Program[2],"TexCoord");
				col=glGetAttribLocation(st.renderer.Program[2],"Color");

				glEnableVertexAttribArray(pos);
				glEnableVertexAttribArray(texc);
				glEnableVertexAttribArray(col);

				glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);
				glVertexAttribPointer(texc,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) (12*sizeof(GLfloat)));
				glVertexAttribPointer(col,4,GL_UNSIGNED_BYTE,GL_TRUE,0,(GLvoid*) ((12*sizeof(GLfloat))+(8*sizeof(GLfloat))));
				
				glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,0);
				
				glDisableVertexAttribArray(pos);
				glDisableVertexAttribArray(texc);
				glDisableVertexAttribArray(col);
				
				glBindBuffer(GL_ARRAY_BUFFER,0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

				glUseProgram(0);
			}
		}

		//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		glBindFramebuffer(GL_FRAMEBUFFER,0);
		//glDrawBuffers(1,&st.renderer.Buffers[1]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[2]);

		glGenerateMipmap(GL_TEXTURE_2D);

		glUseProgram(st.renderer.Program[3]);

		unif=glGetUniformLocation(st.renderer.Program[3],"texu");
		glUniform1i(unif,0);

		unif=glGetUniformLocation(st.renderer.Program[3],"texu2");
		glUniform1i(unif,2);

		unif=glGetUniformLocation(st.renderer.Program[3],"texu3");
		glUniform1i(unif,1);


		for(i=z_used;i>-1;i--)
		{
			for(j=0;j<z_slot[i];j++)
			{
				if(ent[z_buffer[i][j]].data.vb_id!=-1)
				{
					m=ent[z_buffer[i][j]].data.vb_id;

					glActiveTexture(GL_TEXTURE0);

					if(tex_bound[0]!=vbdt[m].texture)
					{
						glBindTexture(GL_TEXTURE_2D,vbdt[m].texture);
						tex_bound[0]=vbdt[m].texture;
					}

					if(i<16)
						glUniform1i(st.renderer.unifs[4],3);
					else
					{
						glActiveTexture(GL_TEXTURE1);

						if(vbdt[i].normal)
						{
							if(tex_bound[1]!=vbdt[m].texture)
							{
								glBindTexture(GL_TEXTURE_2D,vbdt[m].Ntexture);
								tex_bound[1]=vbdt[m].texture;
							}
							unif=glGetUniformLocation(st.renderer.Program[3],"normal");
							glUniform1f(unif,1);
						}
						else
						{
							if(tex_bound[1]!=vbdt[m].texture)
							{
								glBindTexture(GL_TEXTURE_2D,vbdt[m].texture);
								tex_bound[1]=vbdt[m].texture;
							}

							unif=glGetUniformLocation(st.renderer.Program[3],"normal");
							glUniform1f(unif,2);
						}
					}

					//glBindVertexArray(vbdt[m].vao_id);
					glBindBuffer(GL_ARRAY_BUFFER,vbdt[m].vbo_id);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbdt[m].ibo_id);

					glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
					glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),vbdt[m].vertex);
					glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),vbdt[m].texcoord);
					glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),vbdt[m].color);

					glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*sizeof(GLushort)),vbdt[m].index,GL_STREAM_DRAW);

					pos=glGetAttribLocation(st.renderer.Program[3],"Position");
					texc=glGetAttribLocation(st.renderer.Program[3],"TexCoord");
					col=glGetAttribLocation(st.renderer.Program[3],"Color");

					glEnableVertexAttribArray(pos);
					glEnableVertexAttribArray(texc);
					glEnableVertexAttribArray(col);

					glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);
					glVertexAttribPointer(texc,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) (12*sizeof(GLfloat)));
					glVertexAttribPointer(col,4,GL_UNSIGNED_BYTE,GL_TRUE,0,(GLvoid*) ((12*sizeof(GLfloat))+(8*sizeof(GLfloat))));

					l=0;
					if(j<z_slot[i]-2)
					{
						for(m=j+1;m<z_slot[i];m++)
						{
							if(ent[z_buffer[i][m]].data.vb_id==ent[z_buffer[i][j]].data.vb_id)
								l++;
							else
								break;
						}
					}

					if(!l)
						glDrawRangeElements(GL_TRIANGLES,(ent[z_buffer[i][j]].data.loc*6),(ent[z_buffer[i][j]].data.loc*6)+6,(ent[z_buffer[i][j]].data.loc*6)+6,GL_UNSIGNED_SHORT,0);
					else
						glDrawRangeElements(GL_TRIANGLES,(ent[z_buffer[i][j]].data.loc*6),((ent[z_buffer[i][j]].data.loc+l)*6)+6,((ent[z_buffer[i][j]].data.loc+l)*6)+6,GL_UNSIGNED_SHORT,0);

					glDisableVertexAttribArray(pos);
					glDisableVertexAttribArray(texc);
					glDisableVertexAttribArray(col);

					glBindBuffer(GL_ARRAY_BUFFER,0);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

					if(l)
						j+=l;
				}
				else
				{
					glActiveTexture(GL_TEXTURE0);

					if(tex_bound[0]!=ent[z_buffer[i][j]].data.data)
					{
						glBindTexture(GL_TEXTURE_2D,ent[z_buffer[i][j]].data.data);
						tex_bound[0]=ent[z_buffer[i][j]].data.data;
					}

					if(i<16)
						glUniform1i(st.renderer.unifs[4],3);
					else
					{
						glActiveTexture(GL_TEXTURE1);

						if(ent[z_buffer[i][j]].data.normal)
						{
							unif=glGetUniformLocation(st.renderer.Program[3],"normal");
							glUniform1f(unif,1);

							if(tex_bound[1]!=ent[z_buffer[i][j]].data.data)
							{
								glBindTexture(GL_TEXTURE_2D,ent[z_buffer[i][j]].data.Ndata);
								tex_bound[1]=ent[z_buffer[i][j]].data.data;
							}
						}
						else
						{
							if(tex_bound[1]!=ent[z_buffer[i][j]].data.data)
							{
								glBindTexture(GL_TEXTURE_2D,ent[z_buffer[i][j]].data.data);
								tex_bound[1]=ent[z_buffer[i][j]].data.data;
							}

							unif=glGetUniformLocation(st.renderer.Program[3],"normal");
							glUniform1f(unif,2);
						}
					}

					//glBindVertexArray(vbd.vao_id);

					glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbd.ibo_id);

					glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
					glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),ent[z_buffer[i][j]].vertex);
					glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),ent[z_buffer[i][j]].texcor);
					glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),ent[z_buffer[i][j]].color);

					glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*sizeof(GLushort)),vbd.index,GL_STREAM_DRAW);

					pos=glGetAttribLocation(st.renderer.Program[3],"Position");
					texc=glGetAttribLocation(st.renderer.Program[3],"TexCoord");
					col=glGetAttribLocation(st.renderer.Program[3],"Color");

					glEnableVertexAttribArray(pos);
					glEnableVertexAttribArray(texc);
					glEnableVertexAttribArray(col);

					glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);
					glVertexAttribPointer(texc,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) (12*sizeof(GLfloat)));
					glVertexAttribPointer(col,4,GL_UNSIGNED_BYTE,GL_TRUE,0,(GLvoid*) ((12*sizeof(GLfloat))+(8*sizeof(GLfloat))));

					glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,0);
					
					glDisableVertexAttribArray(pos);
					glDisableVertexAttribArray(texc);
					glDisableVertexAttribArray(col);

					glBindBuffer(GL_ARRAY_BUFFER,0);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
				}
			}

			if(i==0) break;
		}

		//glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[2]);

		//glGenerateMipmap(GL_TEXTURE_2D);

		memset(z_buffer,0,(5*8)*(2048));
		memset(z_slot,0,5*8);
		z_used=0;

		for(i=0;i<MAX_STRINGS;i++)
		{
			if(st.strings[i].stat==1)
			{
				st.strings[i].stat=2;
				continue;
			}
			else
			if(st.strings[i].stat==2)
			{
				glDeleteTextures(1,&st.strings[i].data.data);
				st.strings[i].data.channel=0;
				st.strings[i].stat=0;
			}
		}
		
		for(i=0;i<vbdt_num;i++)
		{
			if(vbdt[i].num_elements>0)
			{
				vbdt[i].num_elements=0;
			}
		}
		
	}
#endif

#ifdef _VA_RENDER

	if(st.renderer.VA_ON)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		//glEnable(GL_ALPHA_TEST);
		//glAlphaFunc(GL_EQUAL,1.0);

		for(i=z_used;i>-1;i--)
		{
			for(j=0;j<z_slot[i];j++)
			{
				if(ent[z_buffer[i][j]].data.vb_id!=-1)
				{
					m=ent[z_buffer[i][j]].data.vb_id;

					//glActiveTexture(GL_TEXTURE0);

					if(tex_bound[0]!=vbdt[m].texture)
					{
						glBindTexture(GL_TEXTURE_2D,vbdt[m].texture);
						tex_bound[0]=vbdt[m].texture;
					}

					//glBindTexture(GL_TEXTURE_2D,vbdt[i].texture);

					glEnableClientState(GL_VERTEX_ARRAY);
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glEnableClientState(GL_COLOR_ARRAY);

					glVertexPointer(3,GL_FLOAT,0,vbdt[m].vertex);
					glTexCoordPointer(2,GL_FLOAT,0,vbdt[m].texcoord);
					glColorPointer(4,GL_UNSIGNED_BYTE,0,vbdt[m].color);

					//glDrawRangeElements(GL_TRIANGLES,0,vbdt[i].num_elements*6,vbdt[i].num_elements*6,GL_UNSIGNED_SHORT,vbdt[i].index);

					l=0;
					if(j<z_slot[i]-2)
					{
						for(m=j+1;m<z_slot[i];m++)
						{
							if(ent[z_buffer[i][m]].data.vb_id==ent[z_buffer[i][j]].data.vb_id)
								l++;
							else
								break;
						}
					}

					if(!l)
						glDrawRangeElements(GL_TRIANGLES,(ent[z_buffer[i][j]].data.loc*6),(ent[z_buffer[i][j]].data.loc*6)+6,(ent[z_buffer[i][j]].data.loc*6)+6,GL_UNSIGNED_SHORT,vbdt[m].index);
					else
						glDrawRangeElements(GL_TRIANGLES,(ent[z_buffer[i][j]].data.loc*6),((ent[z_buffer[i][j]].data.loc+l)*6)+6,((ent[z_buffer[i][j]].data.loc+l)*6)+6,GL_UNSIGNED_SHORT,vbdt[m].index);

					//glBindVertexArray(0);

					if(l)
						j+=l;

					glDisableClientState(GL_VERTEX_ARRAY);
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
					glDisableClientState(GL_COLOR_ARRAY);
				}
				else
				{
					//glActiveTexture(GL_TEXTURE0);

					if(tex_bound[0]!=ent[z_buffer[i][j]].data.data)
					{
						glBindTexture(GL_TEXTURE_2D,ent[z_buffer[i][j]].data.data);
						tex_bound[0]=ent[z_buffer[i][j]].data.data;
					}

					glEnableClientState(GL_VERTEX_ARRAY);
					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
					glEnableClientState(GL_COLOR_ARRAY);

					glVertexPointer(3,GL_FLOAT,0,ent[z_buffer[i][j]].vertex);
					glTexCoordPointer(2,GL_FLOAT,0,ent[z_buffer[i][j]].texcor);
					glColorPointer(4,GL_UNSIGNED_BYTE,0,ent[z_buffer[i][j]].color);

					glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,vbd.index);

					glDisableClientState(GL_VERTEX_ARRAY);
					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
					glDisableClientState(GL_COLOR_ARRAY);
				}
			}
		}

		glDisable(GL_DEPTH_TEST);
		
		glBlendFunc(GL_DST_COLOR, GL_ZERO);

		if(st.num_lightmap>0)
		{
			for(i=0;i<st.num_lightmap;i++)
			{
				glBindTexture(GL_TEXTURE_2D,lmp[i].data.data);

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);

				glVertexPointer(3,GL_FLOAT,0,lmp[i].vertex);
				glTexCoordPointer(2,GL_FLOAT,0,texcoord);
				glColorPointer(4,GL_UNSIGNED_BYTE,0,vbd.color);

				glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,vbd.index);

				glDisableClientState(GL_VERTEX_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_COLOR_ARRAY);
			}
		}

		glEnable(GL_DEPTH_TEST);

		memset(z_buffer,0,(5*8)*(2048));
		memset(z_slot,0,5*8);
		z_used=0;

		for(i=0;i<MAX_STRINGS;i++)
		{
			if(st.strings[i].stat==1)
			{
				st.strings[i].stat=2;
				continue;
			}
			else
			if(st.strings[i].stat==2)
			{
				glDeleteTextures(1,&st.strings[i].data.data);
				st.strings[i].data.channel=0;
				st.strings[i].stat=0;
			}
		}
		
		for(i=0;i<vbdt_num;i++)
		{
			if(vbdt[i].num_elements>0)
			{
				vbdt[i].num_elements=0;
			}
		}
	}

#endif

	st.num_entities=0;

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)
	texone_num=0;
#endif

	st.num_lightmap=0;
	memset(&ent,0,MAX_GRAPHICS);
	memset(&lmp,0,MAX_LIGHTMAPS);

	SDL_GL_SwapWindow(wn);

	
}

void Finish()
{
	register uint16 i=0;

	//for(i=0;i<st.num_calls;i++)
		//if(st.renderer.ppline[i].type>3)
			//free(st.renderer.ppline[i].text);

	st.num_calls=0;

	memset(st.renderer.ppline,MAX_DRAWCALLS,sizeof(PIPELINE));
}

int8 LoadSoundList(char *name)
{
	FILE *file, *f;
	int16 i, j, value, tmp;
	char buf[1024], *tok, str[256];

	if((file=fopen(name,"r"))==NULL)
	{
		LogApp("Error reading sound list file");
		return 0;
	}

	while(!feof(file))
	{
		fgets(buf,1024,file);

		tok=strtok(buf," \"");

		if(strcmp(tok,"SOUND")==NULL)
		{
			tok=strtok(NULL," \"");

			value=atoi(tok);

			tok=strtok(NULL," \"");

			strcpy(str,tok);
			strcpy(st.sounds[value].path,tok);

			if ((f = fopen(tok, "rb")) == NULL)
				LogApp("Error opening audio file %s", tok);

			fclose(f);

			tok=strtok(NULL," \"");

			if(strcmp(tok,"NOLOOP")==NULL)
				st.sounds[value].loop=0;

			if(strcmp(tok,"LOOP")==NULL)
				st.sounds[value].loop=1;

			if(st.sounds[value].loop)
				FMOD_System_CreateSound(st.sound_sys.Sound_System,str,FMOD_SOFTWARE | FMOD_LOOP_NORMAL,0,&st.sounds[value].sound);
			else
				FMOD_System_CreateSound(st.sound_sys.Sound_System,str,FMOD_SOFTWARE | FMOD_LOOP_OFF,0,&st.sounds[value].sound);

			tok=strtok(NULL," \"");

			if(strcmp(tok,"AMBIENT")==NULL)
			{
				st.sounds[value].type=0;
				st.sounds[value].priority=151;
			}

			if(strcmp(tok,"FX")==NULL)
			{
				st.sounds[value].type=1;
				st.sounds[value].priority=201;
			}

			if(strcmp(tok,"PLAYER")==NULL)
			{
				st.sounds[value].type=2;
				st.sounds[value].priority=1;
			}

			if(strcmp(tok,"NPC")==NULL)
			{
				st.sounds[value].type=3;
				st.sounds[value].priority=101;
			}

			if(strcmp(tok,"TALK")==NULL)
			{
				st.sounds[value].type=4;
				st.sounds[value].priority=51;
			}

			tok=strtok(NULL," \"");

			tmp=atoi(tok);

			if(tmp>49)
				tmp=49;

			st.sounds[value].priority+=tmp;

			st.num_sounds++;
		}
		else
		if(strcmp(tok,"MUSIC")==NULL)
		{
			tok=strtok(NULL," \"");

			value=atoi(tok);

			tok=strtok(NULL," \"");

			strcpy(str,tok);
			strcpy(st.musics[value].path,tok);

			if ((f = fopen(tok, "rb")) == NULL)
				LogApp("Error opening audio file %s", tok);

			if(strcmp(tok,"NOLOOP")==NULL)
				st.musics[value].loop=0;

			if(strcmp(tok,"LOOP")==NULL)
				st.musics[value].loop=1;

			if(st.sounds[value].loop)
				FMOD_System_CreateSound(st.sound_sys.Sound_System,str,FMOD_SOFTWARE | FMOD_LOOP_NORMAL,0,&st.musics[value].sound);
			else
				FMOD_System_CreateSound(st.sound_sys.Sound_System,str,FMOD_SOFTWARE | FMOD_LOOP_OFF,0,&st.musics[value].sound);

			st.musics[value].type=5;
			st.musics[value].priority=0;

			st.num_musics++;
		}
	}

}

void PlayMusic(int16 id, uint8 loop)
{
	FMOD_CHANNEL *ch;

	FMOD_System_PlaySound(st.sound_sys.Sound_System,FMOD_CHANNEL_FREE,st.musics[id].sound,0,&st.sound_sys.music);

	FMOD_Channel_SetPriority(st.sound_sys.music,0);
}

void PlaySound(int16 id, uint8 loop)
{
	FMOD_CHANNEL *ch;

	FMOD_System_PlaySound(st.sound_sys.Sound_System,FMOD_CHANNEL_FREE,st.sounds[id].sound,0,&ch);

	FMOD_Channel_SetPriority(ch,st.sounds[id].priority);
}

void MainSound()
{
	FMOD_System_Update(st.sound_sys.Sound_System);
}

void StopAllSounds()
{
	uint16 i;

	int ch;

	FMOD_CHANNEL *chn;

	FMOD_System_GetChannelsPlaying(st.sound_sys.Sound_System,&ch);

	for(i=0;i<ch;i++)
	{
		FMOD_System_GetChannel(st.sound_sys.Sound_System,i,&chn);

		if(chn)
			FMOD_Channel_Stop(chn);
	}
}

void StopMusic()
{
	FMOD_Channel_Stop(st.sound_sys.music);
}
