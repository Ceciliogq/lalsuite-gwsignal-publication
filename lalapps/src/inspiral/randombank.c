/*
*  Copyright (C) 2007 Duncan Brown
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

/*-----------------------------------------------------------------------
 *
 * File Name: randombank.c
 *
 * Author: Brown, D. A.
 *
 *
 *-----------------------------------------------------------------------
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <regex.h>
#include <time.h>

#include <series.h>

#include <lal/LALConfig.h>
#include <lal/LALgetopt.h>
#include <lal/LALStdio.h>
#include <lal/LALStdlib.h>
#include <lal/LALError.h>
#include <lal/LALDatatypes.h>
#include <lal/AVFactories.h>
#include <lal/LIGOLwXML.h>
#include <lal/LIGOLwXMLRead.h>
#include <lal/LIGOMetadataTables.h>
#include <lal/LIGOMetadataUtils.h>
#include <lal/Units.h>
#include <lal/Date.h>
#include <lal/LALInspiral.h>
#include <lal/LALInspiralBank.h>
#include "inspiral.h"
#include <LALAppsVCSInfo.h>

#define CVS_ID_STRING "$Id$"
#define CVS_NAME_STRING "$Name$"
#define CVS_REVISION "$Revision$"
#define CVS_SOURCE "$Source$"
#define CVS_DATE "$Date$"
#define PROGRAM_NAME "tmpltbank"

int arg_parse_check( int argc, char *argv[], ProcessParamsTable *procparams );

/*
 *
 * variables that control program behaviour
 *
 */

/* debugging */
extern int vrbflg;                      /* verbocity of lal function    */

/* template bank generation parameters */
LIGOTimeGPS gpsStartTime = { 0, 0 };    /* input data GPS start time    */
LIGOTimeGPS gpsEndTime = { 0, 0 };      /* input data GPS end time      */
REAL4   minMass         = -1;           /* minimum component mass       */
REAL4   maxMass         = -1;           /* maximum component mass       */
REAL4  fLow             = -1;           /* low frequency cutoff         */
INT4    numTmplts       = -1;           /* number of template to create */
enum { unset, urandom, user } randSeedType = unset;    /* sim seed type */
INT4  randomSeed        = 0;            /* value of sim rand seed       */

/* output parameters */
CHAR  *userTag          = NULL;
INT4                 outCompress = 0;

