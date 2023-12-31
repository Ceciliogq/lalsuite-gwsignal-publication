/*
*  Copyright (C) 2007 Jolien Creighton, Kipp Cannon
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

#include <stdio.h>
#include <lal/LALDatatypes.h>
#include <lal/LALCache.h>
#include <lal/LALFrameIO.h>

#ifndef _LALFRSTREAM_H
#define _LALFRSTREAM_H

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}       /* so that editors will match preceding brace */
#endif

/**
 * @defgroup LALFrStream_h Header LALFrStream.h
 * @ingroup lalframe_general
 *
 * \author Jolien D. E. Creighton
 *
 * \brief High-level routines for manipulating frame data streams.
 *
 * ### Synopsis ###
 *
 * \code
 * #include <lal/LALFrStream.h>
 * \endcode
 *
 * ### Description ###
 *
 * A frame stream is like a file stream except that it streams along the set
 * of frames in a set of frame files.  These routines are high-level routines
 * that allow you to extract frame data.  Many of these routines have names
 * similar to the standard C file stream manipulation routines and perform
 * similar functions.  These routines are found in Module LALFrameStream.c.
 *
 * Routines for reading data from a frame stream, either in sequential
 * or random-access mode, are found in Module LALFrameStreamRead.c.
 *
 * @{
 * @defgroup LALFrStream_c     Module LALFrStream.c
 * @defgroup LALFrStreamRead_c Module LALFrStreamRead.c
 * @}
 *
 * @addtogroup LALFrStream_c
 * @{
 */

/** Enum listing different stream states */
typedef enum tagLALFrStreamState {
    LAL_FR_STREAM_OK = 0,       /**< nominal */
    LAL_FR_STREAM_ERR = 1,      /**< error in frame stream */
    LAL_FR_STREAM_END = 2,      /**< end of frame stream */
    LAL_FR_STREAM_GAP = 4,      /**< gap in frame stream */
    LAL_FR_STREAM_URL = 8,      /**< error opening frame URL */
    LAL_FR_STREAM_TOC = 16      /**< error reading frame TOC */
} LALFrStreamState;

/** Enum listing different stream modes */
typedef enum tagLALFrStreamMode {
    LAL_FR_STREAM_SILENT_MODE = 0,	/**< silent mode */
    LAL_FR_STREAM_TIMEWARN_MODE = 1,    /**< display warning for invalid time requests */
    LAL_FR_STREAM_GAPINFO_MODE = 2,     /**< display info for gaps in data */
    LAL_FR_STREAM_VERBOSE_MODE = 3,     /**< display warnings and info */
    LAL_FR_STREAM_IGNOREGAP_MODE = 4,   /**< ignore gaps in data */
    LAL_FR_STREAM_IGNORETIME_MODE = 8,  /**< ignore invalid times requested */
    LAL_FR_STREAM_DEFAULT_MODE = 15,    /**< ignore time/gaps but report warnings & info */
    LAL_FR_STREAM_CHECKSUM_MODE = 16    /**< ensure that file checksums are OK */
} LALFrStreamMode;

/**
 * This structure details the state of the frame stream.  The contents are
 * private; you should not tamper with them!
 */
typedef struct tagLALFrStream {
    LALFrStreamState state;
    INT4 mode;
    LIGOTimeGPS epoch;
    LALCache *cache;
    UINT4 fnum;
    LALFrFile *file;
    INT4 pos;
} LALFrStream;

/**
 * This structure contains a record of the state of a frame stream; this
 * record can be used to restore the stream to the state when the record
 * was made (provided the stream has not been closed).
 */
typedef struct tagLALFrStreamPos {
  LIGOTimeGPS epoch;	/**< the GPS time of the open frame when the record  was made */
  UINT4 fnum;		/**< the file number of a list of frame files that was open when the record was made */
  INT4 pos;		/**< the position within the frame file that was open when the record was made */
} LALFrStreamPos;

/** @} */

LALFrStream *XLALFrStreamCacheOpen(LALCache * cache);
LALFrStream *XLALFrStreamOpen(const char *dirname, const char *pattern);
int XLALFrStreamClose(LALFrStream * stream);
int XLALFrStreamGetMode(LALFrStream * stream);
int XLALFrStreamSetMode(LALFrStream * stream, int mode);

