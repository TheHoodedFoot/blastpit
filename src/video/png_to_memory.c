/* structure to store PNG image bytes */
struct mem_encode
{
  char *buffer;
  size_t size;
}


void
my_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
  /* with libpng15 next line causes pointer deference error; use libpng12 */
  struct mem_encode* p=(struct mem_encode*)png_get_io_ptr(png_ptr); /* was png_ptr->io_ptr */
  size_t nsize = p->size + length;

  /* allocate or grow buffer */
  if(p->buffer)
    p->buffer = realloc(p->buffer, nsize);
  else
    p->buffer = malloc(nsize);

  if(!p->buffer)
    png_error(png_ptr, "Write Error");

  /* copy new bytes to end of buffer */
  memcpy(p->buffer + p->size, data, length);
  p->size += length;
}

/* This is optional but included to show how png_set_write_fn() is called */
void
my_png_flush(png_structp png_ptr)
{
}



int save_png_to_file(RGBBitmap *bitmap, const char *path)
{
...
/* static */
struct mem_encode state;

/* initialise - put this before png_write_png() call */
state.buffer = NULL;
state.size = 0;

/* if my_png_flush() is not needed, change the arg to NULL */
png_set_write_fn(png_ptr, &state, my_png_write_data, my_png_flush);

... call png_write_png() ...

/* now state.buffer contains the PNG image of size s.size bytes */

/* cleanup */
if(state.buffer)
  free(state.buffer);