#ifndef RBUV_DEBUG_H_
#define RBUV_DEBUG_H_

# ifndef RBUV_DEBUG

#  define RBUV_DEBUG_LOG(...)
#  define RBUV_DEBUG_LOG_DETAIL(...)

# else

#  include <stdio.h>

#  define RBUV_DEBUG_LOG(...) do {\
  fprintf(stderr, "[rbuv] [%s] ", __func__);\
  fprintf(stderr, __VA_ARGS__);\
  fprintf(stderr, "\n");\
} while (0)

#  if RBUV_DEBUG > 1
#   define RBUV_DEBUG_LOG_DETAIL(...) RBUV_DEBUG_LOG(__VA_ARGS__)
#  else
#   define RBUV_DEBUG_LOG_DETAIL(...)
#  endif

# endif

#endif  /* RBUV_DEBUG_H_ */