











#include "texture.h"

static FILE* savepngfp = NULL;

void png_file_write(png_structp png_ptr, png_bytep data, png_size_t length) 
{
	fwrite(data, 1, length, savepngfp);
}

void png_file_flush(png_structp png_ptr)
{
	fflush(savepngfp);
}

int savepng(const char* full, unsigned char* data, int sizex, int sizey, int channels)
{
	//FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	//png_colorp palette;
	int color_type;
	int bit_depth;
	png_colorp palette;	
	png_color_16 Colors[PNG_MAX_PALETTE_LENGTH];
	png_byte Trans[PNG_MAX_PALETTE_LENGTH];
	int npal;
	png_bytep* row_pointers;
	int y;
	png_text text_ptr[1];
	char srcstr[123];

	/* Open the file */
	savepngfp = fopen(full, "wb");
	if (savepngfp == NULL)
		return 0;

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also check that
	* the library version is compatible with the one used at compile time,
	* in case we are using dynamically linked libraries.  REQUIRED.
	*/
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		(png_voidp) NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		fclose(savepngfp);
		return 0;
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(savepngfp);
		png_destroy_write_struct(&png_ptr,  NULL);
		return 0;
	}

	color_type = PNG_COLOR_TYPE_RGB;

	if(channels == 4)
		color_type = PNG_COLOR_TYPE_RGBA;
	if(channels == 1)
		color_type = PNG_COLOR_TYPE_GRAY;

	bit_depth = 8;

	png_set_IHDR(png_ptr, info_ptr, sizex, sizey, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* Set error handling.  REQUIRED if you aren't supplying your own
	* error handling functions in the png_create_write_struct() call.
	*/
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem writing the file */
		fclose(savepngfp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return 0;
	}

	/* One of the following I/O initialization functions is REQUIRED */

	/* Set up the output control if you are using standard C streams */
	//png_init_io(png_ptr, savepngfp);
	png_set_write_fn(png_ptr, NULL, png_file_write, png_file_flush);

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * sizey);

	if(!row_pointers)
	{
//		OUTMEM();
		return 0;
	}

	for (y=0; y<sizey; y++)
		row_pointers[y] = (png_byte*)&data[y*sizex*channels];

	sprintf(srcstr, "Rendered using %s Version %d", "chart", -1);
#ifdef USESTEAM
	strcat(srcstr, " Authorized Steam Build");
#endif
	text_ptr[0].key = "Source";
	text_ptr[0].text = srcstr;
	text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
	// text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;
#ifdef PNG_iTXt_SUPPORTED
	text_ptr[0].lang = NULL;
#endif
	text_ptr[0].lang_key = NULL;

	png_set_text(png_ptr, info_ptr, text_ptr, 1);

	png_write_info(png_ptr, info_ptr);

	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, NULL);

	//for (y=0; y<sizey; y++)
	//   free(row_pointers[y]);
	free(row_pointers);


	/* This is the easy way.  Use it if you already have all the
	* image info living in the structure.  You could "|" many
	* PNG_TRANSFORM flags into the png_transforms integer here.
	*/
	//png_write_png(png_ptr, info_ptr, NULL, NULL);

	/* If you png_malloced a palette, free it here (don't free info_ptr->palette,
	* as recommended in versions 1.0.5m and earlier of this example; if
	* libpng mallocs info_ptr->palette, libpng will free it).  If you
	* allocated it with malloc() instead of png_malloc(), use free() instead
	* of png_free().
	*/
	//png_free(png_ptr, palette);
	//palette = NULL;

	/* Similarly, if you png_malloced any data that you passed in with
	* png_set_something(), such as a hist or trans array, free it here,
	* when you can be sure that libpng is through with it.
	*/
	//png_free(png_ptr, trans);
	//trans = NULL;
	/* Whenever you use png_free() it is a good idea to set the pointer to
	* NULL in case your application inadvertently tries to png_free() it
	* again.  When png_free() sees a NULL it returns without action, thus
	* avoiding the double-free security problem.
	*/

	/* Clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* Close the file */
	fclose(savepngfp);

	/* That's it */
	return 1;
}

