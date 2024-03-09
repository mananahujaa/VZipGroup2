// #include <dirent.h>
// #include <stdio.h>
// #include <assert.h>
// #include <stdlib.h>
// #include <string.h>
// #include <zlib.h>
// #include <time.h>
// #include <pthread.h>
// #include <limits.h>

// #ifndef PATH_MAX
// #define PATH_MAX 4096 // A common default value for PATH_MAX
// #endif PATH_MAX

// #define BUFFER_SIZE 1048576 // 1MB
// #define MAX_THREADS 8

// int cmp(const void *a, const void *b)
// {
//     return strcmp(*(char **)a, *(char **)b);
// }

// struct ThreadData
// {
//     char **files;
//     int start;
//     int end;
//     const char *folder;
//     int total_in;
//     int total_out;
// };

// // void *compress_thread(void *arg)
// // {

// //     printf("Inside compress_thread\n");
// //     struct ThreadData *data = (struct ThreadData *)arg;
// //     printf("Thread start: %d, end: %d\n", data->start, data->end);

// //     unsigned char buffer_in[BUFFER_SIZE];
// //     unsigned char buffer_out[BUFFER_SIZE];

// //     for (int i = data->start; i < data->end; i++)
// //     {
// //         printf("Processing file: %s\n", data->files[i]);
// //         char *filename = data->files[i];

// //         int len = strlen(data->folder) + strlen(filename) + 2;
// //         char *full_path = (char *)malloc(len * sizeof(char));
// //         assert(full_path != NULL);
// //         strcpy(full_path, data->folder);
// //         strcat(full_path, "/");
// //         strcat(full_path, filename);

// //         FILE *f_in = fopen(full_path, "rb");
// //         assert(f_in != NULL);

// //         fseek(f_in, 0, SEEK_END);
// //         long file_size = ftell(f_in);
// //         fseek(f_in, 0, SEEK_SET);

// //         data->total_in += file_size;

// //         while (!feof(f_in))
// //         {
// //             int nbytes = fread(buffer_in, sizeof(unsigned char), BUFFER_SIZE, f_in);
// //             if (nbytes <= 0)
// //             {
// //                 if (!feof(f_in))
// //                 {
// //                     perror("Error reading input file");
// //                     fclose(f_in);
// //                     free(full_path);
// //                     pthread_exit(NULL);
// //                 }
// //             }
// //             else
// //             {
// //                 z_stream strm;
// //                 int ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
// //                 assert(ret == Z_OK);

// //                 strm.avail_in = nbytes;
// //                 strm.next_in = buffer_in;
// //                 strm.avail_out = BUFFER_SIZE;
// //                 strm.next_out = buffer_out;

// //                 ret = deflate(&strm, Z_FINISH);
// //                 assert(ret == Z_STREAM_END);

// //                 int nbytes_zipped = BUFFER_SIZE - strm.avail_out;
// //                 data->total_out += nbytes_zipped;

// //                 // Write compressed data to file or any other output destination
// //                 // fwrite(buffer_out, sizeof(unsigned char), nbytes_zipped, f_out);

// //                 deflateEnd(&strm);
// //             }
// //         }

// //         fclose(f_in);
// //         free(full_path);
// //         printf("File %s processed\n", data->files[i]);
// //     }
// //     printf("Thread finished\n");

// //     pthread_exit(NULL);
// // }


// void *compress_thread(void *arg)
// {
//     printf("Inside compress_thread\n");
//     struct ThreadData *data = (struct ThreadData *)arg;
//     printf("Thread start: %d, end: %d\n", data->start, data->end);

//     unsigned char buffer_in[BUFFER_SIZE];
//     unsigned char buffer_out[BUFFER_SIZE];

//     for (int i = data->start; i < data->end; i++)
//     {
//         printf("Processing file: %s\n", data->files[i]);
//         char *filename = data->files[i];

//         // Construct full path
//         char full_path[PATH_MAX];
//         snprintf(full_path, sizeof(full_path), "%s/%s", data->folder, filename);

//         FILE *f_in = fopen(full_path, "rb");
//         if (f_in == NULL)
//         {
//             perror("Error opening input file");
//             continue; // Move to the next file
//         }

//         // Initialize zlib stream
//         z_stream strm;
//         int ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
//         if (ret != Z_OK)
//         {
//             perror("deflateInit failed");
//             fclose(f_in);
//             continue; // Move to the next file
//         }

