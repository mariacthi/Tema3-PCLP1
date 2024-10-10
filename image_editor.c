// Tudor Maria-Elena 311CA

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define NMAX 100
#define MAXV 256

typedef struct {
	char magicword[2];
	int maxvalue;
	int width;
	int height;
	unsigned int **mat;
	unsigned int **copy;
	int wcopy;
	int hcopy;
} image_t;

typedef struct {
	int x1;
	int x2;
	int y1;
	int y2;
} coordinates_t;

unsigned int clamp(double x, int minvalue, int maxvalue)
{
	if (x < minvalue)
		return minvalue;
	else if (x > maxvalue)
		return maxvalue;
	else
		return (unsigned int)x;
}

int verify(void *p)
{
	// verifies if pointer p is NULL
	if (!p) {
		fprintf(stderr, "Dynamic allocation failed");
		free(p);
		return 0;
	}
	return 1;
}

void verify2(void *p)
{
	// verifies if pointer p is NULL and stops the programm
	if (!p) {
		fprintf(stderr, "Dynamic allocation failed");
		free(p);
		exit(-1);
	}
}

void verify3(unsigned int **p)
{
	// verifies if double pointer p is NULL and stops the programm
	if (!p) {
		fprintf(stderr, "Dynamic allocation failed");
		free(p);
		exit(-1);
	}
}

void start(image_t *img)
{
	// allocating the matrixes and using 0 as a value
	// for the height and width that tells if any image
	// has been loaded in the memory

	// matrix in which image will be stored
	img->mat = (unsigned int **)malloc(sizeof(unsigned int *));
	verify3(img->mat);

	img->width = 0;
	img->height = 0;
	img->maxvalue = 0;

	// copy of the original matrix
	img->copy = (unsigned int **)malloc(sizeof(unsigned int *));
	verify3(img->copy);

	// height and width for the copy
	img->hcopy = 0;
	img->wcopy = 0;
}

void start2(coordinates_t *coord)
{
	// using -1 as a value that tells if a selection has been made

	coord->x1 = -1;
	coord->x2 = -1;
	coord->y1 = -1;
	coord->y2 = -1;
}

void gracefulexit(unsigned int **mat, int height)
{
	if (height != 0) {
		// freeing matrix' lines
		for (int i = 0; i < height; i++)
			free(mat[i]);
	}

	// freeing matrix
	free(mat);
}

int max(int x, int y)
{
	if (x > y)
		return x;
	else
		return y;
}

int min(int x, int y)
{
	if (x < y)
		return x;
	else
		return y;
}

int read_coordinates(char *str, coordinates_t *coord)
{
	// making a copy of the original string,
	// to check if the SELECT command is valid
	char *copy = (char *)malloc(NMAX * sizeof(char));
	strcpy(copy, str);

	// based on the fact that there are four integer numbers
	// that have to be read (not something else, not less/more than four)
	int contor = 0;
	char *token = strtok(copy, " ");

	while (token) {
		for (int i = 0; i < (int)strlen(token); i++) {
			// checking that it's a number
			if (!((token[i] >= '0' && token[i] <= '9') || token[i] == '-')) {
				free(copy);
				return 0;
			}
		}
		contor++;
		token = strtok(NULL, " ");
	}

	if (contor != 4) {
		// if there aren't exactly four numbers
		free(copy);
		return 0;
	}

	sscanf(str, "%d %d %d %d", &coord->x1, &coord->y1, &coord->x2, &coord->y2);
	free(copy);
	return 1;
}

unsigned int **read_ascii(FILE *in, image_t *img)
{
	unsigned int **mat = (unsigned int **)malloc(img->height *
						sizeof(unsigned int *));
	verify3(mat);

	unsigned int value;

	// if the format is P3 then for one pixel, three values are read
	if (img->magicword[1] == '3')
		(img->width) *= 3;

	for (int i = 0; i < img->height; i++) {
		// dynamic allocation for lines of the matrix
		mat[i] = (unsigned int *)malloc(img->width * sizeof(unsigned int));
		verify2(mat[i]);

		for (int j = 0; j < img->width; j++) {
			// reading every number and storing it in the matrix
			fscanf(in, "%u", &value);
			mat[i][j] = value;
		}
	}

	return mat;
}

