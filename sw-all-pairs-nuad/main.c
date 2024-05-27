/*
  Coded by Chris Thachuk, 2015.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "sw-vector.h"

struct alignment_spec {
    int match;
    int mismatch;
    int gap_open;
    int gap_extend;
    int num;
    int length;
    const int8_t * sequence;
};

void usage() {
    fprintf(stdout, "usage: sw-score-all-pairs <match> <mismatch> <gap_open> <gap_extend> <num> <length> <sequence>\n\n");
    fprintf(stdout, "\t<match>      score for matching characters\n");
    fprintf(stdout, "\t<mismatch>   score for mismatching characters\n");
    fprintf(stdout, "\t<gap_open>   score to open a gap\n");
    fprintf(stdout, "\t<gap_extend> score to extend a gap\n");
    fprintf(stdout, "\t<num>        total number of equi-length sequences to compare\n");
    fprintf(stdout, "\t<length>     length of every sequence\n");
    fprintf(stdout, "\t<sequence>   concatenation of every sequence - must have length <num>*<length>\n");
    fprintf(stdout, "\n");
}

bool parse_cmd(int argc, const char *argv[], struct alignment_spec *spec) {
    if (argc != 8) {
        usage();
        return false;
    }

    spec->match = atoi(argv[1]);
    spec->mismatch = atoi(argv[2]);
    spec->gap_open = atoi(argv[3]);
    spec->gap_extend = atoi(argv[4]);
    spec->num = atoi(argv[5]);
    spec->length = atoi(argv[6]);
    spec->sequence = (int8_t *)argv[7];

    assert(spec->match >= 0);
    assert(spec->mismatch >= spec->match);
    assert(spec->gap_open < 0);
    assert(spec->gap_extend < 0);
    assert(spec->length >= 1);
    assert(spec->num >= 2);
    assert(spec->length >= 1);
    assert((unsigned long)(spec->num * spec->length) == strlen((const char *)spec->sequence));
        
    return true;
}

/* Returns the complement base if b is in set 'acgtACGT'.  For any
 * other character, returns 'a'. */
inline int8_t rc_base(int8_t b) {
    switch((char)b) {
    case 'a': case 'A':
        return (int8_t)'t';
    case 'c': case 'C':
        return (int8_t)'g';
    case 'g': case 'G':
        return (int8_t)'c';
    default:
        return (int8_t)'a';
    }
}

/* Writes the reverse complement of sequence beginning at src, of
 * length len, into dst.  Assumes memory for dst has been
 * pre-allocated. */
inline void rc(const int8_t *src, int len, int8_t *dst) {
    int i;
    for(i=0; i < len; ++i) {
        dst[len-i-1] = rc_base(src[i]);
    }
}

int main(int argc, const char *argv[]) {
    int i, j, aoff, boff, score;
    struct alignment_spec spec;
    int8_t *rc_seq;
    
    if(!parse_cmd(argc, argv, &spec)) { return -1; };

    if (sw_vector_setup(spec.length, spec.length, spec.gap_open, spec.gap_extend, spec.match, spec.mismatch) != 0) {
        fprintf(stderr, "Error initializing alignment algorithm.  Aborting.\n");
        return -1;
    }

    /* allocate memory to hold reverse complement sequence */
    rc_seq = malloc(spec.length * sizeof(int8_t));

    for(i = 0; i < spec.num - 1; ++i) {
        aoff = i * spec.length;
        rc(&spec.sequence[aoff], spec.length, rc_seq);
        for(j = i + 1; j < spec.num; ++j) {
            boff = j * spec.length;
            score = sw_vector(rc_seq, 0, spec.length, spec.sequence, boff, spec.length);
            fprintf(stdout, "%d %d %d\n", i, j, score);
        }
    }

    /* release allocated memory */
    sw_vector_cleanup();
    free(rc_seq);

    return 0;
}