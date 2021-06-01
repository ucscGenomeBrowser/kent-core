/* geoStormToHcaStorm - Convert output of geoToTagStorm to something closer to what the Human Cell 
 * Atlas wants.. */
#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "obscure.h"
#include "tagStorm.h"
#include "csv.h"

struct hash *gSrxToSrr;

void usage()
/* Explain usage and exit. */
{
errAbort(
  "geoStormToHcaStorm - Convert output of geoToTagStorm to something closer to what the Human \n"
  "Cell Atlas wants.\n"
  "usage:\n"
  "   geoStormToHcaStorm in.tags srxToSrr.tab out.tags\n"
  "Where in.tags is geoToTagStorm output, srxToSrr.tab is a two column file with\n"
  "NCBI short read archive SRX IDs followed by SRR ones, and out.tags is the output\n"
  "\n"
  "OBSOLETE - you probably want to use geoToHcaStorm instead"
  );
}

/* Command line validation table. */
static struct optionSpec options[] = {
   {NULL, 0},
};

char *weeds[] =  
/* Tags we'll get rid of */
    {
    "platform.contact_country",
    "platform.contact_name",
    "platform.data_row_count",
    "platform.distribution",
    "platform.geo_accession",
    "platform.last_update_date",
    "platform.status",
    "platform.submission_date",
    "platform.technology",
    "platform.title",
    "sample.channel_count",
    "sample.data_row_count",
    "sample.platform_id",
    "sample.series_id",
    "sample.type",
    "series.platform_id",
    "series.platform_taxid",
    "series.sample_id",
    "series.sample_taxid",
    "series.type",
    "series.status", 
    "sample.status",
    };


char *substitutions[][2] = 
/* Tags we'll rename */
{
    {"platform.organism", "sample.donor.species.text"},
    {"platform.taxid", "sample.donor.species.ontology"},
    {"sample.characteristics_age", "sample.donor.age"},
    {"sample.characteristics_bmi", "sample.donor.body_mass_index"},
    {"sample.characteristics_cell_type", "sample.well.cell_type.text"},
    {"sample.characteristics_developmental_stage", "sample.donor.life_stage"},
    {"sample.characteristics_ethnicity", "sample.donor.ancestry.1.text"},
    {"sample.characteristics_genotype", "sample.donor.genotype"},
    {"sample.characteristics_exome_capture", "assay.genome.method"},
    {"sample.characteristics_sex", "sample.donor.sex"},
    {"sample.characteristics_Sex", "sample.donor.sex"},
    {"sample.characteristics_strain", "sample.donor.strain.1.text"},
    {"sample.characteristics_tissue", "sample.body_part.text"},
    {"sample.contact_address", "project.submitter.street_address"},
    {"sample.contact_city", "project.submitter.city"},
    {"sample.contact_country", "project.submitter.country"},
    {"sample.contact_department", "project.contact.department"},
    {"sample.contact_email", "project.submitter.email"},
    {"sample.contact_institute", "project.submitter.institution"},
    {"sample.contact_laboratory", "project.submitter.laboratory"},
    {"sample.contact_name", "project.submitter.name"},
    {"sample.contact_phone", "project.contact.phone"},
    {"sample.contact_state", "project.submitter.country_division"},
    {"sample.contact_zip/postal_code", "project.contact.postal_code"},
    {"sample.contributor", "project.contributor"},
    {"sample.description", "sample.name"},
    {"sample.geo_accession", "sample.geo_sample"},
    {"sample.instrument_model", "assay.seq.instrument_model"},
    {"sample.update_date sample.update_date"},
    {"sample.library_selection", "assay.rna.primer"},
    {"sample.library_source", "assay.rna.library_construction"},
    {"sample.library_strategy", "assay.seq.library_construction"},
    {"sample.molecule", "assay.seq.molecule"},
    {"sample.organism", "sample.donor.species.text"},
    {"sample.source_name", "sample.body_part.text"},
    {"sample.submit_date", "sample.submit_date"},
    {"sample.taxid", "sample.donor.species.ontology"},
    {"sample.title", "sample.title"},
    {"series.contact_address", "project.contact.street_address"},
    {"series.contact_city", "project.submitter.city"},
    {"series.contact_country", "project.submitter.country"},
    {"series.contact_department", "project.contact.department"},
    {"series.contact_email", "project.submitter.email"},
    {"series.contact_institute", "project.submitter.institution"},
    {"series.contact_laboratory", "project.submitter.laboratory"},
    {"series.contact_name", "project.submitter.name"},
    {"series.contact_phone", "project.contact.phone"},
    {"series.contact_state", "project.submitter.country_division"},
    {"series.contact_zip/postal_code", "project.contact.postal_code"},
    {"series.contributor", "project.contributor"},
    {"series.geo_accession", "project.geo_series"},
    {"series.last_update_date", "project.update_date"},
    {"series.organism", "sample.donor.species.text"},
    {"series.overall_design", "project.experimental_design.1.text"},
    {"series.pubmed_id", "project.publications.1.pmid"},
    {"series.relation_SubSeries_of", "project.geo_parent_series"},
    {"series.submission_date", "project.submit_date"},
    {"series.summary", "project.description"},
    {"series.supplementary_file", "project.supplementary_files"},
    {"series.taxid", "sample.donor.species.ontology"},
    {"series.title", "project.title"},
};

