#include <stdlib.h>
#include <string.h>
#include "mgv.h" 
#include <conio.h>
//#include <crtdbg.h>
//#include <assert.h>


int main(int argc, char *argv[])
{
	FILE *file, *file2, *file3;
	MGVFORMAT mgv;
	memset(&mgv,0,sizeof(MGVFORMAT));
	char FileName[256], filename[256], framename[256], framename2[256], loading_str[256];
	char header[21]={ "MGV File Version 1.1"};
	unsigned char loading_pct, *buf, *buffer;
	register int32 i, j, k, l, m;
	size_t size, totalsize;
	uint32 *framesize;

	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);

	printf("MGV Creator 1.0\nMaster Gear Video Creator 2018\n");

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
	strcat(framename2,"\\MGV.WAV");

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

	fwrite(header,21,1,file);

	fwrite(&mgv,sizeof(MGVFORMAT),1,file);
	//fseek(file,sizeof(MGVFORMAT)+21,SEEK_SET);
	
	framesize=malloc((mgv.num_frames + 1)*sizeof(uint32));

	//assert(framesize);

	totalsize=sizeof(MGVFORMAT)+((mgv.num_frames + 1)*sizeof(uint32))+21;

	printf("\n");

	for (i = 0, m = 0, k = 0, j = 0; i < mgv.num_frames + 1; i++)
	{
		loading_pct = (i * 100) / mgv.num_frames;

		m = (i * 70) / mgv.num_frames;

		memset(loading_str, 0, sizeof(loading_str));

		for (k = 0; k < m; k++)
			strcat(loading_str, ".");

		printf("\rEncoding frames %d/%d [%d%%]%s", i, mgv.num_frames, loading_pct, loading_str);

		//if(i==33) j=42;
		strcpy(framename2,framename);
		if(j<10) sprintf(filename,"\\frame0000%d.jpg",j); else
		if(j<100) sprintf(filename,"\\frame000%d.jpg",j); else
		if(j<1000) sprintf(filename,"\\frame00%d.jpg",j); else
		if(j<10000) sprintf(filename,"\\frame0%d.jpg",j); else
		if (j<100000) sprintf(filename, "\\frame%d.jpg", j);

		strcat(framename2,filename);

		if((file2=fopen(framename2,"rb"))==NULL)
		{
			printf("%s\n",&framename2);
			printf("Error: could not read frame number %d\n",j);
			fflush(stdin);
			getch();
			j++;
			fclose(file);
			exit(1);
		}
		else
		{
			//printf("Writing frame number %d ...\n",j);
			fseek(file2,0,SEEK_END);

			size=ftell(file2);
			rewind(file2);

			framesize[i]=size;
	
			buf=malloc(size);
			fread(buf,size,1,file2);
			//rewind(file);
			
			if(i==0) fseek(file,totalsize,SEEK_SET); 
			else
			{
				totalsize+=framesize[i-1];
				fseek(file,totalsize,SEEK_SET);
			}

			fwrite(buf,size,1,file);
			fclose(file2);
			free(buf);
			//printf("Wrote frame number %d\n",j);
		}
		j++;
	}

	loading_pct = 100;

	memset(loading_str, 0, sizeof(loading_str));

	for (j = 0; j < 70; j++)
		strcat(loading_str, ".");

	printf("\rEncoding frames %d/%d [%d%%]%sDone\n", mgv.num_frames, mgv.num_frames, loading_pct, loading_str);

	//rewind(file);
	fseek(file,sizeof(MGVFORMAT)+21,SEEK_SET);
	fwrite(framesize,sizeof(uint32),mgv.num_frames + 1,file);

	printf("Writing audio data...\n");

	//rewind(file);
	fseek(file,totalsize,SEEK_SET);

	rewind(file3);
	
	buffer=malloc(mgv.sound_buffer_lenght);
	fread(buffer,mgv.sound_buffer_lenght,1,file3);
	fwrite(buffer,mgv.sound_buffer_lenght,1,file);

	free(buffer);

	fclose(file3);

	fclose(file);

	printf("Audio data wrote\n");

	printf("MGV file created sucessfully\n");
	printf("Press any key to continue...\n");

	fflush(stdin);
	getch();

	return 0;
}

