#pragma once

#include <chrono>
#include <cstdint>


namespace sjtu {

static constexpr int INVALID_FRAME_ID = -1;  // invalid frame id
static constexpr int INVALID_PAGE_ID = -1;   // invalid page id

static constexpr int SJTU_PAGE_SIZE = 64;                                        // size of a data page in byte
static constexpr int BUFFER_POOL_SIZE = 128;                                         // size of buffer pool
static constexpr int DEFAULT_DB_IO_SIZE = 16;                                        // starting size of file on disk
static constexpr int LRUK_REPLACER_K = 10;                                           // backward k-distance for lru-k

using frame_id_t = int32_t;    // frame id type
using page_id_t = int32_t;     // page id type

}  // namespace sjtu

