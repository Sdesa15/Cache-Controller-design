/******************************************************************************
//Cache Controller Code
//Supritha Desa
*******************************************************************************/
//Include files
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <conio.h>
#include<math.h>

#define _CRT_SECURE_NO_DEPRECATE
// macros and global variables
#define MAXLINES 131072   // Maximum number of lines
#define MAXWAYS  16      //maximum number of ways
#define WTNA  3   //Write through non-allocate
#define WTA  2    //Write through allocate
#define WB  1    //write back 


#define MATRIXSIZE  256
double chol_matrix[MATRIXSIZE][MATRIXSIZE];
double p[MATRIXSIZE];
double G_b[MATRIXSIZE];
double G_x[MATRIXSIZE];
double solutionmatrix[MATRIXSIZE];
typedef unsigned long int bool;
uint32_t l_lru, maximum_lru;
uint8_t g_lru[MAXLINES][MAXWAYS];
bool g_dirty[MAXLINES][MAXWAYS];
bool g_valid[MAXLINES][MAXWAYS];
uint32_t g_tag[MAXLINES][MAXWAYS];
uint8_t N, BL;
uint32_t totalbytesread, hit;
uint32_t totalbyteswritten, wayp, tagp, writethrough_memory, write_replace_count;
uint32_t writecachecount, readcachecount, writemisscount, readmisscount, readhitcount, writehitcount, read_replace_count;
uint32_t write_dirty_count, read_writeback_count, writememtocachecount, readmemtocachecount, flushcount;
int WS;
uint32_t  l, b, L, B;
uint32_t line, way;
uint32_t S = 262144;
double totalaccesstime;
//function to get the current line
uint32_t getline(uint32_t add, int n, int bl)
{

	B = bl * (2);
	L = S / (B * n);
	b = log(B) / log(2);
	l = log(L) / log(2);
	uint32_t temp = pow(2, l) - 1;
	line = (add >> b) & temp;
	return (line);

}
//function to get the current tag
uint32_t gettag(uint32_t add, int n, int bl)
{
	uint32_t l_tag;
	B = bl * (2);
	L = S / (B * n);
	b = log(B) / log(2);
	l = log(L) / log(2);
	l_tag = add >> (b + l);
	return (l_tag);
}
// is dirty function
uint8_t isdirty(uint32_t line, uint32_t way)
{
	if (g_dirty[line][way] == 1)
	{
		return 1;

	}
	else
	{
		return 0;
	}
}
//inva;idate function
void invalidate(uint32_t line, uint32_t way)
{

	g_valid[line][way] = 0;
}
//validate function
void validate(uint32_t line, uint32_t way)
{
	g_valid[line][way] = 1;
}
//cleardirty function
void clearDirty(uint32_t line, int way)
{
	g_dirty[line][way] = 0;
}
//update Lru function
void updateLRU(uint32_t line, uint32_t way)
{
	uint32_t i;
	for (i = 0; i < N; i++)
	{
		g_lru[line][i]++;
	}
	g_lru[line][way] = 0;
}

//set tag function
void settag(uint32_t line, uint32_t way, uint32_t tagp)
{

	g_tag[line][way] = tagp;
}
//finding oldest way function
uint32_t findoldest(uint32_t line, int n)
{

	uint32_t l_way = 0;

	for (int i = 0; i < n; i++)
	{
		if (g_lru[line][i] == n - 1)
		{
			l_lru = g_lru[line][i];
			l_way = i;
		}
	}

	maximum_lru = l_lru;
	return l_way;
}
// find tag function
uint32_t findtag(uint32_t line, uint32_t tagp, int n)
{
	uint32_t l_way;
	for (int i = 0; i < n;i++)
	{
		if (g_tag[line][i] == tagp && g_valid[line][i] == 1)
		{
			//hit = 1;
			l_way = i;
			return l_way;

		}
		else
		{
			//hit = 0;

		}

	}
	return -1;

}


void readmemtocache()
{

	readmemtocachecount++;
}
void readcachetocpu()
{

	readcachecount++;
}


void writecacheblock()
{

	writecachecount++;
}

