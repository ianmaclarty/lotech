/* ANSI-C code produced by gperf version 3.0.4 */
/* Command-line: gperf -tTE -LANSI-C -H LTParticleSystem_field_hash -N LTParticleSystem_field_info  */
/* Computed positions: -k'1,8,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif


#include "ltcommon.h"
#include "ltparticles.h"
/* maximum key range = 79, duplicates = 0 */

#if (defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || defined(__cplusplus) || defined(__GNUC_STDC_INLINE__)
inline
#elif defined(__GNUC__)
__inline
#endif
static unsigned int
LTParticleSystem_field_hash (register const char *str, register unsigned int len)
{
  static unsigned char asso_values[] =
    {
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 20, 87, 30, 87, 87,
       5,  0, 87, 15, 87,  5, 87, 87, 10, 87,
      20,  5, 35, 87, 15,  0, 15,  0, 87, 87,
      25, 20, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
      87, 87, 87, 87, 87, 87
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[7]];
      /*FALLTHROUGH*/
      case 7:
      case 6:
      case 5:
      case 4:
      case 3:
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

#ifdef __GNUC__
__inline
#if defined __GNUC_STDC_INLINE__ || defined __GNUC_GNU_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
struct LTFieldInfo *
LTParticleSystem_field_info (register const char *str, register unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 45,
      MIN_WORD_LENGTH = 4,
      MAX_WORD_LENGTH = 26,
      MIN_HASH_VALUE = 8,
      MAX_HASH_VALUE = 86
    };

  static struct LTFieldInfo wordlist[] =
    {
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {"end_size", ((size_t) ( (char *)&((LTParticleSystem *)(0))->end_size - (char *)0 ))},
      {""},
      {"speed", ((size_t) ( (char *)&((LTParticleSystem *)(0))->speed - (char *)0 ))},
      {""},
      {"elapsed", ((size_t) ( (char *)&((LTParticleSystem *)(0))->elapsed - (char *)0 ))},
      {""},
      {"life", ((size_t) ( (char *)&((LTParticleSystem *)(0))->life - (char *)0 ))},
      {"start_size", ((size_t) ( (char *)&((LTParticleSystem *)(0))->start_size - (char *)0 ))},
      {""},
      {"end_size_variance", ((size_t) ( (char *)&((LTParticleSystem *)(0))->end_size_variance - (char *)0 ))},
      {""},
      {"end_color_blue", ((size_t) ( (char *)&((LTParticleSystem *)(0))->end_color.b - (char *)0 ))},
      {""},
      {"start_color_blue", ((size_t) ( (char *)&((LTParticleSystem *)(0))->start_color.b - (char *)0 ))},
      {""},
      {"end_color_red", ((size_t) ( (char *)&((LTParticleSystem *)(0))->end_color.r - (char *)0 ))},
      {"start_size_variance", ((size_t) ( (char *)&((LTParticleSystem *)(0))->start_size_variance - (char *)0 ))},
      {"start_color_red", ((size_t) ( (char *)&((LTParticleSystem *)(0))->start_color.r - (char *)0 ))},
      {""},
      {"emit_counter", ((size_t) ( (char *)&((LTParticleSystem *)(0))->emit_counter - (char *)0 ))},
      {"end_color_variance_blue", ((size_t) ( (char *)&((LTParticleSystem *)(0))->end_color_variance.b - (char *)0 ))},
      {""},
      {"start_color_variance_blue", ((size_t) ( (char *)&((LTParticleSystem *)(0))->start_color_variance.b - (char *)0 ))},
      {""},
      {"end_color_variance_red", ((size_t) ( (char *)&((LTParticleSystem *)(0))->end_color_variance.r - (char *)0 ))},
      {"emission_rate", ((size_t) ( (char *)&((LTParticleSystem *)(0))->emission_rate - (char *)0 ))},
      {"start_color_variance_red", ((size_t) ( (char *)&((LTParticleSystem *)(0))->start_color_variance.r - (char *)0 ))},
      {"angle", ((size_t) ( (char *)&((LTParticleSystem *)(0))->angle - (char *)0 ))},
      {""},
      {"end_spin_variance", ((size_t) ( (char *)&((LTParticleSystem *)(0))->end_spin_variance - (char *)0 ))},
      {"life_variance", ((size_t) ( (char *)&((LTParticleSystem *)(0))->life_variance - (char *)0 ))},
      {""},
      {"end_color_green", ((size_t) ( (char *)&((LTParticleSystem *)(0))->end_color.g - (char *)0 ))},
      {""},
      {"start_color_green", ((size_t) ( (char *)&((LTParticleSystem *)(0))->start_color.g - (char *)0 ))},
      {""},
      {"speed_variance", ((size_t) ( (char *)&((LTParticleSystem *)(0))->speed_variance - (char *)0 ))},
      {"tangential_accel_variance", ((size_t) ( (char *)&((LTParticleSystem *)(0))->tangential_accel_variance - (char *)0 ))},
      {"tangential_accel", ((size_t) ( (char *)&((LTParticleSystem *)(0))->tangential_accel - (char *)0 ))},
      {""},
      {"end_spin", ((size_t) ( (char *)&((LTParticleSystem *)(0))->end_spin - (char *)0 ))},
      {"end_color_variance_green", ((size_t) ( (char *)&((LTParticleSystem *)(0))->end_color_variance.g - (char *)0 ))},
      {"end_color_alpha", ((size_t) ( (char *)&((LTParticleSystem *)(0))->end_color.a - (char *)0 ))},
      {"start_color_variance_green", ((size_t) ( (char *)&((LTParticleSystem *)(0))->start_color_variance.g - (char *)0 ))},
      {"start_color_alpha", ((size_t) ( (char *)&((LTParticleSystem *)(0))->start_color.a - (char *)0 ))},
      {"duration", ((size_t) ( (char *)&((LTParticleSystem *)(0))->duration - (char *)0 ))},
      {"start_spin_variance", ((size_t) ( (char *)&((LTParticleSystem *)(0))->start_spin_variance - (char *)0 ))},
      {""}, {""}, {""}, {""},
      {"end_color_variance_alpha", ((size_t) ( (char *)&((LTParticleSystem *)(0))->end_color_variance.a - (char *)0 ))},
      {""},
      {"start_color_variance_alpha", ((size_t) ( (char *)&((LTParticleSystem *)(0))->start_color_variance.a - (char *)0 ))},
      {"aspect_ratio", ((size_t) ( (char *)&((LTParticleSystem *)(0))->aspect_ratio - (char *)0 ))},
      {""},
      {"gravity_y", ((size_t) ( (char *)&((LTParticleSystem *)(0))->gravity.y - (char *)0 ))},
      {"start_spin", ((size_t) ( (char *)&((LTParticleSystem *)(0))->start_spin - (char *)0 ))},
      {"radial_accel_variance", ((size_t) ( (char *)&((LTParticleSystem *)(0))->radial_accel_variance - (char *)0 ))},
      {"radial_accel", ((size_t) ( (char *)&((LTParticleSystem *)(0))->radial_accel - (char *)0 ))},
      {""},
      {"gravity_x", ((size_t) ( (char *)&((LTParticleSystem *)(0))->gravity.x - (char *)0 ))},
      {""}, {""},
      {"source_position_y", ((size_t) ( (char *)&((LTParticleSystem *)(0))->source_position.y - (char *)0 ))},
      {""},
      {"angle_variance", ((size_t) ( (char *)&((LTParticleSystem *)(0))->angle_variance - (char *)0 ))},
      {""}, {""},
      {"source_position_x", ((size_t) ( (char *)&((LTParticleSystem *)(0))->source_position.x - (char *)0 ))},
      {""}, {""}, {""},
      {"source_position_variance_y", ((size_t) ( (char *)&((LTParticleSystem *)(0))->source_position_variance.y - (char *)0 ))},
      {""}, {""}, {""}, {""},
      {"source_position_variance_x", ((size_t) ( (char *)&((LTParticleSystem *)(0))->source_position_variance.x - (char *)0 ))}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = LTParticleSystem_field_hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
