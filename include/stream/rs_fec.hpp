#pragma once

// ---------------------------------------------------------------------------
// rs_fec.hpp — Header-only GF(2^8) Reed-Solomon erasure code
//
// Systematic (k, k+m) code:
//   Encode: given k data shards, compute m parity shards.
//           Data shards are unchanged.
//   Decode: given any k of k+m shards, recover all k data shards.
//
// Generator matrix:
//   Rows  0 .. k-1:   identity          (data shards pass through)
//   Rows  k .. k+m-1: Cauchy parity rows (any square submatrix is invertible)
//
//   Cauchy element at parity row i (0..m-1), data col j (0..k-1):
//     G[k+i][j] = gf_inv( i  XOR  (m + j) )
//   Requires k + m <= 255 and k >= 1, m >= 1.
// ---------------------------------------------------------------------------

#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>

namespace rs
{

// ---------------------------------------------------------------------------
// GF(2^8) over primitive polynomial x^8+x^4+x^3+x^2+1 = 0x11d
// ---------------------------------------------------------------------------
namespace detail
{

inline constexpr uint16_t GF_POLY = 0x11d;

struct Tables
{
    uint8_t exp[512]; // exp[i] = alpha^(i mod 255); doubled so log[a]+log[b] is in-range
    uint8_t log[256]; // log[x] = i s.t. alpha^i = x;  log[0] is unused (guard = 0)

    Tables() noexcept
    {
        uint16_t x = 1;
        for (int i = 0; i < 255; ++i)
        {
            exp[i]  = static_cast<uint8_t>(x);
            log[x]  = static_cast<uint8_t>(i);
            x     <<= 1;
            if (x & 0x100)
                x ^= GF_POLY;
        }
        for (int i = 255; i < 512; ++i)
            exp[i] = exp[i - 255]; // wrap so log[a]+log[b] indexes directly
        log[0] = 0;                // guard: 0 has no log, never accessed legitimately
    }
};

inline const Tables& T()
{
    static const Tables t;
    return t;
}

inline uint8_t mul(uint8_t a, uint8_t b) noexcept
{
    return (a && b) ? T().exp[T().log[a] + T().log[b]] : uint8_t{0};
}

// inv(a) = alpha^(255 - log[a]);  undefined for a==0
inline uint8_t inv(uint8_t a) noexcept
{
    assert(a != 0);
    return T().exp[255 - T().log[a]]; // 255-log[a] in 1..255; exp[255]=exp[0]=1
}

// dst[i] ^= s * src[i]  for i in [0, n)
inline void mul_xor(uint8_t* __restrict dst, const uint8_t* __restrict src, uint8_t s, size_t n) noexcept
{
    if (!s)
        return;
    if (s == 1)
    {
        for (size_t i = 0; i < n; ++i)
            dst[i] ^= src[i];
        return;
    }
    const int ls = T().log[s];
    for (size_t i = 0; i < n; ++i)
        dst[i] ^= src[i] ? T().exp[T().log[src[i]] + ls] : uint8_t{0};
}

// row[i] *= s  for i in [0, n)
inline void mul_row(uint8_t* row, uint8_t s, size_t n) noexcept
{
    if (s == 1)
        return;
    const int ls = T().log[s];
    for (size_t i = 0; i < n; ++i)
        row[i] = row[i] ? T().exp[T().log[row[i]] + ls] : uint8_t{0};
}

} // namespace detail

// ---------------------------------------------------------------------------
// Generator matrix element at row r (0..k+m-1), col c (0..k-1)
// ---------------------------------------------------------------------------
inline uint8_t gen_elem(int r, int c, int k, int m) noexcept
{
    if (r < k)
        return static_cast<uint8_t>(r == c); // identity rows
    // Cauchy parity row (r-k), col c:  gf_inv( (r-k)  XOR  (m+c) )
    // r-k is in 0..m-1, m+c is in m..m+k-1 — they are never equal so XOR != 0
    return detail::inv(static_cast<uint8_t>((r - k) ^ (m + c)));
}

// ---------------------------------------------------------------------------
// Codec
// ---------------------------------------------------------------------------
class Codec
{
public:
    // k = data shards, m = parity shards.  Requires k >= 1, m >= 1, k+m <= 255.
    Codec(int k, int m) : m_k(k), m_m(m)
    {
        assert(k >= 1 && m >= 1 && k + m <= 255);
        m_P.resize(static_cast<size_t>(m * k));
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < k; ++j)
                m_P[static_cast<size_t>(i * k + j)] = gen_elem(k + i, j, k, m);
    }

