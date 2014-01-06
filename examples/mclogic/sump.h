#include <mchck.h>

typedef ssize_t (sump_writer)(const uint8_t* buf, size_t len);

void sump_init(sump_writer *w);
void sump_data_sent(size_t value);
void sump_process(uint8_t* data, size_t len);
