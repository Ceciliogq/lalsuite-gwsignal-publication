/* Copyright (C) 2012 Evan Ochsner
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with with program; see the file COPYING. If not, write to the
 *  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#ifndef _LALSIMINSPIRALWAVEFORMFLAGS_H
#define _LALSIMINSPIRALWAVEFORMFLAGS_H

#include <stdbool.h>
#include <lal/LALMalloc.h>
#include <lal/LALError.h>
#include <lal/LALDict.h>
#include <lal/Sequence.h>

#if defined(__cplusplus)
extern "C" {
#elif 0
} /* so that editors will match preceding brace */
#endif

/**
 * @addtogroup LALSimInspiral_h
 * @{
 */

/** Default values for all enumerated flags */
#define LAL_SIM_INSPIRAL_SPIN_ORDER_DEFAULT LAL_SIM_INSPIRAL_SPIN_ORDER_ALL
#define LAL_SIM_INSPIRAL_TIDAL_ORDER_DEFAULT LAL_SIM_INSPIRAL_TIDAL_ORDER_ALL
#define LAL_SIM_INSPIRAL_GMTIDES_DEFAULT LAL_SIM_INSPIRAL_GMTIDES_OFF
#define LAL_SIM_INSPIRAL_FRAME_AXIS_DEFAULT LAL_SIM_INSPIRAL_FRAME_AXIS_ORBITAL_L
#define LAL_SIM_INSPIRAL_MODES_CHOICE_DEFAULT LAL_SIM_INSPIRAL_MODES_CHOICE_RESTRICTED

/** Maximum L spherical harmonic mode that is supported in Mode Array*/
#define LAL_SIM_L_MAX_MODE_ARRAY 8


/**
 * Enumeration of allowed PN orders of spin effects. All effects up to and
 * including the given order will be included in waveforms.
 * They can be specified by integers equal to twice the PN order,
 * so e.g. LAL_SIM_INSPIRAL_SPIN_ORDER_25PN = 5
 * In addition, LAL_SIM_INSPIRAL_SPIN_ORDER_ALL = -1
 * is a flag to include all available spin effects
 */
typedef enum tagLALSimInspiralSpinOrder {
    LAL_SIM_INSPIRAL_SPIN_ORDER_0PN  = 0,
    LAL_SIM_INSPIRAL_SPIN_ORDER_05PN = 1,
    LAL_SIM_INSPIRAL_SPIN_ORDER_1PN  = 2,
    LAL_SIM_INSPIRAL_SPIN_ORDER_15PN = 3,
    LAL_SIM_INSPIRAL_SPIN_ORDER_2PN  = 4,
    LAL_SIM_INSPIRAL_SPIN_ORDER_25PN = 5,
    LAL_SIM_INSPIRAL_SPIN_ORDER_3PN  = 6,
    LAL_SIM_INSPIRAL_SPIN_ORDER_35PN = 7,
    LAL_SIM_INSPIRAL_SPIN_ORDER_ALL  = -1
} LALSimInspiralSpinOrder;

/**
 * Enumeration of allowed PN orders of tidal effects. All effects up to and
 * including the given order will be included in waveforms.
 * Numerically, they are equal to twice the PN order, so e.g.
 * LAL_SIM_INSPIRAL_TIDAL_ORDER_5PN = 10
 * In addition, LAL_SIM_INSPIRAL_TIDAL_ORDER_ALL = -1
 * is a flag to include all tidal PN orders up to the default
 * value (which currently is 7PN for TaylorF2, 6PN for all
 * other approximants).
 */
typedef enum tagLALSimInspiralTidalOrder {
    LAL_SIM_INSPIRAL_TIDAL_ORDER_0PN =  0,
    LAL_SIM_INSPIRAL_TIDAL_ORDER_5PN = 10,
    LAL_SIM_INSPIRAL_TIDAL_ORDER_6PN = 12,
    LAL_SIM_INSPIRAL_TIDAL_ORDER_65PN = 13,
    LAL_SIM_INSPIRAL_TIDAL_ORDER_7PN = 14,
    LAL_SIM_INSPIRAL_TIDAL_ORDER_75PN = 15,
    LAL_SIM_INSPIRAL_TIDAL_ORDER_ALL = -1,
} LALSimInspiralTidalOrder;

