#!/usr/bin/env perl

# DO NOT EDIT the /cluster/bin/scripts copy of this file --
# edit ~/kent/src/hg/utils/automation/doCpgIslands.pl instead.

use Getopt::Long;
use warnings;
use strict;
use FindBin qw($Bin);
use lib "$Bin";
use HgAutomate;
use HgRemoteScript;
use HgStepManager;

# Option variable names, both common and peculiar to this script:
use vars @HgAutomate::commonOptionVars;
use vars @HgStepManager::optionVars;
use vars qw/
    $opt_buildDir
    $opt_maskedSeq
    $opt_chromSizes
    $opt_tableName
    /;

# Specify the steps supported with -continue / -stop:
my $stepper = new HgStepManager(
    [ { name => 'hardMask',   func => \&doHardMask },
      { name => 'cpg', func => \&doCpg },
      { name => 'makeBed', func => \&doMakeBed },
      { name => 'load', func => \&doLoadCpg },
      { name => 'cleanup', func => \&doCleanup },
    ]
				);

my $cpgLh = "/hive/data/staging/data/cpgIslandExt/cpglh";

# Option defaults:
# my $bigClusterHub = 'swarm';
my $bigClusterHub = 'ku';
# my $smallClusterHub = 'encodek';
my $smallClusterHub = 'ku';
my $workhorse = 'hgwdev';
my $dbHost = 'hgwdev';
my $defaultWorkhorse = 'hgwdev';
my $maskedSeq = "$HgAutomate::clusterData/\$db/\$db.2bit";
my $chromSizes = "$HgAutomate::clusterData/\$db/chrom.sizes";
my $maxSplitSize = 0;	# will be set to maxSeqSize or 1,000,000,000 when split
my $splitRun = 0;	# normally not, will become true if total genome
                        # size if more than 4Gb or if single largest sequence
                        # is more than 1Gb

my $base = $0;
$base =~ s/^(.*\/)?//;

sub usage {
  # Usage / help / self-documentation:
  my ($status, $detailed) = @_;
  # Basic help (for incorrect usage):
  print STDERR "
usage: $base db
options:
";
  print STDERR $stepper->getOptionHelp();
  print STDERR <<_EOF_
    -buildDir dir         Use dir instead of default
                          $HgAutomate::clusterData/\$db/$HgAutomate::trackBuild/cpgIslands
                          (necessary when continuing at a later date).
    -maskedSeq seq.2bit   Use seq.2bit as the masked input sequence instead
                          of default ($maskedSeq).
    -chromSizes chrom.sizes   Use chrom.sizes instead of
                          default ($chromSizes).
    -tableName name       Load table 'name' instead of default cpgIslandExt
                          e.g.: -tableName cpgIslandExtUnmasked
_EOF_
  ;
  print STDERR &HgAutomate::getCommonOptionHelp('dbHost' => $dbHost,
                                'bigClusterHub' => $bigClusterHub,
                                'smallClusterHub' => $smallClusterHub,
                                'workhorse' => $defaultWorkhorse);
  print STDERR "
Automates UCSC's CpG Island finder for genome database \$db.  Steps:
    hardMask:  Creates hard-masked fastas needed for the CpG Island program.
    cpg:       Run /hive/data/staging/data/cpgIslandExt/cpglh on the hard-masked fastas
    makeBed:   Transform output from cpglh into cpgIsland.bed
    load:      Load cpgIsland.bed into \$db.
    cleanup:   Removes hard-masked fastas and output from cpglh.
All operations are performed in the build directory which is
$HgAutomate::clusterData/\$db/$HgAutomate::trackBuild/cpgIslands unless -buildDir is given.
";
  # Detailed help (-help):
  print STDERR "
Assumptions:
1. $HgAutomate::clusterData/\$db/\$db.2bit contains RepeatMasked sequence for
   database/assembly \$db.
" if ($detailed);
  print "\n";
  exit $status;
}


# Globals:
# Command line args: db
my ($db);
# Other:
my ($buildDir, $tableName, $secondsStart, $secondsEnd);

