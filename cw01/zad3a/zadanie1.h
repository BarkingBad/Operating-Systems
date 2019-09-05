struct FilePath {
    char * dir;
    char * filename;
} FilePath;
struct WrappedBlock {
    char** table;
    int size;
    int index;
} WrappedBlock;
struct WrappedBlock *create(int size);
void set_directory(struct FilePath * fp, char * directory);
void set_filename(struct FilePath * fp, char * filename);
void find(struct FilePath fp);
int copy_result_to_memory(struct WrappedBlock *wp);
void remove_block_at_index(struct WrappedBlock * wp, int index);
void free_memory(struct WrappedBlock *wp);


