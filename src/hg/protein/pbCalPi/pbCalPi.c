/* pbCalPi - Calculate pI values from a list of protein IDs */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "pbCalPi.h"
#include "math.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "pbCalPi - Calculate pI values from a list of protein IDs \n"
  "usage:\n"
  "   pbCalPi spDb outFile\n"
  "      spDb     is the swissprot database name\n"
  "      outFile  is the output file name of tab delineated file of protein accession number and pI value\n"
  "example: pbCalPi prot.lis sp040115 pepPi.tab\n");
}

float myExp10(float x)
{
return(powf(10.0, x));
}

double calPi(char *sequence)
/* the calPi() is based on the parameters and algorithm obtained */
/* from SWISS-PROT.  The calculation results should be identical to that */
/* from the Web page: http://us.expasy.org/tools/pi_tool.html */
{
int   i, sequenceLength;
int   comp[ALPHABET_LEN];
int   nTermResidue, cTermResidue;
double charge, phMin, phMid = 0.0, phMax;
double carg, cter, nter, ctyr, chis, clys, casp, cglu, ccys;
int    R, H, K, D, E, C, Y;

R = (int)('R' - 'A');
H = (int)('H' - 'A');
K = (int)('K' - 'A');
D = (int)('D' - 'A');
E = (int)('E' - 'A');
C = (int)('C' - 'A');
Y = (int)('Y' - 'A');

sequenceLength = strlen(sequence);
for (i=0; i<ALPHABET_LEN; i++) comp[i] = 0.0;
      
/* Compute the amino-acid composition. */
for (i = 0; i < sequenceLength; i++)
    comp[sequence[i] - 'A']++;

/* Look up N-terminal and C-terminal residue. */
nTermResidue = sequence[0] - 'A';
cTermResidue = sequence[sequenceLength - 1] - 'A';

phMin = PH_MIN;
phMax = PH_MAX;

for (i = 0, charge = 1.0; i < MAXLOOP && (phMax - phMin) > EPSI; i++)
    {
    phMid = phMin + (phMax - phMin) / 2.0;

    cter = myExp10(-cPk[cTermResidue][0]) /
           (myExp10(-cPk[cTermResidue][0]) + myExp10(-phMid));
    nter = myExp10(-phMid) /
           (myExp10(-cPk[nTermResidue][1]) + myExp10(-phMid));

    carg = comp[R] * myExp10(-phMid) /
           (myExp10(-cPk[R][2]) + myExp10(-phMid));
    chis = comp[H] * myExp10(-phMid) /
           (myExp10(-cPk[H][2]) + myExp10(-phMid));
    clys = comp[K] * myExp10(-phMid) /
           (myExp10(-cPk[K][2]) + myExp10(-phMid));
    casp = comp[D] * myExp10(-cPk[D][2]) /
           (myExp10(-cPk[D][2]) + myExp10(-phMid));
    cglu = comp[E] * myExp10(-cPk[E][2]) /
           (myExp10(-cPk[E][2]) + myExp10(-phMid));
    ccys = comp[C] * myExp10(-cPk[C][2]) /
           (myExp10(-cPk[C][2]) + myExp10(-phMid));
    ctyr = comp[Y] * myExp10(-cPk[Y][2]) /
           (myExp10(-cPk[Y][2]) + myExp10(-phMid));

    charge = carg + clys + chis + nter -
             (casp + cglu + ctyr + ccys + cter);

    if (charge > 0.0)
	phMin = phMid;
    else
  	phMax = phMid;
    }
return(phMid);
}

int main(int argc, char *argv[])
{
if (argc != 3)
   usage();

char *spDb      = argv[1];
char *outfName  = argv[2];
FILE *f = mustOpen(outfName, "w");

struct sqlConnection *conn = sqlConnect(spDb);
char query[1024];
sqlSafef(query, sizeof query, "select acc,val from protein");
struct sqlResult *sr = sqlGetResult(conn, query);
char **row;
while ((row = sqlNextRow(sr)) != NULL)
    {
    fprintf(f, "%s\t%.2f\n", row[0], calPi(row[1]));
    }
carefulClose(&f);
return(0);
}
