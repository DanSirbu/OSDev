#include "types.h"

typedef size_t (*block_read_func)(uint8_t *dst, uint32_t block);
typedef size_t (*block_write_func)(uint8_t *dst, uint32_t block);
