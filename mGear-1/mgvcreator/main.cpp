#include <stdlib.h>
#include <string.h>
#include "mgv.h" 
#include <conio.h>


int main(int argc, char *argv[])
{
	FILE *file, *file2, *file3;
	_MGVFORMAT mgv;
	memset(&mgv,0,sizeof(_MGVFORMAT));
	char FileName[256], filename[256], framename[256], framename2[256];

	if(argc<3)
	{
		printf("Creates an MGV file from a JPG sequence.\n");
		printf("-o output file name\n");
		printf("-p path to the folder with frames and the audio ( MUST BE NAMED FRAME0000.JPG, FRAME0001.JPG... AND THE AUDIO MUST BE MGV.WAV\n");
		printf("-fps number of frames per second\n");
		printf("-n total number of frames\n");
		printf("EXAMPLE: -o test.mgv -p test//frames -fps 30 -n 100\n");
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
			if(strcmp(argv[i],"-fps")==NULL)
				mgv.fps=atoi(argv[i+1]);
			else
			if(strcmp(argv[i],"-n")==NULL)
				mgv.num_frames=atoi(argv[i+1]);
		}
	}

	printf("Creating MGV file...\n");

	if((file=fopen(FileName,"wb"))==NULL)
	{
		printf("Error while creating file\n"); 
		fflush(stdin);
		getch();
		exit(1);
	}

	strcpy(framename2,framename);
	strcat(framename2,"//MGV.WAV");

	if((file3=fopen(framename2,"rb"))==NULL)
	{
		printf("Error: sound %s not found\n",&framename2);
		fflush(stdin);
		getch();
		exit(1);
	}

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
		strcpy(framename2,framename);
		if(j<10) sprintf(filename,"//frame000%d.jpg",j); else
		if(j<100) sprintf(filename,"//frame00%d.jpg",j); else
		if(j<1000) sprintf(filename,"//frame0%d.jpg",j); else
		if(j<10000) sprintf(filename,"//frame%d.jpg",j);

		strcat(framename2,filename);

		if((file2=fopen(framename2,"rb"))==NULL)
		{
			printf("%s\n",&framename2);
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
	fseek(file,((sizeof(_MGVFORMAT)+512)),SEEK_CUR);
	fwrite(framesize,sizeof(uint32),mgv.num_frames,file);

	printf("Writing audio data...\n");

	rewind(file);
	fseek(file,totalsize+512,SEEK_CUR);

	rewind(file3);
	
	void *buffer=(void*) malloc(mgv.sound_buffer_lenght);
	fread(buffer,mgv.sound_buffer_lenght,1,file3);
	fwrite(buffer,mgv.sound_buffer_lenght,1,file);

	free(buffer);

	fclose(file3);

	fclose(file);

	printf("Audio data wrote\n");

	printf("MGV file created sucessfully\n");

	fflush(stdin);
	getch();

	return 0;
}

