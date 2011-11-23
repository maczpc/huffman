/*-
 * Copyright (c) 2011-2012 Zhao pengcheng. All Rights Reserved.
 *
 * Huffman coding implement.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "huffman.h"

/* internal function declaration */
static size_t strlcpy(char *dst, const char *src, size_t siz);
static char *strrev(char * string);

HUFFMAN_H *huffman_init(const char *name) {
	HUFFMAN_H *ret;
	size_t s;

	ret = calloc(1, sizeof(HUFFMAN_H));
	assert(ret != NULL);

	s = strlcpy(ret->name, name, NAME_MAX);
	assert(s < NAME_MAX);

	return (ret);

}

/*
 * inorder tree walk
 */
static void inorder_tree_walk(CHAR_NODE *tp) {
	if (tp != NULL) {
		inorder_tree_walk(tp->tree_entry.lc);
		free(tp);
		inorder_tree_walk(tp->tree_entry.rc);
	}
}

void huffman_deinit(HUFFMAN_H *hfh) {
	if (TAILQ_EMPTY(&hfh->mpq)) {
		//printf("empty\n");
	}
	inorder_tree_walk(hfh->huff_tree);
	free(hfh);
}

void compute_frequency(HUFFMAN_H *hfh, const unsigned char *b, size_t l) {
	assert(hfh != NULL);
	assert(b != NULL);

	size_t i = 0;
	for (i = 0; i < l; i++) {
		hfh->frequency[b[i]]++;
	}
}

int load_wetghts_log(HUFFMAN_H *hfh, const char *file){
	FILE *f;
	char path[PATH_MAX];
	unsigned int i;
	char buf[NAME_MAX];

	if (file != NULL) {
		strlcpy(path, file, PATH_MAX);
	} else {
		snprintf(path, PATH_MAX, "%s.wlog", hfh->name);
	}
	f = fopen(path, "r");
	if (NULL == f) {
		fprintf(stderr, "open \"%s\" faild. [%s]",
				path, strerror_r(errno, NULL, 0));
		return (HUFFMAN_ERR);
	}
	for (i = 0; i < RANGE_SIZE; i++) {
		fgets(buf, 1024, f);
		hfh->frequency[i] =
			(uintmax_t)strtoull(buf, NULL, 10);
	}
	if (fclose(f) == EOF) {
		fprintf(stderr, "close \"%s\" faild. [%s]",
				hfh->name, strerror_r(errno, NULL, 0));
		return (HUFFMAN_OK);
	}

	return (HUFFMAN_OK);
}

int write_wetghts_log(HUFFMAN_H *hfh) {
	FILE *f;
	int s;
	char path[PATH_MAX];

	s = snprintf(path, PATH_MAX, "%s.wlog", hfh->name);
	assert(s < PATH_MAX);

	f = fopen(path, "w");
	if (NULL == f) {
		fprintf(stderr, "open \"%s\" faild. [%s]",
				path, strerror_r(errno, NULL, 0));
		return (HUFFMAN_ERR);
	}

	for (s = 0; s < RANGE_SIZE; s++) {
		fprintf(f, "%ju\n", hfh->frequency[s]);
	}

	if (fclose(f) == EOF) {
		fprintf(stderr, "close \"%s\" faild. [%s]",
				hfh->name, strerror_r(errno, NULL, 0));
		return (HUFFMAN_OK);
	}

	return (HUFFMAN_OK);
}

int character_count(HUFFMAN_H *hfh) {
	int i, n = 0;

	for (i = 0; i < RANGE_SIZE; i++) {
		if (hfh->frequency[i] > 0) {
			n++;
		}
	}
	return (n);
}

