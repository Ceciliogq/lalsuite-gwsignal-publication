##---------- names of codes and input/output files
mfd_code="lalpulsar_Makefakedata_v5"
cfs_code="lalpulsar_ComputeFstatistic_v2"
gct_code="lalpulsar_HierarchSearchGCT"

SFTfiles="./*.sft"
SFTfiles_H1="./H1-*.sft"
SFTfiles_L1="./L1-*.sft"

## Tolerance of comparison
Tolerance=5e-2	## 5%

## ---------- fixed parameter of our test-signal -------------
Alpha=3.1
Delta=-0.5
h0=1.0
cosi=-0.3
psi=0.6
phi0=1.5
Freq=100.123456789
f1dot=-1e-9
f2dot=1e-18

## perfectly targeted search in sky
AlphaSearch=$Alpha
DeltaSearch=$Delta

## Produce skygrid file for the search
skygridfile="./tmpskygridfile.dat"
echo "$AlphaSearch $DeltaSearch" > $skygridfile

mfd_FreqBand=0.20;
mfd_fmin=100;
numFreqBands=2;	## produce 'frequency-split' SFTs used in E@H

Dterms=8
RngMedWindow=50

gct_FreqBand=0.01
gct_F1dotBand=2.0e-10
gct_F2dotBand=0
gct_dFreq=0.000002 #"2.0e-6"
gct_dF1dot=1.0e-10
gct_dF2dot=0
gct_nCands=10

sqrtSh=1
assumeSqrtSh=1

## --------- Generate fake data set time stamps -------------
echo "----------------------------------------------------------------------"
echo " STEP 0: Generating time-stamps and segments files "
echo "----------------------------------------------------------------------"
echo

Tsft=1800
startTime=852443819
refTime=862999869
Tsegment=90000
if [ -n "$NSEGMENTS" ]; then
    Nsegments=${NSEGMENTS}
else
    Nsegments=3
fi

seggap=$(echo ${Tsegment} | awk '{printf "%.0f", $1 * 1.12345}')

tsFile_H1="./timestampsH1.dat"  # for makefakedata
tsFile_L1="./timestampsL1.dat"  # for makefakedata
segFile="./segments.dat"

tmpTime=$startTime
iSeg=1
while [ $iSeg -le $Nsegments ]; do
    t0=$tmpTime
    t1=$(($t0 + $Tsegment))

    if [ $iSeg -eq 1 -o $iSeg -eq $Nsegments ]; then
        ## first and last segment will be single-IFO only
        NSFT=`echo $Tsegment $Tsft |  awk '{print int(1.0 * $1 / $2 + 0.5) }'`
    else
	## while all other segments are 2-IFO
        NSFT=`echo $Tsegment $Tsft |  awk '{print int(2.0 * $1 / $2 + 0.5) }'`
    fi
    echo "$t0 $t1 $NSFT" >> $segFile

    segs[$iSeg]=$tmpTime # save seg's beginning for later use
    echo "Segment: $iSeg of $Nsegments	GPS start time: ${segs[$iSeg]}"

    Tseg=$Tsft
    while [ $Tseg -le $Tsegment ]; do
        if [ $iSeg -ne 1 ]; then
	    echo "${tmpTime} 0" >> $tsFile_H1
        fi
        if [ $iSeg -ne $Nsegments ]; then	            ## we skip segment N for L1
	    echo "${tmpTime} 0" >> $tsFile_L1
        fi
	tmpTime=$(($tmpTime + $Tsft))
	Tseg=$(($Tseg + $Tsft))
    done

    tmpTime=$(($tmpTime + $seggap))
    iSeg=$(($iSeg + 1))
done

echo
echo "----------------------------------------------------------------------"
echo " STEP 1: Generate Fake Signal with MFDv5"
echo "----------------------------------------------------------------------"
echo