int main ( int argc, char *argv[] )
{
  /* lal function variables */
  LALStatus             XLAL_INIT_DECL(status);

  /* templates */
  RandomParams         *randParams = NULL;
  InspiralTemplate      newTmplt;
  SnglInspiralTable    *thisTmplt  = NULL;

  /* output data */
  SnglInspiralTable    *templateBank = NULL;
  ProcessTable         *proctable;
  ProcessParamsTable   *procparams;
  LIGOLwXMLStream      *results;

  /* counters and other variables */
  INT4 i;
  CHAR  fname[256];


  /*
   *
   * initialization
   *
   */


  /* set up inital debugging values */
  lal_errhandler = LAL_ERR_EXIT;

  /* create the process and process params tables */
  proctable = (ProcessTable *)
    calloc( 1, sizeof(ProcessTable) );
  XLALGPSTimeNow(&(proctable->start_time));
  XLALPopulateProcessTable(proctable, PROGRAM_NAME, lalAppsVCSIdentInfo.vcsId,
      lalAppsVCSIdentInfo.vcsStatus, lalAppsVCSIdentInfo.vcsDate, 0);
  procparams = (ProcessParamsTable *) calloc( 1, sizeof(ProcessParamsTable) );

  /* call the argument parse and check function */
  arg_parse_check( argc, argv, procparams );

  /* can use LALMalloc() / LALCalloc() from here */


  /*
   *
   * create the radom number seed
   *
   */


  if ( randSeedType == urandom )
  {
    FILE   *fpRand = NULL;
    INT4    randByte;

    if ( vrbflg )
      fprintf( stdout, "obtaining random seed from /dev/urandom: " );

    randomSeed = 0;
    fpRand = fopen( "/dev/urandom", "r" );
    if ( fpRand )
    {
      for ( randByte = 0; randByte < 4 ; ++randByte )
      {
        INT4 tmpSeed = (INT4) fgetc( fpRand );
        randomSeed += tmpSeed << ( randByte * 8 );
      }
      fclose( fpRand );
    }
    else
    {
      perror( "error obtaining random seed from /dev/urandom" );
      exit( 1 );
    }
  }
  else if ( randSeedType == user )
  {
    if ( vrbflg )
      fprintf( stdout, "using user specified random seed: " );
  }

  if ( vrbflg ) fprintf( stdout, "%d\n", randomSeed );

  /* create the tmplt bank random parameter structure */
  LAL_CALL( LALCreateRandomParams( &status, &randParams, randomSeed ),
      &status );


  /*
   *
   * create a random template bank
   *
   */


  for ( i = 0; i < numTmplts; ++i )
  {
    memset( &newTmplt, 0, sizeof(InspiralTemplate) );
    newTmplt.massChoice = m1Andm2;
    newTmplt.order = LAL_PNORDER_TWO;
    newTmplt.fLower = fLow;

    /* set up the injection masses */
    if ( maxMass == minMass )
    {
      newTmplt.mass1 = (REAL8) maxMass;
      newTmplt.mass2 = (REAL8) maxMass;
    }
    else
    {
      REAL4 mass;
      /* generate random parameters for the injection */
      LAL_CALL( LALUniformDeviate( &status, &mass, randParams ), &status );
      newTmplt.mass1 = (maxMass - minMass) * mass;
      newTmplt.mass1 += minMass;

      LAL_CALL( LALUniformDeviate( &status, &mass, randParams ), &status );
      newTmplt.mass2 = (maxMass - minMass) * mass;
      newTmplt.mass2 += minMass;
    }

    LAL_CALL( LALInspiralParameterCalc( &status, &newTmplt ), &status );

    if ( ! templateBank )
    {
      thisTmplt = templateBank =
        (SnglInspiralTable *) LALCalloc(1, sizeof(SnglInspiralTable));
    }
    else
    {
      thisTmplt = thisTmplt->next =
        (SnglInspiralTable *) LALCalloc(1, sizeof(SnglInspiralTable));
    }

    thisTmplt->mass1 = newTmplt.mass1;
    thisTmplt->mass2 = newTmplt.mass2;
    thisTmplt->mchirp = newTmplt.chirpMass;
    thisTmplt->eta = newTmplt.eta;
    thisTmplt->tau0 = newTmplt.t0;
    thisTmplt->tau2 = newTmplt.t2;
    thisTmplt->tau3 = newTmplt.t3;
    thisTmplt->tau4 = newTmplt.t4;
    thisTmplt->tau5 = newTmplt.t5;
    thisTmplt->ttotal = newTmplt.tC;
    thisTmplt->psi0 = newTmplt.psi0;
    thisTmplt->psi3 = newTmplt.psi3;
    thisTmplt->f_final = newTmplt.fFinal;
    thisTmplt->eta = newTmplt.eta;
    thisTmplt->beta = newTmplt.beta;
    snprintf( thisTmplt->ifo, LIGOMETA_IFO_MAX, "P1" );
    snprintf( thisTmplt->search, LIGOMETA_SEARCH_MAX, "randombank" );
    snprintf( thisTmplt->channel, LIGOMETA_CHANNEL_MAX, "SIM-BANK" );
  }


  /*
   *
   * write the output data
   *
   */

  /* open the output xml file */
  if ( userTag && !outCompress )
  {
    snprintf( fname, sizeof(fname), "P1-TMPLTBANK_%s-%d-%d.xml",
        userTag, gpsStartTime.gpsSeconds,
        gpsEndTime.gpsSeconds - gpsStartTime.gpsSeconds );
  }
  else if ( userTag && outCompress )
  {
    snprintf( fname, sizeof(fname), "P1-TMPLTBANK_%s-%d-%d.xml.gz",
        userTag, gpsStartTime.gpsSeconds,
        gpsEndTime.gpsSeconds - gpsStartTime.gpsSeconds );
  }
  else if ( !userTag && outCompress )
  {
    snprintf( fname, sizeof(fname), "P1-TMPLTBANK-%d-%d.xml.gz",
        gpsStartTime.gpsSeconds,
        gpsEndTime.gpsSeconds - gpsStartTime.gpsSeconds );
  }
  else
  {
    snprintf( fname, sizeof(fname), "P1-TMPLTBANK-%d-%d.xml",
        gpsStartTime.gpsSeconds,
        gpsEndTime.gpsSeconds - gpsStartTime.gpsSeconds );
  }
  results = XLALOpenLIGOLwXMLFile( fname );

  /* write the process table */
  snprintf( proctable->ifos, LIGOMETA_IFO_MAX, "P1" );
  XLALGPSTimeNow(&(proctable->end_time));
  XLALWriteLIGOLwXMLProcessTable( results, proctable );
  free( proctable );

  /* erase the first empty process params entry */
  {
    ProcessParamsTable *emptyPPtable = procparams;
    procparams = procparams->next;
    free( emptyPPtable );
  }

  /* write the process params table */
  XLALWriteLIGOLwXMLProcessParamsTable( results, procparams );
  XLALDestroyProcessParamsTable( procparams );

  /* write the template bank to the file */
  if ( templateBank )
    XLALWriteLIGOLwXMLSnglInspiralTable( results, templateBank );
  while ( templateBank )
  {
    thisTmplt = templateBank;
    templateBank = templateBank->next;
    LALFree( thisTmplt );
  }

  /* close the output xml file */
  XLALCloseLIGOLwXMLFile( results );

  /* free the rest of the memory, check for memory leaks and exit */
  LAL_CALL( LALDestroyRandomParams( &status, &randParams ), &status );
  LALCheckMemoryLeaks();
  exit( 0 );
}