void mpq_insert(struct priority_queue *mpq, CHAR_NODE *chp) {
	CHAR_NODE *var;
	CHAR_NODE *max;

	if (TAILQ_EMPTY(mpq)) {
		/* create sentinel */
		max = (CHAR_NODE *)calloc(1, sizeof(CHAR_NODE));
		assert(max != NULL);
		max->weight = ULLONG_MAX;
		TAILQ_INSERT_HEAD(mpq, max, queue_entry);
		TAILQ_INSERT_HEAD(mpq, chp, queue_entry);
		return;
	}
	TAILQ_FOREACH(var, mpq, queue_entry) {
		if (chp->weight <= var->weight) {
			break;
		}
	}
	TAILQ_INSERT_BEFORE(var, chp, queue_entry);
}

CHAR_NODE *mpq_extract_min(struct priority_queue *mpq) {
	CHAR_NODE *res;

	if (TAILQ_EMPTY(mpq)) {
		return (NULL);
	}
	res = TAILQ_FIRST(mpq);
	TAILQ_REMOVE(mpq, res, queue_entry);

	return (res);
}

/*
 * create minimum priority queue
 * return: queue size.
 */
int create_mp_queue(HUFFMAN_H *hfh) {
	int i = 0, j = 0;
	CHAR_NODE *chp = NULL;

	TAILQ_INIT(&hfh->mpq);
	for (i = 0; i < RANGE_SIZE; i++) {
		if (hfh->frequency[i] > 0) {
			chp = (CHAR_NODE *)calloc(1, sizeof(CHAR_NODE));
			assert(chp != NULL);
			hfh->leaf_node[j] = chp;
			chp->type = LEAF_NODE;
			chp->ch = i;
			chp->weight = hfh->frequency[i];
			mpq_insert(&hfh->mpq, chp);
			j++;
		}
	}
	/*chp = TAILQ_LAST(&hfh->mpq, priority_queue);
	  assert(chp != NULL);
	  TAILQ_REMOVE(&hfh->mpq, chp, queue_entry);
	  free(chp);*/
}

int create_huffman_tree(HUFFMAN_H *hfh) {
	int n = 0;
	int i = 0;
	CHAR_NODE *z = NULL;
	CHAR_NODE *x = NULL;
	CHAR_NODE *y = NULL;

	hfh->n = n = character_count(hfh);
	assert(n != 0);
	//printf("n = |C| = %d\n", n);

	/* create minimum priority queue */
	create_mp_queue(hfh);

	for (i = 1; i < n; i++) {
		z = (CHAR_NODE *)calloc(1, sizeof(CHAR_NODE));
		assert(z != NULL);
		x = mpq_extract_min(&hfh->mpq);
		assert(x != NULL);
		x->tree_entry.pa = z;
		y = mpq_extract_min(&hfh->mpq);
		assert(y != NULL);
		y->tree_entry.pa = z;
		z->tree_entry.lc = x;
		z->tree_entry.rc = y;
		z->weight = x->weight + y->weight;
		mpq_insert(&hfh->mpq, z);
	}

	/* record huffman tree root */
	hfh->huff_tree = mpq_extract_min(&hfh->mpq);
	/* free sentinel */
	z = TAILQ_LAST(&hfh->mpq, priority_queue);
	assert(z != NULL);
	TAILQ_REMOVE(&hfh->mpq, z, queue_entry);
	free(z);
	return (HUFFMAN_OK);
}

int create_huffman_code_table(HUFFMAN_H *hfh) {
	int i = 0;
	unsigned char ch;
	CHAR_NODE *np, *pp;
	int code_len = (int)(log10(RANGE_SIZE)/log10(2)) + 1;
	char re_code[(int)(log10(RANGE_SIZE)/log10(2)) + 1] = {0};

	for (i = 0; i < hfh->n; i++) {
		np = hfh->leaf_node[i];
		ch = np->ch;
		while (np != hfh->huff_tree) {
			pp = np->tree_entry.pa;
			if (pp->tree_entry.lc == np) {
				/* 0 */
				strcat(re_code, "0");
			} else {
				/* 1 */
				strcat(re_code, "1");
			}
			np = pp;
		}
		strrev(re_code);
		strlcpy(hfh->code_table[ch], re_code, code_len);
		//printf("%c %s\n", ch, re_code);

		re_code[0] = 0;
	}
	return (HUFFMAN_OK);
}