FreqStep=`echo $mfd_FreqBand $numFreqBands |  awk '{print $1 / $2}'`
mfd_fBand=`echo $FreqStep $Tsft |  awk '{print ($1 - 1.5 / $2)}'`	## reduce by 1/2 a bin to avoid including last freq-bins

# construct common MFD cmd
mfd_CL_common="--Band=${mfd_fBand} --Tsft=$Tsft --randSeed=1000 --outSingleSFT --IFOs=H1,L1 --timestampsFiles=${tsFile_H1},${tsFile_L1}"

if [ "$sqrtSh" != "0" ]; then
    mfd_CL_common="$mfd_CL_common --sqrtSX=${sqrtSh},${sqrtSh}";
fi

iFreq=1
while [ $iFreq -le $numFreqBands ]; do
    mfd_fi=`echo $mfd_fmin $iFreq $FreqStep | awk '{print $1 + ($2 - 1) * $3}'`

    cmdline="$mfd_code $mfd_CL_common --fmin=$mfd_fi --outSFTdir=. --outLabel=freqBand$iFreq"
    if [ $iFreq -eq 2 ]; then
        cmdline="$cmdline --injectionSources=\"{Freq=$Freq; f1dot=$f1dot; f2dot=$f2dot; Alpha=$Alpha; Delta=$Delta; psi=$psi; phi0=$phi0; h0=$h0; cosi=$cosi; refTime=$refTime}\""
    fi
    echo "$cmdline";
    if ! eval "$cmdline"; then
        echo "Error.. something failed when running '$mfd_code' ..."
        exit 1
    fi

    iFreq=$(( $iFreq + 1 ))

done

echo
echo "----------------------------------------------------------------------"
echo "STEP 2: run CFSv2 over segments"
echo "----------------------------------------------------------------------"
echo
outfile_cfs="./CFS.dat";

if [ ! -r "$outfile_cfs" ]; then
    tmpfile_cfs="./__tmp_CFS.dat";
    cfs_CL_common=" --Alpha=$Alpha --Delta=$Delta --Freq=$Freq --f1dot=$f1dot --f2dot=$f2dot --outputLoudest=$tmpfile_cfs --refTime=$refTime --Dterms=$Dterms --RngMedWindow=$RngMedWindow --outputSingleFstats"
    if [ "$sqrtSh" = "0" ]; then
        # assume sqrtS=${assumeSqrtSh} in all detectors, regardless of how many detectors are in segment
        cfs_CL_common="$cfs_CL_common --assumeSqrtSX=${assumeSqrtSh}";
    fi

    TwoFsum=0
    TwoFsum_L1=0
    TwoFsum_H1=0

    for ((iSeg=1; iSeg <= $Nsegments; iSeg++)); do
        minStartGPS=${segs[$iSeg]}
        maxStartGPS=$(($minStartGPS + $Tsegment))
        cfs_CL="$cfs_code $cfs_CL_common --minStartTime=$minStartGPS --maxStartTime=$maxStartGPS"

        # ----- get multi-IFO + single-IFO F-stat values
        cmdline="$cfs_CL --DataFiles='$SFTfiles'"
        echo "$cmdline"
        if ! eval "$cmdline"; then
	    echo "Error.. something failed when running '$cfs_code' ..."
	    exit 1
        fi

        resCFS=$(cat ${tmpfile_cfs} | awk '{if($1=="twoF") {printf "%.11g", $3}}')
        TwoFsum=$(echo $TwoFsum $resCFS | awk '{printf "%.11g", $1 + $2}')

        if [ $iSeg -eq 1 ]; then	## segment 1 has no H1 SFTs
            resCFS_L1=$(cat ${tmpfile_cfs} | awk '{if($1=="twoF0") {printf "%.11g", $3}}')
            TwoFsum_L1=$(echo $TwoFsum_L1 $resCFS_L1 | awk '{printf "%.11g", $1 + $2}')
        elif [ $iSeg -eq $Nsegments ]; then	## segment N has no L1 SFTs
            resCFS_H1=$(cat ${tmpfile_cfs} | awk '{if($1=="twoF0") {printf "%.11g", $3}}')	## therefore 'H1' is the first and only detector
            TwoFsum_H1=$(echo $TwoFsum_H1 $resCFS_H1 | awk '{printf "%.11g", $1 + $2}')
        else
            resCFS_H1=$(cat ${tmpfile_cfs} | awk '{if($1=="twoF0") {printf "%.11g", $3}}')	## 'H1' is first
            TwoFsum_H1=$(echo $TwoFsum_H1 $resCFS_H1 | awk '{printf "%.11g", $1 + $2}')

            resCFS_L1=$(cat ${tmpfile_cfs} | awk '{if($1=="twoF1") {printf "%.11g", $3}}')	## 'L1' second
            TwoFsum_L1=$(echo $TwoFsum_L1 $resCFS_L1 | awk '{printf "%.11g", $1 + $2}')
        fi
    done

    TwoFAvg=$(echo    $TwoFsum    $Nsegments | awk '{printf "%-12.11g", $1 / ($2)}')
    TwoFAvg_H1=$(echo $TwoFsum_H1 $Nsegments | awk '{printf "%-12.11g", $1 / ($2-1)}')	## H1 has one segment less (the first one)
    TwoFAvg_L1=$(echo $TwoFsum_L1 $Nsegments | awk '{printf "%-12.11g", $1 / ($2-1)}')	## L1 also one segment less (the last one)
    echo "$TwoFAvg	$TwoFAvg_H1	$TwoFAvg_L1" > $outfile_cfs
