#include "engine.h"
#include "quicklz.h"
#include "input.h"
#include <math.h>
#include <SDL_image.h>

#ifdef _VBO_RENDER
	#include "shader110.c"
#endif

#ifdef _VAO_RENDER
	#include "shader130.c"
#endif

_SETTINGS st;
_ENTITIES ent[MAX_GRAPHICS];
SDL_Event events;
_LIGHTS game_lights[MAX_LIGHTS];
_ENTITIES lmp[MAX_LIGHTMAPS];
unsigned char *DataN;
GLuint DataNT;

//Foreground + Background + Midground
//Each layer (z position) has 8 sub-layers (slots)
//The allocation is done during initialization
//z_slots keeps track of the current number of sub-layers
//DO NOT ALTER Z_BUFFER WITHOUT ALTERING Z_SLOTS

int16 z_buffer[3*8][2048];
int16 z_slot[3*8];
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
		if(st.FPS<1000)
			st.FPS=1000/st.FPS;
		sprintf(st.WINDOW_NAME,"%s fps: %.2f",WindowTitle,st.FPS);
		st.FPS=0;
		st.FPSTime=SDL_GetTicks();
		SDL_SetWindowTitle(wn,st.WINDOW_NAME);
	}
}

void _fastcall STW(int32 *x, int32 *y)
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

void _fastcall WTS(int32 *x, int32 *y)
{
	*x-=st.Camera.position.x;
	*y-=st.Camera.position.y;

	*x=((*x*st.screenx)/16384)*st.Camera.dimension.x;
	*y=((*y*st.screeny)/8192)*st.Camera.dimension.y;
}

float mCos(int16 ang)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	return st.CosTable[ang];
}

float mSin(int16 ang)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	return st.SinTable[ang];
}

float mTan(int16 ang)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	return st.TanTable[ang];
}


//Signed
void CalCos16(int16 ang, int16 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	*val*=(int16)(st.CosTable[ang]*100);
	*val/=100;
}

void CalSin16(int16 ang, int16 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	*val*=(int16)(st.SinTable[ang]*100);
	*val/=100;
}

void CalTan16(int16 ang, int16 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	*val*=(int16)(st.TanTable[ang]*100);
	*val/=100;
}

void CalCos32(int16 ang, int32 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	*val*=(int32)(st.CosTable[ang]*100);
	*val/=100;
}

void CalSin32(int16 ang, int32 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	*val*=(int32)(st.SinTable[ang]*100);
	*val/=100;
}

void CalTan32(int16 ang, int32 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	*val*=(int32)(st.TanTable[ang]*100);
	*val/=100;
}


//Unsigned
void CalCos16(int16 ang, uint16 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	*val*=(uint16)(st.CosTable[ang]*100);
	*val/=100;
}

void CalSin16(int16 ang, uint16 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	*val*=(uint16)(st.SinTable[ang]*100);
	*val/=100;
}

void CalTan16(int16 ang, uint16 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	*val*=(uint16)(st.TanTable[ang]*100);
	*val/=100;
}

void CalCos32(int16 ang, uint32 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	*val*=(uint32)(st.CosTable[ang]*100);
	*val/=100;
}

void CalSin32(int16 ang, uint32 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	*val*=(uint32)(st.SinTable[ang]*100);
	*val/=100;
}

void CalTan32(int16 ang, uint32 *val)
{
	if(ang>3600) ang-=3600;
	else if(ang<0) ang*=-1;

	*val*=(uint32)(st.TanTable[ang]*100);
	*val/=100;
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

unsigned char *GenerateLightmap(uint16 w, uint16 h)
{
	unsigned char *data;

	data=(unsigned char*) calloc(w*h*3,sizeof(unsigned char));

	if(!data)
		return NULL;

	return data;
}

uint8 AddLightToLightmap(unsigned char *data, uint16 w, uint16 h, uint8 r, uint8 g, uint8 b, float falloff, uint16 x, uint16 y, uint16 z, float intensity)
{
	register uint16 i, j;
	double d, att;
	uint16 col;

	if(!data)
		return NULL;
	
	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			d=((x-j)*(x-j)) + ((y-i)*(y-i)) + (z*z);
			d=sqrt14(d);

			if(d==0)
				d=1;
			att=1.0f/(d*falloff);

			col=r*att*intensity;
			if(col>255)
				col=255;

			if((data[(i*h*3)+(j*3)]+col)>255)
				data[(i*h*3)+(j*3)]=255;
			else
				data[(i*h*3)+(j*3)]+=(unsigned char)col;

			col=g*att*intensity;
			if(col>255)
				col=255;

			if((data[(i*h*3)+(j*3)+1]+col)>255)
				data[(i*h*3)+(j*3)+1]=255;
			else
				data[(i*h*3)+(j*3)+1]+=(unsigned char)col;

			col=b*att*intensity;
			if(col>255)
				col=255;

			if((data[(i*h*3)+(j*3)+2]+col)>255)
				data[(i*h*3)+(j*3)+2]=255;
			else
				data[(i*h*3)+(j*3)+2]+=(unsigned char)col;
		}
	}

	return 1;
}

GLuint GenerateLightmapTexture(unsigned char* data, uint16 w, uint16 h)
{
	GLuint tex;

	if(!data)
		return NULL;

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
		return NULL;

	glBindTexture(GL_TEXTURE_2D,*tex);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,w,h,0,GL_BGR,GL_UNSIGNED_BYTE,data);

	glGenerateMipmap(GL_TEXTURE_2D);

	return 1;
}

