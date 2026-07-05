#ifndef CORE_MATH_H
#define CORE_MATH_H
/*********************************************
 * core/convert: dtoa が必要とする最小 math (移植可能)
 * 旧 include/math.h は presettings.h(→arm.h) を引いていたため置換。
 *********************************************/
#include "core_config.h"

nINLINE double pow(double base, double super);

#endif /* CORE_MATH_H */
