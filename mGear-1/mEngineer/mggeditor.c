#include "mggeditor.h"
#include <math.h> 

MGGEd mged;

int8 LoadImageInSystem(char *filename)
{
	FILE *f;

	unsigned char *buffer;
	size_t size;

	if((f=fopen(filename,"rb"))==NULL)
	{
		LogApp("Error: could not open image file %s",filename);
		return NULL;
	}

	fseek(f,0,SEEK_END);
	size=ftell(f);

	rewind(f);

	buffer=malloc(size);

	fread(buffer,size,1,f);

	mged.data=SOIL_load_image_from_memory(buffer,size,&mged.w,&mged.h,&mged.channel,NULL);

	if(mged.data)
	{
		mged.tex.data=SOIL_create_OGL_texture(mged.data,mged.w,mged.h,mged.channel,NULL,SOIL_FLAG_MIPMAPS);
		mged.tex.vb_id=-1;

		fclose(f);

		return 1;
	}
	else
	{
		fclose(f);
		LogApp("Error: could not load image %s",filename);
		return NULL;
	}
	
}

void DetectBackgroundRGB()
{
	uint8 *rgb, found=0;

	register int32 i, j, k, tc=0, table=256;
	int w, h;

	uint16 *count;

	w=mged.w;
	h=mged.h;

	rgb=malloc(table*3); //starts with 256 color table
	count=malloc(table);

	//Creates a color table
	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			found=0;

			if(i==0 && j==0)
			{
				rgb[0]=mged.data[(i*w*4)+(j*4)];
				rgb[1]=mged.data[(i*w*4)+(j*4)+1];
				rgb[2]=mged.data[(i*w*4)+(j*4)+2];
				count[0]=1;

				tc=1;
			}

			for(k=0;k<tc;k++)
			{
				if(mged.data[(i*w*4)+(j*4)]==rgb[3*k] && mged.data[(i*w*4)+(j*4)+1]==rgb[(3*k)+1] && mged.data[(i*w*4)+(j*4)+2]==rgb[(3*k)+2])
				{
					found=1;
					count[k]++;
					break;
				}
			}

			if(!found)
			{
				rgb[(3*tc)]=mged.data[(i*w*4)+(j*4)];
				rgb[(3*tc)+1]=mged.data[(i*w*4)+(j*4)+1];
				rgb[(3*tc)+2]=mged.data[(i*w*4)+(j*4)+2];

				count[tc]=1;

				tc++;

				if(tc==table-1)
				{
					table+=256;
					rgb=realloc(rgb,table*3);
					count=realloc(count,table);
				}

			}

		}
	}

	//Detects the most predominant color
	for(i=0;i<tc;i++)
	{
		if(i==0)
		{
			j=0;
			continue;
		}

		if(count[i]>count[j])
			j=i;
	}

	mged.bckrgb[0]=rgb[(3*j)];
	mged.bckrgb[1]=rgb[(3*j)+1];
	mged.bckrgb[2]=rgb[(3*j)+2];

	//free(rgb);
	//free(count);

}