#ifdef _VAO_RENDER
static void CreateVAO(VB_DATAT *data, uint8 type, uint8 pr)
{
	GLint pos, texc, col;

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

	glEnableVertexAttribArray(pos);
	glEnableVertexAttribArray(texc);
	glEnableVertexAttribArray(col);

	glGenBuffers(1,&data->vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER,data->vbo_id);

	glBufferData(GL_ARRAY_BUFFER,(((data->buffer_elements*12)*sizeof(GLfloat)))+(((data->buffer_elements*8)*sizeof(GLfloat)))+(((data->buffer_elements*16)*sizeof(GLubyte))),NULL,GL_STREAM_DRAW);
	/*
	glBufferSubData(GL_ARRAY_BUFFER,0,(12*data->num_elements)*sizeof(GLfloat),data->vertex);
	glBufferSubData(GL_ARRAY_BUFFER,(12*data->num_elements)*sizeof(GLfloat),(8*data->num_elements)*sizeof(GLfloat),data->texcoord);
	glBufferSubData(GL_ARRAY_BUFFER,(12*data->num_elements)*sizeof(GLfloat)+(8*data->num_elements)*sizeof(GLfloat),(((data->num_elements*16)*sizeof(GLubyte))),data->color);
	*/

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);
	glVertexAttribPointer(texc,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) ((12*data->buffer_elements)*sizeof(GLfloat)));
	glVertexAttribPointer(col,4,GL_UNSIGNED_BYTE,GL_TRUE,0,(GLvoid*) ((12*data->buffer_elements)*sizeof(GLfloat)+(8*data->buffer_elements)*sizeof(GLfloat)));

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
	GLint pos, texc, col;
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
			
		}
	}
	else
	if(upd_buff==1)
	{
		
		glBindVertexArray(data->vao_id);

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

		glBindVertexArray(0);

		glDisableVertexAttribArray(pos);
		glDisableVertexAttribArray(texc);
		glDisableVertexAttribArray(col);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
		//glBindBuffer(GL_ARRAY_BUFFER,0);

		free(data->texcoord);
		free(data->index);
		free(data->color);
		free(data->vertex);
	}
	else
	if(upd_buff==2)
	{
		glBindVertexArray(data->vao_id);
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
static void CreateVBO(VB_DATAT *data)
{
	glGenBuffers(1,&data->vbo_id);
	glGenBuffers(1,&data->ibo_id);

	glBindBuffer(GL_ARRAY_BUFFER,data->vbo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,data->ibo_id);

	glBufferData(GL_ARRAY_BUFFER,(((data->num_elements*12)*sizeof(GLfloat)))+(((data->num_elements*8)*sizeof(GLfloat)))+(((data->num_elements*16)*sizeof(GLubyte))),NULL,GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER,0,(12*data->num_elements)*sizeof(GLfloat),data->vertex);
	glBufferSubData(GL_ARRAY_BUFFER,(12*data->num_elements)*sizeof(GLfloat),(8*data->num_elements)*sizeof(GLfloat),data->texcoord);
	glBufferSubData(GL_ARRAY_BUFFER,(12*data->num_elements)*sizeof(GLfloat)+(8*data->num_elements)*sizeof(GLfloat),(((data->num_elements*16)*sizeof(GLubyte))),data->color);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*data->num_elements)*sizeof(GLushort),data->index,GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
	/*
	free(data->texcoord);
	free(data->index);
	free(data->color);
	free(data->vertex);
	*/
}
#endif
/*
static int VBBuffProcess(void *data)
{
	register uint16 i=0;
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
	register uint16 i=0, j=0, l=0;
	register float k=0;
	
	CreateLog();

	int check;
	FMOD_RESULT result;

	GLenum checkfb;

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

#if defined (_VAO_RENDER) || defined (_VBO_RENDER)
	GLint statusCM[32], statusLK[32];
	GLchar logs[32][1024];

	vbd.vertex=(float*) malloc(12*sizeof(float));
	vbd.texcoord=(float*) malloc(8*sizeof(float));
#endif
	vbd.color=(GLubyte*) malloc(16*sizeof(GLubyte));
	vbd.index=(GLushort*) malloc(6*sizeof(GLushort));
	
#if defined (_VAO_RENDER) || defined (_VBO_RENDER)

	vbd.vertex[0]=-0.90;
	vbd.vertex[1]=-0.90;
	vbd.vertex[2]=0;
	vbd.vertex[3]=-1;
	vbd.vertex[4]=-0.90;
	vbd.vertex[5]=0;
	vbd.vertex[6]=-1;
	vbd.vertex[7]=-1;
	vbd.vertex[8]=0;
	vbd.vertex[9]=-0.90;
	vbd.vertex[10]=-1;
	vbd.vertex[11]=0;

	vbd.texcoord[0]=1;
	vbd.texcoord[1]=1;
	vbd.texcoord[2]=0;
	vbd.texcoord[3]=1;
	vbd.texcoord[4]=0;
	vbd.texcoord[5]=0;
	vbd.texcoord[6]=1;
	vbd.texcoord[7]=0;

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

	memset(z_buffer,0,((3*8)*2048)*sizeof(int16));
	memset(z_slot,0,(3*8)*sizeof(int16));

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
		if(statusCM[0] && statusCM[1] && statusCM[2] && statusCM[3] && statusCM[4] && statusCM[5] && statusCM[6] && statusCM[7])
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
#ifdef _VAO_RENDER
			CreateVAO(&vbd,1,4);
#elif _VBO_RENDER
			CreateVBO(&vbd);
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

	memset(&ent,0,MAX_GRAPHICS*sizeof(_ENTITIES));
	memset(&lmp,0,MAX_LIGHTMAPS*sizeof(_ENTITIES));

	//Calculates Cos, Sin and Tan tables
	for(k=0.0;k<360.0;k+=0.1)
	{
		i=k*10;
		st.CosTable[i]=cos((k*pi)/180);
		st.SinTable[i]=sin((k*pi)/180);
		st.TanTable[i]=tan((k*pi)/180);
	}

	st.game_lightmaps[0].W_w=16384;
	st.game_lightmaps[0].W_h=8192;

	st.game_lightmaps[0].T_w=256;
	st.game_lightmaps[0].T_h=256;

	st.game_lightmaps[0].num_lights=1;
	st.game_lightmaps[0].w_pos.x=8192;
	st.game_lightmaps[0].w_pos.y=4096;
	st.game_lightmaps[0].w_pos.z=0;

	st.game_lightmaps[0].t_pos[0].x=128;
	st.game_lightmaps[0].t_pos[0].y=128;
	st.game_lightmaps[0].t_pos[0].z=0;

	st.game_lightmaps[0].t_pos[1].x=0;
	st.game_lightmaps[0].t_pos[1].y=0;
	st.game_lightmaps[0].t_pos[1].z=0;

	st.game_lightmaps[0].t_pos[2].x=200;
	st.game_lightmaps[0].t_pos[2].y=95;
	st.game_lightmaps[0].t_pos[2].z=0;

	st.game_lightmaps[0].data=GenerateLightmap(st.game_lightmaps[0].T_w, st.game_lightmaps[0].T_h);
	AddLightToLightmap(st.game_lightmaps[0].data,st.game_lightmaps[0].T_w,st.game_lightmaps[0].T_h,255,255,255,16,st.game_lightmaps[0].t_pos[0].x,st.game_lightmaps[0].t_pos[0].y,st.game_lightmaps[0].t_pos[0].z,255);
	//AddLightToLightmap(st.game_lightmaps[0].data,st.game_lightmaps[0].T_w,st.game_lightmaps[0].T_h,255,255,255,16,st.game_lightmaps[0].t_pos[1].x,st.game_lightmaps[0].t_pos[1].y,st.game_lightmaps[0].t_pos[1].z,128);
	//AddLightToLightmap(st.game_lightmaps[0].data,st.game_lightmaps[0].T_w,st.game_lightmaps[0].T_h,255,255,255,16,st.game_lightmaps[0].t_pos[2].x,st.game_lightmaps[0].t_pos[2].y,st.game_lightmaps[0].t_pos[0].z,255);

	st.game_lightmaps[0].tex=GenerateLightmapTexture(st.game_lightmaps[0].data,st.game_lightmaps[0].T_w,st.game_lightmaps[0].T_h);

	DataN=(unsigned char*) calloc(64*64*3,sizeof(unsigned char));

	for(i=0;i<64;i++)
	{
		for(j=0;j<64;j++)
		{
			DataN[(i*64*3)+(j*3)]=128;
			DataN[(i*64*3)+(j*3)+1]=128;
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

	if(strcmp(header,"MGG File Version 1.1")!=NULL)
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
	_MGGFORMAT mggf;
	char header[21];
	register uint16 i=0, j=0, k=0, l=0, m=0, n=0, o=0;
	uint32 framesize[MAX_FRAMES], frameoffset[MAX_FRAMES];
	uint16 *posx, *posy, *sizex, *sizey, *dimx, *dimy, channel2;
	uint8 *imgatlas;
	int16 *w, *h, *currh;
	int width, height, channel;
	unsigned char *imgdata;
	uint8 normals[MAX_FRAMES];
	uint32 normalsize[MAX_FRAMES];

	memset(&normals,0,MAX_FRAMES*sizeof(uint8));

	if((file=DecompressFile(name))==NULL)
	{
		LogApp("Error reading MGG file %s",name);
			return false;
	}

	rewind(file);

	fread(header,21,1,file);

	if(strcmp(header,"MGG File Version 1.1")!=NULL)
	{
		LogApp("Invalid MGG file header %s",header);
		fclose(file);
		return false;
	}

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
	
	mgg->num_anims=mggf.num_animations;

	mgg->frames=(TEX_DATA*) calloc(mgg->num_frames,sizeof(TEX_DATA));

	_MGGANIM *mga;

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
	fread(framesize,sizeof(uint32),mggf.num_singletex,file);
	fread(frameoffset,sizeof(uint32),mggf.num_singletex,file);
	fread(normals,sizeof(uint8),mggf.num_singletex,file);
	fread(normalsize,sizeof(uint32),mggf.num_singletex,file);

	mgg->size=(Pos*) malloc(mgg->num_frames*sizeof(Pos));
	mgg->atlas=(GLint*) malloc(mggf.num_atlas*sizeof(GLint));

	for(i=0, j=0;i<mggf.num_singletex;i++)
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
			mgg->atlas[i]=SOIL_create_OGL_texture(imgdata,width,height,channel,0,SOIL_FLAG_TEXTURE_REPEATS || SOIL_FLAG_MIPMAPS ); //mgg->atlas[i]=SOIL_load_OGL_texture_from_memory((unsigned char*)data,framesize[i],SOIL_LOAD_AUTO,0,SOIL_FLAG_TEXTURE_REPEATS);

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
				mgg->frames[i].Ndata=SOIL_create_OGL_texture(imgdata,width,height,channel,0,SOIL_FLAG_TEXTURE_REPEATS || SOIL_FLAG_MIPMAPS);

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
			imgdata=SOIL_load_image_from_memory((unsigned char*)data,framesize[i],&width,&height,&channel,SOIL_LOAD_AUTO);
			mgg->frames[i+(mggf.num_frames-mggf.num_singletex)].data=SOIL_create_OGL_texture(imgdata,width,height,channel,0,SOIL_FLAG_TEXTURE_REPEATS || SOIL_FLAG_MIPMAPS);//SOIL_load_OGL_texture_from_memory((unsigned char*)data,framesize[i],SOIL_LOAD_AUTO,0,SOIL_FLAG_TEXTURE_REPEATS);

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
			CreateVAO(&vbdt[vbdt_num-1],0,4);
		}

	}
#endif

	fseek(file,mggf.possize_offset,SEEK_SET);

	posx=(uint16*) malloc((mggf.num_texinatlas+1)*sizeof(uint16));
	posy=(uint16*) malloc((mggf.num_texinatlas+1)*sizeof(uint16));
	sizex=(uint16*) malloc((mggf.num_texinatlas+1)*sizeof(uint16));
	sizey=(uint16*) malloc((mggf.num_texinatlas+1)*sizeof(uint16));
	imgatlas=(uint8*) malloc((mggf.num_texinatlas+1)*sizeof(uint8));

	fread(posx,sizeof(uint16),(mggf.num_texinatlas+1),file);
	fread(posy,sizeof(uint16),(mggf.num_texinatlas+1),file);
	fread(sizex,sizeof(uint16),(mggf.num_texinatlas+1),file);
	fread(sizey,sizeof(uint16),(mggf.num_texinatlas+1),file);
	fread(imgatlas,sizeof(uint8),(mggf.num_texinatlas+1),file);

	for(i=mggf.num_atlas-1;i<mggf.num_texinatlas+1;i++)
	{
		mgg->frames[i].data=mgg->atlas[imgatlas[i]];
		mgg->frames[i].posx=posx[i-1];
		mgg->frames[i].posy=posy[i-1];
		mgg->frames[i].sizex=sizex[i-1];
		mgg->frames[i].sizey=sizey[i-1];
		mgg->frames[i].vb_id=mgg->frames[imgatlas[i]].vb_id;
	}
	
#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

	k=vbdt_num;
	
	if(mggf.num_frames>1)
	{
		for(i=(mggf.num_atlas+mggf.num_texinatlas), j=mggf.num_atlas;i<mggf.num_frames;i++, j++)
		{	
			if(i==(mggf.num_atlas+mggf.num_texinatlas) && (mgg->frames[i].w<1024 && mgg->frames[i].h<1024) && !mgg->frames[i].normal)
			{
				if(!vbdt_num)
				{
					vbdt_num++;
					l=vbdt_num-1;
					vbdt=(VB_DATAT*) malloc(sizeof(VB_DATAT));

					vbdt[l].normal=0;

					vbdt[l].buffer_elements=8;

					CreateVAO(&vbdt[l],0,4);

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

					CreateVAO(&vbdt[l],0,4);

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
					glGenerateMipmap(GL_TEXTURE_2D);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
					/*
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
					*/
				}
			}
		}

		free(w);
		free(h);
		free(currh);
	}

