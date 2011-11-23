/*-
 * Copyright (c) 2011-2012 Zhao pengcheng. All Rights Reserved.
 *
 * Huffman coding implement.
 */

#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include "queue.h"

#define HUFFMAN_OK  1
#define HUFFMAN_ERR 0

#define RANGE_SIZE 256

enum CHAR_NODE_TYPE {
	INTERNAL_NODE,
	LEAF_NODE
};

typedef struct char_node {
	int           type;
	unsigned char ch;
	uintmax_t     weight;
	struct {
		struct char_node *lc;
		struct char_node *rc;
		struct char_node *pa;
	} tree_entry;
	TAILQ_ENTRY(char_node) queue_entry;
} CHAR_NODE;

TAILQ_HEAD(priority_queue, char_node);

typedef struct huffman_h {
	char name[NAME_MAX];
	int n;
	uintmax_t frequency[RANGE_SIZE];
	/* minimum priority queue */
	struct priority_queue mpq;
	CHAR_NODE *huff_tree;
	CHAR_NODE *leaf_node[RANGE_SIZE];
	char code_table[RANGE_SIZE][(int)(log10(RANGE_SIZE)/log10(2))+1];
} HUFFMAN_H;

HUFFMAN_H *huffman_init(const char *name);
void huffman_reinit(HUFFMAN_H *hfh);
void huffman_deinit(HUFFMAN_H *hfh);
void compute_frequency(HUFFMAN_H *hfh, const unsigned char *b, size_t l);
int compute_frequency_file(HUFFMAN_H *hfh, const char *file);
int write_wetghts_log(HUFFMAN_H *hfh);
int load_wetghts_log(HUFFMAN_H *hfh, const char *file);
int create_huffman_tree(HUFFMAN_H *hfh);
int create_huffman_code_table(HUFFMAN_H *hfh);
unsigned char *huffman_compress(HUFFMAN_H *hfh, unsigned char *src, unsigned int *len);
unsigned char *huffman_decompress(HUFFMAN_H *hfh, unsigned char *src, unsigned int *len);

#endif