unsigned int **read_binary(FILE *in, image_t *img, long num_bytes)
{
	// positioning the cursor where the matrix starts
	fseek(in, num_bytes, SEEK_SET);

	unsigned int **mat = (unsigned int **)malloc(img->height *
						sizeof(unsigned int *));
	verify3(mat);

	// if the format is P6 then for one pixel, three values are read
	if (img->magicword[1] == '6')
		(img->width) *= 3;

	for (int i = 0; i < img->height; i++) {
		// dynamic allocation for lines of the matrix
		mat[i] = (unsigned int *)malloc(img->width * sizeof(unsigned int));

		verify2(mat[i]);
		unsigned char *buf = (unsigned char *)malloc(img->width *
							sizeof(unsigned char));
		verify2(buf);

		// reading one line at a time (every number is one byte,
		// so it's an array of unsigned char)
		fread(buf, sizeof(unsigned char), img->width, in);

		for (int j = 0; j < img->width; j++) {
			// storing every number read in the matrix

			unsigned int value = (unsigned int)buf[j];
			mat[i][j] = value;
		}
		free(buf);
	}

	return mat;
}

unsigned int **change(unsigned int **mat1, unsigned int **mat2, int *h1,
					  int *w1, int h2, int w2)
{
	// changes the first matrix and its dimensions with the second matrix

	// freeing the matrix that is going to be changed
	if (*h1 == 0)
		free(mat1);
	else
		gracefulexit(mat1, *h1);

	// dynamic allocation for the new matrix
	unsigned int **new = (unsigned int **)malloc(h2 * sizeof(unsigned int *));
	verify3(new);

	for (int i = 0; i < h2; i++) {
		new[i] = (unsigned int *)malloc(w2 * sizeof(unsigned int));
		verify2(new[i]);

		//the new matrix will have the values from the second matrix
		for (int j = 0; j < w2; j++)
			new[i][j] = mat2[i][j];
	}
	*h1 = h2;
	*w1 = w2;
	return new;
}

void change_dim(int *w1, int *h1, coordinates_t coord, char magicword)
{
	// changing the old dimensions with the ones that the matrix has after
	// the selection
	int w2 = coord.x1 - coord.x2;
	int h2 = coord.y1 - coord.y2;

	if (magicword == '3' || magicword == '6')
		w2 *= 3;

	// checking that new dimensions are positive
	if (h2 < 0)
		h2 *= (-1);
	if (w2 < 0)
		w2 *= (-1);

	// changing old dimensions with new dimensions
	*w1 = w2;
	*h1 = h2;
}

void read_dim_com(FILE *in, image_t *img)
{
	int stop = 1, contor = 0;
	char ch;

	// checking for commented lines and ignoring them
	while (stop != 0) {
		fscanf(in, "%c", &ch);

		//checking to see if character is new line
		if (ch == '\n') {
			continue;
		} else if (ch == '#') {
			// if the line starts with #, then it's a commented line, so
			// I keep reading until the end of the line
			while (ch != '\n')
				fscanf(in, "%c", &ch);
		} else {
			if (contor == 0) {
				// going back for what was read in ch
				fseek(in, -1, SEEK_CUR);
				fscanf(in, "%d %d", &img->width, &img->height);
				contor++;
			} else if (contor == 1) {
				// going back for what was read in ch
				fseek(in, -1, SEEK_CUR);
				fscanf(in, "%d", &img->maxvalue);
				contor++;
			} else {
				fseek(in, -1, SEEK_CUR);
				stop = 0;
			}
		}
	}
}

int load(char *name, image_t *img)
{
	// freeing the old matrix
	gracefulexit(img->mat, img->height);

	// opening the file in mode "readtext" first
	FILE *in = fopen(name, "rt");
	if (!in) {
		free(in);
		return 0;
	}

	// reading the magic word
	fscanf(in, "%c%c", &img->magicword[0], &img->magicword[1]);

	// checking to see if the format is valid
	if (img->magicword[0] != 'P')
		return 0;

	read_dim_com(in, img);

	long num_bytes = ftell(in);
	// figuring out what format the image is in
	switch (img->magicword[1]) {
	case('2'):
	img->mat = read_ascii(in, img);
	break;
	case('3'):
	img->mat = read_ascii(in, img);
	break;
	case('5'):
	fclose(in);
	// opening the file to read as binary
	in = fopen(name, "rb");
	if (verify(in) == 0)
		return 0;
	img->mat = read_binary(in, img, num_bytes);
	break;
	case('6'):
	fclose(in);
	// opening the file to read as binary
	in = fopen(name, "rb");
	if (verify(in) == 0)
		return 0;
	img->mat = read_binary(in, img, num_bytes);
	break;
	default:
		return 0;
	}

	fclose(in);
	return 1;
}

