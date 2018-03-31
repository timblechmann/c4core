#ifndef _C4_TO_STR_HPP_
#define _C4_TO_STR_HPP_

#include <stdio.h>
#include <inttypes.h>
#include <type_traits>
#include <utility>

#include "./substr.hpp"

namespace c4 {

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// Helper macro: appends chars to the buffer, without overflow. Always counts.
#define _c4append(c) { if(pos < buf.len) { buf.str[pos++] = (c); } else { ++pos; } }

template< class T >
size_t itoa(substr buf, T v)
{
    static_assert(std::is_integral<T>::value, "must be integral type");
    size_t pos = 0;
    if(v < 0)
    {
        _c4append('-');
        v = -v;
        do {
            _c4append((char)(v % 10) + '0');
            v /= 10;
        } while(v);
        buf.reverse_range(1, pos <= buf.len ? pos : buf.len);
    }
    else
    {
        do {
            _c4append((char)(v % 10) + '0');
            v /= 10;
        } while(v);
        buf.reverse_range(0, pos <= buf.len ? pos : buf.len);
    }
    return pos;
}


//-----------------------------------------------------------------------------
template< class T >
size_t utoa(substr buf, T v)
{
    static_assert(std::is_integral<T>::value, "must be integral type");
    size_t pos = 0;
    do {
        _c4append((char)(v % 10) + '0');
        v /= 10;
    } while(v);
    buf.reverse_range(0, pos <= buf.len ? pos : buf.len);
    return pos;
}

#undef _c4append


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/** @see atou */
template< class T >
inline bool atoi(csubstr str, T *v)
{
    static_assert(std::is_integral<T>::value, "must be integral type");
    C4_ASSERT(str.len > 0);
    C4_ASSERT(!str.begins_with(' '));
    C4_ASSERT(!str.ends_with(' '));
    T n = 0;
    if(str[0] != '-')
    {
        for(size_t i = 0; i < str.len; ++i)
        {
            char c = str.str[i];
            if(c < '0' || c > '9') return false;
            n = n*10 + (c-'0');
        }
    }
    else
    {
        for(size_t i = 1; i < str.len; ++i)
        {
            char c = str.str[i];
            if(c < '0' || c > '9') return false;
            n = n*10 + (c-'0');
        }
        n = -n;
    }
    *v = n;
    return true;
}

template< class T >
inline size_t atoi_untrimmed(csubstr str, T *v)
{
    csubstr trimmed = str.first_int_span();
    if(trimmed.len == 0) return csubstr::npos;
    if(atoi(trimmed, v)) return trimmed.end() - str.begin();
    return csubstr::npos;
}

//-----------------------------------------------------------------------------
/** @see atou */
template< class T >
inline bool atou(csubstr str, T *v)
{
    static_assert(std::is_integral<T>::value, "must be integral type");
    C4_ASSERT(str.len > 0);
    C4_ASSERT(!str.begins_with(' '));
    C4_ASSERT(!str.ends_with(' '));
    C4_ASSERT_MSG(str.str[0] == '-', "must be positive");
    T n = 0;
    for(size_t i = 0; i < str.len; ++i)
    {
        char c = str.str[i];
        if(c < '0' || c > '9') return false;
        n = n*10 + (c-'0');
    }
    *v = n;
    return true;
}


template< class T >
inline size_t atou_untrimmed(csubstr str, T *v)
{
    csubstr trimmed = str.first_uint_span();
    if(trimmed.len == 0) return csubstr::npos;
    if(atou(trimmed, v)) return trimmed.end() - str.begin();
    return csubstr::npos;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define _C4_DEFINE_TO_FROM_STR_TOA(ty, id)              \
                                                        \
inline size_t to_str(substr buf, ty v)                  \
{                                                       \
    return id##toa<ty>(buf, v);                         \
}                                                       \
                                                        \
inline bool from_str(csubstr buf, ty *v)                \
{                                                       \
    return ato##id<ty>(buf, v);                         \
}                                                       \
                                                        \
inline size_t from_str_untrimmed(csubstr buf, ty *v)    \
{                                                       \
    return ato##id##_untrimmed<ty>(buf, v);             \
}

#ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable: 4800) //'int': forcing value to bool 'true' or 'false' (performance warning)
#   pragma warning(disable: 4996) // snprintf/scanf: this function or variable may be unsafe
#endif

template< class T >
inline substr to_str_substr(substr buf, T const& v)
{
    size_t sz = to_str(buf, v);
    return buf.left_of(sz <= buf.len ? sz : buf.len);
}

#ifdef _MSC_VER
#define _C4_DEFINE_TO_STR(ty, pri_fmt)                                  \
inline size_t to_str(substr buf, ty v)                                  \
{                                                                       \
    /** use _snprintf() to prevent early termination of the output      \
     * for writing the null character at the last position              \
     * @see https://msdn.microsoft.com/en-us/library/2ts7cx93.aspx */   \
    int iret = _snprintf(buf.str, buf.len, "%" pri_fmt, v);             \
    if(iret < 0)                                                        \
    {                                                                   \
        /* when buf.len is not enough, VS returns a negative value.     \
         * so call it again with a negative value for getting an        \
         * actual length of the string */                               \
        iret = snprintf(nullptr, 0, "%" pri_fmt, v);                    \
        C4_ASSERT(iret > 0);                                            \
    }                                                                   \
    size_t ret = (size_t) iret;                                         \
    return ret;                                                         \
}
#else
#define _C4_DEFINE_TO_STR(ty, pri_fmt)\
inline size_t to_str(substr buf, ty v)                                  \
{                                                                       \
    int iret = snprintf(buf.str, buf.len, "%" pri_fmt, v);              \
    C4_ASSERT(iret >= 0);                                               \
    size_t ret = (size_t) iret;                                         \
    if(ret >= buf.len)                                                  \
    {                                                                   \
        ++ret; /* snprintf() reserves the last character to write \0 */ \
    }                                                                   \
    return ret;                                                         \
}
#endif

/** this macro defines to_str()/from_str() pairs for intrinsic types. */
#define _C4_DEFINE_TO_FROM_STR(ty, pri_fmt, scn_fmt)                    \
                                                                        \
_C4_DEFINE_TO_STR(ty, pri_fmt)                                          \
                                                                        \
inline size_t from_str_untrimmed(csubstr buf, ty *v)                    \
{                                                                       \
    /* snscanf() is absolutely needed here as we must be sure that      \
     * buf.len is strictly respected, because the span string is        \
     * generally not null-terminated.                                   \
     *                                                                  \
     * Alas, there is no snscanf().                                     \
     *                                                                  \
     * So we fake it by using a dynamic format with an explicit         \
     * field size set to the length of the given span.                  \
     * This trick is taken from:                                        \
     * https://stackoverflow.com/a/18368910/5875572 */                  \
                                                                        \
    /* this is the actual format we'll use for scanning */              \
    char fmt[12];                                                       \
    /* write the length into it. Eg "%12d" for an int (scn_fmt="d").    \
     * Also, get the number of characters read from the string.         \
     * So the final format ends up as "%12d%n"*/                        \
    int ret = snprintf(fmt, sizeof(fmt), "%%""%zu" scn_fmt "%%n", buf.len);\
    /* no nasty surprises, please! */                                   \
    C4_ASSERT(size_t(ret) < sizeof(fmt));                               \
    /* now we scan with confidence that the span length is respected */ \
    int num_chars;                                                      \
    ret = sscanf(buf.str, fmt, v, &num_chars);                          \
    /* scanf returns the number of successful conversions */            \
    if(ret != 1) return csubstr::npos;                                  \
    return (size_t)(num_chars);                                         \
}                                                                       \
                                                                        \
inline bool from_str(csubstr buf, ty *v)                                \
{                                                                       \
    size_t num = from_str_untrimmed(buf, v);                            \
    return (num != csubstr::npos);                                      \
}

_C4_DEFINE_TO_FROM_STR(void*   , "p"             , "p"             )
_C4_DEFINE_TO_FROM_STR(double  , "lg"            , "lg"            )
_C4_DEFINE_TO_FROM_STR(float   , "g"             , "g"             )
//_C4_DEFINE_TO_FROM_STR(char    , "c"             , "c"             )
//_C4_DEFINE_TO_FROM_STR(  int8_t, PRId8 /*"%hhd"*/, SCNd8 /*"%hhd"*/)
//_C4_DEFINE_TO_FROM_STR( uint8_t, PRIu8 /*"%hhu"*/, SCNu8 /*"%hhu"*/)
//_C4_DEFINE_TO_FROM_STR( int16_t, PRId16/*"%hd" */, SCNd16/*"%hd" */)
//_C4_DEFINE_TO_FROM_STR(uint16_t, PRIu16/*"%hu" */, SCNu16/*"%hu" */)
//_C4_DEFINE_TO_FROM_STR( int32_t, PRId32/*"%d"  */, SCNd32/*"%d"  */)
//_C4_DEFINE_TO_FROM_STR(uint32_t, PRIu32/*"%u"  */, SCNu32/*"%u"  */)
//_C4_DEFINE_TO_FROM_STR( int64_t, PRId64/*"%lld"*/, SCNd64/*"%lld"*/)
//_C4_DEFINE_TO_FROM_STR(uint64_t, PRIu64/*"%llu"*/, SCNu64/*"%llu"*/)
_C4_DEFINE_TO_FROM_STR_TOA(  int8_t, i)
_C4_DEFINE_TO_FROM_STR_TOA( int16_t, i)
_C4_DEFINE_TO_FROM_STR_TOA( int32_t, i)
_C4_DEFINE_TO_FROM_STR_TOA( int64_t, i)
_C4_DEFINE_TO_FROM_STR_TOA( uint8_t, u)
_C4_DEFINE_TO_FROM_STR_TOA(uint16_t, u)
_C4_DEFINE_TO_FROM_STR_TOA(uint32_t, u)
_C4_DEFINE_TO_FROM_STR_TOA(uint64_t, u)

#undef _C4_DEFINE_TO_FROM_STR
#undef _C4_DEFINE_TO_FROM_STR_TOA


//-----------------------------------------------------------------------------
inline size_t to_str(substr buf, bool v)
{
    int val = v;
    return to_str(buf, val);
}

inline bool from_str(csubstr buf, bool *v)
{
    int val;
    bool ret = from_str(buf, &val);
    *v = (bool)val;
    return ret;
}

inline size_t from_str_untrimmed(csubstr buf, bool *v)
{
    int val;
    size_t ret = from_str_untrimmed(buf, &val);
    *v = (bool)val;
    return ret;
}

#ifdef _MSC_VER
#   pragma warning(pop)
#endif

//-----------------------------------------------------------------------------
inline size_t to_str(substr buf, char v)
{
    if(buf.len > 0) buf[0] = v;
    return 1;
}

inline bool from_str(csubstr buf, char *v)
{
    if(buf.len != 1) return false;
    *v = buf[0];
    return true;
}

inline size_t from_str_untrimmed(csubstr buf, char *v)
{
    if(buf.len < 1) return csubstr::npos;
    *v = buf[0];
    return 1;
}

//-----------------------------------------------------------------------------
inline size_t to_str(substr buf, csubstr v)
{
    size_t len = buf.len < v.len ? buf.len : v.len;
    memcpy(buf.str, v.str, len);
    return v.len;
}

inline bool from_str(csubstr buf, csubstr *v)
{
    *v = buf;
    return true;
}

inline size_t from_str_untrimmed(substr buf, csubstr *v)
{
    csubstr trimmed = buf.first_non_empty_span();
    if(trimmed.len == 0) return csubstr::npos;
    *v = trimmed;
    return trimmed.end() - buf.begin();
}

//-----------------------------------------------------------------------------
inline size_t to_str(substr buf, substr const& v)
{
    size_t len = buf.len < v.len ? buf.len : v.len;
    memcpy(buf.str, v.str, len);
    return v.len;
}

inline bool from_str(csubstr buf, substr *v)
{
    size_t len = buf.len > v->len ? v->len : buf.len;
    memcpy(v->str, buf.str, len);
    return buf.len <= v->len;
}

inline size_t from_str_untrimmed(csubstr buf, substr *v)
{
    csubstr trimmed = buf.first_non_empty_span();
    if(trimmed.len == 0) return csubstr::npos;
    size_t len = trimmed.len > v->len ? v->len : trimmed.len;
    memcpy(v->str, trimmed.str, len);
    if(trimmed.len > v->len) return csubstr::npos;
    return trimmed.end() - buf.begin();
}

//-----------------------------------------------------------------------------

template< size_t N >
inline size_t to_str(substr buf, const char (&v)[N])
{
    csubstr sp(v);
    return to_str(buf, sp);
}

inline size_t to_str(substr buf, const char *v)
{
    return to_str(buf, to_csubstr(v));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

inline size_t cat(substr /*buf*/)
{
    return 0;
}

/** serialize the arguments to the given span.
 * @return the number of characters written to the buffer. */
template< class Arg, class... Args >
size_t cat(substr buf, Arg const& a, Args const& ...more)
{
    size_t num = to_str(buf, a);
    buf  = buf.len >= num ? buf.sub(num) : substr{};
    num += cat(buf, more...);
    return num;
}

inline size_t uncat(csubstr /*buf*/)
{
    return 0;
}

/** deserialize the arguments from the given span.
 *
 * @return the number of characters read from the buffer. If a
 * conversion was not successful, return npos. */
template< class Arg, class... Args >
size_t uncat(csubstr buf, Arg & a, Args & ...more)
{
    size_t num = from_str_untrimmed(buf, &a);
    if(num == csubstr::npos) return csubstr::npos;
    buf  = buf.len >= num ? buf.sub(num) : substr{};
    num += uncat(buf, more...);
    return num;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template< class Sep, class Arg, class... Args >
size_t catsep(substr buf, Sep const& sep, Arg const& a, Args const& ...more)
{
    size_t num = to_str(buf, sep);
    buf  = buf.len >= num ? buf.sub(num) : substr{};
    num += to_str(buf, a);
    buf  = buf.len >= num ? buf.sub(num) : substr{};
    num += catsep(buf, sep, more...);
    return num;
}

template< class Sep >
inline size_t catsep(substr /*buf*/, Sep const& /*sep*/)
{
    return 0;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

inline size_t format(substr buf, csubstr fmt)
{
    return to_str(buf, fmt);
}

template< class Arg, class... Args >
size_t format(substr buf, csubstr fmt, Arg const& a, Args const& ...more)
{
    auto pos = fmt.find("{}");
    if(pos != csubstr::npos)
    {
        size_t num = to_str(buf, fmt.sub(0, pos));
        size_t out = num;
        buf  = buf.len >= num ? buf.sub(num) : substr{};
        num  = to_str(buf, a);
        out += num;
        buf  = buf.len >= num ? buf.sub(num) : substr{};
        num  = format(buf, fmt.sub(pos + 2), more...);
        out += num;
        return out;
    }
    else
    {
        return format(buf, fmt);
    }
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template< class CharOwningContainer, class... Args >
inline void catrs(CharOwningContainer *cont, Args const& ...args)
{
    substr buf = to_substr(*cont);
    size_t ret = cat(buf, args...);
    cont->resize(ret);
    if(ret > buf.len)
    {
        buf = to_substr(*cont);
        ret = cat(buf, args...);
        if(ret != buf.len)
        {
            cont->resize(ret);
        }
    }
}

template< class CharOwningContainer, class Sep, class... Args >
inline void catseprs(CharOwningContainer *cont, Sep const& sep, Args const& ...args)
{
    substr buf = to_substr(*cont);
    size_t ret = catsep(buf, sep, args...);
    cont->resize(ret);
    if(ret > buf.len)
    {
        buf = to_substr(*cont);
        ret = catsep(buf, sep, args...);
        if(ret != buf.len)
        {
            cont->resize(ret);
        }
    }
}

template< class CharOwningContainer, class... Args >
inline void formatrs(CharOwningContainer *cont, csubstr fmt, Args const& ...args)
{
    substr buf = to_substr(*cont);
    size_t ret = format(buf, fmt, args...);
    cont->resize(ret);
    if(ret > buf.len)
    {
        buf = to_substr(*cont);
        ret = format(buf, fmt, args...);
        if(ret != buf.len)
        {
            cont->resize(ret);
        }
    }
}

} // namespace c4

#endif /* _C4_TO_STR_HPP_ */