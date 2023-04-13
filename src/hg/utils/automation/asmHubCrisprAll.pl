#!/usr/bin/env perl

use strict;
use warnings;
use FindBin qw($Bin);
use lib "$Bin";
use AsmHub;
use File::Basename;

my $argc = scalar(@ARGV);

if ($argc != 3) {
  printf STDERR "usage: asmHubCrisprAll.pl asmId asmId.names.tab .../trackData/\n";
  printf STDERR "where asmId is the assembly identifier,\n";
  printf STDERR "and .../trackData/ is the path to the /trackData/ directory.\n";
  exit 255;
}

# from Perl Cookbook Recipe 2.17, print out large numbers with comma
# delimiters:
sub commify($) {
    my $text = reverse $_[0];
    $text =~ s/(\d\d\d)(?=\d)(?!\d*\.)/$1,/g;
    return scalar reverse $text
}

my $asmId = shift;
my $namesFile = shift;
my $trackDataDir = shift;
my $crisprAllBbi = "$trackDataDir/crisprAll/crispr.bb";
my $track = "crisprAll";

if ( ! -s $crisprAllBbi ) {
  printf STDERR "ERROR: can not find trackData/crisprAll/crispr.bb file\n";
  exit 255;
}

my $totalBases = `ave -col=2 $trackDataDir/../${asmId}.chrom.sizes | grep "^total" | awk '{printf "%d", \$2}'`;
chomp $totalBases;
my $itemsBases = `bigBedInfo trackData/crisprAll/crispr.bb | egrep "itemCount|basesCovered" | awk '{print \$NF}' | sed -e 's/,//g;' | xargs echo`;
my ($itemCount, $basesCovered) = split('\s+', $itemsBases);

my $percentCoverage = sprintf("%.2f", 100.0 * $basesCovered / $totalBases);
$itemCount = commify($itemCount);
$basesCovered = commify($basesCovered);
$totalBases = commify($totalBases);

my $em = "<em>";
my $noEm = "</em>";
my $assemblyDate = `grep -v "^#" $namesFile | cut -f9`;
chomp $assemblyDate;
my $ncbiAssemblyId = `grep -v "^#" $namesFile | cut -f10`;
chomp $ncbiAssemblyId;
my $organism = `grep -v "^#" $namesFile | cut -f5`;
chomp $organism;

print <<_EOF_

<h2>Description</h2>

<p>
This track shows the DNA sequences targetable by CRISPR RNA guides using
the Cas9 enzyme from <em>S. pyogenes</em> (PAM: NGG) over the entire
$organism ($asmId) genome.  CRISPR target sites were annotated with
predicted specificity (off-target effects) and predicted efficiency
(on-target cleavage) by various
algorithms through the tool <a href="http://crispor.tefor.net/"
target="_blank">CRISPOR</a>. Sp-Cas9 usually cuts double-stranded DNA three or 
four base pairs 5' of the PAM site.
</p>

<p>
This track has $itemCount items in the track, covering $basesCovered bases
in the sequence which is % $percentCoverage of the total sequence of size
$totalBases nucleotides.
</p>

<h2>Display Conventions and Configuration</h2>

<p>
The track &quot;CRISPR Targets&quot; shows all potential -NGG target sites across the genome.
The target sequence of the guide is shown with a thick (exon) bar. The PAM
motif match (NGG) is shown with a thinner bar. Guides
are colored to reflect both predicted specificity and efficiency. Specificity
reflects the &quot;uniqueness&quot; of a 20mer sequence in the genome; the less unique a
sequence is, the more likely it is to cleave other locations of the genome
(off-target effects). Efficiency is the frequency of cleavage at the target
site (on-target efficiency).</p>

<p>Shades of gray stand for sites that are hard to target specifically, as the
20mer is not very unique in the genome:</p>
<table class="stdTbl" style="width:100%">
<tr><td style="width:50px; background-color:#969696"></td><td>impossible to target: target site has at least one identical copy in the genome and was not scored</td></tr>
<tr><td style="width:50px; background-color:#787878"></td><td>hard to target: many similar sequences in the genome that alignment stopped, repeat?</td></tr>
<tr><td style="width:50px; background-color:#505050"></td><td>hard to target: target site was aligned but results in a low specificity score &lt;= 50 (see below)</td></tr>
</table>

<p>Colors highlight targets that are specific in the genome (MIT specificity &gt; 50) but have different predicted efficiencies:</p>
<table class="stdTbl" style="width:100%">
<tr><td style="width:50px; background-color:#000064"></td><td>unable to calculate Doench/Fusi 2016 efficiency score</td></tr>
<tr><td style="width:50px; background-color:#FF7070"></td><td>low predicted cleavage: Doench/Fusi 2016 Efficiency percentile &lt;= 30</td></tr>
<tr><td style="width:50px; background-color:#FFFF00"></td><td>medium predicted cleavage: Doench/Fusi 2016 Efficiency percentile &gt; 30 and &lt; 55</td></tr>
<tr><td style="width:50px; background-color:#00b300"></td><td>high predicted cleavage: Doench/Fusi 2016 Efficiency &gt; 55</td></tr>
</table><BR>

