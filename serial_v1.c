#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define H_ERR 10
#define H_WIDTH 1000

// Struct 'conteggio' contains number of matches as
// well as their position in the sequence
typedef struct {
	int count;
	long long int *corresp;
} conteggio;

// Randomly generate a sequence of genomes:
void populate(char str[], long long int length) {
	const char az_base[4] = {'A', 'C', 'G', 'T'};
	// Remove after testing
	srand(4);

	for (int i=0; i<length; i++) {
		int r = rand() % 4;
		str[i] = az_base[r];
	}

	str[length] = '\0';
}

// Fwd sequencing in case of genes' undesired modifications
int hole_forward(char seq[], char pat[], long long int s_len, long long int p_len, conteggio *c) {
	// 'i' index: sequence starting position;
	// 'j' index: pattern vs sub-sequence comparison.
	int i = 0, j = 0, dim = 2;
	int err_cnt = 0, count_L_hole = 0;
	int max_holes = (int) p_len / H_ERR;
	int max_L_hole = (int) p_len / H_WIDTH;

	c->count = 0;
	c->corresp = malloc(dim * sizeof(long long int));

	if (!c->corresp) {
		fprintf(stderr, "Malloc failed\n");
		return 2;
	}

	for (i = 0; i <= s_len-p_len; i++) {
		for (j = 0; j < p_len; j++) {

			// Realloc first, if needed
			if ((c->count) >= dim) {
				dim *= 2;
				c->corresp = realloc(c->corresp, dim * sizeof(long long int));

				if (!c->corresp) {
					fprintf(stderr, "Realloc failed!\n");
					return 3;
				}
			}

			// Count only if the comparison is true given the
			// max number of allowed 'holes':
			if (err_cnt <= max_holes && count_L_hole <= max_L_hole) {
				if (seq[i+j] != pat[j]) {
					count_L_hole++;

					if (count_L_hole == 1)
						err_cnt++;

					if (count_L_hole > max_L_hole) {
						break;
					}
				} else {
					count_L_hole = 0;
				}
				
				if (j == p_len-1 && err_cnt <= max_holes) {
					c->corresp[c->count] = i;
					printf("\n** DEBUGGONE:\n");
					printf("\nCounter - Position: %i - %lli\n", c->count, c->corresp[c->count]);
					(c->count)++;
				}
			}
		}

		err_cnt = 0;
		count_L_hole = 0;
	}

	// Trim memory:
	c->corresp = realloc(c->corresp, c->count * sizeof(long long int));

	return c->count;
}

int main() {

	long long int seq_len, pat_len;
	conteggio c_fw, c_bck;
	
	// User-acquired inputs:
	printf("\n* Insert desired sequence length: ");
	scanf("%lli", &seq_len);
	printf("\n* Insert desired pattern length: ");
	scanf("%lli", &pat_len);

	printf("\n==> Maximum allowed number of holes: %i\n", (int) pat_len / H_ERR);
	
	char *sequence = malloc(seq_len * sizeof(char));
	char *pattern = malloc(pat_len * sizeof(char));
	char *rev_pat = malloc(pat_len * sizeof(char));

	if(!sequence || !pattern || !rev_pat) {
		fprintf(stderr, "Malloc failed\n");
		return 2;
	}

	if (pat_len > seq_len) {
		fprintf(stderr, "ERROR ON SELECTED SIZES!\n");
		return 4;
	}

	double time_init = omp_get_wtime();

	// (Random) generation of sequence and pattern
	populate(sequence, seq_len);
	populate(pattern, pat_len);

	for (int i = 0; i < pat_len; i++) {
		rev_pat[i] = pattern[pat_len-i-1];
	}
	rev_pat[pat_len] = '\0';

	// check patterns for errors (to be removed)
	//printf("\nPattern : %s <---> %s \n", pattern, reverse_pat);
	printf("\n\nSEQUENCE: %s;\nPATTERN: %s\n\n", sequence, pattern);
 
 	// Fwd sequencing with holes:
 	// Backward sequencing is the same as 'fwd', but with reversed
 	// pattern
	c_fw.count = hole_forward(sequence, pattern, seq_len, pat_len, &c_fw);
	c_bck.count = hole_forward(sequence, rev_pat, seq_len, pat_len, &c_bck);
	printf("\n%i correspondances found! (%i forward, %i backward)\n", (c_fw.count + c_bck.count), c_fw.count, c_bck.count);

	printf("\nPositions of the matches:\n");
	for (int i = 0; i < c_fw.count; i++) {
		printf("* %lli\n", c_fw.corresp[i] + 1);
	}

	// In bckwd add pat_len as corresp. idx points to the end of the comparison
	for (int i = 0; i < c_bck.count; i++) {
		printf("* %lli\n", c_bck.corresp[i] + pat_len);
	}

	double time_end = omp_get_wtime();
	printf("\nTiming: %.5f\n\n", time_end-time_init);

	free(sequence);
	free(pattern);
	free(rev_pat);
	free(c_fw.corresp);
	free(c_bck.corresp);

	return 0;
}