else
    echo "CFS result file '$outfile_cfs' exists already ... reusing it"
    cfs_res=$(cat $outfile_cfs)
    TwoFAvg=$(echo $cfs_res | awk '{print $1}')
    TwoFAvg_H1=$(echo $cfs_res | awk '{print $2}')
    TwoFAvg_L1=$(echo $cfs_res | awk '{print $3}')
fi

echo
echo "==>   Average <2F_multi>=$TwoFAvg, <2F_H1>=$TwoFAvg_H1, <2F_L1>=$TwoFAvg_L1"

## ---------- run GCT code on this data ----------------------------------------

gct_CL_common="--gridType1=3 --nCand1=$gct_nCands --skyRegion='allsky' --Freq=$Freq --DataFiles='$SFTfiles' --skyGridFile='$skygridfile' --printCand1 --semiCohToplist --df1dot=$gct_dF1dot --f1dot=$f1dot --f1dotBand=$gct_F1dotBand  --df2dot=$gct_dF2dot --f2dot=$f2dot --f2dotBand=$gct_F2dotBand --dFreq=$gct_dFreq --FreqBand=$gct_FreqBand --refTime=$refTime --segmentList=$segFile --Dterms=$Dterms --blocksRngMed=$RngMedWindow"
if [ "$sqrtSh" = "0" ]; then
    # assume sqrtS=${assumeSqrtSh} in both H1 and L1 detectors
    gct_CL_common="$gct_CL_common --assumeSqrtSX=${assumeSqrtSh},${assumeSqrtSh}";
fi

BSGL_flags="--computeBSGL --Fstar0=10 --oLGX='0.5,0.5' --recalcToplistStats"

echo
echo "----------------------------------------------------------------------------------------------------"
echo " STEP 3: run HierarchSearchGCT using Resampling and default 2F toplist"
echo "----------------------------------------------------------------------------------------------------"
echo

rm -f checkpoint.cpt # delete checkpoint to start correctly
outfile_GCT_RS="./GCT_RS.dat"
timingsfile_RS="./timing_RS.dat"

cmdline="$gct_code $gct_CL_common --FstatMethod=ResampGeneric --fnameout='$outfile_GCT_RS' --outputTiming='$timingsfile_RS' ${BSGL_flags}"
echo "$cmdline"
if ! eval "$cmdline"; then
    echo "Error.. something failed when running '$gct_code' ..."
    exit 1
