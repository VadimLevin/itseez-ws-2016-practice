#include "skeleton_filter.hpp"

#if defined __SSSE3__  || (defined _MSC_VER && _MSC_VER >= 1500)
#  include "tmmintrin.h"
#  define HAVE_SSE
#endif

#include <string>
#include <sstream>
using namespace std;

// Function for debug prints
template <typename T>
std::string __m128i_toString(const __m128i var)
{
    std::stringstream sstr;
    const T* values = (const T*) &var;
    if (sizeof(T) == 1)
    {
        for (unsigned int i = 0; i < sizeof(__m128i); i++)
        {
            sstr << (int) values[i] << " ";
        }
    }
    else
    {
        for (unsigned int i = 0; i < sizeof(__m128i) / sizeof(T); i++)
        {
            sstr << values[i] << " ";
        }
    }
    return sstr.str();
}

void ConvertColor_BGR2GRAY_BT709(const cv::Mat& src, cv::Mat& dst)
{
    CV_Assert(CV_8UC3 == src.type());
    cv::Size sz = src.size();
    dst.create(sz, CV_8UC1);

    const int bidx = 0;
    const float bias = 0.5f;
    const float red_const_f = 0.2126f;
    const float green_const_f = 0.7152f;
    const float blue_const_f = 0.0722f;
    const uint16_t shift = 16;
    const uint16_t red_const_i = (uint16_t)(ldexp(red_const_f, shift) + 0.5f);
    const uint16_t green_const_i = (uint16_t)(ldexp(green_const_f, shift) + 0.5f);
    const uint16_t blue_const_i = (uint16_t)(ldexp(blue_const_f, shift) + 0.5f);
    const uint16_t bias_i = (uint16_t)(ldexp(bias, shift) + 0.5f);

    for (int y = 0; y < sz.height; y++)
    {
        const cv::Vec3b *psrc = src.ptr<cv::Vec3b>(y);
        uchar *pdst = dst.ptr<uchar>(y);

        for (int x = 0; x < sz.width; x++)
        {
            pdst[x] = (uchar)((red_const_i * psrc[x][2-bidx] + green_const_i * psrc[x][1] + blue_const_i * psrc[x][bidx] + bias_i) >> shift);
        }
    }
}

void ConvertColor_BGR2GRAY_BT709_fpt(const cv::Mat& src, cv::Mat& dst)
{
    CV_Assert(CV_8UC3 == src.type());
    cv::Size sz = src.size();
    dst.create(sz, CV_8UC1);

    const int bidx = 0;
    const float bias = 0.5f;
    const float red_const_f = 0.2126f;
    const float green_const_f = 0.7152f;
    const float blue_const_f = 0.0722f;
    const uint16_t shift = 8;
    const uint16_t red_const_i = (uint16_t)(ldexp(red_const_f, shift) + 0.5f);
    const uint16_t green_const_i = (uint16_t)(ldexp(green_const_f, shift) + 0.5f);
    const uint16_t blue_const_i = (uint16_t)(ldexp(blue_const_f, shift) + 0.5f);
    const uint16_t bias_i = (uint16_t)(ldexp(bias, shift) + 0.5f);

    for (int y = 0; y < sz.height; y++)
    {
        const uchar *psrc = src.ptr<uchar>(y);
        uchar *pdst = dst.ptr<uchar>(y);

        for (int x = 0; x < sz.width; x++)
        {
            pdst[x] = (uchar)((red_const_i * psrc[3*x + 2] + green_const_i * psrc[3*x + 1] + blue_const_i * psrc[3*x] + bias_i) >> shift);
        }
    }
}

