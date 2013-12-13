
/**
 * @file
 * @brief file manager that allows user to dump data to sparse files
 * @author  Willem Thiart himself@willemthiart.com
 * @version 0.1
 *
 * @section LICENSE
 * Copyright (c) 2011, Willem-Hendrik Thiart
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * The names of its contributors may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL WILLEM-HENDRIK THIART BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "sparsefile_allocator.h"

typedef struct
{
    char *path;
    void *files;
    int nfiles;

    /* total size taken up by files */
    int size;
} sfa_t;

typedef struct
{
    const char *path;
    int size;
} file_t;

#if WIN32
static char* strndup(const char* str, const unsigned int len)
{
    char* new;

    new = malloc(len+1);
    strncpy(new,str,len);
    new[len] = '\0';
    return new;
}
#endif

/**
 * Using this global byte pos, get the file that holds this byte
 *
 * @param file_bytepos Position on the stream where the file starts
 * @return the file that the global byte pos is in */
static file_t *__get_file_from_bytepos(
    sfa_t * me,
    const int global_bytepos,
    int *file_bytepos
)
{
    int ii, bytes;

    for (ii = 0, bytes = 0; ii < me->nfiles; ii++)
    {
        file_t *file;

        file = &((file_t *) me->files)[ii];

        if (bytes <= global_bytepos && global_bytepos < bytes + file->size)
        {
            *file_bytepos = bytes;
            return file;
        }

        bytes += file->size;
    }

    return NULL;
}

static void __create_empty_file(
    const char *fname,
    const int size
)
{
    int fd;

//  fd = open(fname, O_CREAT | O_RDWR | O_TRUNC);
    fd = open(fname, O_CREAT | O_RDWR, 0777);

#if 0
#define RANDOM_BITS_LEN 256
    int ii;
    char str[1024];

    int rand_num;

    for (ii = size; ii > 0; ii -= RANDOM_BITS_LEN)
    {
//        rand_num = random();
//        sprintf(str, "%32d", rand_num);
        write(fd, str, ii < RANDOM_BITS_LEN ? ii : RANDOM_BITS_LEN);
    }
#else
    write(fd, "\0", 1);
    lseek(fd, size - 1, SEEK_SET);
    write(fd, "\0", 1);
#endif

    close(fd);
}

int sfa_write(
    void *flo,
    int global_bytepos,
    const int write_len,
    const void *data
)
{
    sfa_t *me = flo;
    int bytes_to_write;

    bytes_to_write = write_len;

//    printf(">>>\n");
//    printf("WRITING %dB to %d\n", bytes_to_write, global_bytepos);

    while (0 < bytes_to_write)
    {
        int fd, file_bytepos, file_pos, write_len;
        file_t *file;

        /* find file and position we need to write to */
        file = __get_file_from_bytepos(me, global_bytepos, &file_bytepos);
        file_pos = global_bytepos - file_bytepos;
        write_len = bytes_to_write;

        if (!file)
            return 1;

        assert(file);

        /*  check if it is in the boundary of this file */
        if (file->size < file_pos + write_len)
        {
            write_len = file->size - file_pos;
        }

        int bwrite = 0;

#if 1
        if (-1 == (fd = open(file->path, O_RDWR, 0777)))
        {
            perror("couldn't open file");
            return 0;
        }

        if (-1 == lseek(fd, file_pos, SEEK_SET))
        {
            printf("couldn't seek to %d on file: %s\n", file_pos, file->path);
            perror("couldn't seek\n");
            exit(0);
        }

//        printf("writing %d: %d to %s (%d,%d,fs:%d/%d)\n", torr_bytepos,
//               write_len, file->path, file_pos, fd, file_pos, file->size);

        if (-1 == (bwrite = write(fd, data, write_len)))
        {
            //perror("Couldn't write to file %s\n", err_str(errno));
            printf("Couldn't write to file %s\n", strerror(errno));
//            exit(0);
        }

        if (bwrite == 0)
        {
            printf("couldn't write\n");
            exit(0);
        }

        bytes_to_write -= bwrite;
        global_bytepos += bwrite;
        data += bwrite;

        close(fd);
#else
	FILE *f;

	if (!(f = fopen(file->path, "wb+")))
	{
            fprintf(stderr, "Unable to open file %s", file->path);
	}
	
	if (0 != fseek(f, file_pos, SEEK_SET))
        {
            printf("ERROR: couldn't seek: %s %d\n", strerror(errno), file_pos);
        }

        while (0 < write_len)
        {
            if (512 > write_len)
                bwrite = fwrite(data, 1, write_len, f);
            else
                bwrite = fwrite(data, 1, 512, f);

//            printf("a %d %d\n", write_len, bwrite);

            if (bwrite <= 0)
            {
                printf("ERRORz: %s %d\n", strerror(errno), bytes_to_write);
                exit(0);
            }

            bytes_to_write -= bwrite;
            write_len -= bwrite;
            global_bytepos += bwrite;
            data += bwrite;
        }
	fclose(f);
#endif

        /*  update our position */

//        printf("wrote:%d left:%d pos:%d\n", bwrite, bytes_to_write, 0);//torr_bytepos);
    }

    return 1;
}