#endif
	
	free(posx);
	free(posy);
	free(sizex);
	free(sizey);
	free(imgatlas);

	fclose(file);

	return true;
		
}

void FreeMGG(_MGG *file)
{
	file->type=NONE;

	for(register uint32 i=0; i<file->num_frames; i++)
	{
		//glDeleteTextures(1, &file->frames[i]);
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
	for(register uint16 i=0; i<MAX_MGG; i++)
	{
		memset(&mgg[i],0,sizeof(_MGG));
		mgg[i].type=NONE;
	}
}

uint8 CheckCollisionSector(float x, float y, float xsize, float ysize, float ang, Pos vert[4])
{
	float xb, xl, yb, yl, xtb, xtl, ytb, ytl, tmpx, tmpy;

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

uint8 CheckColision(float x, float y, float xsize, float ysize, float tx, float ty, float txsize, float tysize, float ang, float angt)
{
	float xb, xl, yb, yl, xtb, xtl, ytb, ytl, tmpx, tmpy;

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

uint8 CheckColisionMouse(float x, float y, float xsize, float ysize, float ang)
{

	float xb, xl, yb, yl, xtb, xtl, ytb, ytl, tmpx, tmpy;

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

uint8 CheckColisionMouseWorld(float x, float y, float xsize, float ysize, float ang)
{

	float xb, xl, yb, yl, xtb, xtl, ytb, ytl, tmpx, tmpy;

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

int8 DrawSprite(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, TEX_DATA data, uint8 a, int32 z)
{
	float tmp, ax, ay, az;

	uint8 valx=0, valy=0;
	register uint32 i=0, j=0, k=0;
	register int32 t1, t2, t3, t4, timej, timel;
	
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

	az=mCos(ang);
	az*=100;
	j=(int32) az;

	ay=mSin(ang);
	ay*=100;
	k=(int32) ay;

	//if(CheckBounds(x,y,sizex,sizey,t3,t4,j,k)) return 1;

	if(valx == 4 || valy == 4)
		return 1;
			
	if(st.num_entities==MAX_GRAPHICS-1)
		return 2;
	else
		i=st.num_entities;
	
			ent[i].data=data;

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)

			sizex*=st.Camera.dimension.x;
			sizey*=st.Camera.dimension.y;

			if(r==0 && g==0 && b==0)
				r=g=b=1;

			if(z>23) z=23;

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
				ent[i].texcor[0]=0;
				ent[i].texcor[1]=0;
				ent[i].texcor[2]=1;
				ent[i].texcor[3]=0;
				ent[i].texcor[4]=1;
				ent[i].texcor[5]=1;
				ent[i].texcor[6]=0;
				ent[i].texcor[7]=1;
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
				
				if(j<8 && data.vb_id!=-1)
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

			//timel=GetTicks() - timej;

			st.num_entities++;

	return 0;
}

int8 DrawLight(int32 x, int32 y, int32 z, int16 ang, uint8 r, uint8 g, uint8 b, LIGHT_TYPE type, uint8 intensity, float falloff, int32 radius)
{
	register uint16 i=0;

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
	//for(register uint32 i=0;i<MAX_GRAPHICS+1;i++)
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

int8 DrawLightmap(int32 x, int32 y, int32 z, int32 sizex, int32 sizey, GLuint data, LIGHT_TYPE type)
{
	float tmp, ax, ay, az;

	uint8 val=0;

	int16 ang=0;

	register uint32 i=0, j=0, k=0;
	
	PosF dim=st.Camera.dimension;
	
	x-=st.Camera.position.x;
	y-=st.Camera.position.y;

	i=st.num_lightmap;

	if(i==MAX_LIGHTMAPS)
		return 2;

	sizex*=st.Camera.dimension.x;
	sizey*=st.Camera.dimension.y;

	if(dim.x<0) dim.x*=-1;
	if(dim.y<0) dim.y*=-1;

	if(dim.x<10) dim.x=16384/dim.x;
	else dim.x*=16384;
	if(dim.y<10) dim.y=8192/dim.y;
	else dim.y*=8192;

	lmp[i].data.data=data;

			lmp[i].vertex[0]=(float)x+(((x-(sizex/2))-x)*mCos(ang) + ((y-(sizey/2))-y)*mSin(ang));
			lmp[i].vertex[1]=(float)y+(((x-(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			lmp[i].vertex[2]=0;

			lmp[i].vertex[3]=(float)x+(((x+(sizex/2))-x)*mCos(ang) + ((y-(sizey/2))-y)*mSin(ang));
			lmp[i].vertex[4]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y-(sizey/2))-y)*mCos(ang));
			lmp[i].vertex[5]=0;

			lmp[i].vertex[6]=(float)x+(((x+(sizex/2))-x)*mCos(ang) + ((y+(sizey/2))-y)*mSin(ang));
			lmp[i].vertex[7]=(float)y+(((x+(sizex/2))-x)*mSin(ang) + ((y+(sizey/2))-y)*mCos(ang));
			lmp[i].vertex[8]=0;

			lmp[i].vertex[9]=(float)x+(((x-(sizex/2))-x)*mCos(ang) + ((y+(sizey/2))-y)*mSin(ang));
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
				lmp[i].color[j]=1;
				lmp[i].color[j+1]=1;
				lmp[i].color[j+2]=1;
				lmp[i].color[j+3]=1;
			}

				st.num_lightmap++;

	return 0;
}

int8 DrawGraphic(float x, float y, float sizex, float sizey, float ang, uint8 r, uint8 g, uint8 b, GLuint data, float a, float texpanX, float texpanY, float texsizeX, float texsizeY)
{
	float tmp;

	PosF dim=st.Camera.dimension;

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
			/*
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
			*/
			break;
		}
	}

	return 0;
}

int8 DrawHud(float x, float y, float sizex, float sizey, float ang, uint8 r, uint8 g, uint8 b, float x1, float y1, float x2, float y2, GLuint data, float a)
{
	float tmp;
	uint8 val=0;

	for(register uint32 i=0;i<MAX_GRAPHICS+1;i++)
	{
		if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
			return 2;

		if(ent[i].stat==DEAD)
		{
			/*
			ent[i].stat=USED;
			ent[i].ang=ang;
			ent[i].pos.x=x;
			ent[i].pos.y=y;
			ent[i].size.x=sizex;
			ent[i].size.y=sizey;
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
			*/
			break;
		}
	}

	return 0;
}

int8 DrawUI(float x, float y, float sizex, float sizey, float ang, uint8 r, uint8 g, uint8 b, float x1, float y1, float x2, float y2, GLuint data, float a)
{
	float tmp;
	uint8 val=0;

	for(register uint32 i=0;i<MAX_GRAPHICS+1;i++)
	{
		if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
			return 2;

		if(ent[i].stat==DEAD)
		{
			/*
			ent[i].stat=USED;
			ent[i].ang=ang;
			ent[i].pos.x=x;
			ent[i].pos.y=y;
			ent[i].size.x=sizex;
			ent[i].size.y=sizey;
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
			*/
			break;
		}
	}

	return 0;
}

int8 DrawLine(float x, float y, float x2, float y2, uint8 r, uint8 g, uint8 b, float a, float linewidth, int32 z)
{
	uint8 valx=0, valy=0;

	uint16 i=0, j=0, k=0;

	int32 x3, y3;

	uint32 a1;

	int16 ang;

	float ax=1/(16384/2), ay=1/(8192/2), az=1/(4096/2), ang2;

	PosF dim=st.Camera.dimension;

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

	if(x>dim.x) valx++;
	if(y>dim.y) valy++;

	if(x2>dim.x) valx++;
	if(y2>dim.y) valy++;

	if(valx==2 || valy==2) return 1;

	i=st.num_entities;

	if(i==MAX_GRAPHICS-1 && ent[i].stat==USED)
		return 2;

	if(ent[i].stat==DEAD)
	{
		ent[i].stat=USED;
		st.num_entities++;
		ent[i].data.data=6;
		ent[i].data.vb_id=-1;

		x3=x2-x;
		y3=y2-y;

		ang2=atan2((float)y3,(float)x3);
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

		texone_ids[texone_num]=i;
		texone_num++;
	}

	return 0;
}

int32 MAnim(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, _MGG *mgf, uint16 id, int16 speed, uint8 a)
{
	uint16 curf=0;

	if((mgf->anim[id].current_frame)>mgf->anim[id].endID*10) 
		mgf->anim[id].current_frame=mgf->anim[id].startID*10; 
	else
	if((mgf->anim[id].current_frame/10)==0) 
		curf=mgf->anim[id].startID; 
	
	curf=mgf->anim[id].current_frame/10;

	DrawSprite(x,y,sizex,sizey,ang,r,g,b, mgf->frames[curf],a,0);

	mgf->anim[id].current_frame+=speed;

	return mgf->anim[id].current_frame/10;
}

int8 DrawString(const char *text, float x, float y, float sizex, float sizey, float ang, uint8 r, uint8 g, uint8 b, float a, TTF_Font *f)
{	
	SDL_Color co;
	co.r=255;
	co.g=255;
	co.b=255;
	co.a=255;
	uint16 formatt;

	float tmp;

	uint8 val=0;
	
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
			/*
			glGenTextures(1,&ent[i].data);
			glBindTexture(GL_TEXTURE_2D,ent[i].data);
			glTexImage2D(GL_TEXTURE_2D,0,msg->format->BytesPerPixel,msg->w,msg->h,0,formatt,GL_UNSIGNED_BYTE,msg->pixels);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			ent[i].ang=ang;
			ent[i].stat=USED;
			ent[i].type=TEXT;
			ent[i].pos.x=x;
			ent[i].pos.y=y;
			ent[i].size.x=sizex;
			ent[i].size.y=sizey;
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
			*/
			break;
		}
	}

	return 0;

}

int8 DrawStringUI(const char *text, float x, float y, float sizex, float sizey, float ang, uint8 r, uint8 g, uint8 b, float a, TTF_Font *f)
{	
	SDL_Color co;
	co.r=255;
	co.g=255;
	co.b=255;
	co.a=255;
	uint16 formatt;

	float tmp;

	uint8 val=0;
	
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
			/*
			glGenTextures(1,&ent[i].data);
			glBindTexture(GL_TEXTURE_2D,ent[i].data);
			glTexImage2D(GL_TEXTURE_2D,0,msg->format->BytesPerPixel,msg->w,msg->h,0,formatt,GL_UNSIGNED_BYTE,msg->pixels);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			ent[i].ang=ang;
			ent[i].stat=USED;
			ent[i].type=TEXT_UI;
			ent[i].pos.x=x;
			ent[i].pos.y=y;
			ent[i].size.x=sizex;
			ent[i].size.y=sizey;
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
			*/
			break;
		}
	}

	return 0;

}

