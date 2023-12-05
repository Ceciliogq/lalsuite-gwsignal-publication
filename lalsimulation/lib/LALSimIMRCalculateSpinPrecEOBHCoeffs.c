#ifndef _LALSimIMRCalculateSpinPrecEOBHCoeffs_C
#define _LALSimIMRCalculateSpinPrecEOBHCoeffs_C
#include "LALSimIMRSpinEOBHamiltonian.h"

/**
 * \author Craig Robinson, Yi Pan, Stas Babak, Prayush Kumar, Andrea Taracchini
 *
 * This function was originally part of LALSimIMRSpinEOBHamiltonianPrec.c,
 * and moved here during the development of v3_opt.  Function relocation
 * implemented by R. Devine, Z. Etienne, D. Buch, and T. Knowles.  In comments,
 * R.H. refers to Roland Hass.
 */

#include <stdio.h>
#include <math.h>
#include <LALSimIMRSpinEOB.h>

/*------------------------------------------------------------------------------------------
 *
 *          Prototypes of functions defined in this code.
 *
 *------------------------------------------------------------------------------------------
 */

static int XLALSimIMRCalculateSpinPrecEOBHCoeffs(
        SpinEOBHCoeffs *coeffs,
        const REAL8    eta,
        const REAL8    a,
        const UINT4    SpinAlignedEOBversion
        );


/*------------------------------------------------------------------------------------------
 *
 *          Defintions of functions.
 *
 *------------------------------------------------------------------------------------------
 */

/**
 *
 * This function is used to calculate some coefficients which will be used in the
 * spinning EOB Hamiltonian. It takes the following inputs:
 *
 * coeffs - a (non-null) pointer to a SpinEOBParams structure. This will be populated
 * with the output.
 * eta - the symmetric mass ratio.
 * sigmaKerr - the spin of the effective Kerr background (a combination of the individual spins).
 *
 * If all goes well, the function will return XLAL_SUCCESS. Otherwise, XLAL_FAILURE is returned.
 */
