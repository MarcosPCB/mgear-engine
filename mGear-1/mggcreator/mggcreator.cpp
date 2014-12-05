// mggcreator.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include "mgg.h"
#include "quicklz.h"
#include <conio.h>


int main(int argc, char *argv[])
{
	FILE *file, *file2, *file3, *file4;
	_MGGFORMAT mgg;
	_MGGANIM *mga;
	char FileName[256], filename[256], animfile[256], tmp[32], str[2][16], framename[256], framename2[256];
	int16 t=0, value, p=0, a=-1, val[16];
	char header[21]={"MGG File Version 1.1"};
	uint16 *posx, *posy, *sizex, *sizey, num_img_in_atlas=0, *dimx, *dimy;
	uint8 *imgatlas;
	uint32 frameoffset[MAX_FRAMES];
	uint32 framesize[MAX_FRAMES];
	size_t totalsize;
	register uint16 i=0, j=0, k=0;

	memset(&mgg,0,sizeof(_MGGFORMAT));

	posx=(uint16*) malloc(sizeof(uint16));
	posy=(uint16*) malloc(sizeof(uint16));
	sizex=(uint16*) malloc(sizeof(uint16));
	sizey=(uint16*) malloc(sizeof(uint16));
	imgatlas=(uint8*) malloc(sizeof(uint8));
	dimx=(uint16*) malloc(sizeof(uint16));
	dimy=(uint16*) malloc(sizeof(uint16));

	if(argc<3)
	{
		printf("Creates an MGG file.\n");
		printf("-o output file name\n");
		printf("-p path to the folder with frames ( MUST BE NAMED FRAME0000.TGA, FRAME0001.TGA...\n");
		printf("-a path to the file that contains information about the mgg. Check test.txt\n");
		printf("EXAMPLE: -o test.mgg -p test//frames -a test//frames//test.txt\n");
		printf("OBS: Use double // for paths\n");
		exit(0);
	}
	else
	{
		for(i=0; i<argc;i++)
		{
			if(strcmp(argv[i],"-o")==NULL)
			{
				strcpy(FileName,argv[i+1]);
				i++;
			}
			else
			if(strcmp(argv[i],"-p")==NULL)
			{
				strcpy(framename,argv[i+1]);
				i++;
			}
			else
			if(strcmp(argv[i],"-a")==NULL)
			{
				strcpy(animfile,argv[i+1]);
				i++;
			}
		}
	}

	if((file4=fopen(animfile,"r"))==NULL)
	{
		printf("Error while reading MGG file\n");
		fflush(stdin);
		getch();
		exit(1);
	}

	memset(&mgg,0,sizeof(_MGGFORMAT));

	printf("Reading %s...\n",&animfile);

	while(!feof(file4))
	{
		memset(str,0,sizeof(str));
		fgets(tmp,sizeof(tmp),file4);
		sscanf(tmp,"%s %s",str[0], str[1]);
		if(strcmp(str[0],"\0")==NULL)
			continue;
		if(p==1)
		{
			if(a==-1)
			{
				if(strcmp(str[0],"ANIM")==NULL)
					a=atoi(str[1]);
				else
				{
					printf("Error: you must define ANIM just after BEGIN. Command defined is %s\n",&str[0]);
					fflush(stdin);
					getch();
					exit(1);
				}
			}
			else
			if(a!=-1)
			{
				if(strcmp(str[0],"NAME")==NULL)
					strcpy(mga[a].name,str[1]);
				else
				if(strcmp(str[0],"ANIM")==NULL)
				{
					printf("Error: Another ANIM was defined before this one: ANIM value is %d\n",a);
					fflush(stdin);
					getch();
					exit(1);
				}
				else
				if(strcmp(str[0],"STARTF")==NULL)
				{
					value=atoi(str[1]);
					mga[a].startID=value;
					mga[a].current_frame=value;
				}
				else
				if(strcmp(str[0],"ENDF")==NULL)
				{
					value=atoi(str[1]);
					mga[a].endID=value;
				}
				else
				if(strcmp(str[0],"FRAMESA")==NULL)
				{
					value=atoi(str[1]);
					mga[a].num_frames=value;
				}
				else
				if(strcmp(str[0],"SPEED")==NULL)
				{
					value=atoi(str[1]);
					mga[a].speed=value;
				}
				else if(strcmp(str[0],"ENDA")==NULL)
				{
					p=0;
					a=-1;
				}
			}
		}
		else
		if(p==0)
		{
			if(strcmp(str[0],"MGGNAME")==NULL)
				strcpy(mgg.name,str[1]);
			else
			if(strcmp(str[0],"FRAMES")==NULL)
			{
				value=atoi(str[1]);
				mgg.num_frames=value;
			}
			else
			if(strcmp(str[0],"ANIMS")==NULL)
			{
				value=atoi(str[1]);
				mgg.num_animations=value;
				mga=(_MGGANIM*) malloc(value*sizeof(_MGGANIM));
			}
			else
			if(strcmp(str[0],"ATLAS")==NULL)
			{
				value=atoi(str[1]);
				mgg.num_atlas=value;

			}
			else
			if(strcmp(str[0],"TYPE")==NULL)
			{
				if(strcmp(str[1],"MULT")==NULL)
					mgg.type=SPRITEM;
				else
				if(strcmp(str[1],"TEX")==NULL)
					mgg.type=TEXTUREM;
			}
			else
			if(strcmp(str[0],"BEGIN")==NULL)
			{
				if(mgg.num_animations==0)
				{
					printf("Error: number of animations not defined in %s before BEGIN code\n",&animfile);
					fflush(stdin);
					getch();
					exit(1);
				}
				else p++;
			}
			else
			if(strcmp(str[0],"SET")==NULL)
			{
				if(!mgg.num_atlas || !num_img_in_atlas)
				{
					if(!mgg.num_atlas)
						printf("Error: Number of atlas textures not previously defined in %s\n",&animfile);
					
					if(!num_img_in_atlas)
						printf("Error: Number of images in atlases not previously defined in %s\n",&animfile);

					fflush(stdin);
					getch();
					exit(1);
				}
				else
				{
					sscanf(tmp,"%s %d %d %d %d %d %d %d %d %d", str[1], &val[0], &val[1], &val[2], &val[3], &val[4], &val[5], &val[6], &val[7], &val[8]);
				
					k=val[6];

					for(i=0;i<val[8];i++)
					{
						if(k==val[5]) break;

						for(j=0;j<val[7];j++)
						{
							if(k==val[5]) break;

							imgatlas[k]=val[0];

							posx[k]=val[1]+(j*val[3]);
							posy[k]=val[2]+(i*val[4]);
							sizex[k]=val[3];
							sizey[k]=val[4];

							posx[k]=(posx[k]*32768)/dimx[val[0]];
							posy[k]=(posy[k]*32768)/dimy[val[0]];
							sizex[k]=(sizex[k]*32768)/dimx[val[0]];
							sizey[k]=(sizey[k]*32768)/dimy[val[0]];

							k++;
						}
					}
				}
			}
			else
			if(strcmp(str[0],"ONEIMG")==NULL)
			{
				if(!mgg.num_atlas || !num_img_in_atlas)
				{
					if(!mgg.num_atlas)
						printf("Error: Number of atlas textures not previously defined in %s\n",&animfile);
					
					if(!num_img_in_atlas)
						printf("Error: Number of images in atlases not previously defined in %s\n",&animfile);

					fflush(stdin);
					getch();
					exit(1);
				}
				else
				{
					sscanf(tmp,"%s %d %d %d %d %d %d",str[1], &val[0], &val[1], &val[2], &val[3], &val[4], &val[5]);

					imgatlas[val[5]]=val[0];

					posx[val[5]]=val[1];
					posy[val[5]]=val[2];
					sizex[val[5]]=val[3];
					sizey[val[5]]=val[4];

					posx[k]=(posx[k]*32768)/dimx[val[0]];
					posy[k]=(posy[k]*32768)/dimy[val[0]];
					sizex[k]=(sizex[k]*32768)/dimx[val[0]];
					sizey[k]=(sizey[k]*32768)/dimy[val[0]];
				}
			}
			else
			if(strcmp(str[0],"ATLASDIM")==NULL)
			{
				if(!mgg.num_atlas || !num_img_in_atlas)
				{
					if(!mgg.num_atlas)
						printf("Error: Number of atlas textures not previously defined in %s\n",&animfile);
					
					if(!num_img_in_atlas)
						printf("Error: Number of images in atlases not previously defined in %s\n",&animfile);

					fflush(stdin);
					getch();
					exit(1);
				}
				else
				{
					sscanf(tmp,"%s %d %d %d",str[1], &val[0], &val[1], &val[2]);

					dimx[val[0]]=val[1];
					dimy[val[0]]=val[2];
				}
			}
			else
			if(strcmp(str[0],"ATLASIMGNUM")==NULL)
			{
				if(!mgg.num_atlas)
				{
					if(!mgg.num_atlas)
						printf("Error: Number of atlas textures not previously defined in %s\n",&animfile);
					
					fflush(stdin);
					getch();
					exit(1);
				}
				else
				{
					sscanf(tmp,"%s %d %d", str[0], &val[0], &val[1]);

					posx=(uint16*) realloc(posx,val[1]*sizeof(uint16));
					posy=(uint16*) realloc(posy,val[1]*sizeof(uint16));
					sizex=(uint16*) realloc(sizex,val[1]*sizeof(uint16));
					sizey=(uint16*) realloc(sizey,val[1]*sizeof(uint16));
					imgatlas=(uint8*) realloc(imgatlas,val[1]*sizeof(uint8));
					dimx=(uint16*) realloc(dimx,val[1]*sizeof(uint16));
					dimy=(uint16*) realloc(dimy,val[1]*sizeof(uint16));

					num_img_in_atlas+=val[1];
				}
			}
		}
	}

	mgg.num_singletex=mgg.num_frames-num_img_in_atlas;
	mgg.num_texinatlas=num_img_in_atlas;

	fclose(file4);

	printf("Read %s, no errors found\n",&animfile);

	file=tmpfile();

	printf("Creating MGG file...\n");

	if((file3=fopen(FileName,"wb"))==NULL)
	{
		printf("Error while creating file\n"); 
		fflush(stdin);
		getch();
		exit(1);
	}

	fseek(file,0,SEEK_SET);

	fwrite(header,21,1,file);

	//fwrite(&mgg,sizeof(_MGGFORMAT),1,file);

	fseek(file,sizeof(_MGGFORMAT),SEEK_CUR);
	
	fwrite(mga,sizeof(_MGGANIM),mgg.num_animations,file);

	mgg.textures_offset=ftell(file);
	
	//totalsize=((sizeof(_MGGFORMAT)+512)+(MAX_FRAMES*sizeof(uint32)+512)+(512+(MAX_ANIMATIONS*sizeof(_MGGANIM))))+21;

	j=0;
	for(i=0;i<mgg.num_singletex;i++)
	{
		//if(i==33) j=42;
		strcpy(framename2,framename);
		if(j<10) sprintf(filename,"//frame000%d.tga",j); else
		if(j<100) sprintf(filename,"//frame00%d.tga",j); else
		if(j<1000) sprintf(filename,"//frame0%d.tga",j); else
		if(j<10000) sprintf(filename,"//frame0%d.tga",j); else
		if(j<100000) sprintf(filename,"//frame%d.tga",j);

		strcat(framename2,filename);

		if((file2=fopen(framename2,"rb"))==NULL)
		{
			printf("Error: could not read frame number %d\n",j);
			fflush(stdin);
			getch();
			j++;
			continue;
		}
		else
		{
			printf("Writing frame number %d ...\n",j);
			fseek(file2,0,SEEK_END);

			size_t size=ftell(file2);
			rewind(file2);

			framesize[i]=size;
	
			void *buf=(void*) malloc(size);
			fread(buf,size,1,file2);
			//rewind(file);
			
			if(i==0) fseek(file,mgg.textures_offset+1,SEEK_SET); 
			else
			{
				//totalsize+=(framesize[i-1]+2048);
				fseek(file,frameoffset[i-1]+1,SEEK_SET);
			}
			
			fwrite(buf,size,1,file);
			frameoffset[i]=ftell(file);
			fclose(file2);
			free(buf);
			printf("Wrote frame number %d\n",j);
		}
		j++;
	}

	mgg.possize_offset=ftell(file);

	fwrite(posx,sizeof(uint16),num_img_in_atlas,file);
	fwrite(posy,sizeof(uint16),num_img_in_atlas,file);
	fwrite(sizex,sizeof(uint16),num_img_in_atlas,file);
	fwrite(sizey,sizeof(uint16),num_img_in_atlas,file);
	fwrite(imgatlas,sizeof(uint8),num_img_in_atlas,file);

	mgg.framesize_offset=ftell(file);

	fwrite(framesize,sizeof(uint32),mgg.num_singletex,file);

	fwrite(frameoffset,sizeof(uint32),mgg.num_singletex,file);

	fseek(file,21,SEEK_SET);

	fwrite(&mgg,sizeof(_MGGFORMAT),1,file);

	printf("Compressing file...\n");

	qlz_state_compress *comp=(qlz_state_compress*) malloc(sizeof(qlz_state_compress));

	rewind(file);
	fseek(file,0,SEEK_END);
	size_t len=ftell(file);
	rewind(file);

	char *bufe;

	bufe=(char*) malloc(len);
	fread(bufe,len,1,file);

	char *buf2;

	buf2=(char*) malloc(len+400);

	size_t len2=qlz_compress(bufe,buf2,len,comp);

	fwrite(buf2,len2,1,file3);

	printf("File compressed!!\n");
	printf("MGG file created sucessfully\n");

	fclose(file);
	fclose(file3);

	free(bufe);
	free(buf2);
	free(mga);

	fflush(stdin);
	getch();

	return 0;
}

