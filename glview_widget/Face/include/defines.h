#ifndef _DEFINES_H_
#define _DEFINES_H_

namespace omesh
{

#ifndef NULL
#define NULL    0
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef SWAP
#define SWAP(a, b, t) (t) = (a); (a) = (b); (b) = (t)
#endif

#ifndef SQUARE
#define SQUARE(x) ((x)*(x))
#endif

#ifndef ROUND_UCHAR
#define ROUND_UCHAR(x) (uchar)((x)+0.5)
#endif

#ifndef ROUND_INT
#define ROUND_INT(x) (int)((x)+0.5)
#endif

#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : -(x))
#endif

#ifndef SIGN
#define SIGN(x) ((x) > 0 ? 1 : -1)
#endif

#ifndef DEGTORAD
#define DEGTORAD(x) ((x)*M_PI/180)
#endif

#ifndef RADTODEG
#define RADTODEG(x) ((x)*180/M_PI)
#endif

#ifndef PI
#define PI 3.14159265358979323846264
#endif

#ifndef M_PI
#define M_PI PI
#endif

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

#ifndef MAXFLOAT
#define MAXFLOAT ((float)3.40282346638528860e+38)   
#endif

#ifndef EQSTR
#define EQSTR(x, y)  (strcmp((x),(y)) == 0)
#endif

#ifndef IS_ODD
#define IS_ODD(x)  ((x)%2 != 0)
#endif

#ifndef IS_EVEN
#define IS_EVEN(x)  ((x)%2 == 0)
#endif


}

#endif