<p>
Mouse-over a target site to show predicted specificity and efficiency scores:<br>
<ol>
<li>The MIT Specificity score summarizes all off-targets into a single number from
0-100. The higher the number, the fewer off-target effects are expected. We
recommend guides with an MIT specificity &gt; 50.</li>
<li>The efficiency score tries to predict if a guide leads to rather strong or
weak cleavage. According to <a href="#References">(Haeussler et al. 2016)</a>, the <a
href="https://portals.broadinstitute.org/gpp/public/analysis-tools/sgrna-design">Doench
2016 Efficiency score</a> should be used to select the guide with the highest
cleavage efficiency when expressing guides from RNA PolIII Promoters such as
U6. Scores are given as percentiles, e.g. &quot;70%&quot; means that 70% of mammalian
guides have a score equal or lower than this guide. The raw score number is
also shown in parentheses after the percentile.</li>
<li>The <a
href="https://www.crisprscan.org/">Moreno-Mateos 2015 Efficiency
score</a> should be used instead of the Doench 2016 score when transcribing the
guide in vitro with a T7 promoter, e.g. for injections in mouse, zebrafish or
Xenopus embryos. The Moreno-Mateos score is given in percentiles and the raw value in parentheses, see the note above.</li> </ol>
</p>

<p>Click onto features to show all scores and predicted off-targets with up to
four mismatches. The Out-of-Frame score by <a href="#References">Bae et al. 2014</a>
is correlated with
the probability that mutations induced by the guide RNA will disrupt the open
reading frame. The authors recommend out-of-frame scores &gt; 66 to create
knock-outs with a single guide efficiently.<p>

<p>Off-target sites are sorted by the CFD (Cutting Frequency Determination)
score (<a href="https://www.nature.com/articles/nbt.3437"
target="_blank">Doench et al. 2016</a>).
The higher the CFD score, the more likely there is off-target cleavage at that site.
Off-targets with a CFD score &lt; 0.023 are not shown on this page, but are available when
following the link to the external CRISPOR tool.
When compared against experimentally validated off-targets by
<a href="#References">Haeussler et al. 2016</a>, the large majority of predicted
off-targets with CFD scores &lt; 0.023 were false-positives. For storage and performance
reasons, on the level of individual off-targets, only CFD scores are available.</p>

<h2>Methods</h2>

<h3>Relationship between predictions and experimental data</h3>

<p>
Like most algorithms, the MIT specificity score is not always a perfect
predictor of off-target effects. Despite low scores, many tested guides
caused few and/or weak off-target cleavage when tested with whole-genome assays
(Figure 2 from <a href="#References">Haeussler
et al. 2016</a>), as shown below, and the published data contains few data points
with high specificity scores. Overall though, the assays showed that the higher
the specificity score, the lower the off-target effects.</p>

<img src="../images/crisprFig_mitScore.png">

<p>Similarly, efficiency scoring is not very accurate: guides with low
scores can be efficient and vice versa. As a general rule, however, the higher
the score, the less likely that a guide is very inefficient. The
following histograms illustrate, for each type of score, how the share of
inefficient guides drops with increasing efficiency scores:
</p>

<img src="../images/crisprFig_effScores.png">

<p>When reading this plot, keep in mind that both scores were evaluated on
their own training data. Especially for the Moreno-Mateos score, the
results are too optimistic, due to overfitting. When evaluated on independent
datasets, the correlation of the prediction with other assays was around 25%
lower, see <a href="#References">Haeussler et al. 2016</a>. At the time of
writing, there is no independent dataset available yet to determine the
Moreno-Mateos accuracy for each score percentile range.</p>

<h3>Track methods</h3>
<p>
The entire $organism ($asmId) genome was scanned for the -NGG motif. Flanking 20mer
guide sequences were
aligned to the genome with BWA and scored with MIT Specificity scores using the
command-line version of crispor.org.  Non-unique guide sequences were skipped.
Flanking sequences were extracted from the genome and input for Crispor
efficiency scoring, available from the <a
href="http://crispor.tefor.net/downloads/">Crispor downloads page</a>, which
includes the Doench 2016, Moreno-Mateos 2015 and Bae
2014 algorithms, among others.</p>
<p>
Note that the Doench 2016 scores were updated by
the Broad institute in 2017 ("Azimuth" update). As a result, earlier versions of
the track show the old Doench 2016 scores and this version of the track shows new
Doench 2016 scores. Old and new scores are almost identical, they are
correlated to 0.99 and for more than 80% of the guides the difference is below 0.02.
However, for very few guides, the difference can be bigger. In case of doubt, we recommend
the new scores. Crispor.org can display both scores and many more with the
"Show all scores" link.</p>

