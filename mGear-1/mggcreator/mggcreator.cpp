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
	memset(&mgg,0,sizeof(_MGGFORMAT));
	_MGGANIM *mga;
	char FileName[256], filename[256], animfile[256], tmp[32], str[2][16], framename[256], framename2[256];
	int16 t=0, value, p=0, a=-1;
	char header[21]={"MGG File Version 1.0"};

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
		for(register uint8 i=0; i<argc;i++)
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
		}
	}

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

	fseek(file,21,SEEK_SET);

	fwrite(&mgg,sizeof(_MGGFORMAT),1,file);
	rewind(file);
	fseek(file,(sizeof(_MGGFORMAT)+512)+21,SEEK_SET);
	
	fwrite(mga,sizeof(_MGGANIM),2,file);
	
	uint32 framesize[MAX_FRAMES];
	size_t totalsize;
	totalsize=((sizeof(_MGGFORMAT)+512)+(MAX_FRAMES*sizeof(uint32)+512)+(512+(MAX_ANIMATIONS*sizeof(_MGGANIM))))+21;

	uint32 j=0;
	for(register uint32 i=0;i<mgg.num_frames;i++)
	{
		//if(i==33) j=42;
		strcpy(framename2,framename);
		if(j<10) sprintf(filename,"//frame000%d.tga",j); else
		if(j<100) sprintf(filename,"//frame00%d.tga",j); else
		if(j<1000) sprintf(filename,"//frame0%d.tga",j); else
		if(j<10000) sprintf(filename,"//frame0%d.tga",j);

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
			printf("Wrote frame number %d\n",j);
		}
		j++;
	}

	rewind(file);
	fseek(file,((sizeof(_MGGFORMAT)+512)+(512+(MAX_ANIMATIONS*sizeof(_MGGANIM))))+21,SEEK_CUR);
	fwrite(framesize,sizeof(uint32),mgg.num_frames,file);

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