int XLALFrStreamState(LALFrStream * stream);
int XLALFrStreamEnd(LALFrStream * stream);
int XLALFrStreamError(LALFrStream * stream);
int XLALFrStreamClearErr(LALFrStream * stream);
int XLALFrStreamRewind(LALFrStream * stream);
int XLALFrStreamNext(LALFrStream * stream);
int XLALFrStreamSeek(LALFrStream * stream, const LIGOTimeGPS * epoch);
int XLALFrStreamSeekO(LALFrStream * stream, double dt, int whence);
int XLALFrStreamTell(LIGOTimeGPS * epoch, LALFrStream * stream);
int XLALFrStreamGetpos(LALFrStreamPos * position, LALFrStream * stream);
int XLALFrStreamSetpos(LALFrStream * stream, const LALFrStreamPos * position);

int XLALFrStreamGetVectorLength(const char *chname, LALFrStream * stream);
LALTYPECODE XLALFrStreamGetTimeSeriesType(const char *chname,
    LALFrStream * stream);

int XLALFrStreamGetINT2TimeSeries(INT2TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetINT4TimeSeries(INT4TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetINT8TimeSeries(INT8TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetUINT2TimeSeries(UINT2TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetUINT4TimeSeries(UINT4TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetUINT8TimeSeries(UINT8TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetREAL4TimeSeries(REAL4TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetREAL8TimeSeries(REAL8TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetCOMPLEX8TimeSeries(COMPLEX8TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetCOMPLEX16TimeSeries(COMPLEX16TimeSeries * series,
    LALFrStream * stream);

int XLALFrStreamGetINT2TimeSeriesMetadata(INT2TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetINT4TimeSeriesMetadata(INT4TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetINT8TimeSeriesMetadata(INT8TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetUINT2TimeSeriesMetadata(UINT2TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetUINT4TimeSeriesMetadata(UINT4TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetUINT8TimeSeriesMetadata(UINT8TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetREAL4TimeSeriesMetadata(REAL4TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetREAL8TimeSeriesMetadata(REAL8TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetCOMPLEX8TimeSeriesMetadata(COMPLEX8TimeSeries * series,
    LALFrStream * stream);
int XLALFrStreamGetCOMPLEX16TimeSeriesMetadata(COMPLEX16TimeSeries * series,
    LALFrStream * stream);

int XLALFrStreamGetREAL4FrequencySeries(REAL4FrequencySeries * series,
    LALFrStream * stream);
int XLALFrStreamGetREAL8FrequencySeries(REAL8FrequencySeries * series,
    LALFrStream * stream);
int XLALFrStreamGetCOMPLEX8FrequencySeries(COMPLEX8FrequencySeries * series,
    LALFrStream * stream);
int XLALFrStreamGetCOMPLEX16FrequencySeries(COMPLEX16FrequencySeries * series,
    LALFrStream * stream);

INT2TimeSeries *XLALFrStreamReadINT2TimeSeries(LALFrStream * stream,
    const char *chname, const LIGOTimeGPS * start, REAL8 duration,
    size_t lengthlimit);
INT4TimeSeries *XLALFrStreamReadINT4TimeSeries(LALFrStream * stream,
    const char *chname, const LIGOTimeGPS * start, REAL8 duration,
    size_t lengthlimit);
INT8TimeSeries *XLALFrStreamReadINT8TimeSeries(LALFrStream * stream,
    const char *chname, const LIGOTimeGPS * start, REAL8 duration,
    size_t lengthlimit);
UINT2TimeSeries *XLALFrStreamReadUINT2TimeSeries(LALFrStream * stream,
    const char *chname, const LIGOTimeGPS * start, REAL8 duration,
    size_t lengthlimit);
UINT4TimeSeries *XLALFrStreamReadUINT4TimeSeries(LALFrStream * stream,
    const char *chname, const LIGOTimeGPS * start, REAL8 duration,
    size_t lengthlimit);
UINT8TimeSeries *XLALFrStreamReadUINT8TimeSeries(LALFrStream * stream,
    const char *chname, const LIGOTimeGPS * start, REAL8 duration,
    size_t lengthlimit);
REAL4TimeSeries *XLALFrStreamReadREAL4TimeSeries(LALFrStream * stream,
    const char *chname, const LIGOTimeGPS * start, REAL8 duration,
    size_t lengthlimit);
REAL8TimeSeries *XLALFrStreamReadREAL8TimeSeries(LALFrStream * stream,
    const char *chname, const LIGOTimeGPS * start, REAL8 duration,
    size_t lengthlimit);
COMPLEX8TimeSeries *XLALFrStreamReadCOMPLEX8TimeSeries(LALFrStream * stream,
    const char *chname, const LIGOTimeGPS * start, REAL8 duration,
    size_t lengthlimit);
COMPLEX16TimeSeries *XLALFrStreamReadCOMPLEX16TimeSeries(LALFrStream * stream,
    const char *chname, const LIGOTimeGPS * start, REAL8 duration,
    size_t lengthlimit);

REAL4FrequencySeries *XLALFrStreamReadREAL4FrequencySeries(LALFrStream *
    stream, const char *chname, const LIGOTimeGPS * epoch);
REAL8FrequencySeries *XLALFrStreamReadREAL8FrequencySeries(LALFrStream *
    stream, const char *chname, const LIGOTimeGPS * epoch);
COMPLEX8FrequencySeries *XLALFrStreamReadCOMPLEX8FrequencySeries(LALFrStream *
    stream, const char *chname, const LIGOTimeGPS * epoch);
COMPLEX16FrequencySeries *XLALFrStreamReadCOMPLEX16FrequencySeries(LALFrStream
    * stream, const char *chname, const LIGOTimeGPS * epoch);

REAL8TimeSeries *XLALFrStreamInputREAL8TimeSeries(LALFrStream * stream,
    const char *channel, const LIGOTimeGPS * start, REAL8 duration,
    size_t lengthlimit);
COMPLEX16TimeSeries *XLALFrStreamInputCOMPLEX16TimeSeries(LALFrStream *
    stream, const char *channel, const LIGOTimeGPS * start, REAL8 duration,
    size_t lengthlimit);

REAL8FrequencySeries *XLALFrStreamInputREAL8FrequencySeries(LALFrStream *
    stream, const char *chname, const LIGOTimeGPS * epoch);
COMPLEX16FrequencySeries
    *XLALFrStreamInputCOMPLEX16FrequencySeries(LALFrStream * stream,
    const char *chname, const LIGOTimeGPS * epoch);



/* ---------- LEGACY CODE ---------- */

/*\name Legacy Error Codes */
/** @{ */
#define FRAMESTREAMH_ENULL 00001
#define FRAMESTREAMH_ENNUL 00002
#define FRAMESTREAMH_EALOC 00004
#define FRAMESTREAMH_EFILE 00010
#define FRAMESTREAMH_EOPEN 00020
#define FRAMESTREAMH_EREAD 00040
#define FRAMESTREAMH_ETIME 00100
#define FRAMESTREAMH_ESIZE 00200
#define FRAMESTREAMH_ECHAN 00400
#define FRAMESTREAMH_ETYPE 01000
#define FRAMESTREAMH_ERROR 02000
#define FRAMESTREAMH_EDONE 04000
#define FRAMESTREAMH_ETREQ 010000
#define FRAMESTREAMH_EDGAP 020000
#define FRAMESTREAMH_MSGENULL "Null pointer"
#define FRAMESTREAMH_MSGENNUL "Non-null pointer"
#define FRAMESTREAMH_MSGEALOC "Memory allocation error"
#define FRAMESTREAMH_MSGEFILE "Frame data files not found"
#define FRAMESTREAMH_MSGEOPEN "Frame file open error"
#define FRAMESTREAMH_MSGEREAD "Frame file read error"
#define FRAMESTREAMH_MSGETIME "Invalid ADC offset time"
#define FRAMESTREAMH_MSGESIZE "Invalid vector length"
#define FRAMESTREAMH_MSGECHAN "Could not find ADC channel"
#define FRAMESTREAMH_MSGETYPE "Invalid ADC type"
#define FRAMESTREAMH_MSGERROR "Frame stream error"
#define FRAMESTREAMH_MSGEDONE "End of frame data"
#define FRAMESTREAMH_MSGETREQ "No data at time requested"
#define FRAMESTREAMH_MSGEDGAP "Gap in the data"
/** @} */

typedef enum tagFrChanType {
    LAL_ADC_CHAN, LAL_SIM_CHAN, LAL_PROC_CHAN
} FrChanType;

/* @name for backwards compatability
 *
 * These are the various types of channel that can be specified for read/write.
 * They are "post-processed data" (ProcDataChannel), "ADC data"
 * (ADCDataChannel), and "simulated data" (SimDataChannel).
 */
/** @{ */
#define ChannelType FrChanType
#define ProcDataChannel LAL_PROC_CHAN
#define ADCDataChannel  LAL_ADC_CHAN
#define SimDataChannel  LAL_SIM_CHAN
/** @} */

#ifdef SWIG /* SWIG interface directives */
SWIGLAL(IMMUTABLE_MEMBERS(tagFrChanIn, name));
#endif /* SWIG */

/*
 * This structure specifies the channel to read as input.
 */
typedef struct tagFrChanIn {
  const CHAR *name;	/**< the name of the channel */
  ChannelType type;	/**< the channel type */
} FrChanIn;

#ifdef SWIG /* SWIG interface directives */
SWIGLAL(IMMUTABLE_MEMBERS(tagFrOutPar, source, description));
#endif /* SWIG */
/*
 *
 * This structure specifies the parameters for output of data to a frame.
 * The output frame file name will be
 * \<source\>-\<description\>-\<GPS start time\>-\<duration\>.gwf
 */
typedef struct tagFrOutPar {
  const CHAR *source;		/**< the source identifier to attach to the output frame file name */
  const CHAR *description;	/**< the description identifier to attach to the output frame file name */
  ChannelType type;		/**< the type of channel to create in the output frames */
  UINT4 nframes;		/**< the number of frames to output in the frame file */
  UINT4 frame;			/**< the number the first frame of output*/
  UINT4 run;			/**< the number this data run */
} FrOutPar;

/** @name Legacy API
 *  @{
 */
void
LALFrCacheOpen(LALStatus * status, LALFrStream ** output, LALCache * cache);

void
LALFrOpen(LALStatus * status, LALFrStream ** stream, const CHAR * dirname,
    const CHAR * pattern);

void LALFrClose(LALStatus * status, LALFrStream ** stream);

void LALFrSeek(LALStatus * status, const LIGOTimeGPS * epoch,
    LALFrStream * stream);

void LALFrGetINT2TimeSeries(LALStatus * status, INT2TimeSeries * series,
    FrChanIn * chanin, LALFrStream * stream);

void LALFrGetINT2TimeSeriesMetadata(LALStatus * status,
    INT2TimeSeries * series, FrChanIn * chanin, LALFrStream * stream);

void LALFrGetINT4TimeSeries(LALStatus * status, INT4TimeSeries * series,
    FrChanIn * chanin, LALFrStream * stream);

void LALFrGetINT4TimeSeriesMetadata(LALStatus * status,
    INT4TimeSeries * series, FrChanIn * chanin, LALFrStream * stream);

void LALFrGetINT8TimeSeries(LALStatus * status, INT8TimeSeries * series,
    FrChanIn * chanin, LALFrStream * stream);

void LALFrGetINT8TimeSeriesMetadata(LALStatus * status,
    INT8TimeSeries * series, FrChanIn * chanin, LALFrStream * stream);

void LALFrGetREAL4TimeSeries(LALStatus * status, REAL4TimeSeries * series,
    FrChanIn * chanin, LALFrStream * stream);

void LALFrGetREAL4TimeSeriesMetadata(LALStatus * status,
    REAL4TimeSeries * series, FrChanIn * chanin, LALFrStream * stream);

void LALFrGetREAL8TimeSeries(LALStatus * status, REAL8TimeSeries * series,
    FrChanIn * chanin, LALFrStream * stream);

void LALFrGetREAL8TimeSeriesMetadata(LALStatus * status,
    REAL8TimeSeries * series, FrChanIn * chanin, LALFrStream * stream);

void LALFrGetCOMPLEX8TimeSeries(LALStatus * status,
    COMPLEX8TimeSeries * series, FrChanIn * chanin, LALFrStream * stream);

void LALFrGetCOMPLEX8TimeSeriesMetadata(LALStatus * status,
    COMPLEX8TimeSeries * series, FrChanIn * chanin, LALFrStream * stream);

void LALFrGetCOMPLEX8FrequencySeries(LALStatus * status,
    COMPLEX8FrequencySeries * series,
    FrChanIn * chanin, LALFrStream * stream);

void LALFrWriteINT4TimeSeries(LALStatus * status,
    INT4TimeSeries * series, FrOutPar * params);

void LALFrWriteREAL4TimeSeries(LALStatus * status,
    REAL4TimeSeries * series, FrOutPar * params);
/** @} */


#if 0
{       /* so that editors will match succeeding brace */
#endif
#ifdef __cplusplus
}
#endif

#endif /* _LALFRSTREAM_H */