int ver_coord(int h, int w, coordinates_t coord, char magicword)
{
	if (magicword == '3' || magicword == '6')
		w /= 3;

	// making sure the coordinates are valid
	int contor = 1;
	if (coord.x1 > w || coord.x1 < 0)
		contor = 0;
	else if (coord.x2 > w || coord.x2 < 0)
		contor = 0;
	else if (coord.y1 > h || coord.y1 < 0)
		contor = 0;
	else if (coord.y2 > h || coord.y2 < 0)
		contor = 0;
	else if (coord.x1 == coord.x2)
		contor = 0;
	else if (coord.y1 == coord.y2)
		contor = 0;

	if (contor == 0) {
		printf("Invalid set of coordinates\n");
		return 0;
	} else {
		return 1;
	}
}

unsigned int **select_coord(unsigned int **origmat, coordinates_t coord,
							char magicw)
{
	// making sure I start from smallest number to biggest number
	int fc = min(coord.x1, coord.x2); // first column
	int lc = max(coord.x1, coord.x2); // last column
	int fl = min(coord.y1, coord.y2); // first line
	int ll = max(coord.y1, coord.y2); // last line

	if (magicw == '3' || magicw == '6') {
		fc *= 3;
		lc *= 3;
	}

	// dynamic allocation for the new matrix
	unsigned int **newmat = (unsigned int **)malloc((ll - fl) *
							sizeof(unsigned int *));
	verify3(newmat);

	for (int i = fl; i < ll; i++) {
		newmat[i - fl] = (unsigned int *)malloc((lc - fc) *
						  sizeof(unsigned int));
		verify2(newmat[i - fl]);

		for (int j = fc; j < lc; j++)
			// filling in the values from the old matrix
			newmat[i - fl][j - fc] = origmat[i][j];
	}

	return newmat;
}

void message_select(coordinates_t *coord)
{
	int fc = min(coord->x1, coord->x2); // first column
	int lc = max(coord->x1, coord->x2); // last column
	int fl = min(coord->y1, coord->y2); // first line
	int ll = max(coord->y1, coord->y2); // last line

	printf("Selected %d %d %d %d\n", fc, fl, lc, ll);
}

int validhistogram(char *command)
{
	char *copy = (char *)malloc(NMAX * sizeof(char));

	// jumping over "HISTOGRAM " (9 letters + space = 10)
	strcpy(copy, command + 10);

	// the command needs two integer numbers
	int contor = 0;
	char *token = strtok(copy, " ");

	while (token) {
		for (int i = 0; i < (int)strlen(token); i++) {
			// checking that it's a number
			if (!((token[i] >= '0' && token[i] <= '9') || token[i] == '-')) {
				free(copy);
				return 0;
			}
		}
		contor++;
		token = strtok(NULL, " ");
	}

	if (contor != 2) {
		// if there aren't exactly two numbers read
		free(copy);
		return 0;
	}

	free(copy);
	return 1;
}

void histogram(char *c, unsigned int **mat, int height, int width)
{
	int x, y;
	if (sscanf(c + 10, "%d %d", &x, &y) != 2)
		exit(-1);

	//using a frequency array
	long *vf = (long *)calloc(MAXV, sizeof(long));

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++)
			vf[mat[i][j]]++;
	}

	// calculating the interval the bins cover
	int interval = (MAXV + 1) / y;
	long *s = (long *)calloc(y, sizeof(long));
	long max = 0;

	for (int i = 0; i < y; i++) {
		// calculating the number of apparitions for every bin
		for (int j = 0; j < interval; j++)
			s[i] += vf[i * interval + j];

		// finding the maximum number of apparitions
		if (s[i] > max)
			max = s[i];
	}

	for (int i = 0; i < y; i++) {
		double star = (double)s[i] / (double)max * (double)x;
		star = trunc(star);
		printf("%d\t|\t", (int)star);

		for (int i = 0; i < (int)star; i++)
			printf("*");
		printf("\n");
	}

	free(s);
	free(vf);
}

void equalize(unsigned int **origmat, int h, int w)
{
	// using a frequency array
	unsigned int *vf = (unsigned int *)calloc(MAXV, sizeof(unsigned int));

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++)
			vf[origmat[i][j]]++;
	}

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			// calculating the number of apparitions
			double s = 0;
			for (unsigned int k = 0; k <= origmat[i][j]; k++)
				s += (double)vf[k];

			double value = (double)(MAXV - 1) * s;
			value /= (double)(w * h);
			value = clamp(value, 0, MAXV - 1);
			origmat[i][j] = (unsigned int)round(value);
		}
	}
	printf("Equalize done\n");
	free(vf);
}