fi
topline=$(sort -nr -k7,7 $outfile_GCT_RS | head -1)
freqGCT_RS=$(echo $topline | awk '{print $1}')
resGCT_RS=$(echo $topline | awk '{print $7}')
resGCT_RS_H1=$(echo $topline | awk '{print $9}')
resGCT_RS_L1=$(echo $topline | awk '{print $10}')
resGCT_RSr=$(echo $topline  | awk '{print $11}')
resGCT_RSr_H1=$(echo $topline  | awk '{print $13}')
resGCT_RSr_L1=$(echo $topline  | awk '{print $14}')

echo
echo "----------------------------------------------------------------------------------------------------"
echo " STEP 4: run HierarchSearchGCT using LALDemod and default 2F toplist"
echo "----------------------------------------------------------------------------------------------------"
echo

rm -f checkpoint.cpt # delete checkpoint to start correctly
outfile_GCT_DM="./GCT_DM.dat"
timingsfile_DM="./timing_DM.dat"

cmdline="$gct_code $gct_CL_common --FstatMethod=DemodBest --fnameout='$outfile_GCT_DM' --outputTiming='$timingsfile_DM' ${BSGL_flags}"

echo $cmdline
if ! eval "$cmdline"; then
    echo "Error.. something failed when running '$gct_code' ..."
    exit 1
fi

topline=$(sort -nr -k7,7 $outfile_GCT_DM | head -1)
resGCT_DM=$(echo $topline  | awk '{print $7}')
resGCT_DM_H1=$(echo $topline  | awk '{print $9}')
resGCT_DM_L1=$(echo $topline  | awk '{print $10}')
freqGCT_DM=$(echo $topline | awk '{print $1}')
resGCT_DMr=$(echo $topline  | awk '{print $11}')
resGCT_DMr_H1=$(echo $topline  | awk '{print $13}')
resGCT_DMr_L1=$(echo $topline  | awk '{print $14}')

echo
echo "----------------------------------------------------------------------------------------------------"
echo " STEP 5: run HierarchSearchGCT using LALDemod and BSGL toplist"
echo "----------------------------------------------------------------------------------------------------"
echo

rm -f checkpoint.cpt # delete checkpoint to start correctly
outfile_GCT_DM_BSGL="./GCT_DM_BSGL.dat"
timingsfile_DM_BSGL="./timing_DM_BSGL.dat"

cmdline="$gct_code $gct_CL_common --FstatMethod=DemodBest ${BSGL_flags} --SortToplist=2 --fnameout='$outfile_GCT_DM_BSGL' --outputTiming='$timingsfile_DM_BSGL'"

echo $cmdline
if ! eval "$cmdline"; then
    echo "Error.. something failed when running '$gct_code' ..."
    exit 1
fi

topline=$(sort -nr -k7,7 $outfile_GCT_DM_BSGL | head -1)
resGCT_DM_BSGL=$(echo $topline  | awk '{print $7}')
resGCT_DM_H1_BSGL=$(echo $topline  | awk '{print $9}')
resGCT_DM_L1_BSGL=$(echo $topline  | awk '{print $10}')
freqGCT_DM_BSGL=$(echo $topline | awk '{print $1}')

echo
echo "----------------------------------------------------------------------------------------------------"
echo " STEP 6: run HierarchSearchGCT using LALDemod and dual toplist: 1st=F, 2nd=BSGL"
echo "----------------------------------------------------------------------------------------------------"
echo

rm -f checkpoint.cpt # delete checkpoint to start correctly
outfile_GCT_DM_DUAL="./GCT_DM_DUAL.dat"
timingsfile_DM_DUAL="./timing_DM_DUAL.dat"

cmdline="$gct_code $gct_CL_common --FstatMethod=DemodBest --SortToplist=3 ${BSGL_flags} --fnameout='$outfile_GCT_DM_DUAL' --outputTiming='$timingsfile_DM_DUAL'"

