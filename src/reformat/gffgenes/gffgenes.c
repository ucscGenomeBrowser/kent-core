/* gffgenes - Creates files that say extents of genes from gff files. */

#include	<stdio.h>
#include "common.h"
#include "sig.h"
#include "wormdna.h"
#include "hash.h"

#ifdef GZ_FILENAMES
static char *inNames[] = {
    "CHROMOSOME_I.gff.gz",
    "CHROMOSOME_II.gff.gz",
    "CHROMOSOME_III.gff.gz",
    "CHROMOSOME_IV.gff.gz",
    "CHROMOSOME_V.gff.gz",
    "CHROMOSOME_X.gff.gz",
};
#else
static char *inNames[] = {
    "CHROMOSOME_I.gff",
    "CHROMOSOME_II.gff",
    "CHROMOSOME_III.gff",
    "CHROMOSOME_IV.gff",
    "CHROMOSOME_V.gff",
    "CHROMOSOME_X.gff",
};
#endif

static char *chromNames[] = {
    "i", "ii", "iii", "iv", "v", "x",
};

struct exon
    {
    struct exon *next;
    int start, end;
    };

int cmpExons(const void *va, const void *vb)
/* Compare two exons to sort by where they first start. */
{
const struct exon *a = *((struct exon **)va);
const struct exon *b = *((struct exon **)vb);
int dif = a->start - b->start;
if (a == 0)
    dif = a->end - b->end;
return dif;
}


struct gene
    {
    struct gene *next;
    char *name;
    struct exon *exons;
    int start, end;
    UBYTE chromIx;
    char strand;
    };

int cmpGenes(const void *va, const void *vb)
/* Compare two genes to sort by where they first start. */
{
const struct gene *a = *((struct gene **)va);
const struct gene *b = *((struct gene **)vb);
int dif = a->start - b->start;
if (a == 0)
    dif = a->end - b->end;
return dif;
}

void writeShortString(FILE *f, char *s)
{
UBYTE count = strlen(s);
writeOne(f, count);
mustWrite(f, s, count);
}

void checkGene(struct gene *gene)
{
int min = 0x7fffffff;
int max = -min;
struct exon *e;
for (e=gene->exons; e != NULL; e = e->next)
    {
    if (e->start < min)
        min = e->start;
    if (e->end > max)
        max = e->end;
    }
if (gene->exons == NULL)
    {
    printf("Gene %s has no exons\n", gene->name);
    }
else if (min != gene->start || max != gene->end)
    {
    printf("Gene %s - goes from %d-%d but exons go from %d-%d\n", 
        gene->name, gene->start, gene->end, min, max);
    gene->start = min;
    gene->end = max;
    }
}

void writeGene(struct gene *gene, FILE *c2g, FILE *gl)
{
short pointCount;
struct exon *exon;

checkGene(gene);
fprintf(c2g, "%s:%d-%d %c %s\n", chromNames[gene->chromIx], gene->start-1, gene->end, gene->strand, gene->name);
writeShortString(gl, gene->name);
writeOne(gl, gene->chromIx);
writeOne(gl, gene->strand);
pointCount = slCount(gene->exons) * 2;
writeOne(gl, pointCount);
for (exon = gene->exons; exon != NULL; exon = exon->next)
    {
    int start = exon->start - 1;
    writeOne(gl, start);
    writeOne(gl, exon->end);
    }
}

char *unquote(char *s, int lineCount, char *fileName)
/* Remove opening and closing quotes from string s. */
{
int len =strlen(s);
if (s[0] != '"')
    errAbort("Expecting begin quote on %s line %d of %s\n", s, lineCount, fileName);
if (s[len-1] != '"')
    errAbort("Expecting end quote on %s line %d of %s\n", s, lineCount, fileName);
s[len-1] = 0;
return s+1;
}

char *unquoteSequence(char *s, int lineCount, char *fileName)
/* Remove quotes and preceding 'Sequence' from string. */
{
int missed = 0;
char *header0 = "Sequence ";
char *header1 = "Transcript ";
char *header2 = "CDS ";
s = trimSpaces(s);
if (!startsWith(header0, s)) { ++missed; }
if (!startsWith(header1, s)) { ++missed; }
if (!startsWith(header2, s)) { ++missed; }
if ( 3 == missed)
    errAbort("Expecting Sequence before gene name line %d of %s", lineCount, fileName);
if (startsWith(header0, s)) {
	return unquote(s + strlen(header0), lineCount, fileName);
    } else if(startsWith(header1, s)) {
	return unquote(s + strlen(header1), lineCount, fileName);
    } else {
	return unquote(s + strlen(header2), lineCount, fileName);
    }
}