// write cache function
void writecache(void *address, int n, int bl)
{

	uint32_t l_line, l_way;
	uint32_t l_add = (uint32_t)address;
	l_line = getline(l_add, n, bl);
	tagp = gettag(l_add, n, bl);
	l_way = findtag(l_line, tagp, n);
	hit = (l_way != -1);
	if (!hit && (WS != WTNA))// write strategy should not be equal to write through non allocate
	{
		wayp = findoldest(l_line, n);

		if (isdirty(l_line, wayp))
		{
			write_dirty_count++;
			clearDirty(l_line, wayp);
		}
		invalidate(l_line, wayp);
		settag(l_line, wayp, tagp);
		validate(l_line, wayp);
		writemisscount++;
	}
	if (hit || (WS != WTNA))
	{
		if (hit == 1) writehitcount++;
		updateLRU(l_line, wayp);
		writecacheblock();
		if (WS == 1)
		{
			g_dirty[l_line][way] = 1;
		}

	}


}


//write memory function
void writemem(void *pmem, uint32_t size)
{
	int i;
	int32_t LastLine = -1;
	uint32_t add = (uint32_t)pmem;
	for (i = 0; i < size; i++)
	{
		line = getline(add, N, BL);
		if (line != LastLine)
		{

			writecache(add, N, BL);
			LastLine = line;

		}
		add++;
	}
	if (WS == 2 || WS == 3)
	{
		writethrough_memory += (size / 4);

	}
	totalbyteswritten += size;
}
// read cache function
void Read_Cache(void *address, int n, int bl)
{
	uint32_t l_line, l_way;
	uint32_t l_add = (uint32_t)address;
	l_line = getline(l_add, n, bl);
	tagp = gettag(l_add, n, bl);
	l_way = findtag(l_line, tagp, n);
	hit = (l_way != -1);
	if (!hit)
	{
		wayp = findoldest(l_line, n);
		validate(l_line, wayp);
		if (g_valid[l_line][wayp] == 1)
		{
			read_replace_count++;//replcae count
		}
		if (isdirty(l_line, wayp))
		{
			read_writeback_count++;
			clearDirty(l_line, wayp);
		}
		invalidate(l_line, wayp);
		settag(l_line, wayp, tagp);
		readmemtocache();
		validate(l_line, wayp);
		readmisscount++;
	}
	updateLRU(line, wayp);
	readcachetocpu();

}
//read memory function
void readmem(void *pmem, uint32_t size)
{
	int i;
	int32_t LastLine = -1;
	uint32_t add = (uint32_t)pmem;
	for (i = 0; i < size; i++)
	{
		line = getline(add, N, BL);
		if (line != LastLine)
		{

			Read_Cache(add, N, BL);
			LastLine = line;

		}
		add++;
	}
	totalbytesread += size;
}

test_read()
{
	uint32_t data[65536];
	uint64_t sum = 0;
	for (int i = 0; i < 65536; i++)
	{
		sum += data[i];
		readmem(&data[i], sizeof(uint32_t));
	}
	printf("%d\n", readcachecount);
	printf("%d\n", totalbytesread);
	printf("%d\n", readmisscount);
}

test_write()
{
	uint8_t x[262144];
	for (int i = 0; i < 262144; i++)

	{
		x[i] = 0;
		writemem(&x[i], sizeof(uint8_t));

	}
	printf("%d\n", totalbyteswritten);
	printf("%d\n", writecachecount);
	printf("%d\n", writemisscount);
	printf("%d\n", writehitcount);
	printf("%d\n", write_replace_count);

	printf("%d\n", write_dirty_count);
}

int resultfile(FILE *f)
{

	if (f == NULL)
	{
		return -1;
	}

	fprintf(f, "%d\t", WS);
	fprintf(f, "%d\t", N);
	fprintf(f, "%d\t", BL);
	fprintf(f, "%lu\t", readmisscount);
	fprintf(f, "%lu\t", writemisscount);
	fprintf(f, "%lu\t", writecachecount);
	fprintf(f, "%lu\t", readcachecount);
	fprintf(f, "%lu\t", totalbytesread);
	fprintf(f, "%lu\t", totalbyteswritten);
	fprintf(f, "%lu\t", read_writeback_count);
	fprintf(f, "%lu\t", write_dirty_count);
	fprintf(f, "%lu\t", writethrough_memory);
	fprintf(f, "%lu\t", read_replace_count);
	fprintf(f, "%lu\t", writehitcount);
	fprintf(f, "%lu\t\n", flushcount);
	return 0;
	
}

//Cholesky Decomposition
void choldc(double **a, int n, double p[])