void DetectSprites()
{
	register int32 i, j, k, l, m, n, num_box=0, boxs=0, buffer=256, marked=0;
	SPBOX *sp;
	int8 *spb, doit=0, boxv=0, *data2;
	float dist;

	int w, h;

	w=mged.w;
	h=mged.h;

	sp=malloc(256*sizeof(SPBOX));
	data2=calloc(mged.w*mged.h,1);
	mged.num_sprites=0;

	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			if(mged.data[(i*w*4)+(j*4)]==mged.bckrgb[0] && mged.data[(i*w*4)+(j*4)+1]==mged.bckrgb[1] && mged.data[(i*w*4)+(j*4)+2]==mged.bckrgb[2])
			{
				if(boxs)
				{
					sp[num_box].sx=j;
					sp[num_box].sy=i;
					sp[num_box].y=i;

					doit=0;
					k=i+1;
					l=sp[num_box].x;
					marked=0;
					while(!doit)
					{
						if(mged.data[(k*w*4)+(l*4)]!=mged.bckrgb[0] || mged.data[(k*w*4)+(l*4)+1]!=mged.bckrgb[1] || mged.data[(k*w*4)+(l*4)+2]!=mged.bckrgb[2])
						{
							if(!data2[(k*w)+l])
								data2[(k*w)+l]=1;

							if(l<sp[num_box].x)
								sp[num_box].x=l;

							if(l>sp[num_box].x+sp[num_box].sx)
								sp[num_box].sx=l-sp[num_box].x;

							marked++;
						}
						else
							boxv++;

						if(!boxv)
							l--;
						else
						if(boxv==1)
							l++;
						else
						{
							if(!marked || l==w-1 || k==h-1)
								break;
							boxv=0;
							l=sp[num_box].x;
							k++;
						}

						if(l==w-1 || k==h-1)
								break;

					}

					//sp[num_box].sx-=sp[num_box].x;
					//sp[num_box].sy-=sp[num_box].y;
					num_box++;
					boxs=0;
				}
				
			}
			else
			{
				if(!boxs && !data2[(i*w)+j])
				{
					sp[num_box].x=j;
					data2[(i*w)+j]=1;
					boxs=1;
				}
				else
				if(boxs && !data2[(i*w)+j])
					data2[(i*2)+j]=1;
			}
		}
	}

	spb=calloc(num_box,sizeof(int8));
	
	for(i=0;i<5;i++)
	{
		for(l=sp[i].y;l<=sp[i].y+sp[i].sy;l++)
			{
				for(k=sp[i].x;k<=sp[i].x+sp[i].sx;k++)
				{
					mged.data[(l*w*4)+(k*4)]=0;
					mged.data[(l*w*4)+(k*4)+1]=0;
					mged.data[(l*w*4)+(k*4)+2]=0;
				}
			}
	}
	
	
	meng.got_it=0;
	for(i=0;i<num_box;i++)
	{
		if(spb[i]==1)
			continue;
		else
			mged.num_sprites++;

		if(spb[i]==2)
			meng.got_it++;
	}

	printf("%d\n",meng.got_it);

	mged.spbox=calloc(mged.num_sprites,sizeof(SPBOX));
	
	for(i=0,j=0;i<num_box;i++)
	{
		if(spb[i]==1)
			continue;
		else
		{
			for(l=sp[i].y;l<=sp[i].y+sp[i].sy;l++)
			{
				for(k=sp[i].x;k<=sp[i].x+sp[i].sx;k++)
				{
					//mged.data[(l*w*4)+(k*4)]=0;
					//mged.data[(l*w*4)+(k*4)+1]=0;
					//mged.data[(l*w*4)+(k*4)+2]=0;
				}
			}

			memcpy(&mged.spbox[j],&sp[i],sizeof(SPBOX));
			j++;
		}
	}

	mged.tex.data=SOIL_create_OGL_texture(mged.data,w,h,4,0,0);

	glBindTexture(GL_TEXTURE_2D,mged.tex.data);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//free(spb);
	//free(sp);
}

void Rendering()
{
	register int32 i, j;
	int32 x, y, sx, sy;
	int w, h;

	w=mged.w;
	h=mged.h;

	GraphicData(8192,4096,8192,4096,0,255,255,255,mged.tex,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,7);
	/*
	for(i=4;i<mged.num_sprites;i++)
	{
		x=(8192*mged.spbox[i].x)/w;
		y=(4096*mged.spbox[i].y)/h;

		x=x+4096;
		y=y+2048;

		sx=(8192*mged.spbox[i].sx)/w;
		sy=(4096*mged.spbox[i].sy)/h;

		sx=sx+4096;
		sy=sy+2048;

		GraphicData(x+(sx/2),y+(sy/2),sx,sy,0,255,255,255,mgg_sys[0].frames[4],128,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,6);
	}
	*/
}

void MGGEditorMain()
{
	char options[2][16]={"MGG Editor", "Sheet Extractor"}, path[512];

	if(mged.mgg_loaded==0)
	{
		switch(UIOptionBox(0,0,CENTER,options,2,ARIAL,2048,UI_COL_NORMAL,UI_COL_SELECTED))
		{
			case UI_SEL:
			{
				mged.mgg_loaded=1;
				mged.atlas_editor=0;
				break;
			}

			case UI_SEL+1:
			{
				mged.mgg_loaded=-1;
				mged.atlas_editor=1;
				
				break;
			}
		}
	}
	else
	{
		if(mged.atlas_editor==1)
		{
			if(UISelectFile("tga, png",path))
			{
				if(!LoadImageInSystem(path))
				{
					mged.mgg_loaded=0;
					mged.atlas_editor=0;
				}
				else
				{
					mged.atlas_editor=2;
					DetectBackgroundRGB();
					DetectSprites();
				}
			}
		}

		if(mged.atlas_editor==2)
		{
			Rendering();
		}
	}
}