/**
 * options.h
 *
 * This is external code. Sets some compression related options
 * (width, height quality, framerate).
 *
 * $Id: options.c 12931 2007-12-17 18:20:48Z theeth $ 
 *
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. The Blender
 * Foundation also sells licenses for use in proprietary software under
 * the Blender License.  See http://www.blender.org/BL/ for information
 * about this.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 *  */

#include "avi.h"
#include "avi_intern.h"
#include "endian.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/* avi_set_compress_options gets its own file... now don't WE feel important? */

AviError AVI_set_compress_option(AviMovie *movie, int option_type, int stream, AviOption option, void *opt_data) {
    int i;

    if (movie->header->TotalFrames != 0) /* Can't change params after we have already started writing frames */
        return AVI_ERROR_OPTION;

    switch (option_type) {
        case AVI_OPTION_TYPE_MAIN:
            switch (option) {
                case AVI_OPTION_WIDTH:
                    movie->header->Width = *((int *) opt_data);
                    movie->header->SuggestedBufferSize = movie->header->Width * movie->header->Height * 3;

                    for (i = 0; i < movie->header->Streams; i++) {
                        if (avi_get_format_type(movie->streams[i].format) == FCC("vids")) {
                            ((AviBitmapInfoHeader *) movie->streams[i].sf)->Width = *((int *) opt_data);
                            movie->streams[i].sh.SuggestedBufferSize = movie->header->SuggestedBufferSize;
                            movie->streams[i].sh.right = *((int *) opt_data);
                            ((AviBitmapInfoHeader *) movie->streams[i].sf)->SizeImage = movie->header->SuggestedBufferSize;
                            fseek(movie->fp, movie->offset_table[1 + i * 2 + 1], SEEK_SET);
                            awrite(movie, movie->streams[i].sf, 1, movie->streams[i].sf_size, movie->fp, AVI_BITMAPH);
                        }
                    }

                    break;

                case AVI_OPTION_HEIGHT:
                    movie->header->Height = *((int *) opt_data);
                    movie->header->SuggestedBufferSize = movie->header->Width * movie->header->Height * 3;

                    for (i = 0; i < movie->header->Streams; i++) {
                        if (avi_get_format_type(movie->streams[i].format) == FCC("vids")) {
                            ((AviBitmapInfoHeader *) movie->streams[i].sf)->Height = *((int *) opt_data);
                            movie->streams[i].sh.SuggestedBufferSize = movie->header->SuggestedBufferSize;
                            movie->streams[i].sh.bottom = *((int *) opt_data);
                            ((AviBitmapInfoHeader *) movie->streams[i].sf)->SizeImage = movie->header->SuggestedBufferSize;
                            fseek(movie->fp, movie->offset_table[1 + i * 2 + 1], SEEK_SET);
                            awrite(movie, movie->streams[i].sf, 1, movie->streams[i].sf_size, movie->fp, AVI_BITMAPH);
                        }
                    }

                    break;

                case AVI_OPTION_QUALITY:
                    for (i = 0; i < movie->header->Streams; i++) {
                        if (avi_get_format_type(movie->streams[i].format) == FCC("vids")) {
                            movie->streams[i].sh.Quality = (*((int *) opt_data))*100;
                            fseek(movie->fp, movie->offset_table[1 + i * 2 + 1], SEEK_SET);
                            awrite(movie, movie->streams[i].sf, 1, movie->streams[i].sf_size, movie->fp, AVI_BITMAPH);
                        }
                    }
                    break;

                case AVI_OPTION_FRAMERATE:
                    movie->header->MicroSecPerFrame = 1000000 / (*((int *) opt_data));

                    for (i = 0; i < movie->header->Streams; i++) {
                        if (avi_get_format_type(movie->streams[i].format) == FCC("vids")) {
                            movie->streams[i].sh.Scale = movie->header->MicroSecPerFrame;
                            fseek(movie->fp, movie->offset_table[1 + i * 2 + 1], SEEK_SET);
                            awrite(movie, movie->streams[i].sf, 1, movie->streams[i].sf_size, movie->fp, AVI_BITMAPH);
                        }
                    }
                    break;
            }

            fseek(movie->fp, movie->offset_table[0], SEEK_SET);
            awrite(movie, movie->header, 1, sizeof (AviMainHeader), movie->fp, AVI_MAINH);

            break;
        case AVI_OPTION_TYPE_STRH:
            break;
        case AVI_OPTION_TYPE_STRF:
            break;
        default:
            return AVI_ERROR_OPTION;
    }

    return AVI_ERROR_NONE;
}
