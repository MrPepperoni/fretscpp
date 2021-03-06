/*******************************************************************
(C) 2011 by Radu Stefan
radu124@gmail.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*******************************************************************/

#include "readpng.h"

#include "configuration.h"
#include "message.h"
#include "verbosity.h"

// http://elysianshadows.com/phpBB3/viewtopic.php?f=32&t=3419&p=38524

void scaleDownWidth(GLubyte *pixels,int components,int &rwidth,int &rheight)
{
	int i,j,k;
	int ns,stride;
	ns=rwidth/2;
	stride=ns*components;
	DBG(SPRITES,"Had to shrink down texture as it was too large\n");
	for (i=0; i<rheight; i++) for (j=0; j<ns; j++) for (k=0; k<components; k++)
	{
		pixels[i*stride+j*components+k]=
			((uint32_t) pixels[i*components*rwidth+j*components*2+k]+
			 (uint32_t) pixels[i*components*rwidth+j*components*2+k+components]
			)/2;
	}
	for (i=rheight*rwidth*components/2; i<rheight*rwidth*components; i++) pixels[i]=0;
	rwidth/=2;
}

void scaleDownHeight(GLubyte *pixels,int components,int &rwidth,int &rheight)
{
	int i,j,k;
	int ns,stride;
	stride=rwidth*components;
	DBG(SPRITES,"Had to shrink down texture as it was too large\n");
	for (i=0; i<rheight/2; i++) for (j=0; j<rwidth; j++) for (k=0; k<components; k++)
	{
		pixels[i*stride+j*components+k]=
			((uint32_t) pixels[i*stride*2+j*components+k]+
			 (uint32_t) pixels[i*stride*2+j*components+k+stride]
			)/2;
	}
	for (i=rheight*rwidth*components/2; i<rheight*rwidth*components; i++) pixels[i]=0;
	rheight/=2;
}


GLuint loadImage(const char *filename, GLuint &texture, int &imgwidth, int &imgheight, GLfloat &twidth, GLfloat &theight)
{
	int rwidth,rheight;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep *row_pointers = NULL;
	int bitDepth, colourType;
	png_uint_32 width,height;
	int i;
	int components;
	FILE *pngFile = fopen(filename, "rb");

	if(!pngFile)
	{
		WARN(PNGREAD,"PNG: could not open file: %s\n", filename);
		return 0;
	}
	png_byte sig[8];

	unusedint=fread(&sig, 8, sizeof(png_byte), pngFile);
	rewind(pngFile);//so when we init io it won't bitch
	if(!png_check_sig(sig, 8))
	{
		WARN(PNGREAD,"PNG: bad file signature: %s\n", filename);
		return 0;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,NULL,NULL);

	if(!png_ptr)
	{
		WARN(PNGREAD,"PNG: could not initialize reader: %s\n", filename);
		return 0;
	}

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		WARN(PNGREAD,"PNG: error during decode: %s\n", filename);
		return 0;
	}

	info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)
		return 0;

	png_init_io(png_ptr, pngFile);

	png_read_info(png_ptr, info_ptr);

	bitDepth = png_get_bit_depth(png_ptr, info_ptr);

	colourType = png_get_color_type(png_ptr, info_ptr);

	if(colourType == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	if(colourType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
	{
#if PNG_LIBPNG_VER < 10004
		png_set_gray_1_2_4_to_8(png_ptr);
#else
		png_set_expand_gray_1_2_4_to_8(png_ptr);
#endif
	}

	if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	if(bitDepth == 16)
		png_set_strip_16(png_ptr);
	else if(bitDepth < 8)
		png_set_packing(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height,
			&bitDepth, &colourType, NULL, NULL, NULL);

	components =
		(colourType==PNG_COLOR_TYPE_GRAY)?1:
		(colourType==PNG_COLOR_TYPE_GRAY_ALPHA)?2:
		(colourType==PNG_COLOR_TYPE_RGB)?3:
		(colourType==PNG_COLOR_TYPE_RGB_ALPHA)?4:-1;

	if(components == -1)
	{
		if(png_ptr)
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return 0;
	}

	rwidth=1;
	rheight=1;
	while (rwidth<width) rwidth*=2;
	while (rheight<height) rheight*=2;
	imgwidth=width;
	imgheight=height;

	twidth=1.0;
	theight=1.0;
	if (rwidth!=imgwidth || rheight!=imgheight)
	{
		WARN(SPRITES,"Sprite: %s has non-power of 2 size\n", filename);
		twidth=((GLfloat)imgwidth)/rwidth;
		theight=((GLfloat)imgheight)/rheight;
	}

	GLubyte *pixels = (GLubyte *)malloc(sizeof(GLubyte) * (rwidth * rheight * components));
	memset(pixels,0,sizeof(GLubyte) * (rwidth * rheight * components));
	row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * rheight);

	for(i = 0; i < rheight; ++i)
		  row_pointers[i] = (png_bytep)(pixels + (i * rwidth * components));

	png_read_image(png_ptr, row_pointers);
	png_read_end(png_ptr, NULL);


	// get a texture id if we don't have one already
	if (!texture)
		glGenTextures(1, &texture);
	// bind the texture
	glBindTexture(GL_TEXTURE_2D, texture);
	// define stretching parameters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// choose type based on number of colors
	GLuint glcolours;
	 (components==4) ? (glcolours = GL_RGBA): (0);
	 (components==3) ? (glcolours = GL_RGB): (0);
	 (components==2) ? (glcolours = GL_LUMINANCE_ALPHA): (0);
	 (components==1) ? (glcolours = GL_LUMINANCE): (0);

	int texsizelimit=maxTexSize;
	while (rwidth>texsizelimit) scaleDownWidth(pixels,components,rwidth,rheight);
	while (rheight>texsizelimit) scaleDownHeight(pixels,components,rwidth,rheight);

	glTexImage2D(GL_TEXTURE_2D, 0, components, rwidth, rheight, 0, glcolours, GL_UNSIGNED_BYTE, pixels);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	fclose(pngFile);
	free(row_pointers);
	free(pixels);

	return texture;

};



