/*
 * jpegdest.c
 *
 * Copyright (C) 2014 Timo Kokkonen
 * All Rights Reserved.
 *
 * custom libjpeg "Destination Manager" for saving into RAM
 *
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include <jerror.h>

#include "jpegoptim.h"



/* custom jpeg destination manager object */

typedef struct {
  struct jpeg_destination_mgr pub; /* public fields */

#ifndef SAFE_MM
  unsigned char **buf_ptr;
  unsigned char *buf;
#else
  mm_array_ptr<unsigned char> *buf_ptr;
  mm_array_ptr<unsigned char> buf;
#endif
  size_t *bufsize_ptr;
  size_t incsize;

  size_t bufsize;

} jpeg_memory_destination_mgr;

typedef jpeg_memory_destination_mgr* jpeg_memory_destination_ptr;




void jpeg_memory_init_destination (j_compress_ptr cinfo)
{
  jpeg_memory_destination_ptr dest = (jpeg_memory_destination_ptr) cinfo->dest;

#ifndef SAFE_MM
  dest->pub.next_output_byte = dest->buf;
#else
  dest->pub.next_output_byte = _getptr_mm_array<unsigned char>(dest->buf);
#endif
  dest->pub.free_in_buffer = dest->bufsize;
}


boolean jpeg_memory_empty_output_buffer (j_compress_ptr cinfo)
{
  jpeg_memory_destination_ptr dest = (jpeg_memory_destination_ptr) cinfo->dest;
#ifndef SAFE_MM
  unsigned char *newbuf;
#else
  mm_array_ptr<unsigned char> newbuf = NULL;
#endif

  /* abort if incsize is 0 (no expansion of buffer allowed) */
  if (dest->incsize == 0) ERREXIT1(cinfo, JERR_OUT_OF_MEMORY, 42);

  /* otherwise, try expanding buffer... */
#ifndef SAFE_MM
  newbuf = realloc(dest->buf,dest->bufsize + dest->incsize);
#else
  newbuf = mm_array_realloc<unsigned char>(dest->buf,dest->bufsize + dest->incsize);
#endif
  if (!newbuf) ERREXIT1(cinfo, JERR_OUT_OF_MEMORY, 42);

#ifndef SAFE_MM
  dest->pub.next_output_byte = newbuf + dest->bufsize;
#else
  dest->pub.next_output_byte = _getptr_mm_array<unsigned char>(newbuf + dest->bufsize);
#endif
  dest->pub.free_in_buffer = dest->incsize;

  *dest->buf_ptr = newbuf;
  dest->buf = newbuf;
  dest->bufsize += dest->incsize;
  dest->incsize *= 2;

  return TRUE;
}



void jpeg_memory_term_destination (j_compress_ptr cinfo)
{
  jpeg_memory_destination_ptr dest = (jpeg_memory_destination_ptr) cinfo->dest;

  *dest->buf_ptr = dest->buf;
  *dest->bufsize_ptr = dest->bufsize - dest->pub.free_in_buffer;
}



void jpeg_memory_dest (j_compress_ptr cinfo, mm_array_ptr<unsigned char> *bufptr, size_t *bufsizeptr, size_t incsize)
{
  jpeg_memory_destination_ptr dest;

  if (!cinfo || !bufptr || !bufsizeptr) 
    fatal("invalid call to jpeg_memory_dest()");
  if (!*bufptr || *bufsizeptr == 0) 
    fatal("invalid buffer passed to jpeg_memory_dest()");
  

  /* allocate destination manager object for compress object, if needed */
  if (!cinfo->dest) {
    cinfo->dest = (struct jpeg_destination_mgr *)
      (*cinfo->mem->alloc_small) ( (j_common_ptr) cinfo,
				   JPOOL_PERMANENT,
				   sizeof(jpeg_memory_destination_mgr) );
  }

  dest = (jpeg_memory_destination_ptr)cinfo->dest;

  dest->buf_ptr = bufptr;
  dest->buf = *bufptr;
  dest->bufsize_ptr = bufsizeptr;
  dest->bufsize = *bufsizeptr;
  dest->incsize = incsize;

  dest->pub.init_destination = jpeg_memory_init_destination;
  dest->pub.empty_output_buffer = jpeg_memory_empty_output_buffer;
  dest->pub.term_destination = jpeg_memory_term_destination;
}

