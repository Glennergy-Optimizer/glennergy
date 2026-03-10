#ifndef TESTREADER_H
#define TESTREADER_H

#include "../Cache/CacheProtocol.h"
int test_reader();
int cache_request(CacheCommand cmd, void *data_out, size_t expected_size);
int cache_SendResults(const ResultRequest *results);

#endif