sub checkOptions {
  # Make sure command line options are valid/supported.
  my $ok = GetOptions(@HgStepManager::optionSpec,
		      'buildDir=s',
		      'maskedSeq=s',
		      'chromSizes=s',
		      'tableName=s',
		      @HgAutomate::commonOptionSpec,
		      );
  &usage(1) if (!$ok);
  &usage(0, 1) if ($opt_help);
  &HgAutomate::processCommonOptions();
  my $err = $stepper->processOptions();
  usage(1) if ($err);
  $workhorse = $opt_workhorse if ($opt_workhorse);
  $bigClusterHub = $opt_bigClusterHub if ($opt_bigClusterHub);
  $smallClusterHub = $opt_smallClusterHub if ($opt_smallClusterHub);
  $dbHost = $opt_dbHost if ($opt_dbHost);
}

#########################################################################
# * step: hard mask [workhorse]
#  the hard masking is always going to happen in the doCpg() step during
#  setup.  If the incoming sequence is not masked at all, there will be
#  no hard masking.  If it is masked, it will be hard masked.
sub doHardMask {
  printf STDERR "# doHardMask: obsolete step, no longer needed\n";
  return 0;
} # doHardMask

#########################################################################
# * step: cpg [workhorse]
sub doCpg {
  # Set up and perform the cluster run to run the CpG function on the
  #     hard masked sequence.
  my $paraHub = $bigClusterHub;
  my $runDir = $buildDir;
  # Second, make sure we're starting clean.
  if (-e "$runDir/cpglh.result") {
    die "doCpg: looks like this was run successfully already " .
      "(cpglh.result exists).  Either run with -continue makeBed or some later " .
	"stage, or move aside/remove $runDir/ and run again.\n";
  }
  &HgAutomate::mustMkdir($runDir);
  my $bossScript;

  if ($splitRun) {
    my $templateCmd = ("./runCpg.bash " . '$(root1) '
                . '{check out exists results/$(root1).cpg}');
    &HgAutomate::makeGsub($runDir, $templateCmd);
    `touch "$runDir/para_hub_$paraHub"`;

    my $fh = &HgAutomate::mustOpen(">$runDir/runCpg.bash");
    print $fh <<_EOF_
#!/bin/bash
set -beEu -o pipefail
export partName=\$1
export part2bit=partFa/\$partName.2bit
export result=\$2
twoBitToFa \$part2bit stdout | /hive/data/staging/data/cpgIslandExt/cpglh /dev/stdin > \$result
_EOF_
    ;
    close($fh);

    $fh = &HgAutomate::mustOpen(">$runDir/oneSplit.bash");
    print $fh <<_EOF_
#!/bin/bash
set -beEu -o pipefail
if [ -d "/data/tmp" ]; then
  export TMPDIR="/data/tmp"
elif [ -d "/scratch/tmp" ]; then
  export TMPDIR="/scratch/tmp"
else
  export TMPDIR="/tmp"
fi

export tmpFile=`mktemp -p \$TMPDIR doCpg.\$\$.XXXXX`
export chromSizes=$chromSizes
export fileSpec="\${1}"
export file=`echo \$fileSpec | cut -d':' -f1`
export seq=`echo \$fileSpec | cut -d':' -f2`
export range=`echo \$fileSpec | cut -d':' -f3`
export start=`echo \$range | cut -d'-' -f1`
export end=`echo \$range | cut -d'-' -f2`
export seqSize=`grep -w "\${seq}" \$chromSizes | awk '{print \$2}'`

twoBitToFa \$fileSpec stdout | maskOutFa stdin hard stdout | /hive/data/staging/data/cpgIslandExt/cpglh \\
   /dev/stdin | sed -e "s/\\t /\\t/g;" > "\${tmpFile}"
printf "%d\\t%s:%d-%d\\t%d\\t%s\\t%d\\n" "\${start}" "\${seq}"  "\${start}" "\${end}" "\${seqSize}" "\${seq}" "\${seqSize}" \\
  | liftUp -type=.bed results/\${seq}:\${start}-\${end}.cpg stdin error "\${tmpFile}"

rm -f "\${tmpFile}"
_EOF_
    ;
    close($fh);

  my $whatItDoes = "Run /hive/data/staging/data/cpgIslandExt/cpglh on masked sequence.";
  $bossScript = newBash HgRemoteScript("$runDir/doCpg.bash", $workhorse,
				      $runDir, $whatItDoes);
  my $paraRun = &HgAutomate::paraRun();
  my $gensub2 = &HgAutomate::gensub2();

  $bossScript->add(<<_EOF_
export twoBit=\"$maskedSeq\"
rm -fr parts partFa
mkdir partFa
/cluster/bin/scripts/partitionSequence.pl -lstDir parts $maxSplitSize 0 \$twoBit $chromSizes 1000 > part.list
for L in parts/part*.lst
do
  B=`basename \$L | sed -e 's/.lst//;'`
  sed -e 's/.*.2bit://; s/:0-.*//;' \${L} > \${B}.list
  twoBitToFa -seqList=\$B.list \${twoBit} stdout | maskOutFa stdin hard stdout \\
     | faToTwoBit stdin partFa/\$B.t.2bit
  rm -f \${B}.list
  twoBitToFa partFa/\$B.t.2bit stdout | faCount stdin | egrep -v \"^total|^#seq\" | awk '\$2-\$7 > 200 { printf \"%s\\n\", \$1}' > \$B.list
  if [ -s \$B.list ]; then
    twoBitToFa -seqList=\$B.list partFa/\$B.t.2bit stdout | faToTwoBit stdin partFa/\$B.2bit
  fi
  rm -f partFa/\$B.t.2bit \$B.list
done
mkdir -p results
chmod a+x runCpg.bash oneSplit.bash
(grep -v "parts/part" part.list || true) | xargs -L 1 --no-run-if-empty ./oneSplit.bash
rm -f file.list
find ./partFa -type f > file.list
$gensub2 file.list single gsub jobList
$paraRun
catDir -r results | sort -k1,1 -k2,2n > cpglh.result
_EOF_
  );
  } else {

  my $whatItDoes = "Run /hive/data/staging/data/cpgIslandExt/cpglh on masked sequence.";
  $bossScript = newBash HgRemoteScript("$runDir/doCpg.bash", $workhorse,
				      $runDir, $whatItDoes);
  $bossScript->add(<<_EOF_
export twoBit=\"$maskedSeq\"
twoBitToFa \$twoBit stdout | maskOutFa stdin hard stdout \\
   | /hive/data/staging/data/cpgIslandExt/cpglh /dev/stdin 2> cpglh.stderr \\
     > cpglh.result
_EOF_
  );
  }

  $bossScript->execute();
} # doCpg

