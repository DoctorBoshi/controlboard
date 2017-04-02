#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

//#include "fr_type.h"

//#define DEBUG

#define WIN8	8
#define MINUTIAE_MAX		1000

typedef struct {
	int	type;
	int	x;
	int	y;
	int vx[4];
	int vy[4];
	int dir;
	int quality;
} minutiae;

typedef struct {
	int			size;
	int			idx;
	minutiae	item[100];
} minutiae_list;

extern minutiae_list mlist;
extern minutiae minutiaes[MINUTIAE_MAX];

int write_ppm(char* filename, int width, int height, unsigned char *bgdata, unsigned char *rdata, unsigned char *gdata, unsigned char *bdata);

int imageBilinearScale(unsigned char *dst, unsigned char *src, int dst_width, int dst_height, int src_width, int src_height);

int localShadingCorrection(unsigned char *image, int width, int height, int *integralValue);

void imageBilinearScalePasteY(unsigned char *dst, int dst_width, int dst_height, int dst_region_left, int dst_region_top, int dst_region_right, int dst_region_bottom,
    unsigned char *src, int src_width, int src_height, int src_region_left, int src_region_top, int src_region_right, int src_region_bottom);

void globalNormalize(unsigned char *image, int width, int height);

unsigned char median(unsigned char *line0, unsigned char *line1, unsigned char *line2);

int find_mean(int width, int height, unsigned char *din, unsigned char *mean);

int find_variance(int width, int height, unsigned char *din, unsigned char *mean, unsigned char *variance);

int find_border(int width, int height, unsigned char *din, unsigned char *mean, unsigned char *variance, unsigned char *border);

int clear_border(int width, int height, unsigned char *din, unsigned char *border, unsigned char *dout);

int dilation_border(int width, int height, unsigned char *din, unsigned char *dout);

int expand_border(int width, int height, unsigned char *din, unsigned char *dout);

int median_filter(int width, int height, unsigned char *din, unsigned char *dout, int region_left, int region_top, int region_right, int region_bottom);

int mean_filter(int width, int height, signed char *din, signed char *dout);

int gradient_filter(int width, int height, unsigned char *din, short *gx, short *gy);

int find_dir(int width, int height, unsigned char *din, unsigned char *dir);

int expand_dir(int width, int height, unsigned char *dir, unsigned char *data);

int dir_filter(int width, int height, unsigned char *din, unsigned char *dir, unsigned char *mean, unsigned char *variance, unsigned char *dout);

int binary_filter(int width, int height, unsigned char *din, unsigned char *mean, unsigned char *dout);

int smooth_filter(int width, int height, unsigned char *din, unsigned char *dout, int region_left, int region_top, int region_right, int region_bottom);

int thinning_teration(int width, int height, unsigned char *din, unsigned char *dout, int iter, int region_left, int region_top, int region_right, int region_bottom);

int post_thinning(int width, int height, unsigned char *data, int region_left, int region_top, int region_right, int region_bottom);

int thinning_filter(int width, int height, unsigned char *data, unsigned char *tmp, int region_left, int region_top, int region_right, int region_bottom);

int find_minutiae(int width, int height, unsigned char *din, unsigned char *border, minutiae *minutiaes);

int check_minutiae(int size, minutiae *minutiaes, minutiae check);

int search_next(int width, unsigned char *thinning, int x, int y, int prev, minutiae_list *mlist, int cnt, int iter);

int search_end(int width, unsigned char *thinning, minutiae_list *mlist);

int search_fork(int width, unsigned char *thinning, minutiae_list *mlist, int iter);

int direction(int vx, int vy);

int remove_false_minutiae(int width, int height, unsigned char *thinning, int size, minutiae *minutiaes);