int8 DrawString2UI(const char *text, float x, float y, float sizex, float sizey, float ang, uint8 r, uint8 g, uint8 b, float a, TTF_Font *f)
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
			/*
			glGenTextures(1,&ent[i].data);
			glBindTexture(GL_TEXTURE_2D,ent[i].data);
			glTexImage2D(GL_TEXTURE_2D,0,msg->format->BytesPerPixel,msg->w,msg->h,0,formatt,GL_UNSIGNED_BYTE,msg->pixels);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			ent[i].ang=ang;
			ent[i].stat=USED;
			ent[i].type=TEXT_UI;
			
			ent[i].pos.x=x;
			ent[i].pos.y=y;
			ent[i].size.x=(msg->w*sizex);
			ent[i].size.y=(msg->h*sizey);
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
			*/
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
	/*
	//Draw the objects first

	float x, y, sizex, sizey, ang, size;
	

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
	*/
}

void Renderer()
{

	GLint unif, texcat, vertat, pos, col, texc, tex_bound[2]= { -1, -1 };
	GLenum error;

	uint32 num_targets=0;
	register uint32 i=0, j=0, m=0, timej, timel;
	uint16 *k, l=0;

	static uint32 tesg=0;

	float m1, m2;

	float vertex[12]={
		-1,-1,0, 1,-1,0,
		1,1,0, -1,1,0 };

	float texcoord[8]={
		0,0, 1,0,
		1,1, 0,1 };

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
		}
	}

	k=(uint16*) calloc(vbdt_num,sizeof(uint16));

	for(i=0;i<st.num_entities;i++)
	{
		if(ent[i].data.vb_id!=-1)
		{
			for(j=0;j<16;j++)
			{
				vbdt[ent[i].data.vb_id].color[(k[ent[i].data.vb_id]*16)+j]=ent[i].color[j];

				if(j<12)
					vbdt[ent[i].data.vb_id].vertex[(k[ent[i].data.vb_id]*12)+j]=ent[i].vertex[j];

				if(j<8)
					vbdt[ent[i].data.vb_id].texcoord[(k[ent[i].data.vb_id]*8)+j]=ent[i].texcor[j];

					
				if(j<=2)
					vbdt[ent[i].data.vb_id].index[(k[ent[i].data.vb_id]*6)+j]=((k[ent[i].data.vb_id]*6)-(k[ent[i].data.vb_id]*2))+j;

				if(j==3 || j==4)
					vbdt[ent[i].data.vb_id].index[(k[ent[i].data.vb_id]*6)+j]=((k[ent[i].data.vb_id]*6)-(k[ent[i].data.vb_id]*2))+(j-1);

				if(j==5)
					vbdt[ent[i].data.vb_id].index[(k[ent[i].data.vb_id]*6)+j]=((k[ent[i].data.vb_id]*6)-(k[ent[i].data.vb_id]*2));
			}

			k[ent[i].data.vb_id]++;
		}
	}

	free(k);

	for(i=0;i<vbdt_num;i++)
	{
		if(vbdt[i].num_elements>0)
		{
			if(vbdt[i].num_elements<vbdt[i].buffer_elements)
			{
				if(vbdt[i].num_elements!=vbdt[i].num_elements2)
				{
					UpdateVAO(&vbdt[i],0,1,4);
					vbdt[i].num_elements2=vbdt[i].num_elements;
				}
				else
					UpdateVAO(&vbdt[i],0,0,4);
			}
			else
			if(vbdt[i].num_elements<vbdt[i].buffer_elements-8)
			{
				vbdt[i].buffer_elements=vbdt[i].num_elements+8;
				UpdateVAO(&vbdt[i],1,1,4);
			}
			else
			if(vbdt[i].num_elements>vbdt[i].buffer_elements)
			{
				vbdt[i].buffer_elements=vbdt[i].num_elements+8;
				UpdateVAO(&vbdt[i],1,1,4);
			}
		}
	}
		/*
		else
		if(vbdt[i].num_elements==0 && vbdt[i].buffer_elements>8)
		{
			vbdt[i].buffer_elements=8;
			UpdateVAO(&vbdt[i],2,0,2);
		}
		*/
	//tesg=1;
	//}

	//timel=GetTicks() - timej;