typedef enum tagLALSimInspiralGETides {
    LAL_SIM_INSPIRAL_GETIDES_OFF,
    LAL_SIM_INSPIRAL_GETIDES_NNLO,
    LAL_SIM_INSPIRAL_GETIDES_GSF2,
    LAL_SIM_INSPIRAL_GETIDES_GSF23,
    LAL_SIM_INSPIRAL_GETIDES_NOPT
} LALSimInspiralGETides;

typedef enum tagLALSimInspiralGMTides {
    LAL_SIM_INSPIRAL_GMTIDES_OFF,
    LAL_SIM_INSPIRAL_GMTIDES_PN,
    LAL_SIM_INSPIRAL_GMTIDES_GSF,
    LAL_SIM_INSPIRAL_GMTIDES_NOPT
} LALSimInspiralGMTides;


/**
 * Enumerator for choosing the reference frame associated with
 * PSpinInspiralRD waveforms.
 */
typedef enum tagLALSimInspiralFrameAxis {
    LAL_SIM_INSPIRAL_FRAME_AXIS_VIEW, /**< Set z-axis along direction of GW propagation (line of sight) */
    LAL_SIM_INSPIRAL_FRAME_AXIS_TOTAL_J, /**< Set z-axis along the initial total angular momentum */
    LAL_SIM_INSPIRAL_FRAME_AXIS_ORBITAL_L, /**< Set z-axis along the initial orbital angular momentum */
} LALSimInspiralFrameAxis;

/**
 * Enumerator for choosing which modes to include in IMR models.
 *
 * 'ALL' means to use all modes available to that model.
 *
 * 'RESTRICTED' means only the (2,2) mode for non-precessing models
 * or only the set of l=2 modes for precessing models.
 */
typedef enum tagLALSimInspiralModesChoice {
  LAL_SIM_INSPIRAL_MODES_CHOICE_DEFAULT    = 1,                     /**< Include only (2,2) or l=2 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_3L         = 1<<1,                  /**< Inlude only l=3 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_2AND3L     = (1<<2) - 1,            /**< Inlude l=2,3 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_4L         = 1<<2,                  /**< Inlude only l=4 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_2AND3AND4L = (1<<3) - 1,            /**< Include l=2,3,4 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_2AND4L     = (1<<3) - 1 - (1<<1),   /**< Include l=2,4 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_3AND4L     = (1<<3) - (1<<1),       /**< Include l=3,4 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_5L         = 1<<3,                  /**< Inlude only l=5 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_2AND5L     = (1<<3) + 1,            /**< Inlude l=2,5 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_3AND5L     = (1<<3) + (1<<1),       /**< Inlude l=3,5 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_4AND5L     = (1<<3) + (1<<2),       /**< Inlude l=4,5 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_2AND3AND5L = (1<<4) - 1 -(1<<2),    /**< Inlude l=2,3,5 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_2AND4AND5L = (1<<4) - 1 -(1<<1),    /**< Inlude l=2,4,5 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_3AND4AND5L = (1<<4) - (1<<1),       /**< Inlude l=3,4,5 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_2AND3AND4AND5L = (1<<4) - 1,        /**< Include l=2,3,4,5 modes */
  LAL_SIM_INSPIRAL_MODES_CHOICE_ALL        = - 1,                   /**< Include all available modes */
} LALSimInspiralModesChoice;

/**
 * Struct containing several enumerated flags that control specialized behavior
 * for some waveform approximants.
 *
 * Users: Access this struct only through the Create/Destroy/Set/Get/IsDefault
 * functions declared in this file.
 *
 * Developers: Do not add anything but enumerated flags to this struct. Avoid
 * adding extra flags whenever possible.
 */
