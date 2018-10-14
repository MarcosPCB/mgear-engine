#include <stdlib.h>
#include <string.h>
#include "mgg.h"
//#include "quicklz.h"
#include <conio.h>
//#include "rle.c"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STBI_NO_PSD
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_GIF
#define STBI_NO_PNM

int main(int argc, char *argv[])
{
	FILE *file, *file2, *file3, *file4;
	_MGGFORMAT mgg;
	_MGGANIM *mga = NULL;
	unsigned char FileName[260], filename[256], animfile[260], tmp[4096], str[2][32], framename[260], framename2[260], fileframe[64], files[MAX_FILES][64],
		files_n[MAX_FILES][64], *token, debug = 0;
	int16 t = 0, value, p = 0, a = -1, val[16], *offx = NULL, *offy = NULL, start_frame = 0;
	unsigned char header[21] = { "MGG File Version 2" }, *MGIbuf, *MGIimagedata, *imagebuf, *error_str, loading_str[256], loading_pct, RLE = 0, *DataN;
	uint16 *posx, *posy, *sizex, *sizey, num_img_in_atlas = 0, *dimx, *dimy, numfiles = 0;
	uint8 sequence = 0, *c_tmp, *c_atlas[32], *c_atlas_normal[32], *atlas_frames, ca_normal[32];
	int8 *imgatlas;
	uint32 frameoffset[MAX_FRAMES];
	uint32 framesize[MAX_FRAMES];
	uint8 normals[MAX_FRAMES], c_a_normals[32];
	uint32 normalsize[MAX_FRAMES];
	size_t totalsize;
	int i = 0, j = 0, k = 0, l = 0, m = 0, c_tmp_frames = 0, c_a_f0[32], c_a_ff[32], c_a_w[32], c_a_h[32], c_an_w[32], c_an_h[32],
		c_f_w[256], c_f_h[256], c_t_x, c_t_y, c_t_w, c_atlas_frames[32], num_c_atlas = 0, num_c_atlas_normals = 0, mggframes = 0, nms = 0;
	int width, height, MGIcolor, size;
	uint8 framealone[MAX_FRAMES];
	void *buf;
	struct Sizetex *sizetex, Sizes[512];

	int size_rle, size_rle2;

	memset(framealone, 0, MAX_FRAMES*sizeof(uint8));
	memset(&mgg, 0, sizeof(_MGGFORMAT));
	memset(normals, 0, MAX_FRAMES*sizeof(uint8));
	memset(c_a_normals, 0, 32 * sizeof(uint8));
	memset(files, 0, MAX_FILES * 64);
	memset(files_n, 0, MAX_FILES * 64);
	memset(ca_normal, 0, 32);

	printf("MGG Creator 1.0\nMaster Gear Graphics Creator 2018\n");

	if(argc<3)
	{
		printf("Creates an MGG file.\n");
		printf("-o output path file name\n");
		printf("-p path to the folder with frames and the instructions file\n");
		printf("-i instructions file name\n");
		printf("-debug debugs the instruction file");
		printf("EXAMPLE: -o \"test.mgg\" -p \"test/frames\" -i \"test.texprj\" \n");
		//printf("OBS: Use double // for paths\n");
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
				strcpy(framename, argv[i + 1]);
				strcpy(animfile, argv[i + 1]);
				strcat(animfile, "\\");
				strcat(animfile, argv[i + 3]);
				i++;
			}
			else
			if (strcmp(argv[i], "-debug") == NULL)
			{
				debug = 1;
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
		memset(tmp, 0, sizeof(tmp));
		fgets(tmp,sizeof(tmp),file4);
		sscanf(tmp,"%s %s",str[0], str[1]);
		if(strcmp(str[0],"\0")==NULL)
			continue;

		if (debug == 1)
		{
			printf("Found expression: %s", tmp);
			getch();
		}

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
			if (strcmp(str[0], "MGGNAME") == NULL)
				strcpy(mgg.name, str[1]);

			if (strcmp(str[0], "RLE") == NULL)
				RLE = 1;
			
			if (strcmp(str[0], "SEQUENCE") == NULL)
				sequence = 1;
			
			if (strcmp(str[0], "NOTSEQUENCE") == NULL)
				sequence = 0;
			
			if (strcmp(str[0], "FRAMENAMES") == NULL)
			{
				strcpy(fileframe, str[1]);
				sequence = 1;
			}
			
			if(strcmp(str[0],"FRAMES")==NULL)
			{
				value=atoi(str[1]);
				mgg.num_frames=value;

				imgatlas = malloc(value);
				memset(imgatlas, -1, value);

				posx = malloc(value * sizeof(uint16));
				posy = malloc(value * sizeof(uint16));
				sizex = malloc(value * sizeof(uint16));
				sizey = malloc(value * sizeof(uint16));
				dimx = malloc(value * sizeof(uint16));
				dimy = malloc(value * sizeof(uint16));

				offx=calloc(value,sizeof(int16));
				offy=calloc(value,sizeof(int16));
			}

			if (strcmp(str[0], "CONSTRUCT_ATLAS") == NULL)
			{
				if (!mgg.num_frames)
				{
					printf("Error: number of frames not defined in %s before CONSTRUCT_ATLAS code\n", &animfile);
					fflush(stdin);
					getch();
					exit(1);
				}
				else
				{
					sscanf(tmp, "%s %d %d %d", str[1], &val[0], &val[1], &val[2]);

					c_atlas_frames[val[0]] = val[2] - val[1] + 1;
					c_a_f0[val[0]] = val[1];
					c_a_ff[val[0]] = val[2];
				}
			}

			if (strcmp(str[0], "FRAMENAMES_CUSTOM_ATLAS") == NULL)
			{
				if (!mgg.num_frames)
				{
					printf("Error: number of frames not defined in %s before FRAMENAMES_CUSTOM code\n", &animfile);
					fflush(stdin);
					getch();
					exit(1);
				}
				else
				{
					mgg.num_atlas += 1;
					mggframes++;

					strtok(tmp, " ,\"");
					token = strtok(NULL, " ,\"");
					l = atoi(token);

					c_tmp_frames = c_atlas_frames[l];
					/*
					posx = (uint16*)realloc(posx, c_tmp_frames * sizeof(uint16));
					posy = (uint16*)realloc(posy, c_tmp_frames * sizeof(uint16));
					sizex = (uint16*)realloc(sizex, c_tmp_frames * sizeof(uint16));
					sizey = (uint16*)realloc(sizey, c_tmp_frames * sizeof(uint16));
					//imgatlas = (uint8*)realloc(imgatlas, c_tmp_frames * sizeof(uint8));
					dimx = (uint16*)realloc(dimx, c_tmp_frames * sizeof(uint16));
					dimy = (uint16*)realloc(dimy, c_tmp_frames * sizeof(uint16));
					*/

					num_img_in_atlas += c_tmp_frames;

					if (c_tmp_frames > 0)
					{
						//Time to build our atlas
						//maximum dimension 2048x4096

						c_tmp = malloc(2048 * 4096 * 4);
						memset(c_tmp, 0, 2048 * 4096 * 4);
						c_t_x = c_t_y = c_a_w[l] = c_a_h[l] = c_t_w = 0;

						for (i = 0; i < c_tmp_frames; i++)
						{
							loading_pct = (i*100)/c_atlas_frames[l];

							m = (i * 70) / c_atlas_frames[l];

							memset(loading_str, 0, sizeof(loading_str));

							for (k = 0; k < m; k++)
								strcat(loading_str, ".");

							printf("\rBuilding custom atlas %d [%d%%]%s", l, loading_pct, loading_str);

							if (!sequence)
							{
								token = strtok(NULL, ",\"");

								if (strcmp(token, " ") == NULL)
									token = strtok(NULL, ",\"");
							}

							strcpy(framename2, framename);
							strcat(framename2, "//");

							if (!sequence)
								strcat(framename2, token);
							else
							{
								if (i + c_a_f0[l] < 10) sprintf(filename, "//%s000%d.tga", fileframe, i + c_a_f0[l]); else
								if (i + c_a_f0[l] < 100) sprintf(filename, "//%s00%d.tga", fileframe, i + c_a_f0[l]); else
								if (i + c_a_f0[l] < 1000) sprintf(filename, "//%s0%d.tga", fileframe, i + c_a_f0[l]); else
								if (i + c_a_f0[l] < 10000) sprintf(filename, "//%s0%d.tga", fileframe, i + c_a_f0[l]); else
								if (i + c_a_f0[l] < 100000) sprintf(filename, "//%s%d.tga", fileframe, i + c_a_f0[l]);

								strcat(framename2, filename);
							}

							if ((file = fopen(framename2, "rb")) == NULL)
							{
								printf("Error: Frame %s could not be opened\n", i);
								free(c_tmp);
								fflush(stdin);
								getch();
								exit(1);
							}
							
							fseek(file, 0, SEEK_END);
							size = ftell(file);
							rewind(file);

							imagebuf = malloc(size);

							fread(imagebuf, size, 1, file);

							atlas_frames = stbi_load_from_memory(imagebuf, size, &c_f_w[i], &c_f_h[i], 0, 4);

							if (!atlas_frames)
							{
								printf("Error: frame %s could not be loaded\n", framename2);
								free(c_tmp);
								free(imagebuf);
								fflush(stdin);
								getch();
								exit(1);
							}

							Sizes[i + c_a_f0[l]].w = c_f_w[i];
							Sizes[i + c_a_f0[l]].h = c_f_h[i];

							if (c_f_w[i] > 1024 || c_f_h[i] > 1024)
							{
								printf("Error: Frame %d is to big for an atlas. Frame dimensions: %d x %d\n", i, c_f_w[i], c_f_h[i]);
								free(c_tmp);
								free(atlas_frames);
								free(imagebuf);
								fflush(stdin);
								getch();
								exit(1);
							}

							if (c_t_w + c_f_w[i] < 2048)
								c_t_w += c_f_w[i];
							else
							{
								c_t_w = c_f_w[i];
								c_t_x = 0;
								c_t_y = c_a_h[l];
							}

							if (c_t_y + c_f_h[i] > 4096)
							{
								printf("Error: Atlas is full, please, move frame %d to other atlas\n", i);
								if (stbi_write_jpg("Test_error.jpg", 2048, 4096, 4, c_tmp, 100) == NULL)
								{
									printf("error");
								}
								free(c_tmp);
								free(atlas_frames);
								free(imagebuf);
								fflush(stdin);
								getch();
								exit(1);
							}

							if (c_t_y + c_f_h[i] > c_a_h[l] && c_t_y + c_f_h[i] < 4096)
								c_a_h[l] += c_f_h[i];

							/*
							for (j = 0; j < c_f_h[i]; j++)
								memcpy(c_tmp + ((c_t_y + j) * 2048 + c_t_x) * 4, atlas_frames[i] + j, c_f_w[i] * 4);
								*/

							for (j = 0; j < c_f_h[i]; j++)
							{
								for (k = 0; k < c_f_w[i]; k++)
								{
									c_tmp[((j + c_t_y) * 2048 * 4) + ((k + c_t_x) * 4)] = atlas_frames[(j * c_f_w[i] * 4) + (k * 4)];
									c_tmp[((j + c_t_y) * 2048 * 4) + ((k + c_t_x) * 4) + 1] = atlas_frames[(j * c_f_w[i] * 4) + (k * 4) + 1];
									c_tmp[((j + c_t_y) * 2048 * 4) + ((k + c_t_x) * 4) + 2] = atlas_frames[(j * c_f_w[i] * 4) + (k * 4) + 2];
									c_tmp[((j + c_t_y) * 2048 * 4) + ((k + c_t_x) * 4) + 3] = atlas_frames[(j * c_f_w[i] * 4) + (k * 4) + 3];
								}
							}


							posx[i + c_a_f0[l]] = c_t_x;
							posy[i + c_a_f0[l]] = c_t_y;
							sizex[i + c_a_f0[l]] = c_f_w[i];
							sizey[i + c_a_f0[l]] = c_f_h[i];

							c_t_x = c_t_w;

							if (c_a_w[l] < c_t_x)
								c_a_w[l] = c_t_x;

							free(imagebuf);
							free(atlas_frames);

							fclose(file);
						}

						for (i = c_a_f0[l]; i < c_a_ff[l] + 1; i++)
						{
							posx[i] = (posx[i] * 32768) / c_a_w[l];
							posy[i] = (posy[i] * 32768) / c_a_h[l];
							sizex[i] = (sizex[i] * 32768) / c_a_w[l];
							sizey[i] = (sizey[i] * 32768) / c_a_h[l];
							imgatlas[i] = l;
						}

						c_atlas[l] = malloc(c_a_w[l] * c_a_h[l] * 4);
						//memset(c_atlas[l], 0, c_a_w[l] * c_a_h[l] * 4);

						
						for (j = 0; j < c_a_h[l]; j++)
						{
							for (k = 0; k < c_a_w[l]; k++)
							{
								c_atlas[l][(j * c_a_w[l] * 4) + (k * 4)] = c_tmp[(j * 2048 * 4) + (k * 4)];
								c_atlas[l][(j * c_a_w[l] * 4) + (k * 4) + 1] = c_tmp[(j * 2048 * 4) + (k * 4) + 1];
								c_atlas[l][(j * c_a_w[l] * 4) + (k * 4) + 2] = c_tmp[(j * 2048 * 4) + (k * 4) + 2];
								c_atlas[l][(j * c_a_w[l] * 4) + (k * 4) + 3] = c_tmp[(j * 2048 * 4) + (k * 4) + 3];
							}
						}
						
						free(c_tmp);

						loading_pct = 100;

						m = 70;

						memset(loading_str, 0, sizeof(loading_str));

						for (k = 0; k < m; k++)
							strcat(loading_str, ".");

						printf("\rBuilding custom atlas %d [%d%%]%sDone\n", l, loading_pct, loading_str);

						num_c_atlas++;
					}
				}
			}

			if (strcmp(str[0], "FRAMENAMES_CUSTOM_ATLAS_NORMALS") == NULL)
			{
				if (!mgg.num_frames)
				{
					printf("Error: number of frames not defined in %s before FRAMENAMES_CUSTOM code\n", &animfile);
					fflush(stdin);
					getch();
					exit(1);
				}
				else
				{
					strtok(tmp, " ,\"");
					token = strtok(NULL, " ,\"");
					l = atoi(token);

					c_tmp_frames = c_atlas_frames[l];

					if (c_tmp_frames > 0)
					{
						//Time to build our atlas
						//maximum dimension 2048x4096

						c_tmp = malloc(2048 * 4096 * 4);
						memset(c_tmp, 0, 2048 * 4096 * 4);
						c_t_x = c_t_y = c_an_w[l] = c_an_h[l] = c_t_w = 0;

						for (i = 0; i < c_tmp_frames; i++)
						{
							loading_pct = (i * 100) / c_atlas_frames[l];

							m = (i * 70) / c_atlas_frames[l];

							memset(loading_str, 0, sizeof(loading_str));

							for (k = 0; k < m; k++)
								strcat(loading_str, ".");

							printf("\rBuilding custom atlas normal %d [%d%%]%s", l, loading_pct, loading_str);

							if (!sequence)
							{
								token = strtok(NULL, ",\"");

								if (strcmp(token, " ") == NULL)
									token = strtok(NULL, ",\"");
							}

							if (strcmp(token, "null") == 0)
							{
								atlas_frames = malloc(Sizes[i + c_a_f0[l]].w * Sizes[i + c_a_f0[l]].h * 4);
								memset(atlas_frames, 255, Sizes[i + c_a_f0[l]].w * Sizes[i + c_a_f0[l]].h * 4);
								size = Sizes[i + c_a_f0[l]].w * Sizes[i + c_a_f0[l]].h * 4;
								c_f_w[i] = Sizes[i + c_a_f0[l]].w;
								c_f_h[i] = Sizes[i + c_a_f0[l]].h;
							}
							else
							{
								strcpy(framename2, framename);
								strcat(framename2, "//");

								if (!sequence)
									strcat(framename2, token);
								else
								{
									if (i + c_a_f0[l] < 10) sprintf(filename, "//%s000%d_n.tga", fileframe, i + c_a_f0[l]); else
									if (i + c_a_f0[l] < 100) sprintf(filename, "//%s00%d_n.tga", fileframe, i + c_a_f0[l]); else
									if (i + c_a_f0[l] < 1000) sprintf(filename, "//%s0%d_n.tga", fileframe, i + c_a_f0[l]); else
									if (i + c_a_f0[l] < 10000) sprintf(filename, "//%s0%d_n.tga", fileframe, i + c_a_f0[l]); else
									if (i + c_a_f0[l] < 100000) sprintf(filename, "//%s%d_n.tga", fileframe, i + c_a_f0[l]);

									strcat(framename2, filename);
								}

								if ((file = fopen(framename2, "rb")) == NULL)
								{
									printf("Error: Frame %s could not be opened\n", i);
									free(c_tmp);
									fflush(stdin);
									getch();
									exit(1);
								}

								fseek(file, 0, SEEK_END);
								size = ftell(file);
								rewind(file);

								imagebuf = malloc(size);

								fread(imagebuf, size, 1, file);

								atlas_frames = stbi_load_from_memory(imagebuf, size, &c_f_w[i], &c_f_h[i], 0, 4);

								if (!atlas_frames)
								{
									printf("Error: frame %s could not be loaded\n", framename2);
									free(c_tmp);
									free(imagebuf);
									fflush(stdin);
									getch();
									exit(1);
								}

								free(imagebuf);
							}

							if (c_f_w[i] > 1024 || c_f_h[i] > 1024)
							{
								printf("Error: Frame %d is to big for an atlas. Frame dimensions: %d x %d\n", i, c_f_w[i], c_f_h[i]);
								free(c_tmp);
								free(atlas_frames);
								free(imagebuf);
								fflush(stdin);
								getch();
								exit(1);
							}

							if (c_t_w + c_f_w[i] < 2048)
								c_t_w += c_f_w[i];
							else
							{
								c_t_w = c_f_w[i];
								c_t_x = 0;
								c_t_y = c_an_h[l];
							}

							if (c_t_y + c_f_h[i] > 4096)
							{
								printf("Error: Atlas is full, please, move frame %d to other atlas\n", i);
								if (stbi_write_jpg("Test_error.jpg", 2048, 4096, 4, c_tmp, 100) == NULL)
								{
									printf("error");
								}
								free(c_tmp);
								free(atlas_frames);
								free(imagebuf);
								fflush(stdin);
								getch();
								exit(1);
							}

							if (c_t_y + c_f_h[i] > c_an_h[l] && c_t_y + c_f_h[i] < 4096)
								c_an_h[l] += c_f_h[i];

							/*
							for (j = 0; j < c_f_h[i]; j++)
							memcpy(c_tmp + ((c_t_y + j) * 2048 + c_t_x) * 4, atlas_frames[i] + j, c_f_w[i] * 4);
							*/

							for (j = 0; j < c_f_h[i]; j++)
							{
								for (k = 0; k < c_f_w[i]; k++)
								{
									c_tmp[((j + c_t_y) * 2048 * 4) + ((k + c_t_x) * 4)] = atlas_frames[(j * c_f_w[i] * 4) + (k * 4)];
									c_tmp[((j + c_t_y) * 2048 * 4) + ((k + c_t_x) * 4) + 1] = atlas_frames[(j * c_f_w[i] * 4) + (k * 4) + 1];
									c_tmp[((j + c_t_y) * 2048 * 4) + ((k + c_t_x) * 4) + 2] = atlas_frames[(j * c_f_w[i] * 4) + (k * 4) + 2];
									c_tmp[((j + c_t_y) * 2048 * 4) + ((k + c_t_x) * 4) + 3] = atlas_frames[(j * c_f_w[i] * 4) + (k * 4) + 3];
								}
							}

							c_t_x = c_t_w;

							if (c_an_w[l] < c_t_x)
								c_an_w[l] = c_t_x;

							free(atlas_frames);

							fclose(file);
						}

						c_atlas_normal[l] = malloc(c_an_w[l] * c_an_h[l] * 4);
						memset(c_atlas_normal[l], 0, c_an_w[l] * c_an_h[l] * 4);


						for (j = 0; j < c_a_h[l]; j++)
						{
							for (k = 0; k < c_a_w[l]; k++)
							{
								c_atlas_normal[l][(j * c_an_w[l] * 4) + (k * 4)] = c_tmp[(j * 2048 * 4) + (k * 4)];
								c_atlas_normal[l][(j * c_an_w[l] * 4) + (k * 4) + 1] = c_tmp[(j * 2048 * 4) + (k * 4) + 1];
								c_atlas_normal[l][(j * c_an_w[l] * 4) + (k * 4) + 2] = c_tmp[(j * 2048 * 4) + (k * 4) + 2];
								c_atlas_normal[l][(j * c_an_w[l] * 4) + (k * 4) + 3] = c_tmp[(j * 2048 * 4) + (k * 4) + 3];
							}
						}

						free(c_tmp);

						loading_pct = 100;

						m = 70;

						memset(loading_str, 0, sizeof(loading_str));

						for (k = 0; k < m; k++)
							strcat(loading_str, ".");

						printf("\rBuilding custom atlas normal %d [%d%%]%sDone\n", l, loading_pct, loading_str);

						c_a_normals[l] = 1;
						num_c_atlas_normals++;
						ca_normal[l] = 1;
					}
				}
			}

			if (strcmp(str[0], "FRAMEFILE") == NULL)
			{
				if (!mgg.num_frames)
				{
					printf("Error: number of frames not defined in %s before FRAMEFILE code\n", &animfile);
					fflush(stdin);
					getch();
					exit(1);
				}
				else
				{
					strtok(tmp, " ,\"");

					token = strtok(NULL, " ,\"");
					i = atoi(token);

					token = strtok(NULL, ",\"");

					if (strcmp(token, " ") == NULL)
						token = strtok(NULL, ",\"");

					if (token == NULL)
					{
						printf("Error: missing frame file after FRAMEFILE command\n");
						fflush(stdin);
						getch();
						exit(1);
					}

					strcpy(files[i], token);
					printf("File: \"%s\"\n", token);
				}
			}

			if (strcmp(str[0], "FRAMESFILES") == NULL)
			{
				if (!mgg.num_frames)
				{
					printf("Error: number of frames not defined in %s before FRAMESFILES code\n", &animfile);
					fflush(stdin);
					getch();
					exit(1);
				}
				else
				{
					strtok(tmp, " ,\"");

					token = strtok(NULL, " ,\"");
					i = atoi(token);

					token = strtok(NULL, " ,\"");
					j = atoi(token);

					for (k = i; k < j; k++)
					{
						token = strtok(NULL, ",\"");

						if (strcmp(token, " ") == NULL)
							token = strtok(NULL, ",\"");

						if (token == NULL)
						{
							printf("Error: missing frame file after FRAMEFILE command\n");
							fflush(stdin);
							getch();
							exit(1);
						}

						strcpy(files[k], token);
						printf("File: \"%s\"\n", token);
					}
				}
			}

			if (strcmp(str[0], "FRAMEFILE_NORMAL") == NULL)
			{
				if (!mgg.num_frames)
				{
					printf("Error: number of frames not defined in %s before FRAMEFILE_NORMAL code\n", &animfile);
					fflush(stdin);
					getch();
					exit(1);
				}
				else
				{
					strtok(tmp, " ,\"");

					token = strtok(NULL, " ,\"");
					i = atoi(token);

					token = strtok(NULL, ",\"");

					if (strcmp(token, " ") == NULL)
						token = strtok(NULL, ",\"");

					if (token == NULL)
					{
						printf("Error: missing frame file after FRAMEFILE command\n");
						fflush(stdin);
						getch();
						exit(1);
					}

					strcpy(files_n[i], token);
					printf("File: \"%s\"\n", token);

					normals[i] = 1;
				}
			}

			if (strcmp(str[0], "FRAMESFILES_NORMALS") == NULL)
			{
				if (!mgg.num_frames)
				{
					printf("Error: number of frames not defined in %s before FRAMESFILES_NORMALS code\n", &animfile);
					fflush(stdin);
					getch();
					exit(1);
				}
				else
				{
					strtok(tmp, " ,\"");

					token = strtok(NULL, " ,\"");
					i = atoi(token);

					token = strtok(NULL, " ,\"");
					j = atoi(token);

					for (k = i; k < j; k++)
					{
						token = strtok(NULL, ",\"");

						if (strcmp(token, " ") == NULL)
							token = strtok(NULL, ",\"");

						if (token == NULL)
						{
							printf("Error: missing frame file after FRAMEFILE command\n");
							fflush(stdin);
							getch();
							exit(1);
						}

						if (strcmp(token, "null") != NULL)
						{
							strcpy(files_n[k], token);
							printf("File: \"%s\"\n", token);
							normals[k] = 1;
						}
					}
				}
			}
			
			if(strcmp(str[0],"FRAMESALONE")==NULL)
			{
				value=atoi(str[1]);
				framealone[value]=1;
			}
			//else
			if(strcmp(str[0],"ANIMS")==NULL)
			{
				value=atoi(str[1]);
				mgg.num_animations=value;
				mga=(_MGGANIM*) malloc(value*sizeof(_MGGANIM));
			}
			//else
			if(strcmp(str[0],"ATLAS")==NULL)
			{
				value=atoi(str[1]);
				mgg.num_atlas += value;
			}
			//else
			if(strcmp(str[0],"MIPMAP")==NULL)
			{
				//value=atoi(str[1]);
				mgg.mipmap=1;

				if (mgg.mipmap)
					printf("Texture filter: Nearest\n");
				//else
					//printf("Texture filter: Linear\n");
			}
			//else
			//else
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
			//else
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
			//else
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

					posx[val[5]]=(posx[val[5]]*32768)/dimx[val[0]];
					posy[val[5]]=(posy[val[5]]*32768)/dimy[val[0]];
					sizex[val[5]]=(sizex[val[5]]*32768)/dimx[val[0]];
					sizey[val[5]]=(sizey[val[5]]*32768)/dimy[val[0]];
				}
			}
			//else
			if(strcmp(str[0],"FRAMESALONERANGE")==NULL)
			{
				sscanf(tmp,"%s %d %d", str[0], &val[0], &val[1]);

				for(i=val[0];i<=val[1];i++)
					framealone[i]=1;
			}
			//else
			if(strcmp(str[0],"FRAMEOFFSET")==NULL)
			{
				sscanf(tmp,"%s %d %d %d", str[0], &val[0], &val[1], &val[2]);

				offx[val[0]]=val[1];
				offy[val[0]]=val[2];

				printf("Frame: %d Offset x: %d y: %d\n", val[0], val[1], val[2]);
			}
			//else
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
			//else
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
					/*
					posx=(uint16*) realloc(posx,val[1]*sizeof(uint16));
					posy=(uint16*) realloc(posy,val[1]*sizeof(uint16));
					sizex=(uint16*) realloc(sizex,val[1]*sizeof(uint16));
					sizey=(uint16*) realloc(sizey,val[1]*sizeof(uint16));
					//imgatlas=(uint8*) realloc(imgatlas,val[1]*sizeof(uint8));
					dimx=(uint16*) realloc(dimx,val[1]*sizeof(uint16));
					dimy=(uint16*) realloc(dimy,val[1]*sizeof(uint16));
					*/
					num_img_in_atlas+=val[1];
				}
			}
			//else
			if(strcmp(str[0],"NORMALMAPRANGE")==NULL)
			{
				sscanf(tmp,"%s %d %d", str[0], &val[0], &val[1]);

				for(i=val[0];i<=val[1];i++)
					normals[i]=1;
			}
			//else
			if(strcmp(str[0],"NORMALMAP")==NULL)
			{
				sscanf(tmp,"%s %d", str[0], &val[0]);

				normals[val[0]]=1;
			}
		}
	}

	mgg.num_singletex=mgg.num_frames-num_img_in_atlas;
	mgg.num_texinatlas=num_img_in_atlas;

	fclose(file4);

	printf("Read %s, no errors found\n",&animfile);

	//file=tmpfile();

	printf("Creating MGG file...\n");

	if((file=fopen(FileName,"wb"))==NULL)
	{
		printf("Error while creating file\n"); 
		fflush(stdin);
		getch();
		exit(1);
	}

	rewind(file);
	//fseek(file,0,SEEK_SET);

	fwrite(header,21,1,file);

	//fwrite(&mgg,sizeof(_MGGFORMAT),1,file);

	fseek(file,sizeof(_MGGFORMAT),SEEK_CUR);
	//fflush(file);
	
	if (mgg.num_animations > 0)
	{
		fwrite(mga, sizeof(_MGGANIM), mgg.num_animations, file);
		fflush(file);
	}

	mgg.textures_offset=ftell(file);
	
	//totalsize=((sizeof(_MGGFORMAT)+512)+(MAX_FRAMES*sizeof(uint32)+512)+(512+(MAX_ANIMATIONS*sizeof(_MGGANIM))))+21;
	
	for (i = 0, j = 0, k = 0; i < mgg.num_frames + mgg.num_atlas; i++)
	{
		loading_pct = (i * 100) / mgg.num_frames + mgg.num_atlas;

		m = (i * 70) / mgg.num_frames + mgg.num_atlas;

		memset(loading_str, 0, sizeof(loading_str));

		for (l = 0; l < m; l++)
			strcat(loading_str, ".");

		printf("\rWriting frames [%d%%]%s", loading_pct, loading_str);

		if (i < num_c_atlas)
		{
			//printf("Writing builded atlasses...\n");

			//printf("Writing frame numbers from %d to %d...\n", c_a_f0[i], c_a_ff[i]);

			if (RLE)
			{
				c_tmp = rle_encode(c_atlas[i], c_a_w[i] * c_a_h[i] * 4, 4, &size_rle);
				MGIbuf = malloc(size_rle + 12);
			}
			else
				MGIbuf = malloc((c_a_w[i] * c_a_h[i] * 4) + 12);

			MGIbuf[0] = 'M';
			MGIbuf[1] = 'G';
			MGIbuf[2] = 'I';
			MGIbuf[3] = 4;
			MGIbuf[4] = RLE;
			MGIbuf[5] = c_a_w[i] >> 8;
			MGIbuf[6] = c_a_w[i] & 0xFF;
			MGIbuf[7] = c_a_h[i] >> 8;
			MGIbuf[8] = c_a_h[i] & 0xFF;
			MGIbuf[9] = 666 >> 8;
			MGIbuf[10] = 666 & 0xFF;
			MGIbuf[11] = 1;

			if (RLE)
			{
				memcpy(MGIbuf + 12, c_tmp, size_rle);
				free(c_tmp);
				size = size_rle;
			}
			else
			{
				memcpy(MGIbuf + 12, c_atlas[i], c_a_w[i] * c_a_h[i] * 4);
				size = c_a_w[i] * c_a_h[i] * 4;
			}

			framesize[i] = size;

			if (i == 0) fseek(file, mgg.textures_offset + 1, SEEK_SET);
			else fseek(file, frameoffset[i - 1] + 1, SEEK_SET);

			fwrite(MGIbuf, size, 1, file);
			free(MGIbuf);

			//fflush(file);

			frameoffset[i] = ftell(file);

			//printf("Wrote builded atlas\n");

			if (ca_normal[i])
			{
				//printf("Writing builded atlasses normals...\n");

					
				//printf("Writing frame normals numbers from %d to %d...\n", c_a_f0[i], c_a_ff[i]);

				if (RLE)
				{
					c_tmp = rle_encode(c_atlas_normal[i], c_an_w[i] * c_an_h[i] * 4, 4, &size_rle);
					MGIbuf = malloc(size_rle + 12);
				}
				else
					MGIbuf = malloc((c_an_w[i] * c_an_h[i] * 4) + 12);

				MGIbuf[0] = 'M';
				MGIbuf[1] = 'G';
				MGIbuf[2] = 'I';
				MGIbuf[3] = 4;
				MGIbuf[4] = RLE;
				MGIbuf[5] = c_an_w[i] >> 8;
				MGIbuf[6] = c_an_w[i] & 0xFF;
				MGIbuf[7] = c_an_h[i] >> 8;
				MGIbuf[8] = c_an_h[i] & 0xFF;
				MGIbuf[9] = 666 >> 8;
				MGIbuf[10] = 666 & 0xFF;
				MGIbuf[11] = 1;

				if (RLE)
				{
					memcpy(MGIbuf + 12, c_tmp, size_rle);
					free(c_tmp);
					size = size_rle;
				}
				else
				{
					memcpy(MGIbuf + 12, c_atlas_normal[i], c_an_w[i] * c_an_h[i] * 4);
					size = c_an_w[i] * c_an_h[i] * 4;
				}

				normalsize[i] = size;

				fseek(file, frameoffset[i] + 1, SEEK_SET);

				fwrite(MGIbuf, size, 1, file);
				free(MGIbuf);

				//fflush(file);

				frameoffset[i] = ftell(file);

				normals[i + mgg.num_frames] = 1;

				//printf("Wrote builded atlas normal\n");
			}
			k++;
			continue;
		}

		if (imgatlas[j] != -1)
		{
			j++;
			continue;
		}

		strcpy(framename2, framename);

		if (sequence)
		{
			if (j < 10) sprintf(filename, "//%s000%d.tga", fileframe, j); else
			if (j < 100) sprintf(filename, "//%s00%d.tga", fileframe, j); else
			if (j < 1000) sprintf(filename, "//%s0%d.tga", fileframe, j); else
			if (j < 10000) sprintf(filename, "//%s0%d.tga", fileframe, j); else
			if (j < 100000) sprintf(filename, "//%s%d.tga", fileframe, j);

			strcat(framename2, filename);
		}
		else
		{
		//	if (files[j] == NULL)
			//	continue;

			strcat(framename2, "\\");
			strcat(framename2, files[j]);
		}

		if((file2=fopen(framename2,"rb"))==NULL)
		{
			printf("\nError: could not read frame number %d\n",j);
			printf("Could not open file: %s\n", framename2);
			fflush(stdin);
			getch();
			exit(1);
			//continue;
		}
		else
		{
			//printf("Writing frame number %d ... ",i);
			//printf("File: %s\n", framename2);
			
			
			fseek(file2,0,SEEK_END);

			size=ftell(file2);
			rewind(file2);
			
			imagebuf = malloc(size);

			fread(imagebuf, size, 1, file2);

			MGIimagedata = stbi_load_from_memory(imagebuf, size, &width, &height, &MGIcolor, 0);

			if (!MGIimagedata)
			{
				printf("\nCould not read image file format: %s\n", framename2);
				error_str = stbi_failure_reason();
				printf("%s\n", error_str);
				fclose(file2);
				free(imagebuf);
				fflush(stdin);
				getch();
				j++;
				exit(1);
			}

			if (RLE)
			{
				c_tmp = rle_encode(MGIimagedata, width*height*MGIcolor, MGIcolor, &size_rle);
				MGIbuf = malloc(size_rle + 12);
			}
			else
				MGIbuf = malloc((width*height*MGIcolor) + 12);

			MGIbuf[0] = 'M';
			MGIbuf[1] = 'G';
			MGIbuf[2] = 'I';
			MGIbuf[3] = MGIcolor;
			MGIbuf[4] = RLE;
			MGIbuf[5] = width >> 8;
			MGIbuf[6] = width & 0xFF;
			MGIbuf[7] = height >> 8;
			MGIbuf[8] = height & 0xFF;
			MGIbuf[9] = j >> 8;
			MGIbuf[10] = j & 0xFF;
			MGIbuf[11] = 1;

			if (RLE)
			{
				memcpy(MGIbuf + 12, MGIimagedata, size_rle);
				size = size_rle;
				free(c_tmp);
			}
			else
			{
				memcpy(MGIbuf + 12, MGIimagedata, width*height*MGIcolor);
				size = width*height*MGIcolor;
			}

			framesize[k]=size;
	
			//buf=(void*) malloc(size);
			//fread(buf,size,1,file2);
			//rewind(file);
			
			if(k==0) fseek(file,mgg.textures_offset+1,SEEK_SET); 
			else
			{
				//totalsize+=(framesize[i-1]+2048);
				fseek(file,frameoffset[k-1]+1,SEEK_SET);
			}
			
			fwrite(MGIbuf,size,1,file);
			fclose(file2);
			free(MGIbuf);
			free(imagebuf);
			free(MGIimagedata);

			//fflush(file);

			frameoffset[k]=ftell(file);

			//printf("Wrote frame number %d\n",j);

			if(normals[j])
			{
				strcpy(framename2,framename);
				
				if (sequence)
				{
					if (j < 10) sprintf(filename, "//%s_n000%d.tga", fileframe, j); else
					if (j < 100) sprintf(filename, "//%s_n00%d.tga", fileframe, j); else
					if (j < 1000) sprintf(filename, "//%s_n0%d.tga", fileframe, j); else
					if (j < 10000) sprintf(filename, "//%s_n0%d.tga", fileframe, j); else
					if (j < 100000) sprintf(filename, "//%s_n%d.tga", fileframe, j);


					strcat(framename2, filename);
				}
				else
				{
					strcat(framename2, "\\");
					strcat(framename2, files_n[j]);
				}

				if ((file2 = fopen(framename2, "rb")) == NULL)
				{
					printf("\nError: could not read normal map frame number %d\n", j);
					printf("Could not open file: %s\n", framename2);
					fflush(stdin);
					getch();
					exit(1);
				}
				else
				{
					//printf("Writing normal mapping frame number %d ",i);
					//printf("File: %s\n", framename2);

					fseek(file2, 0, SEEK_END);

					size = ftell(file2);
					rewind(file2);

					imagebuf = malloc(size);

					fread(imagebuf, size, 1, file2);

					MGIimagedata = stbi_load_from_memory(imagebuf, size, &width, &height, &MGIcolor, 0);

					if (!MGIimagedata)
					{
						printf("\nCould not read image file format: %s\n", framename2);
						error_str = stbi_failure_reason();
						printf("%s\n", error_str);
						fclose(file2);
						free(imagebuf);
						fflush(stdin);
						getch();
						j++;
						exit(1);
					}

					if (RLE)
					{
						c_tmp = rle_encode(MGIimagedata, width*height*MGIcolor, MGIcolor, &size_rle);
						MGIbuf = malloc(size_rle + 12);
					}
					else
						MGIbuf = malloc((width*height*MGIcolor) + 12);

					MGIbuf[0] = 'M';
					MGIbuf[1] = 'G';
					MGIbuf[2] = 'I';
					MGIbuf[3] = MGIcolor;
					MGIbuf[4] = RLE;
					MGIbuf[5] = width >> 8;
					MGIbuf[6] = width & 0xFF;
					MGIbuf[7] = height >> 8;
					MGIbuf[8] = height & 0xFF;
					MGIbuf[9] = j >> 8;
					MGIbuf[10] = j & 0xFF;
					MGIbuf[11] = 1;

					if (RLE)
					{
						memcpy(MGIbuf + 12, c_tmp,size_rle);
						size = size_rle;
						free(c_tmp);
					}
					else
					{
						memcpy(MGIbuf + 12, MGIimagedata, width*height*MGIcolor);
						size = width*height*MGIcolor;
					}

					normalsize[k]=size;
			
					fseek(file,frameoffset[k]+1,SEEK_SET);
			
					fwrite(MGIbuf,size,1,file);
					fclose(file2);
					free(MGIbuf);
					free(MGIimagedata);
					free(imagebuf);

					//fflush(file);

					frameoffset[k]=ftell(file);

					nms++;

					//printf("Wrote normal mapping frame number %d\n",i);
				}
			}
		}
		j++;
		k++;
	}

	m = (i * 70) / mgg.num_frames + mgg.num_atlas + num_c_atlas;

	memset(loading_str, 0, sizeof(loading_str));

	for (l = 0; l < m; l++)
		strcat(loading_str, ".");

	printf("\rWriting frames [100%%]%sDone\n", loading_str);
	printf("Wrote %d frames, %d atlasses and %d normal maps\n", mgg.num_singletex, num_c_atlas, num_c_atlas_normals + nms);

	mgg.possize_offset=ftell(file);

	fwrite(posx,sizeof(uint16),mgg.num_frames,file);
	fwrite(posy, sizeof(uint16), mgg.num_frames, file);
	fwrite(sizex, sizeof(uint16), mgg.num_frames, file);
	fwrite(sizey, sizeof(uint16), mgg.num_frames, file);
	fwrite(imgatlas,sizeof(int8),mgg.num_frames,file);

	mgg.framesize_offset=ftell(file);

	fwrite(framesize,sizeof(uint32),mgg.num_singletex+mgg.num_atlas,file);

	fwrite(frameoffset,sizeof(uint32),mgg.num_singletex+mgg.num_atlas,file);

	fwrite(normals,sizeof(uint8),mgg.num_frames+mgg.num_atlas,file);

	fwrite(normalsize,sizeof(uint32),mgg.num_singletex+mgg.num_atlas,file);

	mgg.framealone_offset=ftell(file);

	fwrite(framealone,sizeof(uint32),mgg.num_frames,file);

	mgg.frameoffset_offset=ftell(file);

	fwrite(offx,sizeof(int16),mgg.num_frames,file);
	fwrite(offy,sizeof(int16),mgg.num_frames,file);

	//fflush(file);

	fseek(file,21,SEEK_SET);

	//fseek(file, 0, SEEK_SET);

	//fwrite(header, 21, 1, file);

	fwrite(&mgg,sizeof(_MGGFORMAT),1,file);
	
	printf("MGG file %s created sucessfully\n", FileName);
	printf("Press any key to continue...\n");

	fclose(file);
	//fclose(file3);

	//free(bufe);
	//free(buf2);
	if (mga)
		free(mga);

	free(offx);
	free(offy);

	fflush(stdin);
	getch();

	return 0;
}

