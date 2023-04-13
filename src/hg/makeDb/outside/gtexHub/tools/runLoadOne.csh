#!/bin/csh -ef
# Read coverage of GTEx V6 RNA-seq, generated by STAR2 aligned to hg38, with GENCODE v23 
# From John Vivian, CGL group (Benedict Paten lead)
# This is second step in 'runOne.csh' script.  Files are already downloaded,
# in the directory ../in/run/SRR<id>.tar.gz.

# for kolossus (or hgwdev)
set id = $1
set run = $2

if ($id == "" || $run == "") then
    echo "usage: runLoadOne.csh sampleId runId"
    exit 1
endif
set sigdir = /hive/data/outside/gtex/signal
set indir = $sigdir/in/$run
set outdir = $sigdir/out/$run
mkdir -p $outdir $outdir/hg19 $outdir/hg38 $outdir/unmapped

set tmpdir = /dev/shm/gtex
set tmpfile = $tmpdir/$id.hg38.bedGraph

echo -n "$id\t"

# extract bedgraph file (unique+multi signal) from pre-loaded tarball
set file = $indir/$id.tar.gz
tar xfzO $file $id.rnaSignal.UniqueMultiple.str1.out.bedGraph | \
            grep -v chrEBV > $tmpfile

if ($status != 0) then
    echo ERROR
    exit 1
endif
set size = `ls -s $tmpfile | awk '{print $1}'`
echo -n "$size\t$tmpfile:t\t"

# convert hg38 bedgraph to bigwig format
set hg38 = /hive/data/genomes/hg38/chrom.sizes
set outfile = $outdir/hg38/$id.hg38.bigWig
bedGraphToBigWig $tmpfile $hg38 $outfile

if ($status != 0) then
    echo ERROR
    exit 2
endif
set size = `ls -s $outfile | awk '{print $1}'`
echo -n "$size\t$outfile:t\t"

# convert genomic coordinates from hg38 to hg19
set hg19 = /hive/data/genomes/hg19/chrom.sizes
set chain = /hive/data/genomes/hg38/bed/liftOver/hg38ToHg19.over.chain.gz
set tmpfile2 = $tmpdir/$id.hg19.bedGraph
set mapfile = $outdir/unmapped/$id.unmapped
liftOver $tmpfile $chain stdout $mapfile -verbose=0 | \
    bedSort stdin stdout | \
        bedRemoveOverlap stdin $tmpfile2

if ($status != 0) then
    echo ERROR
    exit 1
endif
set size = `ls -s $tmpfile2 | awk '{print $1}'`
set unmapped = `grep chr $mapfile | wc -l`
echo -n "$size\t$tmpfile2:t\t$unmapped\t"

# convert hg19 bedgraph to bigwig format
set sizes = /hive/data/genomes/hg19/chrom.sizes
set outfile = $outdir/hg19/$id.hg19.bigWig
bedGraphToBigWig $tmpfile2 $hg19 $outfile

if ($status != 0) then
    echo ERROR
    exit 1
endif
set size = `ls -s $outfile | awk '{print $1}'`
echo -n "$size\t$outfile:t"

echo
rm -f $tmpfile $tmpfile2