<H2>Data Access</H2>
<p>
Positional data can be explored interactively with the 
<a href="../cgi-bin/hgTables?db=${asmId}&hgta_track=crisprAllTargets&hgta_regionType=range">Table Browser</a>.
For small programmatic positional queries, the track can be accessed using our 
<a href="/goldenPath/help/api.html">REST API</a>. For genome-wide data or 
automated analysis, CRISPR genome annotations can be downloaded from
<a href="http://hgdownload.soe.ucsc.edu/gbdb/$asmId/crisprAll/" target="_blank">our download server</a>
as a bigBedFile.</p>
<p>
The files for this track are called <tt>crispr.bb</tt>, which lists positions and
scores, and <tt>crisprDetails.tab</tt>, which has information about off-target matches. Individual
regions or whole genome annotations can be obtained using our tool <tt>bigBedToBed</tt>,
which can be compiled from the source code or downloaded as a pre-compiled
binary for your system. Instructions for downloading source code and binaries can be found
<a href="http://hgdownload.soe.ucsc.edu/downloads.html#utilities_downloads">here</a>. The tool
can also be used to obtain only features within a given range, e.g.</p>
<p>
<tt>bigBedToBed
http://hgdownload.soe.ucsc.edu/gbdb/${asmId}/${track}/crispr.bb -chrom=chr21
-start=0 -end=1000000 stdout</tt> </p>

<h2>Credits</h2>

<p>
Track created by Maximilian Haeussler, with helpful input
from Jean-Paul Concordet (MNHN Paris) and Alberto Stolfi (NYU).
</p>
<a name="References"></a>
<h2>References</h2>

<p>
Haeussler M, Sch&#246;nig K, Eckert H, Eschstruth A, Miann&#233; J, Renaud JB, Schneider-Maunoury S,
Shkumatava A, Teboul L, Kent J <em>et al</em>.
<a href="https://genomebiology.biomedcentral.com/articles/10.1186/s13059-016-1012-2"
target="_blank">Evaluation of off-target and on-target scoring algorithms and integration into the
guide RNA selection tool CRISPOR</a>.
<em>Genome Biol</em>. 2016 Jul 5;17(1):148.
PMID: <a href="https://www.ncbi.nlm.nih.gov/pubmed/27380939" target="_blank">27380939</a>; PMC: <a
href="https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4934014/" target="_blank">PMC4934014</a>
</p>

<p>
Bae S, Kweon J, Kim HS, Kim JS.
<a href="https://www.nature.com/nmeth/journal/v11/n7/full/nmeth.3015.html" target="_blank">
Microhomology-based choice of Cas9 nuclease target sites</a>.
<em>Nat Methods</em>. 2014 Jul;11(7):705-6.
PMID: <a href="https://www.ncbi.nlm.nih.gov/pubmed/24972169" target="_blank">24972169</a>
</p>

<p>
Doench JG, Fusi N, Sullender M, Hegde M, Vaimberg EW, Donovan KF, Smith I, Tothova Z, Wilen C,
Orchard R <em>et al</em>.
<a href="https://www.nature.com/articles/nbt.3437" target="_blank">
Optimized sgRNA design to maximize activity and minimize off-target effects of CRISPR-Cas9</a>.
<em>Nat Biotechnol</em>. 2016 Feb;34(2):184-91.
PMID: <a href="https://www.ncbi.nlm.nih.gov/pubmed/26780180" target="_blank">26780180</a>; PMC: <a
href="https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4744125/" target="_blank">PMC4744125</a>
</p>

<p>
Hsu PD, Scott DA, Weinstein JA, Ran FA, Konermann S, Agarwala V, Li Y, Fine EJ, Wu X, Shalem O
<em>et al</em>.
<a href="https://www.nature.com/nbt/journal/v31/n9/full/nbt.2647.html" target="_blank">
DNA targeting specificity of RNA-guided Cas9 nucleases</a>.
<em>Nat Biotechnol</em>. 2013 Sep;31(9):827-32.
PMID: <a href="https://www.ncbi.nlm.nih.gov/pubmed/23873081" target="_blank">23873081</a>; PMC: <a
href="https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3969858/" target="_blank">PMC3969858</a>
</p>

<p>
Moreno-Mateos MA, Vejnar CE, Beaudoin JD, Fernandez JP, Mis EK, Khokha MK, Giraldez AJ.
<a href="https://www.nature.com/nmeth/journal/v12/n10/full/nmeth.3543.html" target="_blank">
CRISPRscan: designing highly efficient sgRNAs for CRISPR-Cas9 targeting in vivo</a>.
<em>Nat Methods</em>. 2015 Oct;12(10):982-8.
PMID: <a href="https://www.ncbi.nlm.nih.gov/pubmed/26322839" target="_blank">26322839</a>; PMC: <a
href="https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4589495/" target="_blank">PMC4589495</a>
</p>

_EOF_
   ;
