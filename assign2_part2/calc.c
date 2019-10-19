/* calc.c - Multithreaded calculator */

#include "calc.h"

pthread_t adderThread;
pthread_t degrouperThread;
pthread_t multiplierThread;
pthread_t readerThread;
pthread_t sentinelThread;

char buffer[BUF_SIZE];
int num_ops;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* Utility functions provided for your convenience */

/* int2string converts an integer into a string and writes it in the
   passed char array s, which should be of reasonable size (e.g., 20
   characters).  */
char *int2string(int i, char *s)
{
    sprintf(s, "%d", i);
    return s;
}

/* string2int just calls atoi() */
int string2int(const char *s)
{
    return atoi(s);
}

/* isNumeric just calls isdigit() */
int isNumeric(char c)
{
    return isdigit(c);
}

/* End utility functions */


void printErrorAndExit(char *msg)
{
    msg = msg ? msg : "An unspecified error occurred!";
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

int timeToFinish()
{
    /* be careful: timeToFinish() also accesses buffer */
    return buffer[0] == '.';
}

/* Looks for an addition symbol "+" surrounded by two numbers, e.g. "5+6"
   and, if found, adds the two numbers and replaces the addition subexpression 
   with the result ("(5+6)*8" becomes "(11)*8")--remember, you don't have
   to worry about associativity! */
void *adder(void *arg)
{
    int bufferlen;
    int value1, value2;
    int startOffset, remainderOffset, operator;
    int i, sum;
    char *result;

    // return NULL; /* remove this line */

    while (1) {
	startOffset = remainderOffset = -1;
	value1 = value2 = -1;

	if (timeToFinish()) {
	    return NULL;
	}

	/* storing this prevents having to recalculate it in the loop */

	pthread_mutex_lock(&lock);

	bufferlen = strlen(buffer);
	result = (char*)malloc((bufferlen+1) * sizeof(char));

	for (i = 0; i < bufferlen; i++) {

		// if the first number hasn't been found yet
		if (startOffset == -1) {
			// if the character is numeric 
			if (isNumeric(buffer[i])) {
				// checks to see if the number is followed by +
				if (buffer[i+1] == '+') {
					operator = i+1; // storing the position of the operator '+'
					value1 = string2int(&buffer[i]); // change the character into an int so it can be stored in value1
					startOffset = i; // update the value for the position of value1
				}
			}
		}

		// finding the second number (value 2)
		if (isNumeric(buffer[i])) {
			// checks to make sure that the second number is after the operator
			if (i == (operator+1)) {
				value2 = string2int(&buffer[i]);
				remainderOffset = i;
			}
		}

		if (startOffset != -1 && remainderOffset != -1) {
			// adding the two values together
			sum = value1+value2;
			int2string(sum, result); // converting the sum into a string

			int resultLength = strlen(result);
			int difference = bufferlen-resultLength;

			// adding the sum into the buffer
			strncpy(&buffer[startOffset], result, resultLength);
			// shifting the buffer to the left
			memmove(&buffer[resultLength], &buffer[remainderOffset+1], difference * sizeof(int));
		}
	}
	// something missing?
	int unlockRet = pthread_mutex_unlock(&lock);
	// printf("Add unlock: %d\n", unlockRet);
    }
}

/* Looks for a multiplication symbol "*" surrounded by two numbers, e.g.
   "5*6" and, if found, multiplies the two numbers and replaces the
   multiplication subexpression with the result ("1+(5*6)+8" becomes
   "1+(30)+8"). */
void *multiplier(void *arg)
{
   int bufferlen;
    int value1, value2;
    int startOffset, remainderOffset, operator;
    int i, sum;
    char *result;

    // return NULL; /* remove this line */

    while (1) {
	startOffset = remainderOffset = -1;
	value1 = value2 = -1;

	if (timeToFinish()) {
	    return NULL;
	}

	pthread_mutex_lock(&lock);

	/* storing this prevents having to recalculate it in the loop */
	bufferlen = strlen(buffer);
	result = (char*)malloc((bufferlen+1) * sizeof(char));

	for (i = 0; i < bufferlen; i++) {

		// if the first number hasn't been found yet
		if (startOffset == -1) {
			// if the character is numeric 
			if (isNumeric(buffer[i])) {
				// checks to see if the number is followed by +
				if (buffer[i+1] == '*') {
					operator = i+1; // storing the position of the operator '*'
					value1 = string2int(&buffer[i]); // change the character into an int so it can be stored in value1
					startOffset = i; // update the value for the position of value1
				}
			}
		}

		// finding the second number (value 2)
		if (isNumeric(buffer[i])) {
			// checks to make sure that the second number is after the operator
			if (i == (operator+1)) {
				value2 = string2int(&buffer[i]);
				remainderOffset = i;
			}
		}

		if (startOffset != -1 && remainderOffset != -1) {
			// multiply the two values together
			sum = value1*value2;
			int2string(sum, result); // converting the sum into a string

			int resultLength = strlen(result);

			int difference = bufferlen-resultLength;

			// adding the sum into the buffer
			strncpy(&buffer[startOffset], result, resultLength);
			// shifting the buffer to the left
			memmove(&buffer[resultLength], &buffer[remainderOffset+1], difference * sizeof(int));

			
		}
	}
	// something missing?
	int unlockRet = pthread_mutex_unlock(&lock);
	// printf("multiply unlock: %d\n", unlockRet);
    }
}


/* Looks for a number immediately surrounded by parentheses [e.g.
   "(56)"] in the buffer and, if found, removes the parentheses leaving
   only the surrounded number. */
void *degrouper(void *arg)
{
    int bufferlen;
    int i;

    // return NULL; /* remove this line */

    while (1) {

	if (timeToFinish()) {
	    return NULL;
	}

	pthread_mutex_lock(&lock);

	/* storing this prevents having to recalculate it in the loop */
	bufferlen = strlen(buffer);

	for (i = 0; i < bufferlen; i++) {
	    // check for '('
		if (buffer[i] == '(') {

			// check that it is followed by a naked number
			if (isNumeric(buffer[i+1])) {

				// check that the naked number if followed by ')'
				for (int j = i+1; j < bufferlen; j++) {
					if (buffer[j] == ')') {

						// remove ')' by shifting the tail end of the expression
						memmove(&buffer[j], &buffer[j+1], (bufferlen-j) * sizeof(int));

						// remove '(' by shifting the beginning of the expression
						memmove(&buffer[i], &buffer[i+1], (bufferlen-i) * sizeof(int));
					}
				}
			}
		}
	}
	// something missing?

	int unlockRet = pthread_mutex_unlock(&lock);
	// printf("degrouper unlock: %d\n", unlockRet);

    }
}


/* sentinel waits for a number followed by a ; (e.g. "453;") to appear
   at the beginning of the buffer, indicating that the current
   expression has been fully reduced by the other threads and can now be
   output.  It then "dequeues" that expression (and trailing ;) so work can
   proceed on the next (if available). */
void *sentinel(void *arg)
{
    char numberBuffer[20];
    int bufferlen;
    int i;

    while (1) {
	if (timeToFinish()) {
	    return NULL;
	}

	// pthread_mutex_lock(&lock);
	/* storing this prevents having to recalculate it in the loop */
	bufferlen = strlen(buffer);

	for (i = 0; i < bufferlen; i++) {
	    if (buffer[i] == ';') {
		if (i == 0) {
		    printErrorAndExit("Sentinel found empty expression!");
		} else {
		    /* null terminate the string */
		    numberBuffer[i] = '\0';
		    /* print out the number we've found */
		    fprintf(stdout, "%s\n", numberBuffer);
		    /* shift the remainder of the string to the left */
		    strcpy(buffer, &buffer[i + 1]);
		    break;
		}
	    } else if (!isNumeric(buffer[i])) {
		break;
	    } else {
		numberBuffer[i] = buffer[i];
	    }
	}

	// something missing?
	// pthread_mutex_unlock(&lock);
    }
}

/* reader reads in lines of input from stdin and writes them to the
   buffer */
void *reader(void *arg)
{
    while (1) {
	char tBuffer[100];
	int currentlen;
	int newlen;
	int free;

	fgets(tBuffer, sizeof(tBuffer), stdin);

	/* Synchronization bugs in remainder of function need to be fixed */

	newlen = strlen(tBuffer);
	currentlen = strlen(buffer);

	/* if tBuffer comes back with a newline from fgets, remove it */
	if (tBuffer[newlen - 1] == '\n') {
	    /* shift null terminator left */
	    tBuffer[newlen - 1] = tBuffer[newlen];
	    newlen--;
	}

	// pthread_mutex_lock(&lock);
	/* -1 for null terminator, -1 for ; separator */
	free = sizeof(buffer) - currentlen - 2;

	while (free < newlen) {
		// spinwaiting
	}

	/* we can add another expression now */
	strcat(buffer, tBuffer);
	strcat(buffer, ";");

	// pthread_mutex_unlock(&lock);

	/* Stop when user enters '.' */
	if (tBuffer[0] == '.') {
	    return NULL;
	}
    }
}


/* Where it all begins */
int smp3_main(int argc, char **argv)
{
    void *arg = 0;		/* dummy value */

    /* let's create our threads */
    if (pthread_create(&multiplierThread, NULL, multiplier, arg)
	|| pthread_create(&adderThread, NULL, adder, arg)
	|| pthread_create(&degrouperThread, NULL, degrouper, arg)
	|| pthread_create(&sentinelThread, NULL, sentinel, arg)
	|| pthread_create(&readerThread, NULL, reader, arg)) {
	printErrorAndExit("Failed trying to create threads");
    }

    /* you need to join one of these threads... but which one? */

    // STEP 1: Joined the readerThread instead of detaching it
    void *status;
    pthread_join(readerThread, &status);

    pthread_detach(multiplierThread);
    pthread_detach(adderThread);
    pthread_detach(degrouperThread);
    pthread_detach(sentinelThread);
    // pthread_detach(readerThread);

    /* everything is finished, print out the number of operations performed */
    fprintf(stdout, "Performed a total of %d operations\n", num_ops);
    return EXIT_SUCCESS;
}