#########################################################################
# * step: make bed [workhorse]
sub doMakeBed {
  my $runDir = $buildDir;
  &HgAutomate::mustMkdir($runDir);

  # First, make sure we're starting clean.
  if (-e "$runDir/cpgIsland.bed") {
    die "doMakeBed: looks like this was run successfully already " .
      "(cpgIsland.bed exists).  Either run with -continue load or cleanup " .
	"or move aside/remove $runDir/cpgIsland.bed and run again.\n";
  }
  if (! -e "$runDir/cpglh.result") {
    die "doMakeBed: previous step doCpg has not completed, cpglh.result not found\n";
  }

  my $whatItDoes = "Makes bed from cpglh output.";
  my $bossScript = newBash HgRemoteScript("$runDir/doMakeBed.bash", $workhorse,
				      $runDir, $whatItDoes);

  $bossScript->add(<<_EOF_
awk \'\{\$2 = \$2 - 1; width = \$3 - \$2;  printf\(\"\%s\\t\%d\\t\%s\\t\%s \%s\\t\%s\\t\%s\\t\%0.0f\\t\%0.1f\\t\%s\\t\%s\\n\", \$1, \$2, \$3, \$5, \$6, width, \$6, width\*\$7\*0.01, 100.0\*2\*\$6\/width, \$7, \$9\);}\' cpglh.result \\
     | sort -k1,1 -k2,2n > cpgIsland.bed
bedToBigBed -tab -type=bed4+6 -as=\$HOME/kent/src/hg/lib/cpgIslandExt.as \\
  cpgIsland.bed $chromSizes $db.$tableName.bb
_EOF_
  );
  $bossScript->execute();
} # doMakeBed