echo $cmdline
if ! eval "$cmdline"; then
    echo "Error.. something failed when running '$gct_code' ..."
    exit 1
fi

diff_F=`diff -I "[%][%].*" ${outfile_GCT_DM} ${outfile_GCT_DM_DUAL}`
diff_BSGL=`diff -I "[%][%].*" ${outfile_GCT_DM_BSGL} ${outfile_GCT_DM_DUAL}-BSGL`
if [ -n "${diff_F}" -o -n "${diff_BSGL}" ]; then
    echo "Error: 'dual' toplist handling seems to differ from indidual 'F' or 'BSGL'-sorted toplists"
    exit 1
fi

echo
echo "----------------------------------------------------------------------------------------------------"
echo " STEP 7: run HierarchSearchGCT using Resampling and triple toplists (BSGL, BSGLtL and BtSGLtL), "
echo "          (2F, BSGLtL and BtSGLtL), compared with separate runs for each of these 3 toplist rankings"
echo "----------------------------------------------------------------------------------------------------"
echo

rm -f checkpoint.cpt # delete checkpoint to start correctly
outfile_GCT_RS_triple="./GCT_RS_triple.dat"
timingsfile_RS_triple="./timing_RS_triple.dat"

outfile_GCT_RS_triple2="./GCT_RS_triple2.dat"
timingsfile_RS_triple2="./timing_RS_triple2.dat"

# set plan mode so that results should be deterministic, easier to compare results of different runs this way

export LAL_FSTAT_FFT_PLAN_MODE=ESTIMATE
export LAL_FSTAT_FFT_PLAN_TIMEOUT=30

cmdline="$gct_code $gct_CL_common --FstatMethod=ResampGeneric --fnameout='$outfile_GCT_RS_triple' --outputTiming='$timingsfile_RS_triple' ${BSGL_flags} --getMaxFperSeg --loudestSegOutput --SortToplist=6"

echo $cmdline
if ! eval "$cmdline"; then
    echo "Error.. something failed when running '$gct_code' ..."
    exit 1
fi

# second variant of triple toplists, with 2F sorted first toplist

cmdline="$gct_code $gct_CL_common --FstatMethod=ResampGeneric --fnameout='$outfile_GCT_RS_triple2' --outputTiming='$timingsfile_RS_triple2' ${BSGL_flags} --getMaxFperSeg --loudestSegOutput --SortToplist=7"

echo $cmdline
if ! eval "$cmdline"; then
    echo "Error.. something failed when running '$gct_code' ..."
    exit 1
fi

## re-run, but now create the in total four toplists one by one, then compare results

outfile_GCT_RS_triple="./GCT_RS_triple_0.dat"

cmdline="$gct_code $gct_CL_common --FstatMethod=ResampGeneric --fnameout='$outfile_GCT_RS_triple' ${BSGL_flags} --getMaxFperSeg --loudestSegOutput --SortToplist=0"

echo $cmdline
if ! eval "$cmdline"; then
    echo "Error.. something failed when running '$gct_code' ..."
    exit 1
fi

outfile_GCT_RS_triple="./GCT_RS_triple_1.dat"

cmdline="$gct_code $gct_CL_common --FstatMethod=ResampGeneric --fnameout='$outfile_GCT_RS_triple' ${BSGL_flags} --getMaxFperSeg --loudestSegOutput --SortToplist=2"

echo $cmdline
if ! eval "$cmdline"; then
    echo "Error.. something failed when running '$gct_code' ..."
    exit 1
fi

outfile_GCT_RS_triple="./GCT_RS_triple_2.dat"

cmdline="$gct_code $gct_CL_common --FstatMethod=ResampGeneric --fnameout='$outfile_GCT_RS_triple' ${BSGL_flags} --getMaxFperSeg --loudestSegOutput --SortToplist=4"

echo $cmdline
if ! eval "$cmdline"; then
    echo "Error.. something failed when running '$gct_code' ..."
    exit 1
