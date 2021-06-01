/* gfClient - A client for the genomic finding program that produces a .psl file. */
/* Copyright 2001-2003 Jim Kent.  All rights reserved. */
#include "common.h"
#include "linefile.h"
#include "aliType.h"
#include "fa.h"
#include "genoFind.h"
#include "psl.h"
#include "options.h"
#include "fuzzyFind.h"
#include "errCatch.h"
#include "portable.h"

static struct optionSpec optionSpecs[] = {
    {"prot", OPTION_BOOLEAN},
    {"q", OPTION_STRING},
    {"t", OPTION_STRING},
    {"minIdentity", OPTION_FLOAT},
    {"minScore", OPTION_INT},
    {"dots", OPTION_INT},
    {"out", OPTION_STRING},
    {"maxIntron", OPTION_INT},
    {"nohead", OPTION_BOOLEAN},
    {"genome", OPTION_STRING},
    {"genomeDataDir", OPTION_STRING},
    {NULL, 0}
};

/* Variables that can be overridden by command line. */
int dots = 0;
int minScore = 30;
double minIdentity = 90;
char *outputFormat = "psl";
char *qType = "dna";
char *tType = "dna";
char *genome = NULL;
char *genomeDataDir = NULL;
boolean isDynamic = FALSE;
long enterMainTime = 0;

void usage()
/* Explain usage and exit. */
{
printf(
  "gfClient v. %s - A client for the genomic finding program that produces a .psl file\n"
  "usage:\n"
  "   gfClient host port seqDir in.fa out.psl\n"
  "where\n"
  "   host is the name of the machine running the gfServer\n"
  "   port is the same port that you started the gfServer with\n"
  "   seqDir is the path of the .2bit or .nib files relative to the current dir\n"
  "       (note these are needed by the client as well as the server)\n"
  "   in.fa is a fasta format file.  May contain multiple records\n"
  "   out.psl is where to put the output\n"
  "options:\n"
  "   -t=type       Database type. Type is one of:\n"
  "                   dna - DNA sequence\n"
  "                   prot - protein sequence\n"
  "                   dnax - DNA sequence translated in six frames to protein\n"
  "                 The default is dna.\n"
  "   -q=type       Query type. Type is one of:\n"
  "                   dna - DNA sequence\n"
  "                   rna - RNA sequence\n"
  "                   prot - protein sequence\n"
  "                   dnax - DNA sequence translated in six frames to protein\n"
  "                   rnax - DNA sequence translated in three frames to protein\n"
  "   -prot         Synonymous with -t=prot -q=prot.\n"
  "   -dots=N       Output a dot every N query sequences.\n"
  "   -nohead       Suppresses 5-line psl header.\n"
  "   -minScore=N   Sets minimum score.  This is twice the matches minus the \n"
  "                 mismatches minus some sort of gap penalty.  Default is 30.\n"
  "   -minIdentity=N   Sets minimum sequence identity (in percent).  Default is\n"
  "                 90 for nucleotide searches, 25 for protein or translated\n"
  "                 protein searches.\n"
  "   -out=type     Controls output file format.  Type is one of:\n"
  "                   psl - Default.  Tab-separated format without actual sequence\n"
  "                   pslx - Tab-separated format with sequence\n"
  "                   axt - blastz-associated axt format\n"
  "                   maf - multiz-associated maf format\n"
  "                   sim4 - similar to sim4 format\n"
  "                   wublast - similar to wublast format\n"
  "                   blast - similar to NCBI blast format\n"
  "                   blast8- NCBI blast tabular format\n"
  "                   blast9 - NCBI blast tabular format with comments\n"
  "   -maxIntron=N   Sets maximum intron size. Default is %d.\n"
  "   -genome=name  When using a dynamic gfServer, The genome name is used to \n"
  "                 find the data files relative to the dynamic gfServer root, named \n"
  "                 in the form $genome.2bit, $genome.untrans.gfidx, and $genome.trans.gfidx\n"
  "   -genomeDataDir=path\n"
  "                 When using a dynamic gfServer, this is the dynamic gfServer root directory\n"
  "                 that contained the genome data files.  Defaults to being the root directory.\n"
  "                \n",
  gfVersion, ffIntronMaxDefault);
exit(-1);
}


struct gfOutput *gvo;

void gfClient(char *hostName, char *portName, char *tSeqDir, char *inName, 
	char *outName, char *tTypeName, char *qTypeName)
