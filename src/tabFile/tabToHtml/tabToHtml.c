/* tabToHtml - Convert tab-separated-file to an HTML file that is a table and not much else.. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "htmshell.h"
#include "fieldedTable.h"

char *clTitle = NULL;	    // Title from command line

boolean clEmbed;	// If true we embed in another page

void usage()
/* Explain usage and exit. */
{
errAbort(
  "tabToHtml - Convert tab-separated-file to an HTML file that is a table and not much else.\n"
  "usage:\n"
  "   tabToHtml in.tsv out.html\n"
  "options:\n"
  "   -embed - don't write beginning and end of page, just controls and tree.\n"
  "            useful for making html to be embedded in another page.\n"
  "   -title=\"Some sort of table title\"\n"
  );
}

/* Command line validation table. */
static struct optionSpec options[] = {
   {"embed", OPTION_BOOLEAN},
   {"title", OPTION_STRING},
   {NULL, 0},
};

void writeHtml(struct fieldedTable *table, FILE *f)
/* Write HTML version of table */
{
if (!clEmbed)
    {
    fprintf(f,
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\">\n"
    "<HTML>\n"
    "<HEAD>\n"
    "%s"
    "    <META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html;CHARSET=iso-8859-1\">\n"
    , getCspMetaHeader());
    if (clTitle)
	fprintf(f, "    <TITLE>%s</TITLE>\n", clTitle);
    else
	fprintf(f, "    <TITLE>table %s</TITLE>\n", table->name);
    fputs(
    "</HEAD>\n"
    "<BODY>\n"
    , f);
    fprintf(f, 
	"<style>table, th, td "
	"{ border: 1px solid black; "
	"  border-collapse: collapse; "
	"  padding: 3px;"
	"}</style>");
    if (clTitle)
	fprintf(f, "<H2>%s</H2>\n", clTitle);
    else
	fprintf(f, "<H2>table from %s</H2>\n", table->name);
    }

/* Print table tag and header row. */
fprintf(f, "<table>\n");
fprintf(f, "<tr>");
int i;
for (i=0; i<table->fieldCount; ++i)
    {
    char *field = table->fields[i];
    fprintf(f, " <th>%s</th>", field);
    }
fprintf(f, "</tr>\n");

/* Print data rows. */
struct fieldedRow *fr;
for (fr = table->rowList; fr != NULL; fr = fr->next)
    {
    char **row = fr->row;
    fprintf(f, "<tr>");
    for (i=0; i<table->fieldCount; ++i)
	fprintf(f, " <td>%s</td>", row[i]);
    fprintf(f, "<tr>\n");
    }

/* Close up table */
fprintf(f, "</TABLE>\n");

/* Unless embedded close up web page too. */
if (!clEmbed)
    {
    fputs(
    "</BODY>\n"
    "</HTML>\n"
    "\n"
    , f);
    }
}

void tabToHtml(char *inTsv, char *outHtml)
/* tabToHtml - Convert tab-separated-file to an HTML file that is a table and not much else.. */
{
struct fieldedTable *table = fieldedTableFromTabFile(inTsv, inTsv, NULL, 0);
FILE *f = mustOpen(outHtml, "w");
writeHtml(table, f);
carefulClose(&f);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
clEmbed = optionExists("embed");
clTitle = optionVal("title", NULL);
tabToHtml(argv[1], argv[2]);
return 0;
}