typedef struct tagLALSimInspiralWaveformFlags LALSimInspiralWaveformFlags;

/** @} */

LALSimInspiralWaveformFlags *XLALSimInspiralCreateWaveformFlags(void);
void XLALSimInspiralDestroyWaveformFlags(LALSimInspiralWaveformFlags *waveFlags);
bool XLALSimInspiralWaveformParamsFlagsAreDefault(LALDict *params);
bool XLALSimInspiralWaveformFlagsIsDefaultOLD(LALSimInspiralWaveformFlags *waveFlags);
bool XLALSimInspiralWaveformFlagsEqualOLD(LALSimInspiralWaveformFlags *waveFlags1, LALSimInspiralWaveformFlags *waveFlags2);
bool XLALSimInspiralWaveformFlagsEqual(LALDict *LALpars1, LALDict *LALpars2);
void XLALSimInspiralSetSpinOrder(LALSimInspiralWaveformFlags *waveFlags, LALSimInspiralSpinOrder spinO);
LALSimInspiralSpinOrder XLALSimInspiralGetSpinOrder(LALSimInspiralWaveformFlags *waveFlags);
bool XLALSimInspiralSpinOrderIsDefault(LALSimInspiralSpinOrder spinO);
void XLALSimInspiralSetTidalOrder(LALSimInspiralWaveformFlags *waveFlags, LALSimInspiralTidalOrder tideO);
LALSimInspiralTidalOrder XLALSimInspiralGetTidalOrder(LALSimInspiralWaveformFlags *waveFlags);
bool XLALSimInspiralTidalOrderIsDefault(LALSimInspiralTidalOrder tideO);
void XLALSimInspiralSetFrameAxis(LALSimInspiralWaveformFlags *waveFlags, LALSimInspiralFrameAxis axisChoice);
LALSimInspiralFrameAxis XLALSimInspiralGetFrameAxis(LALSimInspiralWaveformFlags *waveFlags);
bool XLALSimInspiralFrameAxisIsDefault(LALSimInspiralFrameAxis axisChoice);
void XLALSimInspiralSetModesChoice(LALSimInspiralWaveformFlags *waveFlags, LALSimInspiralModesChoice modesChoice);
LALSimInspiralModesChoice XLALSimInspiralGetModesChoice(LALSimInspiralWaveformFlags *waveFlags);
bool XLALSimInspiralModesChoiceIsDefault(LALSimInspiralModesChoice modesChoice);
void XLALSimInspiralSetNumrelDataOLD(LALSimInspiralWaveformFlags *waveFlags, const char* numreldata);
char* XLALSimInspiralGetNumrelDataOLD(LALSimInspiralWaveformFlags *waveFlags);

LALValue * XLALSimInspiralCreateModeArray(void);
LALValue * XLALSimInspiralModeArrayActivateMode(LALValue *modes, unsigned l, int m);
LALValue * XLALSimInspiralModeArrayDeactivateMode(LALValue *modes, unsigned l, int m);
LALValue * XLALSimInspiralModeArrayActivateAllModes(LALValue *modes);
LALValue * XLALSimInspiralModeArrayDeactivateAllModes(LALValue *modes);
int XLALSimInspiralModeArrayIsModeActive(LALValue *modes, unsigned l, int m);
LALValue * XLALSimInspiralModeArrayActivateAllModesAtL(LALValue *modes, unsigned l);
LALValue * XLALSimInspiralModeArrayDeactivateAllModesAtL(LALValue *modes, unsigned l);
int XLALSimInspiralModeArrayPrintModes(LALValue *modes);
INT2Sequence *XLALSimInspiralModeArrayReadModes(LALValue *modes);
char * XLALSimInspiralModeArrayToModeString(LALValue *modes);
LALValue * XLALSimInspiralModeArrayFromModeString(const char *modestr);


#if 0
{ /* so that editors will match succeeding brace */
#elif defined(__cplusplus)
}
#endif

#endif /* _LALSIMINSPIRALWAVEFORMFLAGS_H */
