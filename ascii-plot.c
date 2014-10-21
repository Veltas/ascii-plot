#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

enum {
	PLOT_CHAR  = 'X',
	MIN_COLS   = 5,
	MIN_ROWS   = 5,
	ERROR_NONE = 0,
	ERROR
};

/* Reads a size from a string, given a minimum */
int ReadSize(const char *const str, const int min, int *const size)
{
	if (sscanf(str, "%d", size) != 1) {
		fputs("ReadSize: failed to read from given string\n", stderr);
		return ERROR;
	} else if (*size < min) {
		fprintf(stderr, "ReadSize: given size %d, smaller than min %d\n", *size, min);
		return ERROR;
	} else {
		return ERROR_NONE;
	}
}

/* Gets information about the state of a file ready for reading the graph data */
int DrawGraphPrep(FILE *const   inputFile,
                  int *const    n,
                  double *const xBounds,
                  double *const yBounds)
{
	int    length          = 0;
	char   reading         = 1;
	double currentPoint[2];
	yBounds[1] = yBounds[0] = xBounds[1] = xBounds[0] = 0.0;
	rewind(inputFile);
	while (reading) {
		if (fscanf(inputFile, " %lg", &currentPoint[0]) != 1) {
			reading = 0;
		}
		if (reading) {
			if (fscanf(inputFile, " %lg", &currentPoint[1]) != 1) {
				fputs("DrawGraphPrep: read odd number of valid input numbers\n", stderr);
				return ERROR;
			}
			++length;
			if (currentPoint[0] < xBounds[0]) {
				xBounds[0] = currentPoint[0];
			}
			if (xBounds[1] < currentPoint[0]) {
				xBounds[1] = currentPoint[0];
			}
			if (currentPoint[1] < yBounds[0]) {
				yBounds[0] = currentPoint[1];
			}
			if (yBounds[1] < currentPoint[1]) {
				yBounds[1] = currentPoint[1];
			}
		}
	}
	*n = length;
	assert(length >= 0);
	return ERROR_NONE;
}

int DrawGraph(const int   columns,
              const int   rows,
              FILE *const inputFile,
              FILE *const outputFile)
{
	char   *toWrite;
	int    points,
	       i,
	       j,
	       currentX,
	       currentY;
	double xBounds[2],
	       yBounds[2],
	       columnWidth,
	       rowHeight,
	       currentDat[2];
	assert((columns >= MIN_COLS) && (rows >= MIN_ROWS));
	/* Allocate space for graph buffer */
	toWrite = malloc(columns * rows);
	if (toWrite == NULL) {
		fputs("Failed to allocate graph buffer.\n", stderr);
		return ERROR;
	}
	/* Initialise graph buffer to spaces */
	memset(toWrite, ' ', columns * rows);
	/* Determine bounds and number of points from samples */
	if (DrawGraphPrep(inputFile, &points, xBounds, yBounds)) {
		fputs("Failed to calculate length of file.\n", stderr);
		free(toWrite);
		return ERROR;
	}
	if (fseek(inputFile, 0, SEEK_SET)) {
		fputs("Failed to rewind file after checking info.\n", stderr);
		free(toWrite);
		return ERROR;
	}
	rowHeight = (yBounds[1] - yBounds[0]) / ((double) rows - 1);
	/*fprintf(stderr, "DEBUG: Row height %f\n", rowHeight);*/
	columnWidth = (xBounds[1] - xBounds[0]) / ((double) columns - 1);
	/*fprintf(stderr, "DEBUG: Column width %f\n", columnWidth);
	fprintf(stderr, "DEBUG: %d points\n", points);*/
	for (i = 0; i < points; ++i) {
		if (!(fscanf(inputFile, " %lg", &currentDat[0]) && fscanf(inputFile, " %lg", &currentDat[1]))) {
			fprintf(stderr, "Failed processing line %d of the file.\n", i);
			free(toWrite);
			return ERROR;
		}
		/*fprintf(stderr, "DEBUG: Read point %f %f\n", (float) currentDat[0], (float) currentDat[1]);*/
		currentX = (int) ((currentDat[0] - xBounds[0]) / columnWidth);
		assert((currentX >= 0) && (currentX < columns));
		currentY = (int) ((currentDat[1] - yBounds[0]) / rowHeight);
		/*fprintf(stderr, "DEBUG: Writing point %d %d\n", currentX, currentY);*/
		currentY = rows - currentY - 1;
		assert((currentY >= 0) && (currentY < rows));
		toWrite[currentX + currentY * columns] = PLOT_CHAR;
	}
	for (j = 0; j < rows; ++j) {
		for (i = 0; i < columns; ++i) {
			if (fputc(toWrite[i + j * columns], outputFile) == EOF) {
				fputs("Output error occured while outputting the graph.\n", stderr);
				free(toWrite);
				return ERROR;
			}
		}
		if (fputc('\n', outputFile) == EOF) {
			fputs("Output error occured while outputting the graph.\n", stderr);
			free(toWrite);
			return ERROR;
		}
	}
	free(toWrite);
	return ERROR_NONE;
}

int main(int argc, char **argv)
{
	int  columns    = 79,
	     rows       = 20;
	FILE *inputFile;
	/* Set columns and rows appropriately */
	if (argc == 2) {
		/* Do nothing */
	} else if (argc == 4) {
		/* read columns and rows from arguments */
		if (ReadSize(argv[2], MIN_COLS, &columns)) {
			fputs("Failed to read column count from second argument\n", stderr);
			return EXIT_FAILURE;
		}
		if (ReadSize(argv[3], MIN_ROWS, &rows)) {
			fputs("Failed to read row count from third argument\n", stderr);
			return EXIT_FAILURE;
		}
	} else {
		fputs("Expected three arguments, input file, column count, and then row count.\n", stderr);
		return EXIT_FAILURE;
	}
	/* Open given file */
	inputFile = fopen(argv[1], "r");
	if (inputFile == NULL) {
		fprintf(stderr, "Failed to open file %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	/* Output graph */
	if (DrawGraph(columns, rows, inputFile, stdout)) {
		fputs("Failed to draw the graph.\n", stderr);
		fclose(inputFile);
		return EXIT_FAILURE;
	} else {
		fclose(inputFile);
		return EXIT_SUCCESS;
	}
}
