
int sfa_write(void *sfa, int global_bytepos, const int write_len, const void *data);

void *sfa_read(void *sfa, int global_bytepos, int bytes_to_read);

void *sfa_new();

void sfa_add_file(void* sfa, const char *fname, const int size);

int sfa_get_nfiles(void* sfa);

const char *sfa_file_get_cwd(void* sfa, int idx);

void sfa_set_cwd(void * sfa, const char *path);