void *sfa_read(
    void *flo,
    int global_bytepos,
    int bytes_to_read
)
{
    sfa_t *me = flo;
    char *ptr, *data_out;

    ptr = data_out = malloc(bytes_to_read);

#if 0 /* debugging */
    printf("sf-readblock: %d %d\n",
           global_bytepos, bytes_to_read);
#endif

    while (0 < bytes_to_read)
    {
        int file_global_bytepos, file_pos, read_len;
        file_t *file;

        /* find the file to read from */
        if (!(file =
              __get_file_from_bytepos(me, global_bytepos, &file_global_bytepos)))
        { 
            return data_out;
        }

        /* reposition in file */
        file_pos = global_bytepos - file_global_bytepos;
        read_len = bytes_to_read;

#if 0 /* debugging */
        printf("reading file: %s (%dB)\n", file->path, bytes_to_read);
        printf("%d %d %d %d\n", 
                global_bytepos, file_global_bytepos, file_pos, bytes_to_read);
#endif

#if 0
        /*  check if it is in the boundary of this file */
        if (file->size < file_pos + read_len)
        {
            printf("read shorted\n");
            read_len = file_pos + read_len - file->size;
        }
#endif

        int bread = 0;

#if 0
        /*  Open file */
        if (-1 == (fd = open(file->path, O_RDONLY)))
        {
            perror("Couldn't open file");
            exit(0);
//            goto skip;
        }

        /*  Seek to offset */
        if (0 != file_pos)
        {
            if (-1 == (new_pos = lseek(fd, file_pos, SEEK_SET)))
            {
                perror("Couldn't seek");
                exit(0);
            }
        }

        if (new_pos != file_pos)
        {
            printf("not at correct position\n");
            exit(0);
        }

        /*  Read */
        if (-1 == (bread = read(fd, ptr, read_len)))
        {
            perror("couldn't read");
            exit(0);
        }

        if (0 == bread)
        {
            printf("%s\n", strerror(errno));
            exit(0);
        }

        printf("read: %d\n", bread);

//       printf("  bread: %d  readlen: %d\n", bread, read_len);
//        printf("data_out[0] = %x\n", data_out[0]);

        close(fd);
#else
	FILE *f;

	//Open file
	if (!(f = fopen(file->path, "rb")))
	{
            fprintf(stderr, "Unable to open file %s", file->path);
	}
	
	//Get file length
//	fseek(f, 0, SEEK_END);
//	fileLen=ftell(f);
	fseek(f, file_pos, SEEK_SET);

	//Read file contents into buffer
	bread = fread(ptr, 1, read_len, f);
	fclose(f);
#endif

//        printf("%d\n", bread);

        /*  update our position */
        bytes_to_read -= bread;
        ptr += bread;
        global_bytepos += bread;
    }

    return data_out;
}

void *sfa_new(
)
{
    sfa_t *me;

    me = calloc(1, sizeof(sfa_t));
    return me;
}

static int __mkdir(
    char *path,
#if WIN32
    mode_t mode __attribute__((__unused__))
#else
    mode_t mode
#endif
)
{
    struct stat st;

    if (stat(path, &st) != 0)
    {
#if WIN32
        if (mkdir(path) != 0)
#else
        if (mkdir(path, mode) != 0)
#endif
            return -1;
    }

    return 1;
}

static int __filesize(
    const char *fname
)
{
    struct stat st;

    stat(fname, &st);
    return st.st_size;
}

static int __exists(
    const char *fname
)
{
    struct stat buffer;

    return (stat(fname, &buffer) == 0);
}

static int __mkpath(
    const char *path,
    mode_t mode
)
{
    char *marker = strdup(path);
    char *sp, *pp;

    sp = pp = marker;
    while (NULL != (sp = strchr(pp, '/')))
    {
        *sp = '\0';
        __mkdir(marker, mode);
        *sp = '/';
        pp = sp + 1;
    }

    free(marker);
    return 1;
}

/**
 * Add a file to the allocator
 */
void sfa_add_file(
    void* sfa,
    const char *fname,
    const int fname_len,
    const int size
)
{
    sfa_t * me = sfa;
    file_t *file;

#if 0 /* debug */
    printf("FILELIST: adding %.*s\n", fname_len, fname);
#endif

    me->nfiles += 1;
    me->size += size;
    assert(0 < me->nfiles);

    /* increase size of array */
    me->files = realloc(me->files, me->nfiles * sizeof(file_t));

    /* add file */
    file = &((file_t *) me->files)[me->nfiles - 1];
    file->path = strndup(fname,fname_len);
    file->size = size;

    /* create physical file */
    __mkpath(file->path, 0777);
    if (!__exists(file->path) || __filesize(file->path) != size)
    {
        __create_empty_file(file->path, size);
    }
}

/**
 * @return number of files added to allocator */
int sfa_get_nfiles(
    void* sfa
)
{
    sfa_t* me = sfa;

    return me->nfiles;
}

/**
 * @return the path of this file pointed to by this index */
const char *sfa_file_get_path(
    void* sfa,
    int idx
)
{
    sfa_t* me = sfa;
    file_t *file;

    assert(0 <= idx && idx < me->nfiles);
    file = &((file_t *) me->files)[idx];
    return file->path;
}

/**
 * Set current working directory */
void sfa_set_cwd(
    void* sfa,
    const char *path
)
{
    sfa_t* me = sfa;

    me->path = strdup(path);
    if (0 != chdir(me->path))
    {
        printf("directory: %s\n", me->path);
        perror("could not change directory");
        exit(0);
    }
}

/**
 * @return total file size in bytes */
unsigned int sfa_get_total_size(void * sfa)
{
    sfa_t* me = sfa;
    return me->size;
}

