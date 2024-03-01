#include <dirent.h> 
#include <stdio.h> 
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <pthread.h>
#include <time.h>

#define BUFFER_SIZE 1048576 // 1MB
#define MAX_THREADS 19 // Maximum allowed threads excluding main thread


int cmp(const void *a, const void *b) {
	return strcmp(*(char **) a, *(char **) b);
}

struct ThreadData {
    char* filename;
    char* folder;
    FILE* f_out;
};

void* compress_thread(void* arg) {
    struct ThreadData* data = (struct ThreadData*) arg;
    char* filename = data->filename;
    char* folder = data->folder;
    FILE* f_out = data->f_out;

    int len = strlen(folder) + strlen(filename) + 2;
    char* full_path = malloc(len * sizeof(char));
    assert(full_path != NULL);
    strcpy(full_path, folder);
    strcat(full_path, "/");
    strcat(full_path, filename);

    unsigned char buffer_in[BUFFER_SIZE];
    unsigned char buffer_out[BUFFER_SIZE];

    // load file
    FILE* f_in = fopen(full_path, "r");
    assert(f_in != NULL);
    int nbytes = fread(buffer_in, sizeof(unsigned char), BUFFER_SIZE, f_in);
    fclose(f_in);

    // zip file
    z_stream strm;
    int ret = deflateInit(&strm, 9);
    assert(ret == Z_OK);
    strm.avail_in = nbytes;
    strm.next_in = buffer_in;
    strm.avail_out = BUFFER_SIZE;
    strm.next_out = buffer_out;

    ret = deflate(&strm, Z_FINISH);
    assert(ret == Z_STREAM_END);

    // dump zipped file
    int nbytes_zipped = BUFFER_SIZE - strm.avail_out;
    fwrite(&nbytes_zipped, sizeof(int), 1, f_out);
    fwrite(buffer_out, sizeof(unsigned char), nbytes_zipped, f_out);

    free(full_path);
    deflateEnd(&strm);
    free(data);

    return NULL;
}

int main(int argc, char** argv) {
    // time computation header
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    // end of time computation header

    // do not modify the main function before this point!

    assert(argc == 2);

    DIR* d;
    struct dirent* dir;
    char** files = NULL;
    int nfiles = 0;

    d = opendir(argv[1]);
    if (d == NULL) {
        printf("An error has occurred\n");
        return 0;
    }

    // create sorted list of PPM files
    while ((dir = readdir(d)) != NULL) {
        files = realloc(files, (nfiles + 1) * sizeof(char*));
        assert(files != NULL);

        int len = strlen(dir->d_name);
        if (dir->d_name[len - 4] == '.' && dir->d_name[len - 3] == 'p' && dir->d_name[len - 2] == 'p' && dir->d_name[len - 1] == 'm') {
            files[nfiles] = strdup(dir->d_name);
            assert(files[nfiles] != NULL);

            nfiles++;
        }
    }
    closedir(d);
    qsort(files, nfiles, sizeof(char*), cmp);

    // create a single zipped package with all PPM files in lexicographical order
    int total_in = 0, total_out = 0;
    FILE* f_out = fopen("video.vzip", "w");
    assert(f_out != NULL);

    pthread_t threads[MAX_THREADS];
    int num_threads = 0;

    for (int i = 0; i < nfiles; i++) {
        if (num_threads >= MAX_THREADS) {
            // Wait for threads to finish
            for (int j = 0; j < num_threads; j++) {
                pthread_join(threads[j], NULL);
            }
            num_threads = 0;
        }

        struct ThreadData* data = malloc(sizeof(struct ThreadData));
        data->filename = files[i];
        data->folder = argv[1];
        data->f_out = f_out;

        pthread_create(&threads[num_threads], NULL, compress_thread, (void*)data);
        num_threads++;
    }

    // Wait for remaining threads to finish
    for (int j = 0; j < num_threads; j++) {
        pthread_join(threads[j], NULL);
    }

    fclose(f_out);

    printf("Compression rate: %.2lf%%\n", 100.0 * (total_in - total_out) / total_in);

    // release list of files
    for (int i = 0; i < nfiles; i++)
        free(files[i]);
    free(files);

    // do not modify the main function after this point!

    // time computation footer
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Time: %.2f seconds\n", ((double)end.tv_sec + 1.0e-9 * end.tv_nsec) - ((double)start.tv_sec + 1.0e-9 * start.tv_nsec));
    // end of time computation footer

    return 0;
}