//         // Process input file
//         while (1)
//         {
//             int nbytes = fread(buffer_in, 1, BUFFER_SIZE, f_in);
//             if (nbytes <= 0)
//             {
//                 if (feof(f_in))
//                     break; // End of file reached
//                 else
//                 {
//                     perror("Error reading input file");
//                     break;
//                 }
//             }

//             strm.avail_in = nbytes;
//             strm.next_in = buffer_in;

//             // Compress input data
//             do
//             {
//                 strm.avail_out = BUFFER_SIZE;
//                 strm.next_out = buffer_out;
//                 ret = deflate(&strm, Z_FINISH);
//                 if (ret != Z_OK && ret != Z_STREAM_END)
//                 {
//                     perror("deflate failed");
//                     fclose(f_in);
//                     deflateEnd(&strm);
//                     pthread_exit(NULL);
//                 }
//                 int nbytes_zipped = BUFFER_SIZE - strm.avail_out;
//                 data->total_out += nbytes_zipped;
//                 // Write compressed data to file or any other output destination
//                 // fwrite(buffer_out, sizeof(unsigned char), nbytes_zipped, f_out);
//             } while (strm.avail_out == 0);

//             if (ret != Z_STREAM_END)
//                 break; // Stop compressing if there's no more output space
//         }

//         // Clean up
//         fclose(f_in);
//         deflateEnd(&strm);
//         printf("File %s processed\n", data->files[i]);
//     }

//     printf("Thread finished\n");
//     pthread_exit(NULL);
// }


// int main(int argc, char **argv)
// {
//     printf("Program started\n");
//     struct timespec start, end;
//     clock_gettime(CLOCK_MONOTONIC, &start);

//     assert(argc == 2);

//     DIR *d;
//     struct dirent *dir;
//     char **files = NULL;
//     int nfiles = 0;

//     d = opendir(argv[1]);
//     assert(d != NULL);

//     while ((dir = readdir(d)) != NULL)
//     {
//         files = realloc(files, (nfiles + 1) * sizeof(char*));
//         assert(files != NULL);

//         int len = strlen(dir->d_name);
//         if (len >= 4 && strcmp(dir->d_name + len - 4, ".ppm") == 0)
//         {
//             // files = realloc(files, (nfiles + 1) * sizeof(char *));
//             // assert(files != NULL);

//             files[nfiles] = strdup(dir->d_name);
//             assert(files[nfiles] != NULL);

//             nfiles++;
//         }
//     }
//     closedir(d);
//     qsort(files, nfiles, sizeof(char *), cmp);

//     // Debugging start
//     printf("Number of files: %d\n", nfiles);
//     for (int i = 0; i < nfiles; i++)
//     {
//         printf("File %d: %s\n", i, files[i]);
//     }

//     // Debugging end

//     pthread_t threads[MAX_THREADS];
//     struct ThreadData thread_data[MAX_THREADS];
//     int num_threads = nfiles < MAX_THREADS ? nfiles : MAX_THREADS;
//     int files_per_thread = nfiles / num_threads;
//     int remainder = nfiles % num_threads;
//     int start_index = 0;
//     for (int i = 0; i < num_threads; i++)
//     {
//         thread_data[i].files = files;
//         thread_data[i].folder = argv[1];
//         thread_data[i].total_in = 0;
//         thread_data[i].total_out = 0;
//         thread_data[i].start = start_index;
//         thread_data[i].end = start_index + files_per_thread + (i < remainder ? 1 : 0);
//         start_index = thread_data[i].end;

//         pthread_create(&threads[i], NULL, compress_thread, (void *)&thread_data[i]);
//     }

//     // debuggin start
//     printf("Number of files: %d\n", nfiles);
//     for (int i = 0; i < nfiles; i++) {
//         printf("File %d: %s\n", i, files[i]);
//     }
//     // debuggging end

//     for (int i = 0; i < num_threads; i++)
//     {
//         pthread_join(threads[i], NULL);
//     }

//     int total_in = 0, total_out = 0;
//     for (int i = 0; i < num_threads; i++)
//     {
//         total_in += thread_data[i].total_in;
//         total_out += thread_data[i].total_out;
//     }
//     printf("Compression rate: %.2lf%%\n", 100.0 * (total_in - total_out) / total_in);

//     for (int i = 0; i < nfiles; i++)
//     {
//         free(files[i]);
//     }
//     free(files);

//     clock_gettime(CLOCK_MONOTONIC, &end);
//     printf("Time: %.2f seconds\n", ((double)end.tv_sec + 1.0e-9 * end.tv_nsec) - ((double)start.tv_sec + 1.0e-9 * start.tv_nsec));

//     return EXIT_SUCCESS;
// }