#########################################################################
# * step: load [dbHost]
sub doLoadCpg {
  my $runDir = $buildDir;
  &HgAutomate::mustMkdir($runDir);

  my $whatItDoes = "Loads cpgIsland.bed.";
  my $bossScript = newBash HgRemoteScript("$runDir/doLoadCpg.bash", $dbHost,
				      $runDir, $whatItDoes);

  if (! -e "$runDir/cpgIsland.bed") {
    die "doLoadCpg previous step doMakeBed has not completed, cpgIsland.bed not found\n";
  }

  $bossScript->add(<<_EOF_
export C=`cut -f1 cpgIsland.bed | sort -u | awk '{print length(\$0)}' | sort -rn | sed -n -e '1,1 p'`
sed -e "s/14/\${C}/" \$HOME/kent/src/hg/lib/cpgIslandExt.sql > cpgIslandExt.sql
hgLoadBed -noLoad -sqlTable=cpgIslandExt.sql -tab $db $tableName cpgIsland.bed
hgLoadSqlTab $db $tableName cpgIslandExt.sql bed.tab
checkTableCoords -verboseBlocks -table=$tableName $db
featureBits $db $tableName >&fb.$db.$tableName.txt
_EOF_
  );
  $bossScript->execute();
} # doLoad

#########################################################################
# * step: cleanup [fileServer]
sub doCleanup {
  my $runDir = $buildDir;
  my $whatItDoes = "It cleans up or compresses intermediate files.";
  my $fileServer = &HgAutomate::chooseFileServer($runDir);
  my $bossScript = newBash HgRemoteScript("$runDir/doCleanup.bash", $fileServer,
				      $runDir, $whatItDoes);
  $bossScript->add(<<_EOF_
rm -f bed.tab cpgIslandExt.sql
gzip cpgIsland.bed
_EOF_
  );
  $bossScript->execute();
} # doCleanup

#########################################################################
# main

# Prevent "Suspended (tty input)" hanging:
&HgAutomate::closeStdin();

# Make sure we have valid options and exactly 1 argument:
&checkOptions();
&usage(1) if (scalar(@ARGV) != 1);
$secondsStart = `date "+%s"`;
chomp $secondsStart;
($db) = @ARGV;

# Force debug and verbose until this is looking pretty solid:
#$opt_debug = 1;
#$opt_verbose = 3 if ($opt_verbose < 3);

# Establish what directory we will work in.
$buildDir = $opt_buildDir ? $opt_buildDir :
  "$HgAutomate::clusterData/$db/$HgAutomate::trackBuild/cpgIslands";
$maskedSeq = $opt_maskedSeq ? $opt_maskedSeq :
  "$HgAutomate::clusterData/$db/$db.2bit";
$chromSizes = $opt_chromSizes ? $opt_chromSizes :
  "$HgAutomate::clusterData/$db/chrom.sizes";
$tableName = $opt_tableName ? $opt_tableName : "cpgIslandExt";

my $maxSeqSize=`sort -k2,2nr $chromSizes | head -1 | awk '{printf "%d", \$NF}'`;
my $totalSeqSize=`ave -col=2 $chromSizes | grep -w total | awk '{printf "%d", \$NF}'`;
chomp $maxSeqSize;
chomp $totalSeqSize;
$maxSplitSize = $maxSeqSize;
$splitRun = 0;
#   big genomes are over 4Gb: 4*1024*1024*1024 = 4294967296
#   or if maxSeqSize over 1Gb
if ( ($totalSeqSize > 4*1024**3) || ($maxSeqSize > 1024**3) ) {
   $splitRun = 1;
   $maxSplitSize = 1000000000;
}
printf STDERR "# total genome size %d, max sequence size: %d, splitRun: %s\n", $totalSeqSize, $maxSeqSize, $splitRun ? "TRUE" : "FALSE";

# Do everything.
$stepper->execute();

# Tell the user anything they should know.
my $stopStep = $stepper->getStopStep();
my $upThrough = ($stopStep eq 'cleanup') ? "" :
  "  (through the '$stopStep' step)";

$secondsEnd = `date "+%s"`;
chomp $secondsEnd;
my $elapsedSeconds = $secondsEnd - $secondsStart;
my $elapsedMinutes = int($elapsedSeconds/60);
$elapsedSeconds -= $elapsedMinutes * 60;

&HgAutomate::verbose(1,
	"\n *** All done !  Elapsed time: ${elapsedMinutes}m${elapsedSeconds}s\n");
&HgAutomate::verbose(1,
	"\n *** All done!$upThrough\n");
&HgAutomate::verbose(1,
	" *** Steps were performed in $buildDir\n");
&HgAutomate::verbose(1, "\n");