static int XLALSimIMRCalculateSpinPrecEOBHCoeffs(
        SpinEOBHCoeffs *coeffs, /**<< OUTPUT, EOB parameters including pre-computed coefficients */
        const REAL8    eta,     /**<< symmetric mass ratio */
        const REAL8    a,       /**<< Normalized deformed Kerr spin */
        const UINT4    SpinAlignedEOBversion  /**<< 1 for SEOBNRv1; 2 for SEOBNRv2 */
        )
{

  REAL8 KK, k0, k1, k2, k3, k4, k5, k5l, k1p2, k1p3;
  REAL8 m1PlusEtaKK;

  if ( !coeffs )
  {
    XLAL_ERROR( XLAL_EINVAL );
  }

  coeffs->SpinAlignedEOBversion = SpinAlignedEOBversion;
  const int debugPK = 0;
  if( debugPK )
  {
    XLAL_PRINT_INFO("In XLALSimIMRCalculateSpinPrecEOBHCoeffs: SpinAlignedEOBversion = %d,%d\n",
        (int) SpinAlignedEOBversion, (int) coeffs->SpinAlignedEOBversion );
    fflush( NULL );
  }
  /* Constants are fits taken from PRD 86, 024011 (2012) Eq. 37 */
  static const REAL8 c0  = 1.4467; /* needed to get the correct self-force results */
  static const REAL8 c1  = -1.7152360250654402;
  static const REAL8 c2  = -3.246255899738242;

  static const REAL8 c20  = 1.712;
  static const REAL8 c21  = -1.803949138004582;
  static const REAL8 c22  = -39.77229225266885;
  static const REAL8 c23  = 103.16588921239249;

  static const REAL8 third = 1./3.;
  static const REAL8 fifth = 1./5.;
  static const REAL8 ln2 = 0.6931471805599453094172321214581765680755; // log(2.)

  // RH: this assumes that SpinAlignedEOBversion is either 1 or 2
  // RH: the ifthenelse macros return their ifvalue if cond>=0 (specifically
  // the positive sign is set) and elsevalue otherwise. So:
  // RH: 1.5-SpinAlignedEOBversion is positive for SpinAlignedEOBversion==1 and
  // RH: negative for SpinAlignedEOBversion==2
  // RH: SpinAlignedEOBversion-1.5 is reversed

  // RH: TODO check if b3 can ever be non-zero. If not remove all terms using b3.
  coeffs->b3  = 0.;
  coeffs->bb3 = 0.;
#define ifthenelse(cond, ifvalue, elsevalue) ((elsevalue) + (0.5 + copysign(0.5, cond)) * ((ifvalue)-(elsevalue)))
  coeffs->KK = KK = ifthenelse(1.5-SpinAlignedEOBversion,
                               c0 + c1*eta + c2*eta*eta,
                               c20 + c21*eta + c22*(eta*eta) + c23*(eta*eta)*eta);
    REAL8 chi = a / (1. - 2. * eta);
    REAL8 eta2 = eta * eta, eta3 = eta2 * eta;
    REAL8 chi2 = chi * chi, chi3 = chi2 * chi;
    if (SpinAlignedEOBversion == 4)
    {
        coeffs->KK = KK =
        coeff00K + coeff01K * chi + coeff02K * chi2 + coeff03K * chi3 +
        coeff10K * eta + coeff11K * eta * chi + coeff12K * eta * chi2 +
        coeff13K * eta * chi3 + coeff20K * eta2 + coeff21K * eta2 * chi +
        coeff22K * eta2 * chi2 + coeff23K * eta2 * chi3 + coeff30K * eta3 +
        coeff31K * eta3 * chi + coeff32K * eta3 * chi2 + coeff33K * eta3 * chi3;
        //      printf("KK %.16e\n", KK);
    }
  m1PlusEtaKK = -1. + eta*KK;
  const REAL8 invm1PlusEtaKK = 1./m1PlusEtaKK;
  /* Eqs. 5.77 - 5.81 of PRD 81, 084024 (2010) */
  coeffs->k0 = k0 = KK*(m1PlusEtaKK - 1.);
  coeffs->k1 = k1 = - 2.*(k0 + KK)*m1PlusEtaKK;
  k1p2= k1*k1;
  k1p3= k1*k1p2;
  coeffs->k2 = k2 = (k1 * (k1 - 4.*m1PlusEtaKK)) * 0.5 - a*a*k0*m1PlusEtaKK*m1PlusEtaKK;
  coeffs->k3 = k3 = -(k1*k1)*k1 * third + k1*k2 + (k1*k1)*m1PlusEtaKK - 2.*(k2 - m1PlusEtaKK)*m1PlusEtaKK - a*a*k1*(m1PlusEtaKK*m1PlusEtaKK);
  coeffs->k4 = k4 = ((24./96.)*(k1*k1)*(k1*k1) - (96./96.)*(k1*k1)*k2 + (48./96.)*k2*k2 - (64./96.)*(k1*k1)*k1*m1PlusEtaKK
      + (48./96.)*(a*a)*(k1*k1 - 2.*k2)*(m1PlusEtaKK*m1PlusEtaKK) +
      (96./96.)*k1*(k3 + 2.*k2*m1PlusEtaKK) - m1PlusEtaKK*((192./96.)*k3 + m1PlusEtaKK*(-(3008./96.) + (123./96.)*LAL_PI*LAL_PI)));
#define ifthenelsezero(cond, ifvalue) ((0.5 + copysign(0.5, cond)) * (ifvalue))
  coeffs->k5 = k5 = ifthenelsezero(SpinAlignedEOBversion-1.5,
                     m1PlusEtaKK*m1PlusEtaKK
                     * (-4237./60.+128./5.*LAL_GAMMA+2275.*LAL_PI*LAL_PI/512.
                     - third*(a*a)*(k1p3-3.*(k1*k2)+3.*k3)
                     - ((k1p3*k1p2)-5.*(k1p3*k2)+5.*k1*k2*k2+5.*k1p2*k3-5.*k2*k3-5.*k1*k4)*fifth*invm1PlusEtaKK*invm1PlusEtaKK
                     + ((k1p2*k1p2)-4.*(k1p2*k2)+2.*k2*k2+4.*k1*k3-4.*k4)*0.5*invm1PlusEtaKK+(256./5.)*ln2)
                    );
  coeffs->k5l= k5l= ifthenelsezero(SpinAlignedEOBversion-1.5, (m1PlusEtaKK*m1PlusEtaKK) * (64./5.));
  if ( SpinAlignedEOBversion == 4 ) {
      coeffs->k5 = k5 =  m1PlusEtaKK*m1PlusEtaKK
                                       * (-4237./60.+128./5.*LAL_GAMMA+2275.*LAL_PI*LAL_PI/512.
                                          - third*(a*a)*(k1p3-3.*(k1*k2)+3.*k3)
                                          - ((k1p3*k1p2)-5.*(k1p3*k2)+5.*k1*k2*k2+5.*k1p2*k3-5.*k2*k3-5.*k1*k4)*fifth*invm1PlusEtaKK*invm1PlusEtaKK
                                          + ((k1p2*k1p2)-4.*(k1p2*k2)+2.*k2*k2+4.*k1*k3-4.*k4)*0.5*invm1PlusEtaKK+(256./5.)*ln2  + (41. * LAL_PI * LAL_PI / 32. -
                                                                                                                                    221. / 6.) * eta );
      coeffs->k5l= k5l= (m1PlusEtaKK*m1PlusEtaKK) * (64./5.);
   }


  /* Now calibrated parameters for spin models */
  coeffs->d1 = ifthenelsezero(1.5-SpinAlignedEOBversion, -69.5);
  coeffs->d1v2 = ifthenelsezero(SpinAlignedEOBversion-1.5, -74.71 - 156.*eta + 627.5*eta*eta);
  coeffs->dheffSS = ifthenelsezero(1.5-SpinAlignedEOBversion, 2.75);
  coeffs->dheffSSv2 = ifthenelsezero(SpinAlignedEOBversion-1.5, 8.127 - 154.2*eta + 830.8*eta*eta);
  if (SpinAlignedEOBversion==4) {
      coeffs->d1 = 0.;
      coeffs->dheffSS = 0.;
      // dSO
      coeffs->d1v2 =
      coeff00dSO + coeff01dSO * chi + coeff02dSO * chi2 + coeff03dSO * chi3 +
      coeff10dSO * eta + coeff11dSO * eta * chi + coeff12dSO * eta * chi2 +
      coeff13dSO * eta * chi3 + coeff20dSO * eta2 + coeff21dSO * eta2 * chi +
      coeff22dSO * eta2 * chi2 + coeff23dSO * eta2 * chi3 + coeff30dSO * eta3 +
      coeff31dSO * eta3 * chi + coeff32dSO * eta3 * chi2 + coeff33dSO * eta3 * chi3;

      // dSS
      coeffs->dheffSSv2 =
      coeff00dSS + coeff01dSS * chi + coeff02dSS * chi2 + coeff03dSS * chi3 +
      coeff10dSS * eta + coeff11dSS * eta * chi + coeff12dSS * eta * chi2 +
      coeff13dSS * eta * chi3 + coeff20dSS * eta2 + coeff21dSS * eta2 * chi +
      coeff22dSS * eta2 * chi2 + coeff23dSS * eta2 * chi3 + coeff30dSS * eta3 +
      coeff31dSS * eta3 * chi + coeff32dSS * eta3 * chi2 + coeff33dSS * eta3 * chi3;
      //          printf("dSO %.16e, dSS %.16e\n", coeffs->d1v2,coeffs->dheffSSv2);
  }
  return XLAL_SUCCESS;
}

