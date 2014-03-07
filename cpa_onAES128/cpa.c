#include	<stdio.h>
#include	<stdlib.h>
#include	<math.h>
#include	<time.h>

#define		PT_LENGTH	100
#define		NUM_PTS		20000
#define		NUM_BYTES_BLOCK	16

#define		MIN_KEYBITS	0
#define		MAX_KEYBITS	8
#define		NUM_KEYBITS	8
#define		NUM_POPBITS	9
#define		NUM_KEYS	256

#define		FILE_CIPHER	"cipher.txt"
#define		FILE_PTS	"pts.txt"


double	pts[NUM_PTS][PT_LENGTH];
unsigned char cipher[NUM_PTS][NUM_BYTES_BLOCK];

/* varables for cpa */
double	pts_set[NUM_POPBITS][PT_LENGTH];

int	num_pts0[NUM_KEYS][NUM_KEYBITS];
int	num_pts1[NUM_KEYS][NUM_KEYBITS];

double	pt_delta[NUM_KEYS][PT_LENGTH];
double	pt_delta_max[NUM_KEYS];
int	pt_delta_max_idx[NUM_KEYS];
/* END of varables for cpa */

/* varables for CPA  */
double	pt_corr[NUM_KEYS];
/* END of varables for CPA */

double	pt_delta_max[NUM_KEYS];

char	buffer[10240];

void	load_cipher()
{
	int	i;
	FILE	*fp;

	fp = fopen(FILE_CIPHER, "r");

	if (fp == NULL) {
		printf("Cannot open cipher file.\n");
		exit(0);
	}

	for (i = 0; i < NUM_PTS; i ++ ) {
		int	j;
		char	*p;

		if (!fgets(buffer, 100, fp)) {
			printf("Error: reading cipher %d\n", i); 
			exit(0);
		}
		p = buffer;
		for (j = 0; j < NUM_BYTES_BLOCK; j ++) {
			int 	v;
			int	c;

			c = *p ++;
			v = (c >= 'a') ? (c - 'a' + 10) : (c - '0');

			c = *p ++;
			v = (v << 4) | ((c >= 'a') ? (c - 'a' + 10) : (c - '0'));

			cipher[i][j] = v;
		}

	}
	fclose(fp);
}

void	print_char (unsigned char *p, int n)
{
	const	int	w = 16;
	int	i = 0;

	while (i < n) {
	    int	j;
	    printf("%04X(%5d):\t", i, i);
	    for (j = 0; j < w && i < n; j ++) {
		printf("%02X ", *p ++);
		i ++;
	    }
	    printf("\n");
	}
	puts("");
}

void	print_int (int *p, int n)
{

	const	int	w = 16;
	int	i = 0;

	while (i < n) {
	    int	j;
	    printf("%04X(%5d):\t", i, i);
	    for (j = 0; j < w && i < n; j ++) {
		printf("%6d ", *p ++);
		i ++;
	    }
	    printf("\n");
	}
	puts("");
}

void	print_double (double *p, int n) 
{
	const	int	w = 8;
	int	i = 0;

	while (i < n) {
	    int	j;
	    printf("%04X(%5d):\t", i, i);
	    for (j = 0; j < w && i < n; j ++) {
		printf("%10.3f,", *p ++);
		i ++;
	    }
	    printf("\n");
	}
	puts("");
}

void	load_pts()
{
	FILE 	*fp;
	int	i;

	fp = fopen(FILE_PTS, "r");
	if (fp == NULL) {
		printf("Cannot open the power trace file.\n");
		exit(0);
	}

	for (i = 0; i < NUM_PTS; i ++ ) {
		int	j;
		char	*p;
		double  *pf = pts[i];

		if (!fgets(buffer, 10240, fp)) {
			printf("Error: reading pt %d\n", i); 
			exit(0);
		}
		p = buffer;
		for (j = 0; j < PT_LENGTH; j ++) {
			int iv;
			char * curp = p;

			while (*p && *p != ',') 
				p ++;
			*p ++ = 0;

			iv = atoi(curp);
			*pf++ = (double) iv; 
		}

	}
	fclose(fp);
}