/* ------------------------------------------------------------------------- */

#define ADD_PROCESS_PARAM( pptype, format, ppvalue ) \
this_proc_param = this_proc_param->next = (ProcessParamsTable *) \
  calloc( 1, sizeof(ProcessParamsTable) ); \
  snprintf( this_proc_param->program, LIGOMETA_PROGRAM_MAX, "%s", \
      PROGRAM_NAME ); \
      snprintf( this_proc_param->param, LIGOMETA_PARAM_MAX, "--%s", \
          long_options[option_index].name ); \
          snprintf( this_proc_param->type, LIGOMETA_TYPE_MAX, "%s", pptype ); \
          snprintf( this_proc_param->value, LIGOMETA_VALUE_MAX, format, ppvalue );

#define USAGE \
"  --help                       display this message\n"\
"  --version                    print version information and exit\n"\
"  --user-tag STRING            set the process_params usertag to STRING\n"\
"\n"\
"  --gps-start-time SEC         GPS second of data start time\n"\
"  --gps-end-time SEC           GPS second of data end time\n"\
"  --low-frequency-cutoff F     Compute tau parameters from F Hz\n"\
"  --minimum-mass MASS          set minimum component mass of bank to MASS\n"\
"  --maximum-mass MASS          set maximum component mass of bank to MASS\n"\
"  --number-of-template N       create N random templatess of bank to MASS\n"\
"  --random-seed SEED           set random number seed for injections to SEED\n"\
  "                                 (urandom|integer)\n"\
"  --write-compress             write a compressed xml file\n"\
  "\n"

