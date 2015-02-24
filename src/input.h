#ifndef _INPUT_H
#define _INPUT_H 1

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>
#include "seglist.h"

/* segunit_t is the type used for indexes of (basic) units in the lexicon.
 * It is an unsigned integer type that is large enough to represent
 * all units used (e.g., all phonemes or syllables). Using the
 * smallest unit may save lots of memory.
 */
typedef uint16_t segunit_t;
typedef uint8_t  segfeature_t;

#define COMMENT_CHAR '#'
#define SEG_DELIM " "
#define SEGS_DELIM " | "
#define SYL_DELIM "."
#define SYM_BOS "<"     // beginning of sequence (e.g., utternace)
#define SYM_EOS ">"     // end of sequence

struct basic_unit {
    char *str;      // string representation of the unit
    segfeature_t feat;   // features as a bit mask
};

struct utterance {
    segunit_t *phon; // an array of phon(eme)s
    segunit_t *syl;  // an array of syllables (for convenince)
    segunit_t nphon;
    segunit_t nsyl;
    segunit_t *gs_seg;  // gold-standard segmentation
    segunit_t *syl_seg; // syllable segmentation
    segfeature_t *feat; // array of supasegmental or distorted features
};

#define PHONFEAT_STRESS1 1
#define PHONFEAT_STRESS2 (1 << 1)
#define PHONFEAT_ISSYL   (1 << 2) // entry is a syllable, not phoneme

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

/* Output from a segmentation algorithm - may contain multiple
 * segmentationos per utterance.
 */
struct output {
    size_t              len;
    size_t              alloc; // for memory management
    struct seglist      **seglist; // an array of pointers to 'seglist's
};


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


/* output_new() - create a new output structure
 * @len           number of utterances, for allocating memory at once
 *                if it is known in advance. Otherwise output_add()
 *                will realloc it as needed in BUFSIZ steps.
 */
struct output *output_new(size_t len);


/* output_add() - add a new segmentation list to the output structure
 * @out           pointer to the output structure
 * @segs          the segmentations to add
 *
 * The segmentation list @segs is not copied. The caller should make
 * sure that they are valid during the life time of @out.
 *
 */
void output_add(struct output *out, struct seglist *segs);

/* output_free() - free the memory allocated for the struct output @out
 * @out            the output struture to free
 * @free_seglist   if set to true, the contents of the seglist will
 *                 also be freed. Otherwise only the pointers to the
 *                 lists and the structure itself is destroyed.
 */
void output_free(struct output *out, bool free_seglist);

/* output_write() - write segmentations to the file @outf
 * @outf            The output file
 * @out             The output structure that holds the output of a
 *                  segmentation
 * @inp             The input 
 */
void output_write(char *outf, struct output *out, struct input *inp);

#endif // _INPUT_H