static unsigned char inv_S[256] = 
{
   0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
   0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
   0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
   0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
   0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
   0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
   0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
   0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
   0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
   0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
   0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
   0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
   0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
   0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
   0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
   0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

void	PT_scale(double *pd, double *ps, double c, int len)
{
    if (pd == NULL) {
	pd = ps;
    }
    while (len --) {
	*pd++ = (*ps++) * c;
    }
}

void	PT_add (double *sum, double *p1, double * p2, int n)
{
    int	    i;
    if (! sum)
	sum = p1;
    for (i = 0; i < n; i ++) {
	sum[i] = p1[i] + p2[i];
    }
}

void	PT_mac (double *sum, double * p1, double * p2, int n)
{
    int	    i;
    for (i = 0; i < n; i ++) {
	sum[i] += p1[i] * p2[i];
    }
}

void	PT_mac_scale (double *sum, double * p1, double p2, int n)
{
    int	    i;
    for (i = 0; i < n; i ++) {
	sum[i] += p1[i] * p2;
    }
}

void	PT_mul (double *sum, double * p1, double * p2, int n)
{
    int	    i;
    for (i = 0; i < n; i ++) {
	sum[i] = p1[i] * p2[i];
    }
}

void	PT_div (double *sum, double * p1, double * p2, int n)
{
    int	    i;
    for (i = 0; i < n; i ++) {
    p2[i] = 1.0 / p2[i];
	sum[i] = p1[i] * p2[i];
    }
}

void	PT_square (double *sum, double * p1, int n)
{
    int	    i;
    for (i = 0; i < n; i ++) {
	sum[i] = p1[i] * p1[i];
    }
}

void	PT_square_root (double *sum, double * p1, int n)
{
    int	    i;
    for (i = 0; i < n; i ++) {
	sum[i] = sqrt(p1[i]);
    }
}

void	PT_mac_sub (double *sum, double * p1, double * p2, int n)
{
    int	    i;
    for (i = 0; i < n; i ++) {
	sum[i] -= p1[i] * p2[i];
    }
}

void	PT_zero (double *p, int n)
{
    while (n --) {
	*p ++ = 0.0;
    }
}

void	PT_diff (double *res, double *p1, double * p2, int n)
{
    int	    i;
    if (!res)
	res = p1;
    for (i = 0; i < n; i ++) {
	res[i] = fabs(p1[i] - p2[i]);
    }
}

void	PT_sub (double *res, double *p1, double * p2, int n)
{
    int	    i;
    if (!res)
	res = p1;
    for (i = 0; i < n; i ++) {
	res[i] = (p1[i] - p2[i]);
    }
}

void	PT_abs (double *p1, int n)
{
    int	    i;
    for (i = 0; i < n; i ++) {
	p1[i] = abs(p1[i]);
    }
}

double max_dp (double *p, int n, int *idx)
{
    double max = *p ++;
    int	 ri = 0;
    int	 i;

    for (i = 1; i < n; i ++) { 
	double t = *p ++;
	if (t > max) {
	    max = t;
	    ri = i;
        }
    }
    if (idx)
    	*idx = ri;
    return max;
}

int max_iv (int *p, int n, int *idx)
{
    int max = *p ++;
    int	 ri = 0;
    int	 i;

    for (i = 1; i < n; i ++) { 
	int t = *p ++;
	if (t > max) {
	    max = t;
	    ri = i;
        }
    }
    if (idx)
    	*idx = ri;
    return max;
}

/** Get the difference of a cipher text byte and the S_box input at the same location. 
 */
int	get_difference(unsigned char * cipher, int n, int key) 
{
	static unsigned char shift_row[16] = {0,5,10,15,4,9,14,3,8,13,2,7,12,1,6,11};

	/* put your code here */
	int temp = key ^ cipher[n];
	temp = inv_S[temp];
	return cipher[shift_row[n]] ^ temp; 	// compair bit change of cipher and state key
}

double get_correlation(int hamming_distance, double *sum_W2)
{
	double sum_WH[PT_LENGTH];
	double sum_W[PT_LENGTH];
	double sum_H;
	double sum_H2;
	int ham;

	double right[PT_LENGTH];
	double left[PT_LENGTH];
	double temp;

	double numerator[PT_LENGTH];
	double denominator[PT_LENGTH];	
	double corr[PT_LENGTH];	
	double max;

	//TODO consider init sum_x vars
	// calculate summations
	for(ham = 0; ham < NUM_POPBITS; ham++){
		PT_mac_scale(sum_WH, pts_set[ham], (double)ham, PT_LENGTH);
		PT_add(NULL, sum_W, pts_set[ham], PT_LENGTH);
		sum_H += ham;
		sum_H2 += (ham*ham);
	}

	// numerator
	PT_scale(left, sum_WH, NUM_PTS, PT_LENGTH);		// left of the -
	PT_scale(right, sum_WH, sum_H, PT_LENGTH);		// right of the -
	PT_sub(numerator, left, right, PT_LENGTH);		//TODO abs?????

	// left side of the denominator
	PT_scale(left, sum_W2, NUM_PTS, PT_LENGTH);		// N * sum_W2
	PT_square(right, sum_W, PT_LENGTH);				// sum_W ^ 2
	PT_sub(left, left, right, PT_LENGTH);
	PT_square_root(denominator, left, PT_LENGTH);

	// right side of the denominator
	temp = sqrt((NUM_PTS * sum_H2) - (sum_H * sum_H));

	// denominator
	PT_scale(denominator, denominator, temp, PT_LENGTH);

	// final division and max!!!
	PT_div(corr, numerator, denominator, PT_LENGTH);
	max = max_dp(corr, PT_LENGTH, NULL);

	return max;
}

// return the key byte at location bytenum
int	cpa_aes(int bytenum)
{
	int	i_pt, i, nbits, j;
	int	kv;

	// Initialization
	for (i = 0; i < NUM_KEYS; i ++) {		
	    PT_zero(pt_delta[i], PT_LENGTH);
	}
	for(j = 0; j < NUM_POPBITS; j++){
	    PT_zero(pts_set[j], PT_LENGTH);
	}
        PT_zero(pt_corr, NUM_KEYS);

	// Put your code here
	// rotate through key guess NUM_KEYS
	for(i = 0; i < NUM_KEYS; i++){
		// look at bit bytenum refering to 10th round key byte (0 -> 15)
		// rotate through power traces

		// summations for correlation
		double sum_W2[PT_LENGTH];
		for(j = 0; j < NUM_PTS; j++){
			// cyper_i reg -> XOR key guess -> shift rows -> inv SBox -> get state_i reg
			// compair state_i reg and cyper_i reg bits if a change exists P_i -> S1 otherwise S0
			unsigned int diff_pop = __builtin_popcount(get_difference(cipher[j], bytenum, i));
			
			if(diff_pop > 8) perror("ERROR: too many poped bits\n");
			PT_add(NULL, pts_set[diff_pop], pts[j], PT_LENGTH);

			//calculate some summations for correlation
			PT_mac(sum_W2, pts[j], pts[j], PT_LENGTH);
		}
		//TODO plug in correlation fourmula		
		pt_corr[i] = get_correlation(NUM_POPBITS, sum_W2); //TODO figure out how to pass pts_set
	}
	
	// get the max of all stored correlations and return accosiated i(key value)
	max_dp(&pt_corr[0], NUM_KEYS, &kv);

	return kv;
}

int	main (int argc, char ** argv)
{
    int	i, start, end;
    time_t t0;

    start = end = 0;
    switch (argc) {
    	case 2: 
		start = end = atoi(argv[1]);;
		break;
    	case 3: 
		start = atoi(argv[1]);;
		end = atoi(argv[2]);;
		break;
    }
    
    start &= 0xF;
    end &= 0xF;

    time(&t0);
    load_cipher();
    // print_char(&cipher[0][0], 16 * 10);
    // print_char(&cipher[19990][0], 16 * 10);
    load_pts();
    // print_double(pts[NUM_PTS - 1], 700);
    printf("Time_load_data: %.2fs\n", difftime(time(NULL), t0));

    for (i = start; i <= end; i ++) {
	int  k; 
	time(&t0);
    	// printf("Working on key %d\n", i);
	k = cpa_aes(i);
	printf("KEY%d=%02X(%d) %.2fs\n", i, k, k, difftime(time(NULL),t0));
    }
    return 0;
}