void write_ascii(char *file, image_t *img)
{
	FILE *in = fopen(file, "wt");
	if (!in) {
		printf("Cannot open %s", file);
		free(in);
		exit(-1);
	}

	// printing the magicword, the dimensions, and the maximum value

	if (img->magicword[1] == '2' || img->magicword[1] == '5')
		// since it's an ascii file, the magic word will be P2 for PGM
		fprintf(in, "P2\n");
	else
		//the magic word will be P3 for PPM
		fprintf(in, "P3\n");

	if (img->magicword[1] == '3' || img->magicword[1] == '6')
		// the matrix width is "triple" for color pictures
		fprintf(in, "%d %d\n", img->wcopy / 3, img->hcopy);
	else
		fprintf(in, "%d %d\n", img->wcopy, img->hcopy);

	// writing the full matrix in the file
	fprintf(in, "%d\n", img->maxvalue);
	for (int i = 0; i < img->hcopy; i++) {
		for (int j = 0; j < img->wcopy; j++)
			fprintf(in, "%u ", img->copy[i][j]);

		fprintf(in, "\n");
	}

	fclose(in);
}

void write_binary(char *file, image_t *img)
{
	FILE *in = fopen(file, "wt+b");
	if (!in) {
		printf("Cannot open %s", file);
		free(in);
		exit(-1);
	}

	// printing the magicword, the dimensions, and the maximum value

	if (img->magicword[1] == '2' || img->magicword[1] == '5')
		// since it's a binary file, the magic word will be P2 for PGM
		fprintf(in, "P5\n");
	else
		//the magic word will be P6 for PPM
		fprintf(in, "P6\n");

	if (img->magicword[1] == '3' || img->magicword[1] == '6')
		// the matrix width is "triple" for color pictures
		fprintf(in, "%d %d\n", img->wcopy / 3, img->hcopy);
	else
		fprintf(in, "%d %d\n", img->wcopy, img->hcopy);

	fprintf(in, "%d\n", img->maxvalue);

	// writing the full matrix in the file
	for (int i = 0; i < img->hcopy; i++) {
		unsigned char *buf = (unsigned char *)malloc(img->wcopy *
							  sizeof(unsigned char));
		verify2(buf);

		for (int j = 0; j < img->wcopy; j++) {
			buf[j] = (unsigned char)img->copy[i][j];
			//fwrite(&value, sizeof(unsigned char), 1, in);
		}
		fwrite(buf, sizeof(unsigned char), img->wcopy, in);
		free(buf);
	}

	fclose(in);
}

void edgemat(int mat[3][3])
{
	// filling in the values for the
	// edge detection kernel
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			mat[i][j] = -1;

	mat[1][1] = 8;
}

void sharpenmat(int mat[3][3])
{
	// filling in the values for the
	// sharpen kernel
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			mat[i][j] = -1;

	mat[0][0] = 0;
	mat[2][0] = 0;
	mat[0][2] = 0;
	mat[2][2] = 0;
	mat[1][1] = 5;
}

void blurmat(int mat[3][3])
{
	// filling in the values for the
	// blur kernel
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			mat[i][j] = 1;
}

void gaussian_blurmat(int mat[3][3])
{
	// filling in the values for the
	// gaussian blur kernel
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			mat[i][j] = 1;

	mat[0][1] = 2;
	mat[1][0] = 2;
	mat[1][2] = 2;
	mat[2][1] = 2;
	mat[1][1] = 4;
}

void margins(int *fc, int *lc, int *fl, int *ll, image_t *img)
{
	// if the selection touches the margins
	if (*fc == 0)
		*fc = 3;
	if (*fl == 0)
		*fl = 1;
	if (*lc == img->wcopy)
		*lc -= 3;
	if (*ll == img->hcopy)
		(*ll)--;
}