/*
 * para src: input string
 * para len: val-result, input is src len,
 *           output is dest bits
 * return: result buffer
 */
unsigned char *huffman_compress(HUFFMAN_H *hfh, unsigned char *src, unsigned int *len) {
	int i;
	char *code;
	unsigned char *dest, *res;
	unsigned int bits = 0, bits_sum = 0;
	unsigned char tmp = 0x00;

	dest = (unsigned char*)calloc(*len, sizeof(unsigned char));
	assert(dest != NULL);
	res = dest;
	for (i = 0; i < *len; i++) {
		code = hfh->code_table[src[i]];
		while (*code) {
			if (*code == '0') {
				tmp = (tmp << 1);
			} else {
				tmp = (tmp << 1 | 0x01);
			}
			code++;
			bits++;
			bits_sum++;
			if (bits == 8) {
				*dest++ = tmp;
				tmp = 0x00;
				bits = 0;
			}
		}
	}
	if (bits != 0) {
		tmp = tmp << (8-bits);
		*dest = tmp;
	}
	*len = bits_sum;

	return (res);
}

/*
 * para src: input string
 * para len: val-result, input is src bit len,
 *           output is result buffer len
 * return: result buffer
 */
unsigned char *huffman_decompress(HUFFMAN_H *hfh, unsigned char *src, unsigned int *len) {
	unsigned char *dest, *res, bit_ch;
	unsigned int src_len, bits = 0;
	CHAR_NODE *tp = hfh->huff_tree;
	assert(tp != NULL);

	dest = (unsigned char*)calloc(((*len/8)+1)*7,
			sizeof(unsigned char));
	assert(dest != NULL);

	src_len = *len;
	*len = 0;
	res = dest;
	while (src_len--) {
		bit_ch = *src >> (7 - bits);
		bit_ch &= 0x01;
		if (bit_ch == 0x00) {
			tp = tp->tree_entry.lc;
		} else {
			tp = tp->tree_entry.rc;
		}
		if (tp->type == LEAF_NODE) {
			*dest++ = tp->ch;
			(*len)++;
			tp = hfh->huff_tree;
		}
		bits++;
		if (bits == 8) {
			bits = 0;
			src++;
		}
	}
	*dest = '\0';
	return (res);
}

/*
 *char *_strrev(string) - reverse a string in place
 *
 *Purpose:
 *       Reverses the order of characters in the string.  The terminating
 *       null character remains in place.
 *
 *Entry:
 *       char *string - string to reverse
 *
 *Exit:
 *       returns string - now with reversed characters
 *
 *Exceptions:
 *
 */
static char *strrev(char * string) {
	char *start = string;
	char *left = string;
	char ch;

	while (*string++)                 /* find end of string */
		;
	string -= 2;

	while (left < string)
	{
		ch = *left;
		*left++ = *string;
		*string-- = ch;
	}

	return(start);
}

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
static size_t strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	assert(dst != NULL);
	assert(src != NULL);

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0'; /* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1); /* count does not include NUL */
}

int main(int argc, char *argv[]) {
	HUFFMAN_H *hfh = NULL;
	int re;
	int slen;
	unsigned char *rep;

	hfh = huffman_init("zpc");
	compute_frequency(hfh,
			"this is an example of a huffman tree",
			sizeof("this is an example of a huffman tree") - 1);
	re = write_wetghts_log(hfh);
	re = load_wetghts_log(hfh, NULL);
	create_huffman_tree(hfh);
	create_huffman_code_table(hfh);
	slen = sizeof("this is an example of a huffman tree") - 1;
	printf("%d\n", slen);
	rep = huffman_compress(hfh,
			"this is an example of a huffman tree",
			&slen);
	printf("%u\n", slen/8+1);
	rep = huffman_decompress(hfh, rep, &slen);
	printf("%u\n", slen);
	printf("%s\n", rep);
	huffman_deinit(hfh);
}