#endif

	num_targets=st.num_entities;

#ifdef _VAO_RENDER
	if(st.renderer.VAO_ON)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,st.renderer.FBO[0]);
		

		//glViewport(0,0,st.screenx,st.screeny);

		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(st.num_lightmap>0)
		{
			glDrawBuffers(1,&st.renderer.Buffers[2]);

			//glViewport(0,0,st.screenx/2,st.screeny/2);

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

				glBindVertexArray(vbd.vao_id);

				glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);

				//glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),lmp[i].vertex);
				//glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),texcoord);
				//glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),vbd.color);

				glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,0);

				//glBindVertexArray(0);
				//glBindBuffer(GL_ARRAY_BUFFER,0);

				//glUseProgram(0);
			}
		}
		
		//glDrawBuffers(1,&st.renderer.Buffers[0]);

		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER,0);

		//glViewport(0,0,st.screenx,st.screeny);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[2]);

		glGenerateMipmap(GL_TEXTURE_2D);

		/*
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		*/
		glUseProgram(st.renderer.Program[3]);

		unif=glGetUniformLocation(st.renderer.Program[3],"texu");
		glUniform1i(unif,0);

		unif=glGetUniformLocation(st.renderer.Program[3],"texu2");
		glUniform1i(unif,2);

		unif=glGetUniformLocation(st.renderer.Program[3],"texu3");
		glUniform1i(unif,1);

		for(i=z_used;i>=0;i--)
		{
			for(j=0;j<z_slot[i];j++)
			{
				if(ent[z_buffer[i][j]].data.vb_id!=-1)
				{
					m=ent[z_buffer[i][j]].data.vb_id;

					

					/*
					unif=glGetUniformLocation(st.renderer.Program[2],"normal");
					glUniform1f(unif,0);
					*/
					//glActiveTexture(GL_TEXTURE0);

					if(tex_bound[0]!=vbdt[m].texture)
					{
						glBindTexture(GL_TEXTURE_2D,vbdt[m].texture);
						tex_bound[0]=vbdt[m].texture;
					}

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

					glBindVertexArray(vbdt[m].vao_id);

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
						//glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,0);
						glDrawRangeElements(GL_TRIANGLES,(ent[z_buffer[i][j]].data.loc*6),((ent[z_buffer[i][j]].data.loc+l)*6)+6,((ent[z_buffer[i][j]].data.loc+l)*6)+6,GL_UNSIGNED_SHORT,0);

					glBindVertexArray(0);
					//glUseProgram(0);

					if(l)
						j+=l;
				}
				else
				{
					/*
					glUseProgram(st.renderer.Program[3]);

					unif=glGetUniformLocation(st.renderer.Program[3],"texu");
					glUniform1i(unif,0);

					unif=glGetUniformLocation(st.renderer.Program[3],"texu2");
					glUniform1i(unif,2);

					unif=glGetUniformLocation(st.renderer.Program[3],"texu3");
					glUniform1i(unif,1);
					*/
					//unif=glGetUniformLocation(st.renderer.Program[2],"normal");
					//glUniform1f(unif,0);

					glActiveTexture(GL_TEXTURE0);

					if(tex_bound[0]!=ent[z_buffer[i][j]].data.data)
					{
						glBindTexture(GL_TEXTURE_2D,ent[z_buffer[i][j]].data.data);
						tex_bound[0]=ent[z_buffer[i][j]].data.data;
					}

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

					glBindVertexArray(vbd.vao_id);

					glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);

					//glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
					glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),ent[z_buffer[i][j]].vertex);
					glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),ent[z_buffer[i][j]].texcor);
					glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),ent[z_buffer[i][j]].color);

					glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,0);
				}
			}

			if(i==0) break;
		}
		
		
		//glDrawBuffers(1,&st.renderer.Buffers[1]);

		//glViewport(0,0,st.screenx,st.screeny);

		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/*
		for(i=0;i<vbdt_num;i++)
		{
			if(vbdt[i].num_elements>0)
			{
				//if(l==0) CreateVAO(&vbdt[i],0,2);

				glUseProgram(st.renderer.Program[2]);

				unif=glGetUniformLocation(st.renderer.Program[2],"texu");
				glUniform1i(unif,0);

				glActiveTexture(GL_TEXTURE0);

				if(vbdt[i].normal)
				{
					glBindTexture(GL_TEXTURE_2D,vbdt[i].Ntexture);
					unif=glGetUniformLocation(st.renderer.Program[2],"normal");
					glUniform1f(unif,1);
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D,vbdt[i].texture);

					unif=glGetUniformLocation(st.renderer.Program[2],"normal");
					glUniform1f(unif,2);
				}

				glBindVertexArray(vbdt[i].vao_id);

				glDrawRangeElements(GL_TRIANGLES,0,vbdt[i].num_elements*6,vbdt[i].num_elements*6,GL_UNSIGNED_SHORT,0);
			
				//glBindVertexArray(0);

				vbdt[i].num_elements=0;

				glDeleteVertexArrays(1,&vbdt[i].vao_id);
				glDeleteBuffers(1,&vbdt[i].vbo_id);
				glDeleteBuffers(1,&vbdt[i].ibo_id);

				//glUseProgram(0);
			}
		}

		for(i=0;i<texone_num;i++)
		{
			glActiveTexture(GL_TEXTURE0);

			glUseProgram(st.renderer.Program[2]);
			
			if(ent[texone_ids[i]].data.normal)
			{
				unif=glGetUniformLocation(st.renderer.Program[2],"normal");
				glUniform1f(unif,1);

				if(i==0)
					glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.Ndata);
				else 
				if(i>0 && ent[texone_ids[i]].data.data!=ent[texone_ids[i-1]].data.Ndata)
					glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.Ndata);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.data);

				unif=glGetUniformLocation(st.renderer.Program[2],"normal");
				glUniform1f(unif,2);
			}
			*/
		/*
			for(i=z_used;i>=0;i--)
			{
				for(j=0;j<z_slot[i];j++)
				{
					if(ent[z_buffer[i][j]].data.vb_id!=-1)
					{
						m=ent[z_buffer[i][j]].data.vb_id;

						glUseProgram(st.renderer.Program[2]);

						unif=glGetUniformLocation(st.renderer.Program[2],"texu");
						glUniform1i(unif,0);

						//glActiveTexture(GL_TEXTURE0);
						if(vbdt[i].normal)
						{
							if(tex_bound!=vbdt[m].texture)
							{
								glBindTexture(GL_TEXTURE_2D,vbdt[m].texture);
								tex_bound=vbdt[m].texture;
							}
							unif=glGetUniformLocation(st.renderer.Program[2],"normal");
							glUniform1f(unif,1);
						}
						else
						{
							if(tex_bound!=vbdt[m].texture)
							{
								glBindTexture(GL_TEXTURE_2D,vbdt[m].texture);
								tex_bound=vbdt[m].texture;
							}

							unif=glGetUniformLocation(st.renderer.Program[2],"normal");
							glUniform1f(unif,2);
						}

						glBindVertexArray(vbdt[m].vao_id);

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

						glBindVertexArray(0);
						glUseProgram(0);

						if(l)
							j+=l;
						//glDrawRangeElements(GL_TRIANGLES,0,vbdt[m].num_elements*6,6,GL_UNSIGNED_SHORT,0);
					}
					else
					{
						glUseProgram(st.renderer.Program[2]);

						unif=glGetUniformLocation(st.renderer.Program[2],"texu");
						glUniform1i(unif,0);

						//glActiveTexture(GL_TEXTURE0);

						if(ent[z_buffer[i][j]].data.normal)
						{
							unif=glGetUniformLocation(st.renderer.Program[2],"normal");
							glUniform1f(unif,1);

							if(tex_bound!=ent[z_buffer[i][j]].data.data)
							{
								glBindTexture(GL_TEXTURE_2D,ent[z_buffer[i][j]].data.data);
								tex_bound=ent[z_buffer[i][j]].data.data;
							}
						}
						else
						{
							if(tex_bound!=ent[z_buffer[i][j]].data.data)
							{
								glBindTexture(GL_TEXTURE_2D,ent[z_buffer[i][j]].data.data);
								tex_bound=ent[z_buffer[i][j]].data.data;
							}

							unif=glGetUniformLocation(st.renderer.Program[2],"normal");
							glUniform1f(unif,2);
						}

						glBindVertexArray(vbd.vao_id);

						glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);

						//glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
						glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),ent[z_buffer[i][j]].vertex);
						glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),ent[z_buffer[i][j]].texcor);
						glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),ent[z_buffer[i][j]].color);

						glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,0);
					}
				}

				if(i==0) break;
			}
			*/
			/*
			unif=glGetUniformLocation(st.renderer.Program[2],"texu");
			glUniform1i(unif,0);
				
			glBindVertexArray(vbd.vao_id);

			glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);

			//glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),ent[texone_ids[i]].vertex);
			glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),ent[texone_ids[i]].texcor);
			glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),ent[texone_ids[i]].color);

			glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,0);

			//glBindVertexArray(0);
			//glBindBuffer(GL_ARRAY_BUFFER,0);

			//glUseProgram(0);
		}
		*/