fi

outfile_GCT_RS_triple="./GCT_RS_triple_3.dat"

cmdline="$gct_code $gct_CL_common --FstatMethod=ResampGeneric --fnameout='$outfile_GCT_RS_triple' ${BSGL_flags} --getMaxFperSeg --loudestSegOutput --SortToplist=5"

echo $cmdline
if ! eval "$cmdline"; then
    echo "Error.. something failed when running '$gct_code' ..."
    exit 1
fi

# filter out comments

egrep -v "^%" ./GCT_RS_triple_0.dat > ./GCT_RS_triple_0.txt
egrep -v "^%" ./GCT_RS_triple_1.dat > ./GCT_RS_triple_1.txt
egrep -v "^%" ./GCT_RS_triple_2.dat > ./GCT_RS_triple_2.txt
egrep -v "^%" ./GCT_RS_triple_3.dat > ./GCT_RS_triple_3.txt

egrep -v "^%" ./GCT_RS_triple.dat > ./GCT_RS_triple.txt
egrep -v "^%" ./GCT_RS_triple.dat-BSGLtL > ./GCT_RS_triple-BSGLtL.txt
egrep -v "^%" ./GCT_RS_triple.dat-BtSGLtL > ./GCT_RS_triple-BtSGLtL.txt

egrep -v "^%" ./GCT_RS_triple2.dat > ./GCT_RS_triple2.txt
egrep -v "^%" ./GCT_RS_triple2.dat-BSGLtL > ./GCT_RS_triple2-BSGLtL.txt
egrep -v "^%" ./GCT_RS_triple2.dat-BtSGLtL > ./GCT_RS_triple2-BtSGLtL.txt

if ! eval "diff ./GCT_RS_triple_1.txt ./GCT_RS_triple.txt"; then
    echo "Error: tripple toplists do not match separately generated toplists  (1) "
    exit 1
fi

if ! eval "diff ./GCT_RS_triple_2.txt ./GCT_RS_triple-BSGLtL.txt"; then
    echo "Error: tripple toplists do not match separately generated toplists  (2) "
    exit 1
fi

if ! eval "diff ./GCT_RS_triple_3.txt ./GCT_RS_triple-BtSGLtL.txt"; then
    echo "Error: tripple toplists do not match separately generated toplists  (3) "
    exit 1
fi

if ! eval "diff ./GCT_RS_triple_0.txt ./GCT_RS_triple2.txt"; then
    echo "Error: tripple toplists do not match separately generated toplists  (4) "
    exit 1
fi

if ! eval "diff ./GCT_RS_triple_2.txt ./GCT_RS_triple2-BSGLtL.txt"; then
    echo "Error: tripple toplists do not match separately generated toplists  (5) "
    exit 1
fi

if ! eval "diff ./GCT_RS_triple_3.txt ./GCT_RS_triple2-BtSGLtL.txt"; then
    echo "Error: tripple toplists do not match separately generated toplists  (6) "
    exit 1
fi

## ---------- compute relative differences and check against tolerance --------------------
awk_reldev='{printf "%.2e", sqrt(($1-$2)*($1-$2))/(0.5*($1+$2)) }'

freqreldev_RS=$(echo $Freq $freqGCT_RS | awk "$awk_reldev")
reldev_RS=$(echo $TwoFAvg $resGCT_RS | awk "$awk_reldev")
reldev_RS_H1=$(echo $TwoFAvg_H1 $resGCT_RS_H1 | awk "$awk_reldev")
reldev_RS_L1=$(echo $TwoFAvg_L1 $resGCT_RS_L1 | awk "$awk_reldev")

reldev_RSr=$(echo $TwoFAvg $resGCT_RSr | awk "$awk_reldev")
reldev_RSr_H1=$(echo $TwoFAvg_H1 $resGCT_RSr_H1 | awk "$awk_reldev")
reldev_RSr_L1=$(echo $TwoFAvg_L1 $resGCT_RSr_L1 | awk "$awk_reldev")

