#ifndef SPARSEFILE_ALLOCATOR_H
#define SPARSEFILE_ALLOCATOR_H

int sfa_write(void *sfa, int global_bytepos, const int write_len, const void *data);

void *sfa_read(void *sfa, int global_bytepos, int bytes_to_read);

void *sfa_new();

/**
 * Add a file to the allocator */
void sfa_add_file(
    void* sfa,
    const char *fname,
    const int fname_len,
    const int size
);

/**
 * @return number of files added to allocator */
int sfa_get_nfiles(void* sfa);

/**
 * @return the path of this file pointed to by this index */
const char *sfa_file_get_path(void* sfa, int idx);

/**
 * Set current working directory */
void sfa_set_cwd(void * sfa, const char *path);

/**
 * @return total file size in bytes */
unsigned int sfa_get_total_size(void * sfa);

#endif /* SPARSEFILE_ALLOCATOR_H */