void apply(image_t *img, coordinates_t *coord, int kernel[3][3], double num)
{
	int fc, lc, fl, ll;
	if (coord->x1 == -1) {
		// there has been no selection made, so the full matrix
		// will  be changed, except the edges
		fc = 3; // first column
		lc = img->wcopy - 3;// last column
		fl = 1; // first line
		ll = img->hcopy - 1; // last line
	} else {
		fc = min(coord->x1, coord->x2); // first column
		lc = max(coord->x1, coord->x2); // last column
		fl = min(coord->y1, coord->y2); // first line
		ll = max(coord->y1, coord->y2); // last line
		fc *= 3;
		lc *= 3;
		margins(&fc, &lc, &fl, &ll, img);
	}

	// dynamic allocation for the new matrix
	// where the new value of the pixels will be stored
	unsigned int **newmat = (unsigned int **)malloc((ll - fl) *
							sizeof(unsigned int *));
	verify3(newmat);

	for (int i = fl; i < ll; i++) {
		newmat[i - fl] = (unsigned int *)malloc((lc - fc) *
						  sizeof(unsigned int));
		verify2(newmat[i - fl]);
	}

	// change the selected pixels (or all) with the given algorithm
	for (int i = fl; i < ll; i++) {
		for (int j = fc; j < lc; j++) {
			// img->copy[i][j] is in the middle of the
			// group of 3x3 pixels

			double s = 0;
			// coordinates for the kernel
			int a = 0, b = 0;

			// for each pixel, we take the values from its respective color
			for (int k = i - 1; k <= i + 1; k++) {
				for (int l = j - 3; l <= j + 3; l += 3) {
					s += ((double)kernel[a][b] * (double)img->copy[k][l]);
					b++;
					if (b == 3) {
						b = 0;
						a++;
					}
				}
			}
			if (num != 1)
				s /= num;

			s = clamp(s, 0, MAXV - 1);
			newmat[i - fl][j - fc] = (unsigned int)s;
		}
	}

	for (int i = fl; i < ll; i++) {
		for (int j = fc; j < lc; j++) {
			// filling in the new values in the original matrix
			img->copy[i][j] = newmat[i - fl][j - fc];
		}
	}
	gracefulexit(newmat, ll - fl);

	if (coord->x1 != -1) {
		// selecting again on the matrix with the new values;
		gracefulexit(img->mat, img->height);
		img->mat = select_coord(img->copy, *coord, img->magicword[1]);
	} else {
		img->mat = change(img->mat, img->copy, &img->height,
						  &img->width, img->hcopy, img->wcopy);
	}
}

int verifyangle(char *c)
{
	// reading the value of the angle from the command
	int angle;
	if (sscanf(c + 7, "%d", &angle) != 1)
		exit(-1);

	// the accespted values for the angle of rotation
	int values[9] = {0, 90, -90, 180, -180, 270, -270, 360, -360};

	for (int i = 0; i < 9; i++) {
		if (values[i] == angle)
			return angle;
	}

	printf("Unsupported rotation angle\n");
	return -1;
}

int verifsquare(coordinates_t coord)
{
	// verifies that the selection is a square matrix
	int fc = min(coord.x1, coord.x2); // first column
	int lc = max(coord.x1, coord.x2); // last column
	int fl = min(coord.y1, coord.y2); // first line
	int ll = max(coord.y1, coord.y2); // last line

	if (lc - fc == ll - fl)
		return 1;

	printf("The selection must be square\n");
	return 0;
}

void rotatepgm(int angle, image_t *img, unsigned int **newmat,
			   int hnewmat, int wnewmat)
{
	if (angle == 90 || angle == -270) {
		for (int i = 0; i < hnewmat; i++) {
			for (int j = 0; j < wnewmat; j++) {
				// the lines from the new matrix are the columns from the
				// old one and the columns from the new matrix are
				// complementary with the lines from the old one
				// (their sum is img->height - 1)
				newmat[i][j] = img->mat[(img->height - 1) - j][i];
			}
		}
	} else if (angle == 180 || angle == -180) {
		for (int i = 0; i < hnewmat; i++) {
			for (int j = 0; j < wnewmat; j++) {
				// the lines and columns from the new ones are complementary
				// to the lines and columns from the old matrix

				// strictly for coding style purposes
				int h = img->height - 1;
				int w = img->width - 1;

				newmat[i][j] = img->mat[h - i][w - j];
			}
		}
	} else if (angle == 270 || angle == -90) {
		for (int i = 0; i < hnewmat; i++) {
			for (int j = 0; j < wnewmat; j++) {
				// the columns from the new matrix are the lines from the old
				// one and the lines from the new matrix are complementary with
				// the columns from the old one (their sum is img->width - 1)
				newmat[i][j] = img->mat[j][(img->width - 1) - i];
			}
		}
	}
}

void rotateppm(int angle, image_t *img, unsigned int **newmat,
			   int hnewmat, int wnewmat)
{
	// since there is a group of 3 pixels that has to be moved in the same
	// order, the formulas from earlier (rotatepgm) had to be adapted
	// (I literally just tried a lot of combinations on paper and this is
	// what I came up with)
	if (angle == 90 || angle == -270) {
		for (int i = 0; i < hnewmat; i++) {
			for (int j = 0; j < wnewmat; j++) {
				int h = img->height - 1;

				newmat[i][j] = img->mat[h - j / 3][i * 3 + j % 3];
			}
		}
	} else if (angle == 180 || angle == -180) {
		for (int i = 0; i < hnewmat; i++) {
			for (int j = 0; j < wnewmat; j++) {
				int w = img->width;
				int num = j / 3 * 3 + (3 - j % 3);

				newmat[i][j] = img->mat[img->height - 1 - i][w - num];
			}
		}
	} else if (angle == 270 || angle == -90) {
		for (int i = 0; i < hnewmat; i++) {
			for (int j = 0; j < wnewmat; j++) {
				int w = img->width;
				newmat[i][j] = img->mat[j / 3][w - i * 3 - (3 - j % 3)];
			}
		}
	}
}