int arg_parse_check( int argc, char *argv[], ProcessParamsTable *procparams )
{
  /* LALgetopt arguments */
  struct LALoption long_options[] =
  {
    /* these options set a flag */
    {"verbose",                 no_argument,       &vrbflg,           1 },
    {"write-compress",          no_argument,       &outCompress,      1 },
    {"help",                    no_argument,       0,                'h'},
    {"gps-start-time",          required_argument, 0,                'a'},
    {"gps-end-time",            required_argument, 0,                'b'},
    {"low-frequency-cutoff",    required_argument, 0,                'i'},
    {"minimum-mass",            required_argument, 0,                'A'},
    {"maximum-mass",            required_argument, 0,                'B'},
    {"number-of-templates",     required_argument, 0,                'K'},
    {"random-seed",             required_argument, 0,                'J'},
    {"user-tag",                required_argument, 0,                'Z'},
    {"userTag",                 required_argument, 0,                'Z'},
    {0, 0, 0, 0}
  };
  int c;
  ProcessParamsTable *this_proc_param = procparams;


  /*
   *
   * parse command line arguments
   *
   */


  while ( 1 )
  {
    /* LALgetopt_long stores long option here */
    int option_index = 0;
    size_t LALoptarg_len;

    c = LALgetopt_long_only( argc, argv, "hs:a:b:i:A:B:K:J:Z:",
        long_options, &option_index );

    /* detect the end of the options */
    if ( c == - 1 )
    {
      break;
    }

    switch ( c )
    {
      case 0:
        /* if this option set a flag, do nothing else now */
        if ( long_options[option_index].flag != 0 )
        {
          break;
        }
        else
        {
          fprintf( stderr, "error parsing option %s with argument %s\n",
              long_options[option_index].name, LALoptarg );
          exit( 1 );
        }
        break;

      case 'h':
        fprintf( stdout, USAGE );
        exit( 0 );
        break;

      case 'Z':
        /* create storage for the usertag */
        LALoptarg_len = strlen( LALoptarg ) + 1;
        userTag = (CHAR *) calloc( LALoptarg_len, sizeof(CHAR) );
        memcpy( userTag, LALoptarg, LALoptarg_len );

        this_proc_param = this_proc_param->next = (ProcessParamsTable *)
          calloc( 1, sizeof(ProcessParamsTable) );
        snprintf( this_proc_param->program, LIGOMETA_PROGRAM_MAX, "%s",
            PROGRAM_NAME );
        snprintf( this_proc_param->param, LIGOMETA_PARAM_MAX, "-userTag" );
        snprintf( this_proc_param->type, LIGOMETA_TYPE_MAX, "string" );
        snprintf( this_proc_param->value, LIGOMETA_VALUE_MAX, "%s",
            LALoptarg );
        break;

      case 'A':
        minMass = (REAL4) atof( LALoptarg );
        if ( minMass <= 0 )
        {
          fprintf( stdout, "invalid argument to --%s:\n"
              "miniumum component mass must be > 0: "
              "(%f solar masses specified)\n",
              long_options[option_index].name, minMass );
          exit( 1 );
        }
        ADD_PROCESS_PARAM( "float", "%e", minMass );
        break;

      case 'B':
        maxMass = (REAL4) atof( LALoptarg );
        if ( maxMass <= 0 )
        {
          fprintf( stdout, "invalid argument to --%s:\n"
              "maxiumum component mass must be > 0: "
              "(%f solar masses specified)\n",
              long_options[option_index].name, maxMass );
          exit( 1 );
        }
        ADD_PROCESS_PARAM( "float", "%e", maxMass );
        break;

      case 'J':
        if ( ! strcmp( "urandom", LALoptarg ) )
        {
          randSeedType = urandom;
          ADD_PROCESS_PARAM( "string", "%s", LALoptarg );
        }
        else
        {
          randSeedType = user;
          randomSeed = (INT4) atoi( LALoptarg );
          ADD_PROCESS_PARAM( "int", "%d", randomSeed );
        }
        break;

      case 'K':
        numTmplts = (INT4) atoi( LALoptarg );
        if ( numTmplts < 1 )
        {
          fprintf( stderr, "invalid argument to --%s:\n"
              "number of template bank simulations"
              "must be greater than 1: (%d specified)\n",
              long_options[option_index].name, numTmplts );
          exit( 1 );
        }
        ADD_PROCESS_PARAM( "int", "%d", numTmplts );
        break;

      case 'a':
        {
          long int gstartt = atol( LALoptarg );
          if ( gstartt < 441417609 )
          {
            fprintf( stderr, "invalid argument to --%s:\n"
                "GPS start time is prior to "
                "Jan 01, 1994  00:00:00 UTC:\n"
                "(%ld specified)\n",
                long_options[option_index].name, gstartt );
            exit( 1 );
          }
          gpsStartTime.gpsSeconds = (INT4) gstartt;
          gpsStartTime.gpsNanoSeconds = 0;
          ADD_PROCESS_PARAM( "int", "%ld", gstartt );
        }
        break;

      case 'b':
        {
          long int gendt = atol( LALoptarg );
          if ( gendt < 441417609 )
          {
            fprintf( stderr, "invalid argument to --%s:\n"
                "GPS end time is prior to "
                "Jan 01, 1994  00:00:00 UTC:\n"
                "(%ld specified)\n",
                long_options[option_index].name, gendt );
            exit( 1 );
          }
          gpsEndTime.gpsSeconds = (INT4) gendt;
          gpsEndTime.gpsNanoSeconds = 0;
          ADD_PROCESS_PARAM( "int", "%ld", gendt );
        }
        break;

      case 'i':
        fLow = (REAL4) atof( LALoptarg );
        if ( fLow < 0 )
        {
          fprintf( stdout, "invalid argument to --%s:\n"
              "low frequency cutoff is less than 0 Hz: "
              "(%f Hz specified)\n",
              long_options[option_index].name, fLow );
          exit( 1 );
        }
        ADD_PROCESS_PARAM( "float", "%e", fLow );
        break;

      case '?':
        fprintf( stderr, USAGE );
        exit( 1 );
        break;

      default:
        fprintf( stderr, "unknown error while parsing options\n" );
        fprintf( stderr, USAGE );
        exit( 1 );
    }
  }

  if ( LALoptind < argc )
  {
    fprintf( stderr, "extraneous command line arguments:\n" );
    while ( LALoptind < argc )
    {
      fprintf ( stderr, "%s\n", argv[LALoptind++] );
    }
    exit( 1 );
  }

  /*
   *
   * check validity of arguments
   *
   */


  if ( minMass < 0 )
  {
    fprintf( stderr, "--minimum-mass must be specified\n" );
    exit( 1 );
  }
  if ( maxMass < 0 )
  {
    fprintf( stderr, "--maximum-mass must be specified\n" );
    exit( 1 );
  }

  /* check that the bank parameters have been specified */
  if ( numTmplts < 0 )
  {
    fprintf( stderr, "--number-of-templates must be specified\n" );
    exit( 1 );
  }

  return 0;
}

#undef ADD_PROCESS_PARAM
