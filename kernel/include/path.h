typedef struct {
	list_t *items;
} path_t;

path_t *make_path(const char *raw_path);

void free_path(path_t *path);
void add_path_items(path_t *path, const char *raw_path);
void move_up(path_t *path);
char *get_absolute_path(path_t *path);