/*
		if(st.num_lightmap>0)
		{
			glDrawBuffers(1,&st.renderer.Buffers[2]);

			//glViewport(0,0,st.screenx,st.screeny);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for(i=0;i<st.num_lightmap;i++)
			{
				glUseProgram(st.renderer.Program[2]);
				unif=glGetUniformLocation(st.renderer.Program[2],"texu");
				glUniform1i(unif,0);

				unif=glGetUniformLocation(st.renderer.Program[2],"normal");
				glUniform1f(unif,0);

				//glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D,lmp[i].data.data);

				glBindVertexArray(vbd.vao_id);

				glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);

				//glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
				glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),lmp[i].vertex);
				glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),texcoord);
				glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),vbd.color);

				glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,0);

				//glBindVertexArray(0);
				//glBindBuffer(GL_ARRAY_BUFFER,0);

				//glUseProgram(0);
			}
		}
		*/

/*
			glBindFramebuffer(GL_FRAMEBUFFER,0);

			glViewport(0,0,st.screenx,st.screeny);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[0]);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[1]);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		if(st.num_lightmap>0)
		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[2]);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			//glBindVertexArray(vbd.vao_id);

			//glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);

			//glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),vertex);
			//glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),texcoord);
			//glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),vbd.color);

			glUseProgram(st.renderer.Program[3]);

			unif=glGetUniformLocation(st.renderer.Program[3],"texu2");
			glUniform1i(unif,2);

			unif=glGetUniformLocation(st.renderer.Program[3],"texu");
			glUniform1i(unif,0);

			unif=glGetUniformLocation(st.renderer.Program[3],"texu3");
			glUniform1i(unif,1);

			glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,0);

			glUseProgram(0);
		}
		*/
		//glActiveTexture(GL_TEXTURE0);

		memset(z_buffer,0,(3*8)*(2048));
		memset(z_slot,0,3*8);
		z_used=0;

		
		for(i=0;i<vbdt_num;i++)
		{
			if(vbdt[i].num_elements>0)
			{
				vbdt[i].num_elements=0;

				//glDeleteVertexArrays(1,&vbdt[i].vao_id);
				//glDeleteBuffers(1,&vbdt[i].vbo_id);
				//glDeleteBuffers(1,&vbdt[i].ibo_id);
			}
		}
		
	}

	//SDL_Delay(1000);
	
#endif
	
