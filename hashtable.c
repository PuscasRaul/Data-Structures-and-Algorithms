#include "hashtable.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t max_load_factor = 32; 

void h_init(struct HTab *htab, const size_t size) {
	if (size < 0 && ((size - 1) && size) == 0) {
		htab = NULL;
		perror("invalid size");
		return;
	}

	htab->Tab = (struct HNode**) calloc(size, sizeof(struct HNode*));

	if (!htab->Tab) {
		perror("calloc tab");
		return;
	}

	htab->mask = size - 1;
	htab->size = 0;
}

void h_insert(struct HTab *htab, struct HNode *hnode) {
	// find the position
	// insert at the beggining of the chain

	if (!htab) 
		return;

	if (!htab->Tab) {
		h_init(htab, 4); // go with powers of 2
	}

	if (htab->size >= htab->mask) {
		struct HTab *new_htab = ht_resize(htab);
		if (!new_htab)
			return;
		free(htab);
		htab = new_htab;
	}

	size_t pos = hnode->hcode & htab->mask;
	struct HNode *next = htab->Tab[pos];
	hnode->next = next;
	htab->Tab[pos] = hnode;
	++htab->size;
	printf("insert number %ld\n", hnode->hcode);
}

struct HNode *ht_lookup(const struct HTab *htab, const struct HNode *hnode) {
	if (!htab->Tab) {
		perror("empty htable");
		return NULL;
	}

	// get hnode hashcode
	// go to that pos in tab if it exists
	// keep going on that chain untill it's null or we find it
	
	size_t pos = hnode->hcode & htab->mask;
	if (!htab->Tab[pos]) {
		return NULL;
	}

	struct HNode *node = htab->Tab[pos];
	while (node && node->hcode != hnode->hcode) {
		node = node->next;
	}

	return node;
}

struct HNode *ht_deletion(struct HTab *htab, const struct HNode *hnode) {
	printf("deletion hcode: %ld\n", hnode->hcode);
	if (!htab->Tab) {
		perror("empty htable");
		return NULL;
	}

	if (!hnode) {
		perror("hnode is null");
		return NULL;
	}

	// get the position of the element
	// search for it
	// if we find it
	// update the links 
	// and idk see from there, hopefully it works

	size_t pos = hnode->hcode & htab->mask;
	struct HNode* node = htab->Tab[pos];
	if (!node) 
		return NULL;
	
	struct HNode *previous = node;

	while (node && node != hnode) {
		previous = node;
		node = node->next;
	}

	if (!node) 
		return NULL;
	

	// at the beggining of the chain
	if (node == previous) {
		htab->Tab[pos] = node->next; // we point it to the next element in chain
	}
		
	// otherwise we are somewhere inside the chain
	previous->next = node->next; // we skip over current element


	// go on freeing and returning the value
	struct HNode *return_node = (struct HNode*) realloc(node, sizeof(struct HNode));
	if (!return_node) {
		perror("malloc deletion");
		return NULL;
	}

	--htab->size;
	return return_node;
} 

static struct HTab *ht_resize(struct HTab *htab) {

	if (!htab) {
		return NULL;
	}

	struct HTab *new_htab = (struct HTab*) malloc(sizeof(struct HTab));

	if (!new_htab) {
		perror("malloc resize");
		return NULL;
	}

	h_init(new_htab, 2 * (htab->mask + 1));

	if (!new_htab->Tab) {
		perror("replacement htab error");
		return NULL;
	}

	for (size_t i = 0; i <= htab->mask; i++) {
		if (!htab->Tab[i])
			continue; 

		struct HNode *node = htab->Tab[i];

		// go through chain, add and progressively delete the old htab 
		while (node) {
			struct HNode *next_node = node->next; 
			struct HNode *deleted_node = ht_deletion(htab, node);
			h_insert(new_htab, deleted_node); // this works because i deep 
																									 // copy it in ht_deletion
																									 // might introduce some 
																									 // mem leaks but well
																									 // life sucks don't it
			node = next_node;
		}
	}
	
	htab = new_htab; // this is a very lanky
									 // and i mean very lanky resize

	return new_htab;
}


