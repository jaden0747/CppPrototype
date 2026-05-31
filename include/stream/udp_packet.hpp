#pragma once

#include <cstdint>

namespace stream
{

// ---------------------------------------------------------------------------
// Wire constants
// ---------------------------------------------------------------------------
inline constexpr uint16_t UDP_MTU          = 1400; // safe UDP payload below ethernet MTU
inline constexpr uint16_t UDP_HEADER_SIZE  = 8;
inline constexpr uint16_t UDP_PAYLOAD_SIZE = UDP_MTU - UDP_HEADER_SIZE; // 1392 bytes

// Flags field bits
inline constexpr uint8_t SHARD_FLAG_FIRST = 0x01; // first shard in frame
inline constexpr uint8_t SHARD_FLAG_LAST  = 0x02; // last shard in frame

// ---------------------------------------------------------------------------
// Shard header (8 bytes, big-endian multi-byte fields)
//
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          seq_num (16)         |        frame_index (16)       |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | shard_index(8)| total_shards(8)|  fec_shards(8)|   flags(8)  |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       payload (1392 bytes)                    |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// total_shards = k + m  (data + parity)
// fec_shards   = m      (parity count)
// data_shards  = total_shards - fec_shards
// ---------------------------------------------------------------------------

struct ShardHeader
{
    uint16_t seq_num;      // monotonically increasing sequence number
    uint16_t frame_index;  // which video frame this shard belongs to
    uint8_t  shard_index;  // position within the frame (0 .. total_shards-1)
    uint8_t  total_shards; // k + m
    uint8_t  fec_shards;   // m (parity shards count)
    uint8_t  flags;        // SHARD_FLAG_FIRST | SHARD_FLAG_LAST
};

// Serialise header into 8 bytes at dst (big-endian for multi-byte fields)
inline void encode_shard_header(uint8_t* dst, const ShardHeader& h) noexcept
{
    dst[0] = static_cast<uint8_t>(h.seq_num >> 8);
    dst[1] = static_cast<uint8_t>(h.seq_num);
    dst[2] = static_cast<uint8_t>(h.frame_index >> 8);
    dst[3] = static_cast<uint8_t>(h.frame_index);
    dst[4] = h.shard_index;
    dst[5] = h.total_shards;
    dst[6] = h.fec_shards;
    dst[7] = h.flags;
}

// Deserialise header from 8 bytes at src
inline ShardHeader decode_shard_header(const uint8_t* src) noexcept
{
    ShardHeader h;
    h.seq_num      = static_cast<uint16_t>((static_cast<uint16_t>(src[0]) << 8) | src[1]);
    h.frame_index  = static_cast<uint16_t>((static_cast<uint16_t>(src[2]) << 8) | src[3]);
    h.shard_index  = src[4];
    h.total_shards = src[5];
    h.fec_shards   = src[6];
    h.flags        = src[7];
    return h;
}

} // namespace stream