{
	int i, j, k;
	double sum;

	writemem(&i, sizeof(i));
	readmem(&i, sizeof(i));
	readmem(&n, sizeof(n));
	for (i = 0;i < n;i++)
	{
		readmem(&i, sizeof(i));
		writemem(&j, sizeof(j));
		readmem(&j, sizeof(j));
		readmem(&n, sizeof(n));
		for (j = i;j < n;j++)
		{

			//for loop
			readmem(&i, sizeof(i));
			readmem(&j, sizeof(j));
			readmem(&a[i][j], sizeof(a[i][j]));
			writemem(&sum, sizeof(sum));
			readmem(&i, sizeof(i));
			writemem(&k, sizeof(k));
			readmem(&k, sizeof(k));



			for (sum = a[i][j], k = i - 1;k >= 0;k--)
			{
				//for calculation part

				readmem(&sum, sizeof(sum));
				readmem(&i, sizeof(i));
				readmem(&k, sizeof(k));
				readmem(&a[i][k], sizeof(a[i][k]));
				readmem(&j, sizeof(j));
				readmem(&k, sizeof(k));
				readmem(&a[j][k], sizeof(a[j][k]));
				writemem(&sum, sizeof(sum));

				sum -= a[i][k] * a[j][k];
			}

			readmem(&j, sizeof(j));
			readmem(&i, sizeof(i));
			if (i == j)
			{
				readmem(&sum, sizeof(sum));
				if (sum <= 0.0)
				{
					printf("choldc failed");
				}
				readmem(&sum, sizeof(sum));
				readmem(&i, sizeof(i));
				writemem(&p[i], sizeof(p[i]));
				p[i] = sqrt(sum);

			}
			else
			{
				readmem(&sum, sizeof(sum));
				readmem(&i, sizeof(i));
				readmem(&p[i], sizeof(p[i]));
				readmem(&j, sizeof(j));
				writemem(&a[j][i], sizeof(a[j][i]));
				a[j][i] = sum / p[i];
			}



		}

	}
	writemem(&i, sizeof(i));
	readmem(&i, sizeof(i));
	readmem(&n, sizeof(n));
	for (i = 0;i < n;i++)
	{
		writemem(&j, sizeof(j));
		readmem(&j, sizeof(j));
		readmem(&n, sizeof(n));
		for (j = 0;j < n;j++)
		{
			readmem(&i, sizeof(i));
			readmem(&j, sizeof(j));
			readmem(&a[i][j], sizeof(a[i][j]));
			writemem(&chol_matrix[i][j], sizeof(chol_matrix[i][j]));
			chol_matrix[i][j] = a[i][j];
			// printf("%f\t", chol_matrix[i][j]);
		}
		//printf("\n");
	}

}

void cholsl(double a[][256], int n, double p[], double b[], double x[])

{

	int i, k;
	double sum;

	writemem(&i, sizeof(i));
	readmem(&i, sizeof(i));
	readmem(&n, sizeof(n));
	for (i = 0;i < n;i++)

	{

		//for loop
		readmem(&i, sizeof(i));
		readmem(&b[i], sizeof(b[i]));
		writemem(&sum, sizeof(sum));
		readmem(&i, sizeof(i));
		writemem(&k, sizeof(k));
		readmem(&k, sizeof(k));


		for (sum = b[i], k = i - 1;k >= 0;k--)
		{
			//for calculation part

			readmem(&sum, sizeof(sum));
			readmem(&i, sizeof(i));
			readmem(&k, sizeof(k));
			readmem(&a[i][k], sizeof(a[i][k]));
			readmem(&x[k], sizeof(x[k]));
			writemem(&sum, sizeof(sum));
			sum -= a[i][k] * x[k];
		}

		readmem(&sum, sizeof(sum));
		readmem(&i, sizeof(i));
		readmem(&p[i], sizeof(p[i]));
		writemem(&x[i], sizeof(x[i]));
		x[i] = sum / p[i];

	}

	readmem(&n, sizeof(n));
	writemem(&i, sizeof(i));
	readmem(&i, sizeof(i));
	for (i = n;i >= 0;i--)

	{
		//for loop
		readmem(&i, sizeof(i));
		readmem(&x[i], sizeof(x[i]));
		writemem(&sum, sizeof(sum));
		readmem(&i, sizeof(i));
		writemem(&k, sizeof(k));
		readmem(&k, sizeof(k));


		for (sum = x[i], k = i + 1;k < n;k++)
		{
			//for calculation part
			readmem(&sum, sizeof(sum));
			readmem(&i, sizeof(i));
			readmem(&k, sizeof(k));
			readmem(&a[k][i], sizeof(a[k][i]));
			readmem(&x[k], sizeof(x[k]));
			writemem(&sum, sizeof(sum));
			sum -= a[k][i] * x[k];
		}

		readmem(&sum, sizeof(sum));
		readmem(&i, sizeof(i));
		readmem(&p[i], sizeof(p[i]));
		writemem(&x[i], sizeof(x[i]));
		x[i] = sum / p[i];


	}
	writemem(&k, sizeof(k));
	readmem(&k, sizeof(k));
	readmem(&n, sizeof(n));
	for (k = 0;k < n;k++)
	{
		readmem(&k, sizeof(k));
		readmem(&x[k], sizeof(x[k]));
		writemem(&solutionmatrix[k], sizeof(solutionmatrix[k]));
		solutionmatrix[k] = x[k];
		//printf("%f\t", solutionmatrix[k]);
	}
	//printf("\n");


}