void rotate(int angle, image_t *img, coordinates_t *coord)
{
	if (angle == 360 || angle == -360 || angle == 0)
		// the matrix will not change
		return;

	int fc, lc, fl, ll;
	if (coord->x1 == -1) {
		// there has been no selection made, so
		// the full matrix will  be changed, except
		// the edges
		fc = 0; // first column
		lc = img->wcopy;// last column
		fl = 0; // first line
		ll = img->hcopy; // last line
	} else {
		fc = min(coord->x1, coord->x2); // first column
		lc = max(coord->x1, coord->x2); // last column
		fl = min(coord->y1, coord->y2); // first line
		ll = max(coord->y1, coord->y2); // last line

		if (img->magicword[1] == '3' || img->magicword[1] == '6') {
			lc *= 3;
			fc *= 3;
		}
	}

	int hnewmat = ll - fl, wnewmat = lc - fc;

	if (angle == 90 || angle == (-270) || angle == 270 || angle == (-90)) {
		if (img->magicword[1] == '3' || img->magicword[1] == '6') {
			hnewmat = (lc - fc) / 3;
			wnewmat = (ll - fl) * 3;
		} else {
			// the dimensions of the matrix will be interchanged
			hnewmat = lc - fc;
			wnewmat = ll - fl;
		}
	}

	// dynamic allocation for the new matrix
	unsigned int **newmat = (unsigned int **)malloc(hnewmat *
							sizeof(unsigned int *));
	verify3(newmat);
	for (int i = 0; i < hnewmat; i++) {
		newmat[i] = (unsigned int *)malloc(wnewmat * sizeof(unsigned int));
		verify2(newmat[i]);
	}

	if (img->magicword[1] == '2' || img->magicword[1] == '5')
		rotatepgm(angle, img, newmat, hnewmat, wnewmat);
	else
		rotateppm(angle, img, newmat, hnewmat, wnewmat);

	if (coord->x1 != -1) {
		for (int i = fl; i < ll; i++) {
			for (int j = fc; j < lc; j++) {
				// filling in the new values in the original matrix
				img->copy[i][j] = newmat[i - fl][j - fc];
			}
		}

		// selecting again on the matrix with the new values;
		gracefulexit(img->mat, img->height);
		img->mat = select_coord(img->copy, *coord, img->magicword[1]);

	} else {
		// if there is no selection on the image, then the dimensions might
		// change so I'm replacing the old matrixes with the values form the
		// new one
		img->copy = change(img->copy, newmat, &img->hcopy, &img->wcopy,
						   hnewmat, wnewmat);

		img->mat = change(img->mat, img->copy, &img->height,
						  &img->width, img->hcopy, img->wcopy);
	}

	gracefulexit(newmat, hnewmat);
}

int load_command(char *command, image_t *img, coordinates_t *coord)
{
	if (strcmp(command, "LOAD") == 0)
		// if it is the only word in the command
		return 2;

	char copy[NMAX];
	// jumping over "LOAD " (4 letters + space = 5)
	strcpy(copy, command + 5);
	if (strchr(copy, ' ') != 0)
		// if there is more than one word after LOAD
		return 2;

	char filename[NMAX];
	strcpy(filename, copy);
	int l = load(filename, img);

	if (l == 1) {
		// copying the original matrix
		img->copy = change(img->copy, img->mat, &img->hcopy, &img->wcopy,
						   img->height, img->width);

		printf("Loaded %s\n", filename);

		// no selection has been made yet
		start2(coord);
	} else {
		printf("Failed to load %s\n", filename);
		gracefulexit(img->copy, img->hcopy);
		start(img);
		start2(coord);
	}

	return 0;
}

