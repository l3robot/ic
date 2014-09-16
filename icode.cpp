#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>


#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


/* Fonction pour la barre de progression 
*	i = où rendu
*	n = total
*	w = largeur de barre
*	actualize = actualiser ou non
*/
void showProgress(int i, int n, int w, int actualize)
{
	float ratio = i / (float)n;
	int c = ratio * w;

	printf("%3d%% [", (int)(ratio*100) );

	for (int i = 0; i < c; i++)
		printf("=");

	for (int i = c; i < w; i++)
		printf(" ");
	if(actualize)
		printf("]\n\033[F\033[J");
	else
		printf("]\n");
}

/* Retourne la taille d'un fichier en bytes 
*	f = descripteur de fichier
*/
int whatSize(FILE* f)
{
	int s;

	fseek(f, 0, SEEK_END);
	s = ftell(f);
	fseek(f, 0, SEEK_SET);

	return s;
}

/* Transforme en impair
*	v = le bytes de l'image (r,g ou b)
*/
uchar odd(uchar v)
{
	if(v % 2 != 0) return v; 
	else if(v > 0) return v-1;
	else return 1;
}

/* Transforme en pair
*	v = le bytes de l'image (r,g ou b)
*/
uchar even(uchar v)
{
	if(v % 2 == 0) return v; 
	else if(v > 0) return v-1;
	else return 0;
}

/* De byte à binaire 
*	c = byte
*	bin = binaire
*/
void vBin(char c, char bin[8])
{
	char j = 1;
	for (int i = 0; i < 8; i++)
	{
		int t = (c & j);
		if (t) bin[i] = 1;
		else bin[i] = 0;
		j*=2;
	}
}

/* De int à binaire sur 32 bits
*	c = int
*	bin = binaire
*/
void vBin32(int c, char bin[32])
{
	int j = 1;
	for (int i = 0; i < 32; i++)
	{
		int t = (c & j);
		if (t) bin[i] = 1;
		else bin[i] = 0;
		j*=2;
	}
}

/* De binaire à byte 
*	bin = binaire
*/
char ivBin(char bin[8])
{
	char c;
	char j = 1;
	for (int i = 0; i < 8; i++)
	{
		c = c + bin[i] * j;
		j*=2;
	}

	return c;
}

/* De binaire sur 32 bits à int 
*	bin = binaire
*/
int ivBin32(char bin[32])
{
	int c;
	int j = 1;
	for (int i = 0; i < 32; i++)
	{
		c = c + bin[i] * j;
		j*=2;
	}

	return c;
}

/* Print de test
*	c = byte
*	bin = binaire
*/
void printT(char c, char bin[8])
{
	for (int i = 0; i < 8; i++)
	{
		cout << (int) bin[i];
	}

	cout << " --> " << c << endl;
}

/* Fonction pour encoder
*	i = chemin de l'image
*	f = chemin du fichier
*/
void encode(char* i, char* f)
{
	cout << endl;

	cout << "Encoding phase begins :" << endl << endl;

	Mat image;

	image = imread(i, CV_LOAD_IMAGE_COLOR);

	FILE* file = fopen(f, "r");

	int s = whatSize(file);

	char* text = (char*) malloc(s * sizeof(char));

	fread(text, (size_t) s, 1, file);

	fclose(file);

	uchar* iPter = image.ptr();

	int j = 0;

	showProgress(j, s*8, 75, 1);

	//encode la taille
	char bin32[32];
	vBin32(s, bin32);
	for (int i = 0; i < 32; i++)
	{
		if (!bin32[i])
		{
			iPter[j] = even(iPter[j]);
		}
		else
		{
			iPter[j] = odd(iPter[j]);	
		}
		j++;
	}

	//encode le message
	for (int i = 0; i < s; i++)
	{
		char bin[9] = {0,0,0,0,0,0,0,0};
		vBin(text[i], bin);
		//printT(text[i], bin);
		for (int k = 0; k < 8; k++)
		{
			if (!bin[k])
			{
				iPter[j] = even(iPter[j]);
			}
			else
			{
				iPter[j] = odd(iPter[j]);	
			}
			j++;
			showProgress(j, s*8, 75, 1);
		}
	}

	showProgress(s*8, s*8, 75, 0);

	cout << "done" << endl << endl;

	free(text);

	imwrite("result.png", image);
}

/* Fonction pour décoder
*	i = chemin de l'image
*	f = chemin du fichier
*/
void decode(char* i, char* f)
{
	cout << endl;

	cout << "Decoding phase begins :" << endl << endl;

	Mat image;

	image = imread(i, CV_LOAD_IMAGE_COLOR);

	FILE* file = fopen(f, "w");

	uchar* iPter = image.ptr();

	int j = 0;

	char c = 'a';

	//décode la taille
	char bin32[32];
	for (int i = 0; i < 32; i++)
	{
		if (iPter[j] % 2)
		{
			bin32[i] = 1;
		}
		else
		{
			bin32[i] = 0;	
		}
		j++;
	}
	int s = ivBin32(bin32);

	//décode le message
	for (int i = 0; i < s; i++)
	{
		char bin[9] = {0,0,0,0,0,0,0,0};
		for (int k = 0; k < 8; k++)
		{
			if (iPter[j] % 2)
			{
				bin[k] = 1;
			}
			else
			{
				bin[k] = 0;
			}
			j++;
		}
		c = ivBin(bin);
		//printT(c, bin);
		fputc(c, file);
	}

	cout << "done" << endl << endl;

	fclose(file);
}

/* Menu help
*/
void printHelp()
{
	cout << endl;
	cout << " ----------------------" << endl;
	cout << "| Image Encoding usage |" << endl;
	cout << " ----------------------" << endl << endl;
	cout << "------------------ Help Menu -------------------" << endl << endl;
	cout << "ic -h                    : To print this message" << endl;
	cout << "ic -c <image> <file>     : To encode" << endl;
	cout << "ic -d <image> -o <file>  : To decode" << endl << endl;
}

int main(int argc, char* argv[])
{
	switch((argv[1])[1])
	{
		case 'h':
		printHelp();
		break;
		case 'c':
		if (argc < 3)
		{
			cout << endl;
			cout << "Bad options, try this : " << endl;
			printHelp();
		}
		encode(argv[2], argv[3]);
		break;
		case 'd':
		if (argc < 4)
		{
			cout << endl;
			cout << "Bad options, try this : " << endl;
			printHelp();
		}
		decode(argv[2], argv[4]);
		break;
		default:
		cout << endl;
		cout << "Bad options, try this : " << endl;
		printHelp();
		break;
	}

	return 0;
}


















