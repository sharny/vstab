/**
 * codecs.c
 *
 * This is external code. Identify and convert different avi-files.
 *
 * $Id: codecs.c 12931 2007-12-17 18:20:48Z theeth $
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
 */

#include "avi.h"
#include "avi_intern.h"

#include "avirgb.h"
#include "mjpeg.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

void *avi_format_convert(AviMovie *movie, int stream, void *buffer, AviFormat from, AviFormat to, int *size) {
    if (from == to)
        return buffer;

    if (from != AVI_FORMAT_RGB24 &&
            to != AVI_FORMAT_RGB24)
        return avi_format_convert(movie, stream,
            avi_format_convert(movie, stream, buffer, from, AVI_FORMAT_RGB24, size),
            AVI_FORMAT_RGB24, to, size);

    switch (to) {
        case AVI_FORMAT_RGB24:
            switch (from) {
                case AVI_FORMAT_MJPEG:
                    buffer = avi_converter_from_mjpeg(movie, stream, buffer, size);
                    break;
                default:
                    break;
            }
            break;
        case AVI_FORMAT_MJPEG:
            buffer = avi_converter_to_mjpeg(movie, stream, buffer, size);
            break;
        default:
            break;
    }

    return buffer;
}

int avi_get_data_id(AviFormat format, int stream) {
    char fcc[5];

    if (avi_get_format_type(format) == FCC("vids"))
        sprintf(fcc, "%2.2ddc", stream);
    else if (avi_get_format_type(format) == FCC("auds"))
        sprintf(fcc, "%2.2ddc", stream);
    else
        return 0;

    return FCC(fcc);
}

int avi_get_format_type(AviFormat format) {
    switch (format) {
        case AVI_FORMAT_RGB24:
        case AVI_FORMAT_MJPEG:
            return FCC("vids");
        default:
            return 0;
    }
}

int avi_get_format_fcc(AviFormat format) {
    switch (format) {
        case AVI_FORMAT_MJPEG:
            return FCC("MJPG");
        case AVI_FORMAT_RGB24:
            return FCC("DIB ");
        default:
            return 0;
    }
}

int avi_get_format_compression(AviFormat format) {
    switch (format) {
        case AVI_FORMAT_MJPEG:
            return FCC("MJPG");
        default:
            return 0;
    }
}