char *mon[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

char *geoDateToHcaDate(char *geoDate, char *hcaBuf, int hcaBufSize)
/* Convert Mon DD YYYY format date to YYYY-MM-DD format in hcaBuf, which should be
 * at least 11 long. */
{ 
/* Make sure input is right size and make a copy of it for parsing */
int geoDateSize = strlen(geoDate);
if (geoDateSize != 11)
    errAbort("date %s is not in DD Mon YYY format", geoDate);
char geoParsed[geoDateSize+1];
strcpy(geoParsed, geoDate);

/* Parse out three parts */
char *parser = geoParsed;
char *month = nextWord(&parser);
char *day = nextWord(&parser);
char *year = nextWord(&parser);
if (year == NULL)
    errAbort("date %s is not in DD Mon YYY format", geoDate);

/* Look up month */
int monthIx = stringArrayIx(month, mon, sizeof(mon));
if (monthIx < 0)
    errAbort("unrecognized month in %s", geoDate);

safef(hcaBuf, hcaBufSize, "%s-%02d-%s", year, monthIx+1, day);
return hcaBuf;
}

char *accFromEnd(char *url, char endChar, char *accPrefix, char *type)
/* Parse out something like
 *     https://long/url/etc.etc<endChar><accession>
 * into just <accession>  Make sure accession starts with given prefix.
 * Type is just for error reporting */
{
char *s = strrchr(url, endChar);
if (s == NULL || !startsWith(accPrefix, s+1))
    errAbort("Malformed %s URL\n\t%s", type, url);
s += 1;
if (!isSymbolString(s))
    errAbort("accFromEnd got something that doesn't look like accession: %s", s);
return s;
}

void fixAccessions(struct tagStorm *storm, struct tagStanza *stanza, void *context)
/* Convert various URLs containing accessions to just accessions */
{
/* Lets deal with the SRR/SRX issue as well */
struct dyString *srrDy = dyStringNew(0);

struct slPair *pair;
for (pair = stanza->tagList; pair != NULL; pair = pair->next)
    {
    char *name = pair->name;
    if (sameString("series.relation_BioProject", name))
        {
	/* Convert something like https://www.ncbi.nlm.nih.gov/bioproject/PRJNA189204
	 * to PRJNA189204 */
	pair->name = "project.ncbi_bioproject";
	pair->val = accFromEnd(pair->val, '/', "PRJNA", "BioProject");
	}
    else if (sameString("series.relation_SRA", name))
        {
	/* Convert something like https://www.ncbi.nlm.nih.gov/sra?term=SRP018525
	 * to SRP018525 */
	pair->name = "project.sra_project";
	pair->val = accFromEnd(pair->val, '=', "SRP", "SRA");
	}
    else if (sameString("sample.relation_SRA", name))
        {
	/* Convert something like https://www.ncbi.nlm.nih.gov/sra?term=SRX229786
	 * to SRX229786 */
	pair->name = "assay.seq.sra_experiment";
	char *srx = accFromEnd(pair->val, '=', "SRX", "SRA");
	pair->val = srx;

	/* Now make comma separated list of all SRR id's associated */
	struct hashEl *hel = hashLookup(gSrxToSrr, srx);
	if (hel == NULL)
	    errAbort("Can't find SRX tag %s in srxToSra.tab file", srx);
	else
	    dyStringAppend(srrDy, hel->val);
	while ((hel = hashLookupNext(hel)) != NULL)
	    {
	    dyStringAppendC(srrDy, ',');
	    dyStringAppend(srrDy, hel->val);
	    }
	}
    else if (sameString("sample.relation_BioSample", name))
        {
	/* Convert something like https://www.ncbi.nlm.nih.gov/biosample/SAMN01915417
	 * to SAMN01915417 */
	pair->name = "sample.ncbi_biosample";
	pair->val = accFromEnd(pair->val, '/', "SAMN", "biosample");
	}
    }

if (srrDy->stringSize > 0)
    {
    tagStanzaAppend(storm, stanza, "assay.seq.sra_run", srrDy->string);
    }
dyStringFree(&srrDy);

}

void rFixAccessions(struct tagStorm *storm, struct tagStanza *list)
/* Go through and fix accessions in all stanzas */
{
tagStormTraverse(storm, list, NULL, fixAccessions);
}

void fixDates(struct tagStorm *storm, struct tagStanza *stanza, void *context)
/* Convert various URLs containing accessions to just accessions */
{
struct slPair *pair;
for (pair = stanza->tagList; pair != NULL; pair = pair->next)
    {
    char *name = pair->name;
    char *val = pair->val;
    if (endsWith(name, "_date"))
	geoDateToHcaDate(val, val, strlen(val)+1);
    else if (sameString(name, "series.status") || sameString(name, "sample.status"))
        {
	char *pat = "Public on ";
	if (startsWith(pat, val))
	    geoDateToHcaDate(val + strlen(pat), val, strlen(val)+1);
	}
    }
}

void mergeAddress(struct tagStanza *stanza, struct dyString *dy, char **tags, int tagCount)
/* Look up all of tags in stanza and concatenate together with space separation */
{
char *address = emptyForNull(tagFindVal(stanza, tags[0]));
char *city = emptyForNull(tagFindVal(stanza, tags[1]));
char *state = emptyForNull(tagFindVal(stanza, tags[2]));
char *zip = emptyForNull(tagFindVal(stanza, tags[3]));
char *country = emptyForNull(tagFindVal(stanza, tags[4]));
if (address || city || state || zip || country)
    dyStringPrintf(dy, "\"%s; %s, %s %s %s\"", address, city, state, zip, country);
}

void mergeAddresses(struct tagStorm *storm, struct tagStanza *stanza, void *context)
/* Because we want to be international, we merge the address into a single line.
 * Something like 101 Main St. Sausilito CA 94965 USA with the format of
 * number, street, city, state, postal code is not how they represent things
 * in all countries.  Some don't have states.  Some put the number after the street.
 * So we just store the whole thing as a single string.  We do keep the separate
 * components as well since we are data hoarders. */
{
struct dyString *dy = dyStringNew(0);
char *seriesComponents[] = 
    {
    "series.contact_address", "series.contact_city", "series.contact_state", 
    "series.contact_zip/postal_code", "series.contact_country"
    };
mergeAddress(stanza, dy, seriesComponents, ArraySize(seriesComponents));
if (dy->stringSize == 0)
    {
    char *sampleComponents[] = 
	{
	"sample.contact_address", "sample.contact_city", "sample.contact_state", 
	"sample.contact_zip/postal_code", "sample.contact_country"
	};
    mergeAddress(stanza, dy, sampleComponents, ArraySize(sampleComponents));
    }
if (dy->string != 0)
    tagStanzaAppend(storm, stanza, "project.submitter.address", dy->string);
dyStringFree(&dy);
}


void fixProtocols(struct tagStorm *storm, struct tagStanza *stanza, void *context)
/* Convert the various protocols in sample to and array of protocol descriptions and
 * an array of types.  This helps us be compatible with array express, which has
 * an unbounded set of protocol types */
{
struct protocolInfo { char *tag, *type;} info[] = 
    {
       {"sample.growth_protocol", "growth protocol",},
       {"sample.treatment_protocol", "treatment protocol",},
       {"sample.extract_protocol", "nucleic acid library construction protocol",},
    };
struct dyString *protocol = dyStringNew(0);
struct dyString *type = dyStringNew(0);
struct dyString *scratch = dyStringNew(0);

int i;
for (i=0; i<ArraySize(info); ++i)
    {
    char *pro = tagFindVal(stanza, info[i].tag);
    if (pro != NULL)
	{
	char *escaped = csvParseNext(&pro, scratch);
        csvEscapeAndAppend(protocol, escaped);
	csvEscapeAndAppend(type, info[i].type);
	}
    }

if (protocol->stringSize > 0)
    {
    tagStanzaAppend(storm, stanza, "sample.protocols", protocol->string);
    tagStanzaAppend(storm, stanza, "sample.protocol_types", type->string);
    }
dyStringFree(&scratch);
dyStringFree(&protocol);
dyStringFree(&type);
}

void geoStormToHcaStorm(char *inTags, char *inSrxSrr, char *output)
/* geoStormToHcaStorm - Convert output of geoToTagStorm to something closer to what the Human Cell 
 *  Atlas wants.. */
{
/* Read in input. */
struct tagStorm *storm = tagStormFromFile(inTags);
gSrxToSrr = hashTwoColumnFile(inSrxSrr);
hashReverseAllBucketLists(gSrxToSrr);

/* Get rid of tags we find useless */
tagStormWeedArray(storm, weeds, ArraySize(weeds));

/* Fix up all stanzas with various functions */
tagStormTraverse(storm, storm->forest, NULL, fixAccessions);
tagStormTraverse(storm, storm->forest, NULL, fixDates);
tagStormTraverse(storm, storm->forest, NULL, mergeAddresses);
tagStormTraverse(storm, storm->forest, NULL, fixProtocols);

/* Get rid of protocols stuff we just fixed */
char *protoWeeds[] = 
    {"sample.extract_protocol", "sample.treatment_protocol", "sample.growth_protocol", };
tagStormWeedArray(storm, protoWeeds, ArraySize(protoWeeds));

/* Do simple subsitutions. */
tagStormSubArray(storm, substitutions, ArraySize(substitutions));

/* Save results */
tagStormWrite(storm, output, 0);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 4)
    usage();
geoStormToHcaStorm(argv[1], argv[2], argv[3]);
return 0;
}