#ifdef GZ_FILENAMES
static FILE *
pipeOpen(char * fileName)
{
	char cmd[1024];
	FILE * fh;

	strcpy(cmd, "gzip -dc ");  /* use "gzip -c" for zwrite */
	strncat(cmd, fileName, sizeof(cmd)-strlen(cmd));
	fh = popen(cmd, "r");  /* use "w" for zwrite */
	return fh;
}
#endif

void procOne(char *inName, UBYTE chromIx, FILE *c2g, FILE *gl)
{
#ifdef GZ_FILENAMES
FILE *in = pipeOpen(inName);
#else
FILE *in = mustOpen(inName, "r");
#endif
struct gene *geneList = NULL, *g = NULL;
struct exon *exon;
char line[1024];
int lineCount = 0;
char *words[128];
int wordCount;
int exonCount = 0;
char *type;
char *source;
char *geneName;
struct hash *geneHash = newHash(15);

if ( (FILE *) NULL == in ) {
    errAbort("gffgenes - can not open: %s\n", inName );
}
printf("Processing %s\n", inName);
while (fgets(line, sizeof(line), in))
    {
    ++lineCount;
    if (line[0] == '#')
        continue;
    wordCount = chopTabs(line, words);
    if (wordCount > 0)
        {
        if (wordCount < 9)
	    continue;
	source = words[1];
        type = words[2];
	/*	When type (feature) "exon" is found, check
	*	the "Source" types to skip.  Do not want all exons.
	 *	As of April 2004, the following "Source" types are found
	 *		on the "exon" Feature
	 *	curated
	 *	Pseudogene
	 *	tRNAscan-SE-1.11
	 *	RNA
	 *	snRNA
	 *	miRNA
	 *	hand_built
	 *	rRNA
	 *	DO NOT ELIMINATE THE FOLLOWING, these are the exons
			sameString(source, "hand_built") ||
			sameString(source, "curated") ||
			sameString(source, "mRNA") ||
			sameString(source, "Coding_transcript") ||
			sameString(source, "Genefinder") ||
	 */
        if (sameString(type, "exon") || sameString(type, "coding_exon"))
            {
		if (sameString(source, "Genomic_cannonical") ||
			sameString(source, "Pseudogene") ||
			sameString(source, "non-coding_transcript") ||
			sameString(source, "history") ||
			sameString(source, "Transposon") ||
			sameString(source, "rRNA") ||
			sameString(source, "scRNA") ||
			sameString(source, "snRNA") ||
			sameString(source, "snoRNA") ||
			sameString(source, "stRNA") ||
			sameString(source, "miRNA") ||
			sameString(source, "tRNA") ||
			sameString(source, "tRNAscan-SE-1.23") ||
			sameString(source, "tRNAscan-SE-1.11") ||
			sameString(source, "Link"))
		    {
	    	    continue;
		    }
	    /*	This leaves curated, RNA, and hand_built to be counted */
	    ++exonCount;
	    geneName = unquoteSequence(words[8], lineCount, inName);
	    if ((g = hashFindVal(geneHash, geneName)) == NULL)
	        {
		AllocVar(g);
		slAddHead(&geneList, g);
		hashAddSaveName(geneHash, geneName, g, &g->name);
		g->start = atoi(words[3]);
		g->end = atoi(words[4]);
		g->strand = words[6][0];
		g->chromIx = chromIx;
		g->exons = NULL;
		}
            AllocVar(exon);
            exon->start = atoi(words[3]);
	    if (exon->start < g->start)
	        g->start = exon->start;
            exon->end = atoi(words[4]);
	    if (exon->end > g->end)
	        g->end = exon->end;
            slAddTail(&g->exons, exon);
            }
        }
    }
slSort(&geneList, cmpGenes);
printf("Read %d genes (%d exons) in %s\n", slCount(geneList), exonCount, inName);
for (g = geneList; g != NULL; g=g->next)
    {
    slSort(&g->exons, cmpExons);
    writeGene(g, c2g, gl);
    }
#ifdef GZ_FILENAMES
pclose(in);
#else
fclose(in);
#endif
}

int main(int argc, char *argv[])
{
char *c2gName, *glName;
FILE *c2g, *gl;
int i;
bits32 sig = glSig;

if (argc != 3)
    {
    errAbort("gffgenes - creates files that store extents of genes for intronerator\n"
             "usage:\n"
             "     gffgenes c2g file.gl\n"
             "This needs to be run in the directory with the Xgenes.gff files.");
    }
c2gName = argv[1];
glName = argv[2];
c2g = mustOpen(c2gName, "w");
gl = mustOpen(glName, "wb");
writeOne(gl, sig);
for (i=0; i<ArraySize(inNames); ++i)
    {
    procOne(inNames[i], (UBYTE)i, c2g, gl);
    }
return 0;
}
