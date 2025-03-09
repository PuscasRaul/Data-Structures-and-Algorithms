/*
Să se determine distanța Euclideană între două locații identificate prin perechi de numere. De ex. distanța între (1,5) și (4,1) este 5.0
*/

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// read the input
// and parse the expression, so that we can extract the points on the axis
// check the dimensions
// compute the formula
// easy peasy

size_t extract_point(char *coordinates, double *return_value, size_t *offset) {
	size_t dimension = 0;
	for (size_t i = 0; i < strlen(coordinates); ++i) {
		size_t start_point = i;

		while (coordinates[i] != ',') {
			++i;
		}	

		char *number = malloc(sizeof(char) * (i - start_point + 1));
		memcpy(number, &coordinates[start_point], i - start_point);
		number[i - start_point] = '\0';
		double value = strtod(number, NULL);
		
		if (i == start_point) {
			continue;
		}	

		// printf("%.2f ", value);
		++dimension;
		return_value[*offset] = value;
		++*offset;
	}

	// printf("\ndimension: %ld\n", dimension);
	return dimension;
}

ssize_t parse_expression(char *expression, double *coordinates) {
	// we can be sure
	// unless some mf dumbass fucks it up and sends wrong format
	// that expression[0] is a (
	// but we'll check nonetheless
	// if he does fck him

	size_t offset = 0;
	size_t dimensions;

	while (expression[0] != '(') {
		for (size_t i = 0; i < strlen(expression); i++) 
			expression[i] = expression[i + 1];
	}

	size_t i = 1;
	while (expression[i] != ')') 
		++i;

	// so now we know the start and end 
	// of our first point, yabadabadoo extract it

	char point[128];
	memcpy(point, &expression[1], i-1); // copy until ) 
	point[i - 1] = '\0';
	// printf("%s\n", point);
	dimensions = extract_point(point, coordinates, &offset);	// i kinda interlaced
																			// the words point coordinates
																			// idk what each means anymore
	
	// pretty much do the same thing
	while (expression[i] != '(')
		++i;

	size_t nd_point = i;

	while (expression[i] != ')')
		++i;

	memcpy(point, &expression[nd_point + 1], i - nd_point - 1);
	point[i - nd_point - 1] = '\0';
	// printf("%s\n", point);

	if (dimensions != extract_point(point, coordinates, &offset)) {
		perror("dimensions do not match");
		return -1;
	}

	return dimensions;
}

double_t compute_distance(double *coordinates, ssize_t dimension) {
	double sum = 0;
	for (size_t i = 0; i < dimension; i++) {

		sum += pow(coordinates[dimension + i] - coordinates[i], 2);
	}

	return sqrt(sum);
}

int main(int argc, char *argv[]) {
	char line[256]; // i'll just hope that we are under 100 dimensions
									// or so
									// idk how many there are considering , and spaces

	fprintf(stdout, "Correct format is (a,b,...) (x,y,...,z)\n");
	fprintf(stdout, "Introduce the 2 points: ");
	if (!fgets(line, 256, stdin)) {
		perror("fgets");
		return 1;
	}

	
	double coordinates[256];
	ssize_t dimensions;
	dimensions = parse_expression(line, coordinates);
	if (dimensions < 0) {
		return 1;
	}
	
	printf("distance: %.2f\n", compute_distance(coordinates, dimensions));

	// printf("%s\n", line);
	
	return 0;
}