int main()
{
	FILE *f;
	f = fopen("Project2r.xls", "w+");
#pragma align 16
	int i, j, k, len = 0;
	int n = MATRIXSIZE;

	double **a, **atranspose, **resmatrix, **identity;

	a = (double **)malloc(n * sizeof(double *));
	atranspose = (double **)malloc(n * sizeof(double *));
	resmatrix = (double **)malloc(n * sizeof(double *));
	identity = (double **)malloc(n * sizeof(double *));

	for (i = 0;i < n;i++)
	{
		resmatrix[i] = (double *)malloc(n * sizeof(double));
		atranspose[i] = (double *)malloc(n * sizeof(double));
		a[i] = (double *)malloc(n * sizeof(double));
		identity[i] = (double *)malloc(n * sizeof(double));
	}
	for (WS = 1; WS < 4; WS++)
	{
		for (N = 1; N <= 16; N *= 2)
		{
			for (BL = 1; BL <= 8; BL *= 2)
			{
				for (i = 0; i < n; i++)
				{
					for (j = 0; j < n;j++)
					{
						resmatrix[i][j] = 0;
						atranspose[i][j] = 0;
						a[i][j] = 0;
						identity[i][j] = 0;
					}

				}

				for (i = 0; i < n; i++)
				{
					for (j = 0; j < n;j++)
					{
						a[i][j] = rand() % n;


					}


				}
				for (i = 0; i < n; i++)
				{
					for (j = 0; j < n; j++)
					{
						atranspose[j][i] = a[i][j];
						if (i == j)
						{
							identity[i][j] = 23 * n;
						}
						else
						{
							identity[i][j] = 0;

						}

					}

				}

				for (i = 0; i < n; i++)
				{
					for (j = 0; j < n; j++)
					{
						resmatrix[i][j] = 0;
						for (k = 0;k < n;k++)
						{
							resmatrix[i][j] += (atranspose[i][k] * a[k][j]);
						}
						resmatrix[i][j] += identity[i][j];


					}

				}

				for (j = 0; j < n;j++)
				{
					G_b[j] = rand();


				}


				for (line = 0; line < MAXLINES; line++)
				{
					for (way = 0; way < MAXWAYS; way++)
					{
						g_valid[line][way] = 0;
						g_dirty[line][way] = 0;
						g_lru[line][way] = way;
					}
				}

				choldc(resmatrix, n, p);
				cholsl(chol_matrix, n, p, G_b, G_x);
				for (i = 0; i < MAXLINES; i++)
				{
					for (j = 0; j < MAXWAYS; j++)
					{
						if (g_dirty[i][j] == 1)
						{
							g_dirty[i][j] = 0;
							flushcount++;
						}
					}
				}
				resultfile(f);
				
				printf("\n*************************\n");
				printf("write strategy =%d ways= %d  Burst_Length=%d\n", WS, N, BL);
				printf("WS=%d\n", WS);
				printf("N=%d\n", N);
				printf("BL=%d\n", BL);
				printf("readmisscount=%lu\n", readmisscount);
				printf("writemisscount=%lu\n", writemisscount);
				printf("writecachecount=%lu\n", writecachecount);
				printf("readcachecount=%lu\n", readcachecount);
				printf("totalbytesread=%lu\n", totalbytesread);
				printf("totalbyteswritten=%lu\n", totalbyteswritten);
				printf("read_writeback_count=%lu\n", read_writeback_count);
				printf("write_dirty_count=%lu\n", write_dirty_count);
				printf("writethrough_memory=%lu\n", writethrough_memory);
				printf("read_replace_count=%lu\n", read_replace_count);
				printf("writehitcount=%lu\n", writehitcount);
				printf("flushcount=%lu\n", flushcount);
				printf("**************************");
					
		
			}
		}
	}
	free(a);
	free(atranspose);
	free(resmatrix);
	free(identity);
	fclose(f);
	_getch();

}