reldev_DM=$(echo $TwoFAvg $resGCT_DM | awk "$awk_reldev")
reldev_DM_H1=$(echo $TwoFAvg_H1 $resGCT_DM_H1 | awk "$awk_reldev")
reldev_DM_L1=$(echo $TwoFAvg_L1 $resGCT_DM_L1 | awk "$awk_reldev")

reldev_DMr=$(echo $TwoFAvg $resGCT_DMr | awk "$awk_reldev")
reldev_DMr_H1=$(echo $TwoFAvg_H1 $resGCT_DMr_H1 | awk "$awk_reldev")
reldev_DMr_L1=$(echo $TwoFAvg_L1 $resGCT_DMr_L1 | awk "$awk_reldev")

freqreldev_DM=$(echo $Freq $freqGCT_DM | awk "$awk_reldev")
reldev_DM_BSGL=$(echo $TwoFAvg $resGCT_DM_BSGL | awk "$awk_reldev")
reldev_DM_H1_BSGL=$(echo $TwoFAvg_H1 $resGCT_DM_H1_BSGL | awk "$awk_reldev")
reldev_DM_L1_BSGL=$(echo $TwoFAvg_L1 $resGCT_DM_L1_BSGL | awk "$awk_reldev")
freqreldev_DM_BSGL=$(echo $Freq $freqGCT_DM_BSGL | awk "$awk_reldev")

# ---------- Check relative deviations against tolerance, report results ----------
retstatus=0
awk_isgtr='{if($1>$2) {print "1"}}'

echo
echo "--------- Timings ------------------------------------------------------------------------------------------------"
echo "Timing model: tau = Nseg * Ndet * Ncoh * tau_Fstat + Nseg * Ninc * tau_SumF + Ninc * tau_Bayes + Ncand * tau_Recalc + time_Other"
echo
awk_timing='{printf "tau_Fstat = %-6.1e s, tau_SumF = %-6.1e s, tau_Bayes = %-6.1e s, tau_Recalc = %-6.1e s, time_Other = %-6.1e s", $1, $2, $3, $4, $5}'
timing_DM=$(sed '/^%.*/d' $timingsfile_DM | awk "$awk_timing")
timing_DM_BSGL=$(sed '/^%.*/d' $timingsfile_DM_BSGL | awk "$awk_timing")
timing_DM_DUAL=$(sed '/^%.*/d' $timingsfile_DM_DUAL | awk "$awk_timing")
timing_RS=$(sed '/^%.*/d' $timingsfile_RS | awk "$awk_timing")
timing_RS_triple=$(sed '/^%.*/d' $timingsfile_RS_triple | awk "$awk_timing")
echo " GCT-LALDemod:       $timing_DM"
echo " GCT-LALDemod-BSGL:  $timing_DM_BSGL"
echo " GCT-LALDemod-2top:  $timing_DM_DUAL"
echo " GCT-Resamp:         $timing_RS"
echo " GCT-Resamp-3top:    $timing_RS_triple"

echo
echo "--------- Compare results ----------------------------------------------------------------------------------------"
echo "                     	<2F_multi>	<2F_H1>  	<2F_L1>  	 @ Freq [Hz]     	(reldev, reldev_H1, reldev_L1, reldev_Freq)"
echo    "==>  CFSv2:         	$TwoFAvg 	$TwoFAvg_H1   	$TwoFAvg_L1   	 @ $Freq 	[Tolerance = ${Tolerance}]"

