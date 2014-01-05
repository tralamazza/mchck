#include <mchck.h>

typedef void (sump_writer)(const uint8_t* buf, size_t len);
typedef uint8_t (sump_sample)();

void sump_init(sump_writer *w);
void sump_process(uint8_t* data, size_t len);

/* vim: set ts=8 sw=8 noexpandtab: */