/**
 *
 * This function is used to calculate some coefficients which will be used in the
 * spinning EOB Hamiltonian. It was specifically designed for precessing binaries,
 * such that it allows the spin parameter chi that enters the NR-calibrated terms (like KK) to be different
 * from a that enters derived coefficients like k0,k1,...
 * If all goes well, the function will return XLAL_SUCCESS. Otherwise, XLAL_FAILURE is returned.
 * The paper references are:
 * 1. Barausse and Buonanno  - PRD 81, 084024 (2010) [arXiv:0912.3517]
 * 2. Bohe et al - Phys. Rev. D 95, 044028 – (2017) [arXiv:1611.03703]
 * 3. Steinhoff et al - Phys. Rev. D 94, 104028 (2016) [arXiv:1608.01907]
*/
static int XLALSimIMRCalculateSpinPrecEOBHCoeffs_v2(
    SpinEOBHCoeffs *coeffs,           /**<< OUTPUT, EOB parameters including pre-computed coefficients */
    const REAL8 eta,                  /**<< symmetric mass ratio */
    REAL8 a,                          /**<< Normalized deformed Kerr spin */
    REAL8 chi,                        /**<< The augmented spin, with correct aligned-spin limit */
    const UINT4 SpinAlignedEOBversion /**<< 4 for SEOBNRv4P; Possible to extend this later later */
)
{

  REAL8 KK, k0, k1, k2, k3, k4, k5, k5l, k1p2, k1p3;
  REAL8 m1PlusEtaKK;

  if (!coeffs)
  {
    XLAL_ERROR(XLAL_EINVAL);
  }

  coeffs->SpinAlignedEOBversion = SpinAlignedEOBversion;
  const int debugPK = 0;
  if (debugPK)
  {
    XLAL_PRINT_INFO("In XLALSimIMRCalculateSpinPrecEOBHCoeffs: SpinAlignedEOBversion = %d,%d\n",
                    (int)SpinAlignedEOBversion, (int)coeffs->SpinAlignedEOBversion);
    fflush(NULL);
  }

  static const REAL8 third = 1. / 3.;
  static const REAL8 fifth = 1. / 5.;
  static const REAL8 ln2 = 0.6931471805599453094172321214581765680755; // log(2.)
  coeffs->b3 = 0.;
  coeffs->bb3 = 0.;
  REAL8 eta2 = eta * eta, eta3 = eta2 * eta;
  REAL8 chi2 = chi * chi, chi3 = chi2 * chi;
  coeffs->KK = KK = 0;
  if (SpinAlignedEOBversion == 4)
  {
    // See Eq.(4.8) and Eq.(4.12) in Bohe et al
    coeffs->KK = KK =
        coeff00K + coeff01K * chi + coeff02K * chi2 + coeff03K * chi3 +
        coeff10K * eta + coeff11K * eta * chi + coeff12K * eta * chi2 +
        coeff13K * eta * chi3 + coeff20K * eta2 + coeff21K * eta2 * chi +
        coeff22K * eta2 * chi2 + coeff23K * eta2 * chi3 + coeff30K * eta3 +
        coeff31K * eta3 * chi + coeff32K * eta3 * chi2 + coeff33K * eta3 * chi3;
  }
  m1PlusEtaKK = -1. + eta * KK;
  const REAL8 invm1PlusEtaKK = 1. / m1PlusEtaKK;
  /* Eqs.(5.77 - 5.81) of Baruasse and Buonnano PRD 81, 084024 (2010) */
  coeffs->k0 = k0 = KK * (m1PlusEtaKK - 1.);
  coeffs->k1 = k1 = -2. * (k0 + KK) * m1PlusEtaKK;
  k1p2 = k1 * k1;
  k1p3 = k1 * k1p2;
  coeffs->k2 = k2 = (k1 * (k1 - 4. * m1PlusEtaKK)) * 0.5 - a * a * k0 * m1PlusEtaKK * m1PlusEtaKK;
  coeffs->k3 = k3 = -(k1 * k1) * k1 * third + k1 * k2 + (k1 * k1) * m1PlusEtaKK - 2. * (k2 - m1PlusEtaKK) * m1PlusEtaKK - a * a * k1 * (m1PlusEtaKK * m1PlusEtaKK);
  coeffs->k4 = k4 = ((24. / 96.) * (k1 * k1) * (k1 * k1) - (96. / 96.) * (k1 * k1) * k2 + (48. / 96.) * k2 * k2 - (64. / 96.) * (k1 * k1) * k1 * m1PlusEtaKK + (48. / 96.) * (a * a) * (k1 * k1 - 2. * k2) * (m1PlusEtaKK * m1PlusEtaKK) +
                     (96. / 96.) * k1 * (k3 + 2. * k2 * m1PlusEtaKK) - m1PlusEtaKK * ((192. / 96.) * k3 + m1PlusEtaKK * (-(3008. / 96.) + (123. / 96.) * LAL_PI * LAL_PI)));
  if (SpinAlignedEOBversion == 4)
  {
    // Look at Eq.(A2C) in Steinhoff et al. The last term from Eq.(2.3) in Bohe et al.
    coeffs->k5 = k5 = m1PlusEtaKK * m1PlusEtaKK * (-4237. / 60. + 128. / 5. * LAL_GAMMA + 2275. * LAL_PI * LAL_PI / 512. - third * (a * a) * (k1p3 - 3. * (k1 * k2) + 3. * k3) - ((k1p3 * k1p2) - 5. * (k1p3 * k2) + 5. * k1 * k2 * k2 + 5. * k1p2 * k3 - 5. * k2 * k3 - 5. * k1 * k4) * fifth * invm1PlusEtaKK * invm1PlusEtaKK + ((k1p2 * k1p2) - 4. * (k1p2 * k2) + 2. * k2 * k2 + 4. * k1 * k3 - 4. * k4) * 0.5 * invm1PlusEtaKK + (256. / 5.) * ln2 + (41. * LAL_PI * LAL_PI / 32. - 221. / 6.) * eta);
    // This is the first term in the brackets in Eq.(A2C) in Steinhoff et al.
    coeffs->k5l = k5l = (m1PlusEtaKK * m1PlusEtaKK) * (64. / 5.);
  }

  /* Now calibrated parameters for spin models */
  if (SpinAlignedEOBversion == 4)
  {

    coeffs->d1 = 0.;
    coeffs->dheffSS = 0.;
    // dSO: Eq.(4.13) in Bohe et al
    coeffs->d1v2 =
        coeff00dSO + coeff01dSO * chi + coeff02dSO * chi2 + coeff03dSO * chi3 +
        coeff10dSO * eta + coeff11dSO * eta * chi + coeff12dSO * eta * chi2 +
        coeff13dSO * eta * chi3 + coeff20dSO * eta2 + coeff21dSO * eta2 * chi +
        coeff22dSO * eta2 * chi2 + coeff23dSO * eta2 * chi3 + coeff30dSO * eta3 +
        coeff31dSO * eta3 * chi + coeff32dSO * eta3 * chi2 + coeff33dSO * eta3 * chi3;

    // dSS: Eq.(4.14) in Bohe et al
    coeffs->dheffSSv2 =
        coeff00dSS + coeff01dSS * chi + coeff02dSS * chi2 + coeff03dSS * chi3 +
        coeff10dSS * eta + coeff11dSS * eta * chi + coeff12dSS * eta * chi2 +
        coeff13dSS * eta * chi3 + coeff20dSS * eta2 + coeff21dSS * eta2 * chi +
        coeff22dSS * eta2 * chi2 + coeff23dSS * eta2 * chi3 + coeff30dSS * eta3 +
        coeff31dSS * eta3 * chi + coeff32dSS * eta3 * chi2 + coeff33dSS * eta3 * chi3;
    //          printf("dSO %.16e, dSS %.16e\n", coeffs->d1v2,coeffs->dheffSSv2);
  }
  return XLAL_SUCCESS;
}

#endif // _LALSimIMRCalculateSpinPrecEOBHCoeffs_C
