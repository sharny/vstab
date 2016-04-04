/*
 * Video stabilizer
 *
 * Copyright (c) 2008 Lenny <leonardo.masoni@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
 #include <values.h>

#include "avi/avi.h"

#include "main.h"
#include "utils.h"
#include "vector.h"
#include "motion.h"
#include "resample.h"

int opt_shutter_angle;
int opt_mjpeg_quality = 100;
bool opt_debug_conv = false;

static void print_help(char *argv[]) {

    printf(
            "                                                               \n"
            "Video stabilizer                                               \n"
            "                                                               \n"
            "Usage:                                                         \n"
            " %s [options] <input> <output>                                 \n"
            "                                                               \n"
            "Options:                                                       \n"
            " -r # | Rolling shutter angle | default:   0 | range:  0 - 180 \n"
            " -q # | Output MJPEG quality  | default: 100 | range: 50 - 100 \n"
            " -d   | Debug convolution     |              |                 \n"
            "                                                               \n",
            argv[0]
            );

    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

    AviMovie mv_in, mv_out;

    int nf, i, nc, nr;

    mt_ctx *mt;
    rs_ctx *rs;

    opterr = 0;

    while ((i = getopt(argc, argv, "r:q:d")) != -1) {

        switch (i) {

            case 'r':
                opt_shutter_angle = atoi(optarg);
                break;

            case 'q':
                opt_mjpeg_quality = atoi(optarg);
                break;

            case 'd':
                opt_debug_conv = true;
                break;

            default:
                print_help(argv);
        }
    }

    if (argc < optind + 2)
        print_help(argv);

    printf("status: setup\n");

    if (AVI_open_movie(argv[optind], &mv_in) != AVI_ERROR_NONE) {

        printf("error: can't read from %s\n", argv[optind]);
        return EXIT_FAILURE;
    }

    if (mv_in.header->Streams < 1 || mv_in.streams[0].sh.Type != AVIST_VIDEO) {

        printf("error: video stream not found on %s\n", argv[optind]);
        return EXIT_FAILURE;
    }

    nc = mv_in.header->Width;
    nr = mv_in.header->Height;

    mt = mt_init(nc, nr);

    if (mt == NULL) {

        printf("error: Width and Height must be >= 128 and multiples of 16 on %s\n", argv[optind]);
        return EXIT_FAILURE;
    }

    if (AVI_open_compress(argv[optind + 1], &mv_out, 1, AVI_FORMAT_MJPEG) != AVI_ERROR_NONE) {

        printf("error: can't write to %s\n", argv[optind + 1]);
        return EXIT_FAILURE;
    }

    prepare_lanc_kernels();

    int tfs = mv_in.header->TotalFrames;
    int fps = 1000000 / mv_in.header->MicroSecPerFrame;

	vc *pos_i = (vc *) malloc(tfs * sizeof (vc));
	vc *pos_h = (vc *) malloc(tfs * sizeof (vc));
	vc *pos_y = (vc *) malloc(nr * sizeof (vc));

    AVI_set_compress_option(&mv_out, AVI_OPTION_TYPE_MAIN, 0, AVI_OPTION_WIDTH, &nc);
    AVI_set_compress_option(&mv_out, AVI_OPTION_TYPE_MAIN, 0, AVI_OPTION_HEIGHT, &nr);
    AVI_set_compress_option(&mv_out, AVI_OPTION_TYPE_MAIN, 0, AVI_OPTION_FRAMERATE, &fps);
    AVI_set_compress_option(&mv_out, AVI_OPTION_TYPE_MAIN, 0, AVI_OPTION_QUALITY, &opt_mjpeg_quality);

    rs = rs_init(nc, nr);

    printf("status: estimating\n");

    for (nf = 0; nf < tfs; nf++) {

        unsigned char *fr = (unsigned char *) AVI_read_frame(&mv_in, AVI_FORMAT_RGB24, nf, 0);

        mt_push(mt, fr);

        if (nf > 0)
            pos_i[nf] = vc_add(mt_motion(mt, fr), pos_i[nf - 1]);
        else
            pos_i[nf] = vc_zero();

        if (opt_debug_conv) {
            AVI_write_frame(&mv_out, nf, AVI_FORMAT_RGB24, fr, nc * nr * 3 * sizeof (unsigned char));
        } else {
            free(fr);
        }

        if ((nf + 1) % 10 == 0) {

            printf(".");
            fflush(stdout);
        }
    }

    if (opt_debug_conv)
        goto cleanup;

    printf("\nstatus: filtering\n");

    vc_filter(pos_i, pos_h, tfs, fps / 2, 8, 1);

    printf("status: resampling\n");

    for (nf = 0; nf < tfs; nf++) {

        unsigned char *fr = (unsigned char *) AVI_read_frame(&mv_in, AVI_FORMAT_RGB24, nf, 0);

        for (i = 0; i < nr; i++) {

            pos_y[i] = vc_interp(
                    pos_h, tfs,
                    nf + (i - nr / 2.0) * opt_shutter_angle / (nr * 360.0)
                    );
        }

        rs_resample(rs, fr, pos_y);

        AVI_write_frame(&mv_out, nf, AVI_FORMAT_RGB24, fr, nc * nr * 3 * sizeof (unsigned char));

        if ((nf + 1) % 10 == 0) {

            printf(".");
            fflush(stdout);
        }
    }

cleanup:

    printf("\nstatus: closing\n");

    AVI_close(&mv_in);
    AVI_close_compress(&mv_out);

    return EXIT_SUCCESS;
}

