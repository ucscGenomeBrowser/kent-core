/* gisaidSample - A CGI script to display the sample details page.. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "hCommon.h"
#include "linefile.h"
#include "hash.h"
#include "jksql.h"
#include "cheapcgi.h"
#include "htmshell.h"
#include "cart.h"
#include "hui.h"
#include "dbDb.h"
#include "hdb.h"
#include "web.h"
#include "ra.h"
#include "spDb.h"
#include "genePred.h"
#include "hgColors.h"
#include "hPrint.h"
#include "gisaidSample.h"


/* ---- Global variables. ---- */
struct cart *cart;	/* This holds cgi and other variables between clicks. */
struct hash *oldCart;	/* Old cart hash. */
char *genome;           /* Name of genome - mouse, human, etc. */
char organism[20] = {"hiv"};
char *database;		/* Name of genome database - hg15, mm3, or the like. */
char *curSampleId = NULL;	/* Current Sample ID */
void usage()
/* Explain usage and exit. */
{
errAbort(
  "gisaidSample - A CGI script to display the sample details page.\n"
  "usage:\n"
  "   gisaidSample cgi-vars in var=val format\n"
  "options:\n"
  "   -hgisaid=XXX Session ID to grab vars from session database\n"
  "   -db=XXX  Genome database associated with gene\n"
  "   -org=XXX  Organism associated with gene\n"
  "   -hgs_sample=XXX ID of subject\n"
  );
}

/* --------------- Low level utility functions. ----------------- */

static char *rootDir = "gisaidSampleData";

struct hash *readRa(char *rootName, struct hash **retHashOfHash)
/* Read in ra in root, root/org, and root/org/database. */
{
return hgReadRa(genome, database, rootDir, rootName, retHashOfHash);
}

static struct hash *genomeSettings;  /* Genome-specific settings from settings.ra. */

char *genomeSetting(char *name)
/* Return genome setting value.   Aborts if setting not found. */
{
return hashMustFindVal(genomeSettings, name);
}

char *genomeOptionalSetting(char *name)
/* Returns genome setting value or NULL if not found. */
{
return hashFindVal(genomeSettings, name);
}


boolean checkDatabases(char *databases)
/* Check all databases in space delimited string exist. */
{
char *dupe = cloneString(databases);
char *s = dupe, *word;
boolean ok = TRUE;
while ((word = nextWord(&s)) != NULL)
     {
     if (!sqlDatabaseExists(word))
         {
	 ok = FALSE;
	 break;
	 }
     }
freeMem(dupe);
return ok;
}

/* --------------- Page printers ----------------- */

char *sectionSetting(struct section *section, char *name)
/* Return section setting value if it exists. */
{
return hashFindVal(section->settings, name);
}

char *sectionRequiredSetting(struct section *section, char *name)
/* Return section setting.  Squawk and die if it doesn't exist. */
{
char *res = sectionSetting(section, name);
if (res == NULL)
    errAbort("Can't find required %s field in %s in settings.ra",
    	name, section->name);
return res;
}

boolean sectionAlwaysExists(struct section *section, struct sqlConnection *conn,
	char *sampleId)
/* Return TRUE - for sections that always exist. */
{
return TRUE;
}

void sectionPrintStub(struct section *section, struct sqlConnection *conn,
	char *sampleId)
/* Print out coming soon message for section. */
{
hPrintf("coming soon!");
}

struct section *sectionNew(struct hash *sectionRa, char *name)
/* Create a section loading all but methods part from the
 * sectionRa. */
{
struct section *section = NULL;
struct hash *settings = hashFindVal(sectionRa, name);

if (settings != NULL)
    {
    AllocVar(section);
    section->settings = settings;
    section->name = sectionSetting(section, "name");
    section->shortLabel = sectionRequiredSetting(section, "shortLabel");
    section->longLabel = sectionRequiredSetting(section, "longLabel");
    section->priority = atof(sectionRequiredSetting(section, "priority"));
    section->exists = sectionAlwaysExists;
    section->print = sectionPrintStub;
    }
return section;
}

int sectionCmpPriority(const void *va, const void *vb)
/* Compare to sort sections based on priority. */
{
const struct section *a = *((struct section **)va);
const struct section *b = *((struct section **)vb);
float dif = a->priority - b->priority;
if (dif < 0)
    return -1;
else if (dif > 0)
    return 1;
else
    return 0;
}

static void addGoodSection(struct section *section,
	struct sqlConnection *conn, struct section **pList)
/* Add section to list if it is non-null and exists returns ok. */
{
if (section != NULL && hashLookup(section->settings, "hide") == NULL
   && section->exists(section, conn, curSampleId))
     slAddHead(pList, section);
}

struct section *loadSectionList(struct sqlConnection *conn)
/* Load up section list - first load up sections.ra, and then
 * call each section loader. */
{
struct hash *sectionRa = NULL;
struct section *sectionList = NULL;

readRa("section.ra", &sectionRa);

addGoodSection(demogSection(conn, sectionRa), conn, &sectionList);
//addGoodSection(vaccineSection(conn, sectionRa), conn, &sectionList);
//addGoodSection(clinicalSection(conn, sectionRa), conn, &sectionList);
addGoodSection(sequenceSection(conn, sectionRa), conn, &sectionList);

slSort(&sectionList, sectionCmpPriority);
return sectionList;
}

void printSections(struct section *sectionList, struct sqlConnection *conn,
	char *sampleId)
