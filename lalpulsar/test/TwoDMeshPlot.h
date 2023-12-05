/*
*  Copyright (C) 2007 Jolien Creighton, Teviet Creighton
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


#ifndef _TWODMESHPLOT_H
#define _TWODMESHPLOT_H

#include <lal/LALStdlib.h>

#if defined(__cplusplus)
extern "C" {
#elif 0
} /* so that editors will match preceding brace */
#endif

/**
 * \author Creighton, T. D.
 * \defgroup TwoDMeshPlot_h Header TwoDMeshPlot.h
 * \ingroup lalpulsar_templbank
 * \brief Provides routines to plot two-dimensional parameter meshes.
 *
 * ### Synopsis ###
 *
 * \code
 * #include "TwoDMeshPlot.h"
 * \endcode
 *
 * This header covers routines that display 2-dimensional
 * parameter meshes as generated by the routines in \ref TwoDMesh_h.
 * The output is written to a C \c FILE-type stream using
 * fprintf(). The routines do not provide a system-level interface
 * to open or close file streams; they simply assume that they have been
 * passed an open, writable stream.  Nonetheless, their use of stream I/O
 * means that they cannot be included in LAL (although executables, in
 * particular the \ref TwoDMesh_h test programs, may statically link
 * against their object files).
 */
/** @{ */

/** \name Error Codes */
/** @{ */
#define TWODMESHPLOTH_ENUL    1
#define TWODMESHPLOTH_EOUT    2
#define TWODMESHPLOTH_EMEM    3
#define TWODMESHPLOTH_EMETRIC 4
#define TWODMESHPLOTH_ENOPLOT 5

#define TWODMESHPLOTH_MSGENUL    "Unexpected null pointer in arguments"
#define TWODMESHPLOTH_MSGEOUT    "Output handle points to a non-null pointer"
#define TWODMESHPLOTH_MSGEMEM    "Memory allocation error"
#define TWODMESHPLOTH_MSGEMETRIC "Non-positive metric"
#define TWODMESHPLOTH_MSGENOPLOT "Nothing to plot"
/** @} */

/**
 * \name Constants
 * \brief The values in the table above are stored in the constants \c TWODMESHPLOTH_<tt><</tt>\e name<tt>></tt>.
 */
/** @{ */
#define TWODMESHPLOTH_XSIZE 540 /**< Horizontal size of plotting area (points = \f$ 1/72'' \f$ ) */
#define TWODMESHPLOTH_YSIZE 720 /**< Vertical size of plotting area (points) */
#define TWODMESHPLOTH_XMARG  36 /**< Distance from left of page to plotting area (points) */
#define TWODMESHPLOTH_YMARG  36 /**< Distance from bottom of page to plotting area (points) */
/** @} */

/**
 * \brief This structure stores parameters specifying how to plot a PostScript diagram of the parameter mesh.
 */
typedef struct tagTwoDMeshPlotStruc {
  REAL4 theta;		/**< Angle from the horizontal direction of the plot counterclockwise to the \f$ x \f$ -coordinate axis of the mesh, in degrees */
  REAL4 xScale, yScale;	/**< Plotting scale of the mesh coordinate axes, in points per unit \f$ x \f$ or \f$ y \f$ (a point is 1/72 of an inch) */
  REAL4 bBox[4];	/**< Bounding box surrounding the figure in plot coordinates, measured in points. */
  BOOLEAN autoscale;	/**< If true, \c xScale and \c yScale will be adjusted so that the drawn figure will lie
                         * within the \c bBox.  If false, \c bBox will be adjusted to enclose the figure, given \c xScale and \c yScale.
                         */
  REAL4 clipBox[4];	/**< Four components \f$ x_\mathrm{min} \f$ , \f$ y_\mathrm{min} \f$ , \f$ x_\mathrm{max} \f$ , \f$ y_\mathrm{max} \f$ (in that order)
                         * specifying the corners of a box in the \f$ x \f$ - \f$ y \f$ coordinate system outside of which no marks will be made; if either
                         * max value is less than or equal to the corresponding min value, \c clipBox will be ignored.
                         */
  UINT4 nLevels;	/**< The number of levels of recursive submeshes to plot.  If zero, the mesh will not be plotted (although the boundary may be) */
  UINT4 nBoundary;	/**< \e half the number of points to plot along the boundary of the parameter region; at least 4 points are required;
                         * if \c plotBoundary \f$ <2 \f$ , none will be plotted.
                         */
  INT2 *plotPoints;	/**< An array from <tt>[0]</tt> to <tt>[nLevels]</tt> indicating how to plot the mesh points at each
                         * recursive level: a value of 0 means don't plot mesh points, a positive value means to plot filled
                         * circles of that diameter (in points), a negative value means to plot empty circles of that diameter (in points)
                         */
  BOOLEAN *plotTiles;	/**< An array from <tt>[0]</tt> to <tt>[nLevels]</tt> indicating whether to plot the tiles around each mesh
                         * point, at each recursive level.
                         */
  BOOLEAN *plotEllipses;/**< An array from <tt>[0]</tt> to <tt>[nLevels]</tt> indicating whether to plot the mismatch ellipses
                         * around each mesh point, at each recursive level.
                         */
  TwoDMeshParamStruc *params;/**< An array from <tt>[0]</tt> to <tt>[nLevels]</tt> of parameter structures used to generate the meshes
                              * at each recursive level (making the assumption that \e all submeshes of the same level used the same parameters);
                              * in general only the <tt>getMetric()</tt> and \c mThresh fields are used from parameter structures after the first.
                              */
  /* private */
  BOOLEAN clip;             /**< [private] whether to use clipBox, above */
  REAL4 cosTheta, sinTheta; /**< [private] cosine and sine of theta, above */
} TwoDMeshPlotStruc;


/** \cond DONT_DOXYGEN */
/* Function prototypes. */
void
LALPlotTwoDMesh( LALStatus         *stat,
		 FILE              *stream,
		 TwoDMeshNode      *mesh,
		 TwoDMeshPlotStruc *params );

/** \endcond */

/** @} */

#if 0
{ /* so that editors will match succeeding brace */
#elif defined(__cplusplus)
}
#endif

#endif /* _TWODMESHPLOT_H */