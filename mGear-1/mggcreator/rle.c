#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

unsigned char *rle_encode(unsigned char *data, int size, int color, int *rle_size)
{
	register int i, j;
	int k;
	unsigned char *buffer, r, g, b, a;

	assert(data);
	assert(rle_size);

	//Some extra room just for precaution
	buffer = malloc(size + (size / color));

	i = 0;
	k = 0;
	while (i < size + 1)
	{
		if (i == 0)
		{
			r = data[i];
			g = data[i + 1];
			b = data[i + 2];
			if (color == 4) a = data[i + 3];

			j = 1;

			if (color == 3) i += 3;
			if (color == 4) i += 4;

			continue;
		}
		else
		{
			if (((color == 3 && data[i] == r && data[i + 1] == g && data[i + 2] == b) || (color == 4 && data[i] == r && data[i + 1] == g && data[i + 2] == b && data[i + 3] == a))
				&& j < 255)
			{
				j++;

				if (color == 3) i += 3;
				if (color == 4) i += 4;
				continue;
			}
			else
			{
				buffer[k] = j;
				buffer[k + 1] = r;
				buffer[k + 2] = g;
				buffer[k + 3] = b;
				if (color == 4)
				{
					buffer[k + 4] = a;
					k += 5;
				}
				else
					k += 4;

				if ((color == 3 && i < size - 4) || (color == 4 && i < size - 5))
				{
					j = 1;
					r = data[i];
					g = data[i + 1];
					b = data[i + 2];
					if (color == 4) a = data[i + 3];

					if (color == 3) i += 3;
					if (color == 4) i += 4;
				}
				else
					break;
			}
		}
	}

	*rle_size = k;

	return buffer = realloc(buffer, k);

}

unsigned char *rle_decode(unsigned char *data, int rle_size, int color, int *size)
{
	register int i, j, k;
	int cur_size;
	unsigned char *buffer, r, g, b, a;

	assert(data);
	assert(size);

	buffer = malloc(256 * 256 * color);
	cur_size = 256 * 256 * color;

	i = 0;
	j = 0;
	k = 0;

	while (i < rle_size + 1)
	{
		if (k > cur_size - color)
			buffer = realloc(buffer, k + (256 * 256 * color));

		cur_size = k + (data[i] * color);

		for (j = 0; j < data[i]; j++)
		{
			buffer[k] = data[i + 1];
			buffer[k + 1] = data[i + 2];
			buffer[k + 2] = data[i + 3];
			if (color == 4)
			{
				buffer[k + 3] = data[i + 4];
				k += 4;
			}
			else
				k += 3;
		}

		i += color + 1;
	}

	*size = k;

	return buffer = realloc(buffer, k);
}