/* Print each section in turn. */
{
struct section *section;
for (section = sectionList; section != NULL; section = section->next)
    {
    webNewSection("<A NAME=\"%s\"></A>%s\n", section->name, section->longLabel);
    section->print(section, conn, sampleId);
    }
}

void hotLinks()
/* Put up the hot links bar. */
{
hPrintf("<TABLE WIDTH=\"100%%\" BGCOLOR=\"#000000\" BORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"1\"><TR><TD>\n");
hPrintf("<TABLE WIDTH=\"100%%\" BGCOLOR=\"#2636D1\" BORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"2\"><TR>\n");

/* Home */
hPrintf("<TD ALIGN=CENTER><A HREF='/index.html' class='topbar' style='color:#FFFFFF;'>"
        "Home</A></TD>");
//, orgEnc);

/* Blat */
hPrintf("<TD ALIGN=CENTER><A HREF='../cgi-bin/hgBlat?command=start' class='topbar' "
        "style='color:#FFFFFF;'>Blat</A></TD>");

/* Sequence View */
if (hIsGisaidServer())
    {
    hPrintf("<TD ALIGN=CENTER><A HREF='../cgi-bin/hgTracks?db=%s' class='topbar' "
            "style='color:#FFFFFF;'>Sequence View</A></TD>", database);
    }
else
    {
    hPrintf("<TD ALIGN=CENTER><A HREF='../cgi-bin/hgGateway?db=%s' class='topbar' "
            "style='color:#FFFFFF;'>Sequence View Gateway</A></TD>", database);
    }
/* Table View */
hPrintf("<TD ALIGN=CENTER><A HREF='../cgi-bin/gisaidTable' class='topbar' "
        "style='color:#FFFFFF;'>Table View</A></TD>");

/* Help */
hPrintf("<TD ALIGN=CENTER><A HREF='/goldenPath/help/gisaidTutorial.html#SampleView' "
        "target=_blank class='topbar' style='color:#FFFFFF;'>Help</A></TD>");

hPuts("</TR></TABLE>");
hPuts("</TD></TR></TABLE>\n");
}

void webMain(struct sqlConnection *conn)
/* Set up fancy web page with hotlinks bar and
 * sections. */
{
struct section *sectionList = NULL;
char query[256];
struct sqlResult *sr;
char **row;

if (hIsGisaidServer())
    {
    validateGisaidUser(cart);
    }

sectionList = loadSectionList(conn);

puts("<FORM ACTION=\"/cgi-bin/gisaidSample\" NAME=\"mainForm\" METHOD=\"GET\">\n");

/* display GISAID logo image here */
//printf("<img src=\"/images/gisaid_header.jpg\" alt=\"\" name=\"gisaid_header\" width=\"800\" height=\"86\" border=\"1\" usemap=\"#gisaid_headerMap\">");

//hPrintf("<br><br>");
hotLinks();

printf("<font size=\"5\"><BR><B>Sample View   </B></font>");

if (sameWord(curSampleId, ""))
    {
    printf("<BR><H3>Please enter a sample ID.\n");
    printf("<input type=\"text\" name=\"hgs_sample\" value=\"%s\">\n", curSampleId);
    cgiMakeButton("submit", "Go!");
    printf("</H3>");
    printf("For example: EPI_ISL_29573");fflush(stdout);
    }
else
    {
    sqlSafef(query, sizeof(query),
    	  "select EPI_ISOLATE_ID from %s.gisaidSubjInfo where EPI_ISOLATE_ID = '%s'",
    	  database, curSampleId);
    sr = sqlMustGetResult(conn, query);
    row = sqlNextRow(sr);
    sqlFreeResult(&sr);
    if (row != NULL)
    	{
    	printf(
	"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;search for another sample:&nbsp;");
    	printf("<input type=\"text\" name=\"hgs_sample\" value=\"\">\n");
	cgiMakeButton("submit", "Go!");
    	printSections(sectionList, conn, curSampleId);
    	}
    else
    	{
    	printf("<H3><span style='color:red;'>%s</span> is not a valid sample ID.</H3>", curSampleId);

    	printf("<H3>Please enter a valid sample ID.\n");
    	printf("<input type=\"text\" name=\"hgs_sample\" value=\"%s\">\n", "");
    	cgiMakeButton("submit", "Go!");
    	printf("<BR><BR>For example: GISAID4123");
	printf("</H3>");
	fflush(stdout);
	}
    }
puts("</FORM>\n");
}

void cartMain(struct cart *theCart)
/* We got the persistent/CGI variable cart.  Now
 * set up the globals and make a web page. */
{
struct sqlConnection *conn = NULL;
cart = theCart;
getDbAndGenome(cart, &database, &genome, oldCart);

/* !!! force database to hiv1 until move to server hiv1 is complete
   and the default database of hgcentral on it point to hiv1. */

conn = hAllocConn(database);

curSampleId = cgiOptionalString("hgs_sample");
if (curSampleId == NULL) curSampleId = strdup("");

cartHtmlStart("GISAID Sample View");
webMain(conn);

hFreeConn(&conn);

cartHtmlEnd();
}

char *excludeVars[] = {"Submit", "submit", NULL};

int main(int argc, char *argv[])
/* Process command line. */
{
if (argc != 1)
    usage();

cgiSpoof(&argc, argv);
htmlSetBackground(hBackgroundImage());

oldCart = hashNew(12);
cartEmptyShell(cartMain, hUserCookie(), excludeVars, oldCart);
return 0;
}
