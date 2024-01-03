#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "engine.h"

#define  BUFFER 256 

#define WEBFILE "web.txt"

int main (void) {

	//Dimensioning variables from lab 4
	size_t  dimension_t = 0;
	char line_buffer[BUFFER];
	FILE* web_file = NULL;
		
	 fopen_s(&web_file, WEBFILE, "r");

	char** array_con;		//2D connectivity matrix
	int i, j; 
	char line_buffer2[BUFFER];

	//MATlAB variables
	Engine* ep = NULL; // A pointer to a MATLAB engine object
	mxArray* ConnectivityMatrix = NULL, * result = NULL; // mxArray is the fundamental type underlying MATLAB data

	//start of dimension code taken from lab 4
	dimension_t = strlen(fgets(line_buffer, BUFFER, web_file));
	int dimension = (int)dimension_t;

	fseek(web_file, 0, SEEK_SET);

	/* Checks if text file was created in Windows and contains '\r'
	   IF TRUE reduce strlen by 2 in order to omit '\r' and '\n' from each line
	   ELSE    reduce strlen by 1 in order to omit '\n' from each line */
	if (strchr(line_buffer, '\r') != NULL) {
		// INSERT CODE HERE (1 line)
		dimension -= 2;
	}
	else {
		// INSERT CODE HERE (1 line)
		dimension--;
	}
	//end of dimension code taken from lab 4

	array_con = (char**)malloc(dimension * sizeof(char*));
	for (i = 0; i < dimension; i++) {
		array_con[i] = (char*)malloc(dimension * sizeof(char));
		fgets(array_con[i], dimension, web_file);
	}

	j = 0;
	
	//this is basically the maze parser from lab 4 but modified :) 
	while (fgets(line_buffer2, BUFFER, web_file)) {			//loop continues so long as there's stuff to copy from web_file
		for (j = 0; j < dimension; j++) {					//j essentially corresponds to the column of the matrix
			array_con[i][j] = line_buffer2[i*dimension + j]; //copying from the line buffer
		}
		i++;
	}

	//These are from the in-lab exercises
		/* Starts a MATLAB process */
	if (!(ep = engOpen(NULL))) {
		fprintf(stderr, "\nCan't start MATLAB engine\n");
		system("pause");
		return 1;
	}

	//creates a 2-D double-precision floating-point array initialized to 0
	ConnectivityMatrix = mxCreateDoubleMatrix(dimension, dimension, mxREAL);

	//copy the data from our local 2-D array time to the MATLAB variable testArray
	//mxGetPr which returns a pointer to the first element of our mxArray
	memcpy((void*)mxGetPr(ConnectivityMatrix), (void*)array_con, dimension*dimension * sizeof(double));

	//engPutVariable writes the mxArray to the engine and gives it the specified variable name.
	if (engPutVariable(ep, "ConnectivityMatrix", ConnectivityMatrix)) {
		fprintf(stderr, "\nCannot write the matrix to MATLAB \n");
		system("pause");
		exit(1); // Same as return 1;
	}

	//Technically I could also import the dimensions from the C code, but this should work too
	engEvalString(ep, "[rows, columns] = size(ConnectivityMatrix)");
	engEvalString(ep, "dimension = size(ConnectivityMatrix, 1)");

	//engEvalString. This function accepts two parameters: a) the first parameter is the pointer to the engine; 
//b) the second is a string representing the command we wish the engine to evaluate.
	if (engEvalString(ep, "columnsums = sum(ConnectivityMatrix, 1))")) {
		fprintf(stderr, "\nError summing columns  \n");
		system("pause");
		exit(1);
	}

	//While I should include an if statement for each command, I didn't so that there'd be less for you to look at :)
	//(this isn't just me being lazy I swear)
	//Making the stochastic:
	engEvalString(ep, "p = 0.85");
	engEvalString(ep, "zerocolumns = find(columnsums~=0)");
	engEvalString(ep, "D = sparse( zerocolumns, zerocolumns, 1./columnsums(zerocolumns), dimension, dimension)");
	engEvalString(ep, "StochasticMatrix = ConnectivityMatrix * D");
	engEvalString(ep, "[row, column] = find(columnsums==0)");
	engEvalString(ep, "StochasticMatrix(:, column) = 1./dimension");		

	//Transition Matrix
	engEvalString(ep, "Q = ones(dimension, dimension)");
	engEvalString(ep, "TransitionMatrix = p * StochasticMatrix + (1 - p) * (Q/dimension)");

	engEvalString(ep, "PageRank = ones(dimension, 1)");

	//Multiplication Iteration
	engEvalString(ep, "for i = 1:100 PageRank = TransitionMatrix * PageRank; end");

	//Normalizing
	engEvalString(ep, "PageRank = PageRank / sum(PageRank)");

	//Rerieving PageRank
	printf("\nRetrieving PageRank\n");
	if ((result = engGetVariable(ep, "PageRank")) == NULL) {
		fprintf(stderr, "\nFailed to retrieve eigenvalue vector\n");
		system("pause");
		exit(1);
	}

	//Printing result
	else {
		size_t sizeOfResult = mxGetNumberOfElements(result); //we can just use one size because all the matrices
															 //should have the same size
		i = 0;
		printf("NODE  RANK\n");
		printf("---   ----\n");
		for (i = 1; i < sizeOfResult+1; ++i) {
			printf("%i     %f\n", i, *(mxGetPr(result) + i));
		}
	}

	system("pause"); 
	return 0; 

}