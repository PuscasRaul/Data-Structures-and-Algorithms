#include <stddef.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {

	const char *text = "The original version of the algorithm focused on compression. It replaces the highest-frequency pair of bytes with a new byte that was not contained in the initial dataset. A lookup table of the replacements is required to rebuild the initial dataset. The modified version builds \"tokens\" (units of recognition) that match varying amounts of source text, from single characters (including single digits or single punctuation marks) to whole words (even long compound words)";

	ssize_t size_text = strlen(text);
	for (ssize_t i = 0; i < size_text - 1; i++) {
		char a = text[i];
		char b = text[i + 1];
		printf("%c%c\n", a, b);
	}

	return 0;
}