void ConvertColor_BGR2GRAY_BT709_simd(const cv::Mat& src, cv::Mat& dst)
{
    CV_Assert(CV_8UC3 == src.type());
    cv::Size sz = src.size();
    dst.create(sz, CV_8UC1);
    const int bidx = 0;
    const float bias_f = 0.5f;
    const float red_const_f = 0.2126f;
    const float green_const_f = 0.7152f;
    const float blue_const_f = 0.0722f;
    const uint16_t shift = 8;
    const uint16_t red_const_i = (uint16_t)(ldexp(red_const_f, shift) + 0.5f);
    const uint16_t green_const_i = (uint16_t)(ldexp(green_const_f, shift) + 0.5f);
    const uint16_t blue_const_i = (uint16_t)(ldexp(blue_const_f, shift) + 0.5f);
    const uint16_t bias_i = (uint16_t)(ldexp(bias_f, shift) + 0.5f);



#ifdef HAVE_SSE
    __m128i ssse3_blue_indices_0  = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 12,  9,  6,  3,  0);
    __m128i ssse3_blue_indices_1  = _mm_set_epi8(-1, -1, -1, -1, -1, 14, 11,  8,  5,  2, -1, -1, -1, -1, -1, -1);
    __m128i ssse3_blue_indices_2  = _mm_set_epi8(13, 10,  7,  4,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    __m128i ssse3_green_indices_0 = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, 10,  7,  4,  1);
    __m128i ssse3_green_indices_1 = _mm_set_epi8(-1, -1, -1, -1, -1, 15, 12,  9,  6,  3,  0, -1, -1, -1, -1, -1);
    __m128i ssse3_green_indices_2 = _mm_set_epi8(14, 11,  8,  5,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    __m128i ssse3_red_indices_0   = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 11,  8,  5,  2);
    __m128i ssse3_red_indices_1   = _mm_set_epi8(-1, -1, -1, -1, -1, -1, 13, 10,  7,  4,  1, -1, -1, -1, -1, -1);
    __m128i ssse3_red_indices_2   = _mm_set_epi8(15, 12,  9,  6,  3,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m128i red_coeff   = _mm_set1_epi16(54);
    __m128i green_coeff = _mm_set1_epi16(183);
    __m128i blue_coeff  = _mm_set1_epi16(19); // 19 instead of 18 because the sum of 3 coeffs must be == 256
    __m128i bias = _mm_set1_epi16(128);
    __m128i zero = _mm_setzero_si128();
#endif

    for (int y = 0; y < sz.height; y++)
    {
        const uchar *psrc = src.ptr<uchar>(y);
        uchar *pdst = dst.ptr<uchar>(y);

        int x = 0;

#ifdef HAVE_SSE
        // Here is 16 times unrolled loop for vector processing
        for (; x <= sz.width - 16; x += 16)
        {
            __m128i chunk0 = _mm_loadu_si128((const __m128i*)(psrc + x*3 + 16*0));
            __m128i chunk1 = _mm_loadu_si128((const __m128i*)(psrc + x*3 + 16*1));
            __m128i chunk2 = _mm_loadu_si128((const __m128i*)(psrc + x*3 + 16*2));

            __m128i red = _mm_or_si128(_mm_or_si128(_mm_shuffle_epi8(chunk0, ssse3_red_indices_0),
                                                    _mm_shuffle_epi8(chunk1, ssse3_red_indices_1)),
                                                    _mm_shuffle_epi8(chunk2, ssse3_red_indices_2));
            __m128i green = _mm_or_si128(_mm_or_si128(_mm_shuffle_epi8(chunk0, ssse3_green_indices_0),
                                                    _mm_shuffle_epi8(chunk1, ssse3_green_indices_1)),
                                                    _mm_shuffle_epi8(chunk2, ssse3_green_indices_2));
            __m128i blue = _mm_or_si128(_mm_or_si128(_mm_shuffle_epi8(chunk0, ssse3_blue_indices_0),
                                                    _mm_shuffle_epi8(chunk1, ssse3_blue_indices_1)),
                                                    _mm_shuffle_epi8(chunk2, ssse3_blue_indices_2));
            
            /* ??? */   

            __m128i gray_packed; // Initialize it properly

            _mm_storeu_si128((__m128i*)(pdst + x), gray_packed);
        }
#endif

        // Process leftover pixels
        for (; x < sz.width; x++)
        {
            pdst[x] = (uchar)((red_const_i * psrc[3*x + 2] + green_const_i * psrc[3*x + 1] + blue_const_i * psrc[3*x] + bias_i) >> shift);
        }
    }

    // ! Remove this before writing your optimizations !
    ConvertColor_BGR2GRAY_BT709_fpt(src, dst);
    // ! Remove this before writing your optimizations !
}