int select_command(char *command, image_t *img, coordinates_t *coord)
{
	char copy[NMAX];
	// jumping over "SELECT " (6 letters + space = 7)
	strcpy(copy, command + 7);

	// selecting the second word
	char *word = strtok(copy, " ");

	if (strcmp(word, "ALL") == 0) {
		if (strcmp(command, "SELECT ALL") != 0)
			// if there are more words after SELECT ALL
			return 2;

		if (img->width == 0) {
			printf("No image loaded\n");
			return 0;
		}

		// SELECT ALL
		// using the copy to select the whole matrix
		img->mat = change(img->mat, img->copy, &img->height, &img->width,
						  img->hcopy, img->wcopy);
		start2(coord);
		printf("Selected ALL\n");
		return 0;
	}

	// if it's not a SELECT ALL command, it might be a SELECT command
	if (img->width == 0) {
		printf("No image loaded\n");
		return 0;
	}

	// extracting the coordinates from the command
	coordinates_t cpycoord;
	start2(&cpycoord);
	int r = read_coordinates(command + 7, &cpycoord);
	if (r == 0)
		return 2;

	if (ver_coord(img->hcopy, img->wcopy, cpycoord, img->magicword[1]) == 1) {
		// if the coordinates are valid,
		// free the matrix as it's going to be replaced with
		// the given selection
		gracefulexit(img->mat, img->height);

		// it is safe to change the old coordinates with th new ones
		read_coordinates(command + 7, coord);

		// selecting the coordinates
		img->mat = select_coord(img->copy, *coord, img->magicword[1]);
		change_dim(&img->width, &img->height, *coord, img->magicword[1]);

		message_select(coord);

		//checking if the coordinates are actually
		// selecting the full matrix
		int w = coord->x2 - coord->x1;
		int h = coord->y2 - coord->y1;
		if (w < 0)
			w *= (-1);
		if (h < 0)
			h *= (-1);
		if (img->magicword[1] == '3' || img->magicword[1] == '6')
			w *= 3;
		if (w == img->wcopy && h == img->hcopy)
			start2(coord);
	}
	return 0;
}

int exit_command(char *command, image_t *img)
{
	if (strcmp(command, "EXIT") != 0)
		// if there are more words after EXIT
		return 2;

	if (img->height == 0)
		printf("No image loaded\n");

	gracefulexit(img->mat, img->height);
	gracefulexit(img->copy, img->hcopy);
	return 1;
}

int crop_command(char *command, image_t *img, coordinates_t *coord)
{
	if (strcmp(command, "CROP") != 0)
		// if there are more words after CROP
		return 2;

	if (img->width == 0) {
		printf("No image loaded\n");
	} else {
		img->copy = change(img->copy, img->mat, &img->hcopy, &img->wcopy,
						   img->height, img->width);
		start2(coord);
		printf("Image cropped\n");
	}

	return 0;
}

int histogram_command(char *command, image_t *img)
{
	if (img->width == 0) {
		printf("No image loaded\n");
		return 0;
	}

	if (strcmp(command, "HISTOGRAM") == 0)
		// if it is the only word in the command
		return 2;

	int v = validhistogram(command);
	if (v == 0)
		return 2;

	if (img->magicword[1] == '3' || img->magicword[1] == '6')
		printf("Black and white image needed\n");
	else
		histogram(command, img->copy, img->hcopy, img->wcopy);

	return 0;
}

int equalize_command(char *command, image_t *img, coordinates_t *coord)
{
	if (strcmp(command, "EQUALIZE") != 0)
		// if there are more words after EQUALIZE
		return 2;

	if (img->width == 0) {
		printf("No image loaded\n");
	} else if (img->magicword[1] == '3' || img->magicword[1] == '6') {
		printf("Black and white image needed\n");
	} else {
		equalize(img->copy, img->hcopy, img->wcopy);

		if (coord->x1 != -1) {
			// selecting again on the matrix with the new values;
			gracefulexit(img->mat, img->height);
			img->mat = select_coord(img->copy, *coord, img->magicword[1]);
		}
	}

	return 0;
}

int save_command(char *command, image_t *img)
{
	if (strcmp(command, "SAVE") == 0)
		// if it is the only word in the command
		return 2;

	if (img->width == 0) {
		printf("No image loaded\n");
		return 0;
	}

	// jumping over "SAVE " (4 letters + space = 5)
	char *copy = (char *)malloc(NMAX * sizeof(char));
	strcpy(copy, command + 5);

	// the filename is the next word after SAVE
	// (it can be the last word in the command or followed
	// by the word "ascii")
	char *file = strtok(copy, " \0");

	if (strcmp(command + 5 + strlen(file) + 1, "ascii") == 0) {
		if (strlen(command) != (5 + strlen(file) + 6)) {
		//  "SAVE " = 4 letters + 1 space = 5
		// " ascii" = 1 space + 5 letters = 6
		// if there are more words after "ascii" in the command
			free(copy);
			return 2;
		}

		write_ascii(file, img);
		printf("Saved %s\n", file);
	} else {
		if (strchr(copy, ' ') != 0) {
			// if there are more words after the filename
			free(copy);
			return 2;
		}

		write_binary(file, img);
		printf("Saved %s\n", file);
	}

	free(copy);
	return 0;
}