    // Encode: write m parity shards from k data shards.
    // shards[0..k-1]    = input  data   (read-only)
    // shards[k..k+m-1]  = output parity (written)
    // All shards must have exactly shard_size bytes.
    void encode(uint8_t* const* shards, int shard_size) const
    {
        const size_t sz = static_cast<size_t>(shard_size);
        for (int i = 0; i < m_m; ++i)
        {
            std::memset(shards[m_k + i], 0, sz);
            for (int j = 0; j < m_k; ++j)
                detail::mul_xor(shards[m_k + i], shards[j], m_P[static_cast<size_t>(i * m_k + j)], sz);
        }
    }

    // Decode: recover k data shards from any k of k+m received shards.
    // present[i] = true  →  shards[i] contains valid received data.
    // present[i] = false →  shards[i] is missing (content ignored).
    // On success, shards[0..k-1] hold the original data shards.
    // Returns false if fewer than k shards are present.
    bool decode(uint8_t** shards, const uint8_t* present, int shard_size) const
    {
        // Collect up to k present indices
        std::vector<int> idx;
        idx.reserve(static_cast<size_t>(m_k));
        for (int i = 0; i < m_k + m_m && static_cast<int>(idx.size()) < m_k; ++i)
            if (present[i])
                idx.push_back(i);
        if (static_cast<int>(idx.size()) < m_k)
            return false;

        // Fast path: all data shards already present
        {
            bool all_data = true;
            for (int i = 0; i < m_k; ++i)
                if (!present[i])
                {
                    all_data = false;
                    break;
                }
            if (all_data)
                return true;
        }

        // Build augmented matrix  [A | I_k]  where A is the k×k submatrix
        // of the generator corresponding to the received rows.
        const int K2 = m_k * 2;
        std::vector<uint8_t> aug(static_cast<size_t>(m_k * K2), 0);
        for (int r = 0; r < m_k; ++r)
        {
            for (int c = 0; c < m_k; ++c)
                aug[static_cast<size_t>(r * K2 + c)] = gen_elem(idx[r], c, m_k, m_m);
            aug[static_cast<size_t>(r * K2 + m_k + r)] = 1;
        }

        // Gauss-Jordan elimination over GF(2^8) → [I_k | A^{-1}]
        for (int col = 0; col < m_k; ++col)
        {
            // Find pivot row (first non-zero in this column at or below diagonal)
            int piv = -1;
            for (int r = col; r < m_k; ++r)
                if (aug[static_cast<size_t>(r * K2 + col)])
                {
                    piv = r;
                    break;
                }
            if (piv < 0)
                return false; // singular — cannot happen with Cauchy matrix

            if (piv != col)
                for (int c = 0; c < K2; ++c)
                    std::swap(aug[static_cast<size_t>(col * K2 + c)], aug[static_cast<size_t>(piv * K2 + c)]);

            detail::mul_row(
                &aug[static_cast<size_t>(col * K2)],
                detail::inv(aug[static_cast<size_t>(col * K2 + col)]),
                static_cast<size_t>(K2));

            for (int r = 0; r < m_k; ++r)
            {
                if (r == col)
                    continue;
                const uint8_t f = aug[static_cast<size_t>(r * K2 + col)];
                if (f)
                    detail::mul_xor(
                        &aug[static_cast<size_t>(r * K2)],
                        &aug[static_cast<size_t>(col * K2)],
                        f,
                        static_cast<size_t>(K2));
            }
        }
        // Right half of aug is now A^{-1}

        // recovered[r] = sum_c  A^{-1}[r][c] * shards[idx[c]]
        const size_t         sz  = static_cast<size_t>(shard_size);
        std::vector<uint8_t> tmp(static_cast<size_t>(m_k) * sz, 0);
        for (int r = 0; r < m_k; ++r)
            for (int c = 0; c < m_k; ++c)
                detail::mul_xor(
                    tmp.data() + static_cast<size_t>(r) * sz,
                    shards[idx[static_cast<size_t>(c)]],
                    aug[static_cast<size_t>(r * K2 + m_k + c)],
                    sz);

        for (int r = 0; r < m_k; ++r)
            std::memcpy(shards[r], tmp.data() + static_cast<size_t>(r) * sz, sz);

        return true;
    }

private:
    int                  m_k, m_m;
    std::vector<uint8_t> m_P; // parity coefficient matrix, m × k
};

} // namespace rs