#include <dirent.h> 
#include <stdio.h> 
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <time.h>
#include <pthread.h>

#define BUFFER_SIZE 1048576 // 1MB

int cmp(const void *a, const void *b) 
{
	return strcmp(*(char **) a, *(char **) b);
}

// Make the shared variables globals, so all threads access it
char **files = NULL;
int nfiles = 0;
char *p;
#define TCOUNT 19 // 19 worker threads + 1 master = 20 threads in total

// variables where the worker threads will store their computation results
int bytes_in[TCOUNT] = {0};
int bytes_zipped[TCOUNT] = {0};
unsigned char buffer_out[TCOUNT][BUFFER_SIZE];

// the entry point for thread
void* compress_one_file(void *arg) 
{
    int i = (int)((long)arg);
    int threadnum = i % TCOUNT;
    bytes_zipped[threadnum] = 0;
    if(i >= nfiles) 
    {
        return NULL;
    }

    int len = strlen(p)+strlen(files[i])+2;
    char *full_path = malloc(len*sizeof(char));
    assert(full_path != NULL);
    strcpy(full_path, p);
    strcat(full_path, "/");
    strcat(full_path, files[i]);

    unsigned char buffer_in[BUFFER_SIZE];

    // load file
    FILE *f_in = fopen(full_path, "r");
    assert(f_in != NULL);
    bytes_in[threadnum] = fread(buffer_in, sizeof(unsigned char), BUFFER_SIZE, f_in);
    fclose(f_in);

    // zip file
    z_stream strm;
    int ret = deflateInit(&strm, 9);
    assert(ret == Z_OK);
    strm.avail_in = bytes_in[threadnum];
    strm.next_in = buffer_in;
    strm.avail_out = BUFFER_SIZE;
    strm.next_out = buffer_out[threadnum];

    ret = deflate(&strm, Z_FINISH);
    assert(ret == Z_STREAM_END);
    free(full_path);

    bytes_zipped[threadnum] = BUFFER_SIZE-strm.avail_out;
}

int main(int argc, char **argv) 
{
	// time computation header
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	// end of time computation header

	// do not modify the main function before this point!

	assert(argc == 2);

	DIR *d;
	struct dirent *dir;

	d = opendir(argv[1]);
	if(d == NULL) {
		printf("An error has occurred\n");
		return 0;
	}

	// create sorted list of PPM files
	while ((dir = readdir(d)) != NULL) {
		files = realloc(files, (nfiles+1)*sizeof(char *));
		assert(files != NULL);

		int len = strlen(dir->d_name);
		if(dir->d_name[len-4] == '.' && dir->d_name[len-3] == 'p' && dir->d_name[len-2] == 'p' && dir->d_name[len-1] == 'm') {
			files[nfiles] = strdup(dir->d_name);
			assert(files[nfiles] != NULL);

			nfiles++;
		}
	}
	closedir(d);
	qsort(files, nfiles, sizeof(char *), cmp);

	// create a single zipped package with all PPM files in lexicographical order
	int total_in = 0, total_out = 0;
	FILE *f_out = fopen("video.vzip", "w");
	assert(f_out != NULL);
    p = argv[1];
    int i = 0;

    do {
        pthread_t ptid[TCOUNT];
        // compress in parallel
        for(int j = 0; j < TCOUNT; j++) {
            assert(pthread_create(&ptid[j],NULL,compress_one_file,(void*)((long)(i+j))) == 0);
        }
        // write the data
        for(int j = 0; j < TCOUNT; j++) {
            pthread_join(ptid[j],NULL); // wait for thread to complete
            if(bytes_zipped[j] > 0) {
                total_in += bytes_in[j];
                fwrite(&bytes_zipped[j], sizeof(int), 1, f_out);
                fwrite(buffer_out[j], sizeof(unsigned char), bytes_zipped[j], f_out);
                total_out += bytes_zipped[j];
            }
        }
        i += TCOUNT;
    }
    while(i < nfiles);

	fclose(f_out);

	printf("Compression rate: %.2lf%%\n", 100.0*(total_in-total_out)/total_in);

	// release list of files
	for(int i=0; i < nfiles; i++)
		free(files[i]);
	free(files);

	// do not modify the main function after this point!

	// time computation footer
	clock_gettime(CLOCK_MONOTONIC, &end);
	printf("Time: %.2f seconds\n", ((double)end.tv_sec+1.0e-9*end.tv_nsec)-((double)start.tv_sec+1.0e-9*start.tv_nsec));
	// end of time computation footer

	return 0;
}


