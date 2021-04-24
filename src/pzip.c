#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pzip.h"

/**
 * pzip() - zip an array of characters in parallel
 *
 * Inputs:
 * @n_threads:		   The number of threads to use in pzip
 * @input_chars:		   The input characters (a-z) to be zipped
 * @input_chars_size:	   The number of characaters in the input file
 *
 * Outputs:
 * @zipped_chars:       The array of zipped_char structs
 * @zipped_chars_count:   The total count of inserted elements into the zippedChars array.
 * @char_frequency[26]: Total number of occurences
 *
 * NOTE: All outputs are already allocated. DO NOT MALLOC or REASSIGN THEM !!!
 *
 */
void *callback(void *threadid);
pthread_barrier_t barrier;
pthread_mutex_t mutex;
int reservation[8] = {0};
struct zipped_char *global_zipped_chars;
char *global_input_chars;
int global_size;
int global_threads;
int *frequency;
void pzip(int n_threads, char *input_chars, int input_chars_size,
	  struct zipped_char *zipped_chars, int *zipped_chars_count,
	  int *char_frequency)
{
	frequency = char_frequency;
	global_zipped_chars = zipped_chars;
	global_threads = n_threads;
	global_size = input_chars_size;
	pthread_mutex_init(&mutex, NULL);
	global_input_chars = input_chars;
	pthread_t threads[8];

	pthread_barrier_init(&barrier, NULL, n_threads+1);
	int confusion[] = {0, 1, 2, 3, 4, 5, 6, 7};
	for (int i = 0; i < n_threads; i++) {
		if (pthread_create(&threads[i], NULL, callback, (void *)&confusion[i])) {
			fprintf(stderr, "Error from thread");
			exit(-1);
		}
	}
	pthread_barrier_wait(&barrier);
	pthread_barrier_wait(&barrier);
	int space = 0;
	for (int i = 0; i < 8; i++)
		space += reservation[i];
	*zipped_chars_count = space;
	/*printf("%i",space);*/
}
void *callback(void *thread_Id)
{
	int id = *((int *) thread_Id);

	int size = global_size / global_threads;

	int lower = id * size;

	/*printf("size is:%i, threads is:%i, id is:%i\n ", global_size, global_threads, id);*/
	int upper = lower + size-1;

	/*printf("yeet %i, %i\n",upper, lower);*/
	int zip_Count = 0;

	int current_Count = 1;

	/*printf("checkpoint!\n");*/
	char current_char = global_input_chars[lower];

	/*printf("#%i has arrived.\n",id);*/
	for (int i = lower + 1; i <= upper; i++) {
		/*printf("looping%i\n", i);*/
		if (current_char != global_input_chars[i]) {
			zip_Count++;
			current_char = global_input_chars[i];
		}
	}
	if (global_input_chars[upper] == global_input_chars[upper-1])
		zip_Count++;
	/*printf("%i\n",zip_Count);*/
	struct zipped_char local_Zipped[zip_Count];

	/*printf("nope, its ok\n");*/
	zip_Count = 0;
	current_char = global_input_chars[lower];
	frequency[current_char-97]++;
	for (int i = lower + 1; i <= upper; i++) {
		frequency[global_input_chars[i]-97]++;
		/*printf("looping%i\n", i);*/
		if (current_char == global_input_chars[i])
			current_Count++;
		else {
			struct zipped_char temp;

			temp.character = current_char;
			temp.occurence = current_Count;
			local_Zipped[zip_Count] = temp;
			zip_Count++;
			current_Count = 1;
			current_char = global_input_chars[i];
		}
	}
	if (global_input_chars[upper] == global_input_chars[upper-1]) {
		struct zipped_char temp;

		temp.character = global_input_chars[upper];
		temp.occurence = current_Count;
		local_Zipped[zip_Count] = temp;
		zip_Count++;
	} else {
		struct zipped_char temp;

		temp.character = global_input_chars[upper];
		temp.occurence = current_Count;
		local_Zipped[zip_Count] = temp;
		zip_Count++;
	}
	/*printf("%i\n",zip_Count);*/
	reservation[id] = zip_Count;
	pthread_barrier_wait(&barrier);
	int allocated = 0;

	for (int i = 0; i < id; i++)
		allocated += reservation[i];
	for (int i = 0; i < zip_Count; i++)
		global_zipped_chars[allocated+i] = local_Zipped[i];
	/*printf("thread complete");*/
	pthread_barrier_wait(&barrier);
	pthread_exit(NULL);
}