echo -n "==>  GCT-DM: 		$resGCT_DM	$resGCT_DM_H1	$resGCT_DM_L1  	 @ $freqGCT_DM 	($reldev_DM, $reldev_DM_H1, $reldev_DM_L1, $freqreldev_DM)"
fail1=$(echo $freqreldev_DM $Tolerance | awk "$awk_isgtr")
fail2=$(echo $reldev_DM $Tolerance     | awk "$awk_isgtr")
fail3=$(echo $reldev_DM_H1 $Tolerance  | awk "$awk_isgtr")
fail4=$(echo $reldev_DM_L1 $Tolerance  | awk "$awk_isgtr")
if [ "$fail1" -o "$fail2" -o "$fail3" -o "$fail4" ]; then
    echo " ==> *FAILED*"
    retstatus=1
else
    echo " ==> OK"
fi

echo -n "==>  GCT-DM-recalc:	$resGCT_DMr	$resGCT_DMr_H1	$resGCT_DMr_L1	 @ $freqGCT_DM	($reldev_DMr, $reldev_DMr_H1, $reldev_DMr_L1, $freqreldev_DM)"
fail2r=$(echo $reldev_DMr $Tolerance     | awk "$awk_isgtr")
fail3r=$(echo $reldev_DMr_H1 $Tolerance  | awk "$awk_isgtr")
fail4r=$(echo $reldev_DMr_L1 $Tolerance  | awk "$awk_isgtr")
if [ "$fail2r" -o "$fail3r" -o "$fail4r" ]; then
    echo " ==> *FAILED*"
    retstatus=1
else
    echo " ==> OK"
fi

echo -n "==>  GCT-LALDemodBSGL:	$resGCT_DM_BSGL 	$resGCT_DM_H1_BSGL	$resGCT_DM_L1_BSGL	 @ $freqGCT_DM_BSGL 	($reldev_DM_BSGL, $reldev_DM_H1_BSGL, $reldev_DM_L1_BSGL, $freqreldev_DM_BSGL)"
fail1=$(echo $freqreldev_DM_BSGL $Tolerance | awk "$awk_isgtr")
fail2=$(echo $reldev_DM_BSGL $Tolerance     | awk "$awk_isgtr")
fail3=$(echo $reldev_DM_H1_BSGL $Tolerance  | awk "$awk_isgtr")
fail4=$(echo $reldev_DM_L1_BSGL $Tolerance  | awk "$awk_isgtr")
if [ "$fail1" -o "$fail2" -o "$fail3" -o "$fail4" ]; then
    echo " ==> *FAILED*"
    retstatus=1
else
    echo " ==> OK"
fi

echo -n "==>  GCT-Resamp: 	$resGCT_RS 	$resGCT_RS_H1 	$resGCT_RS_L1  	 @ $freqGCT_RS 	($reldev_RS, $reldev_RS_H1, $reldev_RS_L1, $freqreldev_RS)"
fail1=$(echo $freqreldev_RS $Tolerance | awk "$awk_isgtr")
fail2=$(echo $reldev_RS     $Tolerance | awk "$awk_isgtr")
fail3=$(echo $reldev_RS_H1  $Tolerance | awk "$awk_isgtr")
fail4=$(echo $reldev_RS_L1  $Tolerance | awk "$awk_isgtr")
if [ "$fail1" -o "$fail2" -o "$fail3" -o "$fail4" ]; then
    echo " ==> *FAILED*"
    retstatus=1
else
    echo " ==> OK"
fi

echo -n "==>  GCT-RS-recalc: 	$resGCT_RSr 	$resGCT_RSr_H1 	$resGCT_RSr_L1  	 @ $freqGCT_RS 	($reldev_RSr, $reldev_RSr_H1, $reldev_RSr_L1, $freqreldev_RS)"
fail2r=$(echo $reldev_RSr     $Tolerance | awk "$awk_isgtr")
fail3r=$(echo $reldev_RSr_H1  $Tolerance | awk "$awk_isgtr")
fail4r=$(echo $reldev_RSr_L1  $Tolerance | awk "$awk_isgtr")
if [ "$fail2r" -o "$fail3r" -o "$fail4r" ]; then
    echo " ==> *FAILED*"
    retstatus=1
else
    echo " ==> OK"
fi

exit $retstatus