#ifdef _VBO_RENDER
	if(st.renderer.VBO_ON)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,st.renderer.FBO[0]);
		glDrawBuffers(1,&st.renderer.Buffers[0]);

		glViewport(0,0,st.screenx,st.screeny);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		for(i=0;i<vbdt_num;i++)
		{
			if(vbdt[i].num_elements>0)
			{

				if(l==0) CreateVBO(&vbdt[i]);

				glActiveTexture(GL_TEXTURE0);

				glBindTexture(GL_TEXTURE_2D,vbdt[i].texture);

				glBindBuffer(GL_ARRAY_BUFFER,vbdt[i].vbo_id);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbdt[i].ibo_id);

				glUseProgram(st.renderer.Program[4]);

				unif=glGetUniformLocation(st.renderer.Program[4],"texu");
				glUniform1i(unif,0);

				pos=glGetAttribLocation(st.renderer.Program[4],"Position");
				texc=glGetAttribLocation(st.renderer.Program[4],"TexCoord");
				col=glGetAttribLocation(st.renderer.Program[4],"Color");

				glEnableVertexAttribArray(pos);
				glEnableVertexAttribArray(texc);
				glEnableVertexAttribArray(col);

				glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);
				glVertexAttribPointer(texc,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) ((12*vbdt[i].num_elements)*sizeof(GLfloat)));
				glVertexAttribPointer(col,4,GL_UNSIGNED_BYTE,GL_TRUE,0,(GLvoid*) ((12*vbdt[i].num_elements)*sizeof(GLfloat)+(8*vbdt[i].num_elements)*sizeof(GLfloat)));

				glDrawRangeElements(GL_TRIANGLES,0,vbdt[i].num_elements*6,vbdt[i].num_elements*6,GL_UNSIGNED_SHORT,0);

				glDisableVertexAttribArray(pos);
				glDisableVertexAttribArray(texc);
				glDisableVertexAttribArray(col);
			
				glBindBuffer(GL_ARRAY_BUFFER,0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

				glUseProgram(0);
			}
		}

		for(i=0;i<texone_num;i++)
		{
			glActiveTexture(GL_TEXTURE0);

			if(i==0)
				glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.data);
			else 
			if(i>0 && ent[texone_ids[i]].data.data!=ent[texone_ids[i-1]].data.data)
				glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.data);

			glUseProgram(st.renderer.Program[4]);
			unif=glGetUniformLocation(st.renderer.Program[4],"texu");
			glUniform1i(unif,0);

			glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbd.ibo_id);

			glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),ent[texone_ids[i]].vertex);
			glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),ent[texone_ids[i]].texcor);
			glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),ent[texone_ids[i]].color);

			glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*sizeof(GLushort)),vbd.index,GL_STREAM_DRAW);

			pos=glGetAttribLocation(st.renderer.Program[4],"Position");
			texc=glGetAttribLocation(st.renderer.Program[4],"TexCoord");
			col=glGetAttribLocation(st.renderer.Program[4],"Color");

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

		glDrawBuffers(1,&st.renderer.Buffers[1]);

		glViewport(0,0,st.screenx,st.screeny);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[0]);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbd.ibo_id);

		glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),vertex);
		glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),texcoord);
		glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),vbd.color);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*sizeof(GLushort)),vbd.index,GL_STREAM_DRAW);

		glUseProgram(st.renderer.Program[2]);

		unif=glGetUniformLocation(st.renderer.Program[2],"texu");
		glUniform1i(unif,1);

		unif=glGetUniformLocation(st.renderer.Program[2],"normal");
		glUniform1f(unif,0);

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

		glUseProgram(0);

		for(i=0;i<vbdt_num;i++)
		{
			if(vbdt[i].num_elements>0)
			{
				glActiveTexture(GL_TEXTURE0);

				glBindTexture(GL_TEXTURE_2D,vbdt[i].texture);

				glUseProgram(st.renderer.Program[5]);

				unif=glGetUniformLocation(st.renderer.Program[5],"texu");
				glUniform1i(unif,0);

				unif=glGetUniformLocation(st.renderer.Program[5],"texu2");
				glUniform1i(unif,1);

				glBindBuffer(GL_ARRAY_BUFFER,vbdt[i].vbo_id);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbdt[i].ibo_id);

				pos=glGetAttribLocation(st.renderer.Program[5],"Position");
				texc=glGetAttribLocation(st.renderer.Program[5],"TexCoord");
				col=glGetAttribLocation(st.renderer.Program[5],"Color");

				glEnableVertexAttribArray(pos);
				glEnableVertexAttribArray(texc);
				glEnableVertexAttribArray(col);

				glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);
				glVertexAttribPointer(texc,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) ((12*vbdt[i].num_elements)*sizeof(GLfloat)));
				glVertexAttribPointer(col,4,GL_UNSIGNED_BYTE,GL_TRUE,0,(GLvoid*) ((12*vbdt[i].num_elements)*sizeof(GLfloat)+(8*vbdt[i].num_elements)*sizeof(GLfloat)));

				glDrawRangeElements(GL_TRIANGLES,0,vbdt[i].num_elements*6,vbdt[i].num_elements*6,GL_UNSIGNED_SHORT,0);

				glDisableVertexAttribArray(pos);
				glDisableVertexAttribArray(texc);
				glDisableVertexAttribArray(col);

				glBindBuffer(GL_ARRAY_BUFFER,0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
			
				glUseProgram(0);
			}
		}

		for(i=0;i<texone_num;i++)
		{
			glActiveTexture(GL_TEXTURE0);

			if(i==0)
				glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.data);
			else 
			if(i>0 && ent[texone_ids[i]].data.data!=ent[texone_ids[i-1]].data.data)
				glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.data);

			glUseProgram(st.renderer.Program[5]);

			unif=glGetUniformLocation(st.renderer.Program[5],"texu");
			glUniform1i(unif,0);

			unif=glGetUniformLocation(st.renderer.Program[5],"texu2");
			glUniform1i(unif,1);
				
			//glBindVertexArray(vbd.vao_id);

			glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbd.ibo_id);

			glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),ent[texone_ids[i]].vertex);
			glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),ent[texone_ids[i]].texcor);
			glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),ent[texone_ids[i]].color);

			glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*sizeof(GLushort)),vbd.index,GL_STREAM_DRAW);

			pos=glGetAttribLocation(st.renderer.Program[5],"Position");
			texc=glGetAttribLocation(st.renderer.Program[5],"TexCoord");
			col=glGetAttribLocation(st.renderer.Program[5],"Color");

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

			//glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER,0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

			glUseProgram(0);
		}

		glDrawBuffers(1,&st.renderer.Buffers[0]);

		glViewport(0,0,st.screenx,st.screeny);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for(i=0;i<vbdt_num;i++)
		{
			if(vbdt[i].num_elements>0)
			{
				//if(l==0) CreateVAO(&vbdt[i],0,2);

				glUseProgram(st.renderer.Program[2]);

				unif=glGetUniformLocation(st.renderer.Program[2],"texu");
				glUniform1i(unif,0);

				glActiveTexture(GL_TEXTURE0);

				if(vbdt[i].normal)
				{
					glBindTexture(GL_TEXTURE_2D,vbdt[i].Ntexture);
					unif=glGetUniformLocation(st.renderer.Program[2],"normal");
					glUniform1f(unif,1);
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D,vbdt[i].texture);

					unif=glGetUniformLocation(st.renderer.Program[2],"normal");
					glUniform1f(unif,2);
				}

				glBindBuffer(GL_ARRAY_BUFFER,vbdt[i].vbo_id);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbdt[i].ibo_id);

				pos=glGetAttribLocation(st.renderer.Program[2],"Position");
				texc=glGetAttribLocation(st.renderer.Program[2],"TexCoord");
				col=glGetAttribLocation(st.renderer.Program[2],"Color");

				glEnableVertexAttribArray(pos);
				glEnableVertexAttribArray(texc);
				glEnableVertexAttribArray(col);

				glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);
				glVertexAttribPointer(texc,2,GL_FLOAT,GL_FALSE,0,(GLvoid*) ((12*vbdt[i].num_elements)*sizeof(GLfloat)));
				glVertexAttribPointer(col,4,GL_UNSIGNED_BYTE,GL_TRUE,0,(GLvoid*) ((12*vbdt[i].num_elements)*sizeof(GLfloat)+(8*vbdt[i].num_elements)*sizeof(GLfloat)));

				glDrawRangeElements(GL_TRIANGLES,0,vbdt[i].num_elements*6,vbdt[i].num_elements*6,GL_UNSIGNED_SHORT,0);

				glDisableVertexAttribArray(pos);
				glDisableVertexAttribArray(texc);
				glDisableVertexAttribArray(col);

				glBindBuffer(GL_ARRAY_BUFFER,0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

				vbdt[i].num_elements=0;

				free(vbdt[i].vertex);
				free(vbdt[i].index);
				free(vbdt[i].texcoord);
				free(vbdt[i].color);

				glDeleteBuffers(1,&vbdt[i].vbo_id);
				glDeleteBuffers(1,&vbdt[i].ibo_id);

				glUseProgram(0);
			}
		}

		for(i=0;i<texone_num;i++)
		{
			glActiveTexture(GL_TEXTURE0);

			glUseProgram(st.renderer.Program[2]);

			if(ent[texone_ids[i]].data.normal)
			{
				unif=glGetUniformLocation(st.renderer.Program[2],"normal");
				glUniform1f(unif,1);

				if(i==0)
					glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.Ndata);
				else 
				if(i>0 && ent[texone_ids[i]].data.data!=ent[texone_ids[i-1]].data.Ndata)
					glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.Ndata);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.data);

				unif=glGetUniformLocation(st.renderer.Program[2],"normal");
				glUniform1f(unif,2);
			}
			
			unif=glGetUniformLocation(st.renderer.Program[2],"texu");
			glUniform1i(unif,0);
				
			//glBindVertexArray(vbd.vao_id);

			glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbd.ibo_id);

			glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),ent[texone_ids[i]].vertex);
			glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),ent[texone_ids[i]].texcor);
			glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),ent[texone_ids[i]].color);

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

			//glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER,0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

			glUseProgram(0);
		}

		if(st.num_lightmap>0)
		{
			glDrawBuffers(1,&st.renderer.Buffers[2]);

			glViewport(0,0,st.screenx,st.screeny);

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

			glBindFramebuffer(GL_FRAMEBUFFER,0);

			glViewport(0,0,st.screenx,st.screeny);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[0]);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[1]);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		if(st.num_lightmap>0)
		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D,st.renderer.FBTex[2]);

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glBindBuffer(GL_ARRAY_BUFFER,vbd.vbo_id);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbd.ibo_id);

			glBufferData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float))+(16*sizeof(GLubyte)),NULL,GL_STREAM_DRAW);
			glBufferSubData(GL_ARRAY_BUFFER,0,12*sizeof(float),vertex);
			glBufferSubData(GL_ARRAY_BUFFER,12*sizeof(float),8*sizeof(float),texcoord);
			glBufferSubData(GL_ARRAY_BUFFER,(12*sizeof(float))+(8*sizeof(float)),16*sizeof(GLubyte),vbd.color);

			glBufferData(GL_ELEMENT_ARRAY_BUFFER,(6*sizeof(GLushort)),vbd.index,GL_STREAM_DRAW);

			glUseProgram(st.renderer.Program[3]);

			unif=glGetUniformLocation(st.renderer.Program[3],"texu2");
			glUniform1i(unif,2);

			unif=glGetUniformLocation(st.renderer.Program[3],"texu");
			glUniform1i(unif,1);

			unif=glGetUniformLocation(st.renderer.Program[3],"texu3");
			glUniform1i(unif,0);

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

			glUseProgram(0);
		}
	}
