#ifndef _INPUT_H
#define _INPUT_H 1

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>
#include "seg.h"

#define COMMENT_CHAR '#'
#define SEG_DELIM " "
#define SEGS_DELIM " | "
#define SYL_DELIM "."
#define SYM_BOS "<"     // beginning of sequence (e.g., utternace)
#define SYM_EOS ">"     // end of sequence

/**
 * struct basic_unit - the basic/indivisible unit
 * @str:               a string representation, used for reading/writing
 * @feat:              a bit vector of features, it should be one of UFEAT_*
 *
 * This structure holds the information about the unit types, not
 * tokens. Unit tokens, represented below with struct segunit, holds a
 * pointer to a struct basic_unit together with other information
 * specific to a particular token.
 */
struct basic_unit {
    char *str;
    segfeature_t feat;
};

/**
 * struct units    - the structure for holding a sequence of basic unit tokens
 * @len:             the length of the sequence.
 * @seq:             the indexes pointing to the corresponding 
 *                   'basic_unit's in the input lexicon. 
 * @feat:            an array features for each unit. 
 *                   For the length number of
 *                   features one should check seq[0].
 */
struct unitseq {
    size_t len;
    segunit_t *seq;
    segfeature_t *feat;
};

/**
 * struct utterance - representation of an utterance
 * @phon:             the sequence of phonemes that form the utterance
 * @syl:              the sequence of syllables that form the utterance,
 *                    may be NULL.
 */
struct utterance {
    struct unitseq *phon;
    struct unitseq *syl;
    struct segmentation *gs_seg;  // gold-standard segmentation
    struct segmentation *syl_seg; // syllable segmentation
};

struct input {
    struct utterance **u;     // array of utterances 
    struct basic_unit *sigma; // alphabet of basic units, shared btw. phon/syl
    GHashTable *hash_phon;    // for reverse lookups of phonemes
    GHashTable *hash_syl;     // for reverse lookups of syllables
    segunit_t *sigma_idx;     // silly workaround for GHashTable w/ int values
    size_t len;               // number of phoneme types in the input
    size_t sigma_nph;         // number of phoneme types in the input
    size_t sigma_nsyl;        // number of syllable types in the input
    size_t alloc;             // amount of memory allocated for u (int. use)
    size_t sigma_len;         // size of the basic unit alphabet
    size_t sigma_alloc;       // amount of memory allocated for sigma (int.)
    uint8_t mode;             // input processing mode (see below)
    const char *file_name;    // name of the input file 
    const char *sfile_name;   // name of the stress input file
};

/* input mode: we always do phoneme (or character) processing. the
 * following determine other types of processing of the input:
 */
#define INPMODE_IPA  1 // interpret input characters as IPA characates
#define INPMODE_SYL  (1 << 1) // syllables are marked in the input (by a `.')
#define INPMODE_PSYL (1 << 2) // assign pseudo syllables.
#define INPMODE_STRSEXT (1 << 3) // assign stress from an external file
#define INPMODE_STRSIPA (1 << 4) // use IPA stress assignments


/* input_read() - read the input file(s) and retrun as a struct input
 * @finput        main input file with phonemes
 * @sfstress      input file containing stress patterns (NULL is OK)
 * @mode          input mode specified by a combination of INPMODE_*
 *                macros above.
 */
struct input *input_read(const char *finput, 
        const char *fstress, 
        const unsigned mode);

/* input_shuffle() - shuffle the utterances in @input
 * @input            pointer to the input structure
 * @seed             seed for srand(). If the seed is 0, seed is
 *                   set from the current time.
 */
void input_shuffle(struct input *in, unsigned seed);

/* input_free() - free the memory allocated during input_read()
 * @in            pointer to the input to be freed.
 */
void input_free(struct input *in);


/**
 * segment_to_str() - convert a given segmentation to its string
 *                    representation.
 * @in:               input structure.
 * @idx:              index of the utterance in the input strucutre
 *                    that segmentation belongs to.
 * @seg:              a single segmentation
 */
char *segment_to_str(struct input *in, size_t idx, struct segmentation *seg);

/**
 * write_segs() - output segmentations in @segs to @outf
 * @outf:         name of the file the output should be written. `-'
 *                means standard output.
 * @segs:         array of segmentations (as described in seg.h). The
 *                number of segmentations must be equal to @inp->len.
 * @inp:          input structure, that contains the lexicon for
 *                converting indices in @segs to strings.
 */
void write_segs(char *outf, struct segmentation **segs, struct input *inp);


/** TODO: clean up
 * struct output - Output from a segmentation algorithm
 * @in:            The input that the output is associated with.
 * @seglist:       an array of segmentations. 
 *
 * Each segmentation in @seglist is an array of segunit_t. The first
 * element (with index 0) specifies the number of segments, and the
 * remaining elements indicate the location of the boundaries. The
 * beginning and the end of the whole sequence is not listed.
 * Locations are with respect to the units in in->phon.  For example,
 * assuming each letter is a phoneme, the segmentation 'a def gh' wuld
 * be represented as {2, 1, 4}.
 *
 * The length of the @seglist lhas to be in->len.
struct output {
    struct input        *in;
    segunit_t           **seglist; // an array of pointers to segmentations
};
*/

#endif // _INPUT_H