int apply_command(char *command, image_t *img, coordinates_t *coord)
{
	if (img->width == 0) {
		printf("No image loaded\n");
		return 0;
	}

	if (strcmp(command, "APPLY") == 0)
		// if APPLY is the only word in the command
		return 2;

	if (img->magicword[1] == '2' || img->magicword[1] == '5') {
		printf("Easy, Charlie Chaplin\n");
		return 0;
	}

	// jumping over "APPLY " (5 letters + space = 6)
	char *copy = (char *)malloc(NMAX * sizeof(char));
	//char copy[NMAX];
	strcpy(copy, command + 6);

	int kernel[3][3];
	int number;
	if (img->hcopy >= 3 && img->wcopy >= 3) {
		int ok = 0;
		if (strcmp(copy, "EDGE") == 0) {
			// using the kernel for EDGE
			edgemat(kernel);
			number = 1;
		} else if (strcmp(copy, "SHARPEN") == 0) {
			// using the kernel for SHARPEN
			sharpenmat(kernel);
			number = 1;
		} else if (strcmp(copy, "BLUR") == 0) {
			// using the kernel for BLUR
			blurmat(kernel);
			number = 9;
		} else if (strcmp(copy, "GAUSSIAN_BLUR") == 0) {
			// using the kernel for GAUSSIAN_BLUR
			gaussian_blurmat(kernel);
			number = 16;
		} else {
			ok = 1;
			printf("APPLY parameter invalid\n");
		}

		if (ok == 0) {
			apply(img, coord, kernel, number);
			printf("%s done\n", command);
		}
	} else {
		printf("APPLY parameter invalid\n");
	}

	free(copy);
	return 0;
}

int rotate_command(char *command, image_t *img, coordinates_t *coord)
{
	if (strcmp(command, "ROTATE") == 0)
		// if it is the only word in the command
		return 2;

	if (img->width == 0) {
		printf("No image loaded\n");
		return 0;
	}

	int verif = 1;
	int angle = verifyangle(command);

	if (angle != -1) {
		if (coord->x1 != -1)
			// if a selection has been made
			// check that it is square
			verif = verifsquare(*coord);

		if (verif == 1) {
			rotate(angle, img, coord);
			printf("Rotated %d\n", angle);
		}
	}
	return 0;
}

int figure_command(char *command, image_t *img, coordinates_t *coord)
{
	// this function checks the first word of a command
	// and then uses other function to return 0, 1 or 2

	// if this function returns 2, then it's an invalid command,
	// if it's 1, then the program stops as it received a valid EXIT
	// command and if it's 0, then the program goes on normally

	// making a copy to work without modifying the
	// string with the command
	char copy[NMAX];
	strcpy(copy, command);

	// finding the first word in the command
	char *word = strtok(copy, " ");

	// comparing the first word with the known commands
	if (strcmp(word, "LOAD") == 0)
		return load_command(command, img, coord);

	if (strcmp(word, "SELECT") == 0)
		return select_command(command, img, coord);

	if (strcmp(word, "EXIT") == 0)
		return exit_command(command, img);

	if (strcmp(word, "CROP") == 0)
		return crop_command(command, img, coord);

	if (strcmp(word, "HISTOGRAM") == 0)
		return histogram_command(command, img);

	if (strcmp(word, "EQUALIZE") == 0)
		return equalize_command(command, img, coord);

	if (strcmp(word, "SAVE") == 0)
		return save_command(command, img);

	if (strcmp(word, "APPLY") == 0)
		return apply_command(command, img, coord);

	if (strcmp(word, "ROTATE") == 0)
		return rotate_command(command, img, coord);

	printf("Invalid command\n");
	return 0;
}

int main(void)
{
	image_t img;
	start(&img);

	char command[NMAX] = {'0'};

	// coordinates for the SELECT command
	coordinates_t coord;
	start2(&coord);

	while (1) {
		// read line by line to see commands
		fgets(command, NMAX, stdin);

		int len = strlen(command);

		// replacing \n with \0
		if (command[len - 1] == '\n')
			command[len - 1] = '\0';

		// removing any spaces at the end of the command
		for (int i = len - 2; ; i--) {
			if (command[i] == ' ')
				command[i] = '\0';
			else
				break;
		}
		// figuring out what command is being given
		// and executing it / showing error messages
		int f = figure_command(command, &img, &coord);

		if (f == 1)
			// EXIT
			break;
		else if (f == 2)
			printf("Invalid command\n");
	}
	return 0;
}