#endif

#ifdef _VA_RENDER

	if(st.renderer.VA_ON)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_EQUAL,1.0);

		for(i=0;i<vbdt_num;i++)
		{
			if(vbdt[i].num_elements>0)
			{
				glBindTexture(GL_TEXTURE_2D,vbdt[i].texture);

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);

				glVertexPointer(3,GL_FLOAT,0,vbdt[i].vertex);
				glTexCoordPointer(2,GL_FLOAT,0,vbdt[i].texcoord);
				glColorPointer(4,GL_UNSIGNED_BYTE,0,vbdt[i].color);

				glDrawRangeElements(GL_TRIANGLES,0,vbdt[i].num_elements*6,vbdt[i].num_elements*6,GL_UNSIGNED_SHORT,vbdt[i].index);

				glDisableClientState(GL_VERTEX_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_COLOR_ARRAY);
			}
		}

		for(i=0;i<texone_num;i++)
		{
			if(i==0)
				glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.data);
			else 
			if(i>0 && ent[texone_ids[i]].data.data!=ent[texone_ids[i-1]].data.data)
				glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.data);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);

			glVertexPointer(3,GL_FLOAT,0,ent[texone_ids[i]].vertex);
			glTexCoordPointer(2,GL_FLOAT,0,ent[texone_ids[i]].texcor);
			glColorPointer(4,GL_UNSIGNED_BYTE,0,ent[texone_ids[i]].color);

			glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,vbd.index);

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);

		}

		glAlphaFunc(GL_GREATER,0.5);

		for(i=0;i<vbdt_num;i++)
		{
			if(vbdt[i].num_elements>0)
			{
				glBindTexture(GL_TEXTURE_2D,vbdt[i].texture);

				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glEnableClientState(GL_COLOR_ARRAY);

				glVertexPointer(3,GL_FLOAT,0,vbdt[i].vertex);
				glTexCoordPointer(2,GL_FLOAT,0,vbdt[i].texcoord);
				glColorPointer(4,GL_UNSIGNED_BYTE,0,vbdt[i].color);

				glDrawRangeElements(GL_TRIANGLES,0,vbdt[i].num_elements*6,vbdt[i].num_elements*6,GL_UNSIGNED_SHORT,vbdt[i].index);

				glDisableClientState(GL_VERTEX_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glDisableClientState(GL_COLOR_ARRAY);

				free(vbdt[i].vertex);
				free(vbdt[i].texcoord);
				free(vbdt[i].color);
				free(vbdt[i].index);

				vbdt[i].num_elements=0;
			}
		}

		for(i=0;i<texone_num;i++)
		{
			if(i==0)
				glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.data);
			else 
			if(i>0 && ent[texone_ids[i]].data.data!=ent[texone_ids[i-1]].data.data)
				glBindTexture(GL_TEXTURE_2D,ent[texone_ids[i]].data.data);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);

			glVertexPointer(3,GL_FLOAT,0,ent[texone_ids[i]].vertex);
			glTexCoordPointer(2,GL_FLOAT,0,ent[texone_ids[i]].texcor);
			glColorPointer(4,GL_UNSIGNED_BYTE,0,ent[texone_ids[i]].color);

			glDrawRangeElements(GL_TRIANGLES,0,6,6,GL_UNSIGNED_SHORT,vbd.index);

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);

		}

		glDisable(GL_ALPHA_TEST);

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
	}

#endif

	st.num_hud=0;
	st.num_ui=0;
	st.num_entities=0;

#if defined (_VAO_RENDER) || defined (_VBO_RENDER) || defined (_VA_RENDER)
	texone_num=0;
#endif

	st.num_lights=0;
	st.num_lightmap=0;
	memset(&ent,0,MAX_GRAPHICS);

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
