/*
 * $Id: math_unchecked.h,v 1.0 2026-11-01 00:00:00 clib2devs Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MATH_UNCHECKED_H
#define _MATH_UNCHECKED_H

/****************************************************************************/
/* Without an FPU there are no exception flags, which leaves errno as the only
   way to report a math error. These functions are renamed in libm, where they
   call each other without a word about their intermediate results, and
   math_errno.h gives each one a public name which looks at the result. */

/****************************************************************************/

#if defined(IEEE_FLOATING_POINT_SUPPORT) || (defined(__PPC__) && defined(_SOFT_FLOAT))

#define __MATH_SETS_ERRNO

/****************************************************************************/

#define acos __acos_unchecked
#define acosf __acosf_unchecked
#define acosl __acosl_unchecked
#define asin __asin_unchecked
#define asinf __asinf_unchecked
#define asinl __asinl_unchecked
#define acosh __acosh_unchecked
#define acoshf __acoshf_unchecked
#define acoshl __acoshl_unchecked
#define atanh __atanh_unchecked
#define atanhf __atanhf_unchecked
#define atanhl __atanhl_unchecked
#define log __log_unchecked
#define logf __logf_unchecked
#define logl __logl_unchecked
#define log2 __log2_unchecked
#define log2f __log2f_unchecked
#define log2l __log2l_unchecked
#define log10 __log10_unchecked
#define log10f __log10f_unchecked
#define log10l __log10l_unchecked
#define log1p __log1p_unchecked
#define log1pf __log1pf_unchecked
#define log1pl __log1pl_unchecked
#define sqrt __sqrt_unchecked
#define sqrtf __sqrtf_unchecked
#define sqrtl __sqrtl_unchecked
#define sin __sin_unchecked
#define sinf __sinf_unchecked
#define sinl __sinl_unchecked
#define cos __cos_unchecked
#define cosf __cosf_unchecked
#define cosl __cosl_unchecked
#define tan __tan_unchecked
#define tanf __tanf_unchecked
#define tanl __tanl_unchecked
#define tgamma __tgamma_unchecked
#define tgammaf __tgammaf_unchecked
#define tgammal __tgammal_unchecked
#define lgamma __lgamma_unchecked
#define lgammaf __lgammaf_unchecked
#define lgammal __lgammal_unchecked
#define logb __logb_unchecked
#define logbf __logbf_unchecked
#define logbl __logbl_unchecked
#define exp __exp_unchecked
#define expf __expf_unchecked
#define expl __expl_unchecked
#define exp2 __exp2_unchecked
#define exp2f __exp2f_unchecked
#define exp2l __exp2l_unchecked
#define expm1 __expm1_unchecked
#define expm1f __expm1f_unchecked
#define expm1l __expm1l_unchecked
#define exp10 __exp10_unchecked
#define exp10f __exp10f_unchecked
#define exp10l __exp10l_unchecked
#define sinh __sinh_unchecked
#define sinhf __sinhf_unchecked
#define sinhl __sinhl_unchecked
#define cosh __cosh_unchecked
#define coshf __coshf_unchecked
#define coshl __coshl_unchecked
#define pow __pow_unchecked
#define powf __powf_unchecked
#define powl __powl_unchecked
#define fmod __fmod_unchecked
#define fmodf __fmodf_unchecked
#define fmodl __fmodl_unchecked
#define remainder __remainder_unchecked
#define remainderf __remainderf_unchecked
#define remainderl __remainderl_unchecked
#define hypot __hypot_unchecked
#define hypotf __hypotf_unchecked
#define hypotl __hypotl_unchecked
#define nextafter __nextafter_unchecked
#define nextafterf __nextafterf_unchecked
#define nextafterl __nextafterl_unchecked
#define fdim __fdim_unchecked
#define fdimf __fdimf_unchecked
#define fdiml __fdiml_unchecked
#define remquo __remquo_unchecked
#define remquof __remquof_unchecked
#define remquol __remquol_unchecked
#define fma __fma_unchecked
#define fmaf __fmaf_unchecked
#define fmal __fmal_unchecked
#define scalbn __scalbn_unchecked
#define scalbnf __scalbnf_unchecked
#define scalbnl __scalbnl_unchecked
#define scalbln __scalbln_unchecked
#define scalblnf __scalblnf_unchecked
#define scalblnl __scalblnl_unchecked
#define nexttoward __nexttoward_unchecked
#define nexttowardf __nexttowardf_unchecked
#define nexttowardl __nexttowardl_unchecked

#endif /* IEEE_FLOATING_POINT_SUPPORT || soft float on the PowerPC */

/****************************************************************************/

#endif /* _MATH_UNCHECKED_H */