/* gfClient - A client for the genomic finding program that produces a .psl file. */
{
struct lineFile *lf = lineFileOpen(inName, TRUE);
static bioSeq seq;
FILE *out = mustOpen(outName, "w");
enum gfType qType = gfTypeFromName(qTypeName);
enum gfType tType = gfTypeFromName(tTypeName);
int dotMod = 0;
char databaseName[256];
struct hash *tFileCache = gfFileCacheNew();
boolean gotConnection = FALSE;

snprintf(databaseName, sizeof(databaseName), "%s:%s", hostName, portName);

gvo = gfOutputAny(outputFormat,  round(minIdentity*10), qType == gftProt, tType == gftProt,
	optionExists("nohead"), databaseName, 23, 3.0e9, minIdentity, out);
gfOutputHead(gvo, out);
struct errCatch *errCatch = errCatchNew();
if (errCatchStart(errCatch))
    {
    struct gfConnection *conn = gfConnect(hostName, portName, genome, genomeDataDir);
    gotConnection = TRUE;
    while (faSomeSpeedReadNext(lf, &seq.dna, &seq.size, &seq.name, qType != gftProt))
        {
        if (dots != 0)
            {
            if (++dotMod >= dots)
                {
                dotMod = 0;
                verboseDot();
                }
            }
        if (qType == gftProt && (tType == gftDnaX || tType == gftRnaX))
            {
            gvo->reportTargetStrand = TRUE;
            gfAlignTrans(conn, tSeqDir, &seq, minScore, tFileCache, gvo);
            }
        else if ((qType == gftRnaX || qType == gftDnaX) && (tType == gftDnaX || tType == gftRnaX))
            {
            gvo->reportTargetStrand = TRUE;
            gfAlignTransTrans(conn, tSeqDir, &seq, FALSE, minScore, tFileCache, gvo, qType == gftRnaX);
            if (qType == gftDnaX)
                {
                reverseComplement(seq.dna, seq.size);
                gfAlignTransTrans(conn, tSeqDir, &seq, TRUE, minScore, tFileCache, gvo, FALSE);
                }
            }
        else if ((tType == gftDna || tType == gftRna) && (qType == gftDna || qType == gftRna))
            {
            gfAlignStrand(conn, tSeqDir, &seq, FALSE, minScore, tFileCache, gvo);
            reverseComplement(seq.dna, seq.size);
            gfAlignStrand(conn, tSeqDir, &seq, TRUE,  minScore, tFileCache, gvo);
            }
        else
            {
            errAbort("Comparisons between %s queries and %s databases not yet supported",
                    qTypeName, tTypeName);
            }
        gfOutputQuery(gvo, out);
        }
    gfDisconnect(&conn);
    }	/*	if (errCatchStart(errCatch))	*/
errCatchEnd(errCatch);
if (errCatch->gotError)
    {
    if (isNotEmpty(errCatch->message->string))
	warn("# error: %s", errCatch->message->string);
    if (gotConnection && isDynamic)
	{
	long et = clock1000() - enterMainTime;
        if (et > NET_TIMEOUT_MS)
	    errAbort("the dynamic server at %s:%s is taking too long to respond,\nperhaps overloaded at this time, try again later", hostName, portName);
        else if (et < NET_QUICKEXIT_MS)
	    errAbort("the dynamic server at %s:%s is returning an error immediately,\nperhaps overloaded at this time, try again later", hostName, portName);
	else
	    errAbort("the dynamic server at %s:%s is returning an error at this time,\ntry again later", hostName, portName);
	}
    else
	errAbort("gfClient error exit");
    }
errCatchFree(&errCatch);

if (out != stdout)
    printf("Output is in %s\n", outName);
gfFileCacheFree(&tFileCache);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, optionSpecs);
if (argc != 6)
    usage();
if (optionExists("prot"))
    qType = tType = "prot";
qType = optionVal("q", qType);
tType = optionVal("t", tType);
if (sameWord(tType, "prot") || sameWord(tType, "dnax") || sameWord(tType, "rnax"))
    minIdentity = 25;
minIdentity = optionFloat("minIdentity", minIdentity);
minScore = optionInt("minScore", minScore);
dots = optionInt("dots", 0);
outputFormat = optionVal("out", outputFormat);
genome = optionVal("genome", NULL);
genomeDataDir = optionVal("genomeDataDir", NULL);
if ((genomeDataDir != NULL) && (genome == NULL))
    errAbort("-genomeDataDir requires the -genome option");
if ((genome != NULL) && (genomeDataDir == NULL))
    genomeDataDir = ".";
if (genomeDataDir != NULL)
    isDynamic = TRUE;

enterMainTime = clock1000();
/* set global for fuzzy find functions */
setFfIntronMax(optionInt("maxIntron", ffIntronMaxDefault));
gfClient(argv[1], argv[2], argv[3], argv[4], argv[5], tType, qType);
return 0;
}
