#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define container_of(ptr, T, memb) (T*) ((char*) ptr - offsetof(T, memb));

struct HNode {
	uint_fast32_t hcode;	
	struct HNode *next;
};

struct HTab {
	struct HNode **Tab;
	size_t mask;
	size_t size;
};

void h_init(struct HTab *htab, const size_t size); 
void h_insert(struct HTab *htab, struct HNode *hnode); 
struct HNode *ht_lookup(const struct HTab *htab, const struct HNode *hnode);
struct HNode *ht_deletion(struct HTab *htab, const struct HNode *hnode);
static struct HTab *ht_resize(struct HTab *htab);
