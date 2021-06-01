/* hcaStormToTabDir - Convert a tag storm file to a directory full of tab separated files that 
 * express pretty much the same thing from the point of view of HCA V4 ingest. 
 *
 * Overall doing this involves a several things.  Most of the simple conversion of
 * tag names from V3 to V4 format is done before this program runs.  However a little
 * bit is saved for this program mostly for one of the following reasons:
 *    1) V4 has multiple sample outputs, not a single one.  Instead of having
 *       one sample with donor, and other major subobjects, there are multiple
 *       samples, chained together with a derived_from field.  There is basically
 *       only one major subobject per sample,  and there is a seperate tab for each.
 *       This begs the question of what to do with the fields of sample that aren't
 *       part of a major subobject.  EBI's solution is to duplicate the genus_species and
 *       ncbi_taxon_id field on each sample.  Other fields such as id and name are
 *       also now outside of the major subobject.   These fields are kept as sample.donor.id
 *       and so forth in the tag storm, and then resolved to sample.id's and the like
 *       here.   Also there are some fields like protocol_ids that are moved to the last
 *       sample in the chain.
 *    2) Some fields are specified as arrays in the json schema but are in the spreadsheet
 *       as single fields with various, inconsistent delimiters.  I don't want to mess up the
 *       tagStorm representation with these, so I save that conversion for here.
 *    3) Some of the fields are ontologies in the json schema, with .text and .ontology subfields
 *       but in the spreadsheet they are simple fields and the .ontology bit is lost.  Again
 *       not wanting to mess up the tagStorm I save that conversion for here.
 *    4) The files tab is a real hodgepodge of things because EBI wants file to be a high level
 *       entity and the json schema represents files as just file names.
 *
 * Beyond the tag name conversion, the spreadsheet conversion means that we have to put in
 * id's in many cases to link an what is an embedded subobject in the tagStorm and JSON-schema
 * view to the parent tab.  Similarly we have to put in the derived_from links in the sample
 * subobjects.
 *
 * When all the new fields are added the core of the program runs.  This is the call to
 *      tagStormTraverse(tags, tags->forest, &cc, fillInTables);
 * which makes a separate fieldedTable for each prefix in out_objs[] and merges identical
 * rows in each table into a single row.  
 *
 * Since everything is prefix driven, before the "fillInTables" traversal, there is some gaming
 * to add prefixes to some things to direct them to a specific tab.  This is most dramatic for 
 * the files tab.
 *
 * Then the program does a little renaming of some of the fields in the table,  stripping
 * out the table prefix so that you get something like "description" in the project table rather
 * than table.description.  This is a bit complicated by the sample.donor and the like but not
 * too badly.   
 *
 * The last step is outputting the tables from memory into tab-separated-files.  For most of the
 * tables the simple fieldedTableToTabFile() library call suffices for this.  In the case of
 * the project though, rather than a single row spreadsheet that is very wide, EBI chose
 * to make a two column spreadsheet with labels in the first column and values on the second.
 * Also for the publications and contacts the array gets unpacked to one row per element.
 *
 * Most of the tab-sep-files are just named after the objects, but in a few cases,
 * for the contact related fields,  EBI chose to name the table in part after the object type
 * (contact) rather than where it occurs (project.contributors becomes contact.contributors)
 *
 * Well, quite a bit of work for a version that I can't say I like as well as V3.  Hopefully
 * I'll be able to convince folks to smooth out the worst kinks in V5. 
 *     -Jim Kent   Dec 15, 2017 */

#include "common.h"
#include "linefile.h"
#include "hash.h"
#include "options.h"
#include "localmem.h"
#include "dystring.h"
#include "portable.h"
#include "tagStorm.h"
#include "tagSchema.h"
#include "fieldedTable.h"
#include "csv.h"

void usage()
/* Explain usage and exit. */
{
errAbort(
  "hcaStormToTabDir - Convert a tag storm file to a directory full of tab separated files that\n"
  "express pretty much the same thing from the point of view of HCA V4 ingest.\n"
  "usage:\n"
  "   hcaStormToTabDir in.tag outdir\n"
  );
}

/* Command line validation table. */
static struct optionSpec options[] = {
   {NULL, 0},
};

struct convert
/* A structure to assist conversion to tab separated file dir */
    {
    struct convert *next;   // Next in list
    char *objectName;	    // Name of object, something like assay.single_cell.barcode
    char *tabName;	    // Name of tab, may omit some of object name like  single_cell.barcode
    struct hash *fieldHash;  // Fields in output table. Value is index of field in table
    struct slName *fieldList;  // Fields in output table as simple list */
    char **scratchRow;	    // Scratch space for a single row of table
    struct fieldedTable *table;  // Fields for this object, each row will be unique
    struct hash *uniqRowHash;  // Helps us keep each row of table unique
    };
    
char *outObjs[] = 
/* One of these for each table in output. It's necessary that the more specific
 * ones be ahead of the less specific.*/
{
    "project.publications",
    "project.submitters",   // shortcut
    "project.contributors", // shortcut
    "project",
    "sample.donor",
    "sample.specimen_from_organism",
    "sample.state_of_specimen",
    "sample.cell_suspension",
    "sample.cell_suspension.enrichment",    // shortcut
    "sample.cell_suspension.well",
    "sample.organoid",
    "sample.immortalized_cell_line",
    "sample.primary_cell_line",
    "sample",
    "assay.single_cell.cell_barcode",	// shortcut
    "assay.single_cell",		    // shortcut
    "assay.rna",    // shortcut
    "assay.seq.umi_barcode", // shortcut
    "assay.seq",    // shortcut
    "assay",
    "file",
    "protocols",
};


struct shortcut
/* Just a shortened name for object */
    {
    char *fullName;
    char *shortcut;
    };

struct shortcut shortcuts[] = 
{
   {"project.submitters", "contact.submitters"},
   {"project.contributors", "project.contributors"},
   {"sample.cell_suspension.enrichment", "cell_suspension.enrichment"},
   {"assay.single_cell", "single_cell"},
   {"assay.single_cell.barcode", "single_cell.barcode"},
   {"assay.rna", "rna"},
   {"assay.seq", "seq"},
   {"assay.seq.umi_barcode", "seq.umi_barcode"},
};

char *sampleSubtypePrefixes[] = 
    {
    "sample.donor.",
    "sample.specimen_from_organism.",
    "sample.organoid.",
    "sample.immortalized_cell_line.",
    "sample.primary_cell_line.",
    "sample.cell_suspension.",
    };

char *assaySubtypePrefixes[] = 
    {
    "assay.single_cell.",
    "assay.rna.",
    "assay.seq.",
    "assay.single_cell.cell_barcode.",
    "assay.seq.umi_barcode."
    };

struct entitySubtype
/* A type of sample, we'll make a separate tab for each of these that exist */
    {
    struct entitySubtype *next;
    char *name;	    /* Name of subtype,  excludes both sample. and trailing dot */
    char *fieldPrefix;  /* An element in sampleSubtypePrefixes array */
    };

boolean anyStartWithPrefix(char *prefix, struct slName *list)
/* Return TRUE if anything of the names in list start with prefix */
{
struct slName *el;
for (el = list; el != NULL; el = el->next)
   if (startsWith(prefix, el->name))
       return TRUE;
return FALSE;
}

struct entitySubtype *createEntitySubtypes(char **subtypePrefixes, int subtypeCount, 
    struct slName *origFieldList, char *entityPrefix)
/* Create an entity subtype list for sample or assay */
{
struct entitySubtype *subtypeList = NULL, *subtype;
int i;
int skipSize = strlen(entityPrefix);
for (i=0; i<subtypeCount;  ++i)
    {
    char *prefix = subtypePrefixes[i];
    if (anyStartWithPrefix(prefix, origFieldList))
        {
	AllocVar(subtype);
	subtype->fieldPrefix = prefix;
	int prefixSize = strlen(prefix);
	int nameSize = prefixSize - skipSize - 1;  // 1 for trailing dot
	subtype->name = cloneStringZ(prefix + skipSize, nameSize);
	slAddHead(&subtypeList, subtype);
	}
    }
slReverse(&subtypeList);
return subtypeList;
}

char *findShortcut(char *fullName)
/* Look up full name and return shortcut if it exists */
{
int i;
for (i=0; i<ArraySize(shortcuts); ++i)
    if (sameString(shortcuts[i].fullName, fullName))
        return shortcuts[i].shortcut;
return fullName;
}

struct convert *makeConvertList()
/* Return a list of all convert objects */
{
struct convert *list = NULL;
int i;
for (i=0; i<ArraySize(outObjs); ++i)
    {
    struct convert *conv;
    AllocVar(conv);
    conv->objectName = outObjs[i];
    conv->tabName = findShortcut(conv->objectName);
    conv->fieldHash = hashNew(8);
    slAddHead(&list, conv);
    }
slReverse(&list);
return list;
}

struct convContext
/* Information for recursive tagStorm traversing function collectFields and fillInTable */
    {
    struct convert *convList;
    struct dyString *scratch;
    };

struct convert *findConvertFromPrefix(struct convert *list, char *name)
/* Find first convert whose object name is a prefix of name */
{
struct convert *conv;
for (conv = list; conv != NULL; conv = conv->next)
    {
    if (startsWith(conv->objectName, name))
        return conv;
    }
return NULL;
}

void collectFields(struct tagStorm *storm, struct tagStanza *stanza, void *context)
/* Add fields from stanza to list of converts in context */
{
struct convContext *cc = context;
struct slPair *pair;
for (pair = stanza->tagList; pair != NULL; pair = pair->next)
    {
    char *fieldName = pair->name;
    struct convert *conv = findConvertFromPrefix(cc->convList, fieldName);
    if (conv == NULL)
       errAbort("Can't find converter for %s", fieldName);
    struct hash *hash = conv->fieldHash;
    if (!hashLookup(hash, fieldName))
	{
	hashAddInt(hash, fieldName, hash->elCount);
	slNameAddTail(&conv->fieldList, fieldName);
	}
    }
}

void fillInTables(struct tagStorm *storm, struct tagStanza *stanza, void *context)
/* Add fields from stanza to list of converts in context */
{
struct convContext *cc = context;
if (stanza->children == NULL)  // Only do leaf stanzas
    {
    /* Loop through all possible output tables */
    struct convert *conv;
    for (conv = cc->convList; conv != NULL; conv = conv->next)
        {
	/* Short circuit loop if table didn't actually have any fields anywhere */
	struct fieldedTable *table = conv->table;
	if (table == NULL)
	    continue;

	/* Make up a single row of output table in scratchRow */
	int usedFields = 0;
	int i;
	for (i=0; i<table->fieldCount; ++i)
	    {
	    char *field = table->fields[i];
	    char *val = tagFindVal(stanza, field);
	    if (val != NULL)
		++usedFields;
	    conv->scratchRow[i] = emptyForNull(val);
	    }

	/* If row is non-empty add it to table if the row doesn't already exist */
	if (usedFields > 0)
	    {
	    struct dyString *dy = cc->scratch;
	    dyStringClear(dy);
	    for (i=0; i<table->fieldCount; ++i)
	       {
	       dyStringAppendC(dy, 1);  // Using 1 as a rare non-printable separator
	       dyStringAppend(dy, conv->scratchRow[i]);
	       }
	    if (hashLookup(conv->uniqRowHash, dy->string) == NULL)
	       {
	       fieldedTableAdd(table, conv->scratchRow, table->fieldCount, 
		    conv->uniqRowHash->elCount);
	       hashAdd(conv->uniqRowHash, dy->string, NULL);
	       }
	    }
	}
    }
}

struct slName *namesMatchingPrefix(struct slPair *pairList, char *prefix)
/* Return a list of the name fields on pairList that start with prefix */
{
struct slName *retList = NULL;
struct slPair *pair;
for (pair = pairList; pair != NULL; pair = pair->next)
    {
    if (startsWith(prefix, pair->name))
       slNameAddHead(&retList, pair->name);
    }
slReverse(&retList);
return retList;
}

struct arrayToSepStringContext
/* Context structure for traversal function stanzaArrayToSeparatedString */
   {
   char *arrayName;  /* Name of array without .# at end */
   char *separator;  /* String to use for separator */
   };

void stanzaArrayToSeparatedString(struct tagStorm *tags, struct tagStanza *stanza, void *context)
/* Convert somethigns like
 *     experimental_array.1 this
 *     experimental_array.2 that
 * to
 *     experimental_array   this<separator>that */
{
struct arrayToSepStringContext *con = context;
char prefix[128];
safef(prefix, sizeof(prefix), "%s.", con->arrayName);
struct slName *arrayTags = namesMatchingPrefix(stanza->tagList, prefix);
if (arrayTags != NULL)
    {
    slSort(&arrayTags, slNameCmpStringsWithEmbeddedNumbers);
    struct dyString *dy = dyStringNew(0);
    struct slName *el;
    for (el = arrayTags; el != NULL; el = el->next)
        {
	if (dy->stringSize != 0)
	    dyStringAppend(dy, con->separator);
	dyStringAppend(dy, tagFindLocalVal(stanza, el->name));
	tagStanzaDeleteTag(stanza, el->name);
	}
    tagStanzaAppend(tags, stanza, con->arrayName, dy->string);
    dyStringFree(&dy);
    }
}

void trimFieldNames(struct fieldedTable *table, char *objectName)
/* Remove objectName from table's field names */
{
int trimSize = strlen(objectName) + 1;
int i;
for (i=0; i<table->fieldCount; ++i)
    table->fields[i] += trimSize;
}

void trimSampleFieldNames(struct fieldedTable *table, char *objectName)
/* Simplify some of sample field names. */
{
int sampleTrimSize = strlen("sample.");
int fullTrimSize = strlen(objectName) + 1;
int i;
for (i=0; i<table->fieldCount; ++i)
    {
    char *field = table->fields[i];
    if (  endsWith(field, ".genus_species") 
       || endsWith(field, ".derived_from")
       || endsWith(field, ".supplementary_files")
       || (endsWith(field, ".name") && !stringIn(".well.name", field))
       || endsWith(field, ".ncbi_taxon_id")
       || endsWith(field, ".sample_id")
       || endsWith(field, ".sample_accessions"))
	table->fields[i] = field + fullTrimSize;
    else
	table->fields[i] = field + sampleTrimSize;
    }
}

void trimAssayFieldNames(struct fieldedTable *table, char *objectName)
/* Simplify some of assay field names. */
{
int sampleTrimSize = strlen("assay.");
int fullTrimSize = strlen(objectName) + 1;
int i;
for (i=0; i<table->fieldCount; ++i)
    {
    char *field = table->fields[i];
    if (endsWith(field, ".assay_id"))
	table->fields[i] = field + fullTrimSize;
    else
	table->fields[i] = field + sampleTrimSize;
    }
}


void oneRowTableSideways(struct fieldedTable *table, char *path)
/* Write out a table of a single row as a two column table with label/value */
{
if (table->rowCount != 1)
    errAbort("Expecting exactly one row in %s table, got %d\n", table->name, table->rowCount);
FILE *f = mustOpen(path, "w");
char **row = table->rowList->row;
int i;
struct dyString *scratch = dyStringNew(0);
for (i=0; i<table->fieldCount; ++i)
    {
    fprintf(f, "%s", table->fields[i]);
    char *pos = row[i], *val;
    while ((val = csvParseNext(&pos, scratch)) != NULL)
        fprintf(f, "\t%s", val);
    fprintf(f, "\n");
    }
dyStringFree(&scratch);
carefulClose(&f);
}

void slNameListToStringArray(struct slName *list, char ***retArray, int *retSize)
/* Given a slName list return an array of the names in it*/
{
char **array = NULL;
int size = slCount(list);
if (size > 0)
    {
    AllocArray(array, size);
    int i;
    struct slName *el;
    for (i=0, el=list; i<size; ++i, el=el->next)
        array[i] = el->name;
    }
*retArray = array;
*retSize = size;
}


void oneRowTableUnarray(struct fieldedTable *table, char *path)
/* Table has one row that look like:
 *    1.this 1.that 2.this 2.that
 * Convert this instead to multiple row table (in this case 2)
 *    this   that  */
{
/* Make sure we have exactly one row, and put that row in inRow variable. */
if (table->rowCount != 1)
    errAbort("Expecting exactly one row in %s table, got %d\n", table->name, table->rowCount);
char **inRow = table->rowList->row;

/* First make list of all fields after number is stripped off */
int i;
struct slName *fieldList = NULL;
struct hash *uniqHash = hashNew(0);
for (i=0; i<table->fieldCount; ++i)
    {
    char *numField = table->fields[i];
    char *field = strchr(numField, '.');
    if (!isdigit(numField[0]) || field == NULL)
        errAbort("Expecting something of format number.field,  got %s while making %s", 
	    numField, path);
    field += 1;  // Skip over dot.
    if (!hashLookup(uniqHash, field))
	{
	hashAddInt(uniqHash, field, uniqHash->elCount);
	slNameAddHead(&fieldList, field);
	}
    }
slReverse(&fieldList);

/* Create new fielded table */
char **fieldArray;
int fieldCount;
slNameListToStringArray(fieldList, &fieldArray, &fieldCount);
struct fieldedTable *ft = fieldedTableNew(table->name, fieldArray, fieldCount);
for (i=1; ; ++i)
    {
    char numPrefix[16];
    safef(numPrefix, sizeof(numPrefix), "%d.", i);
    int numPrefixSize = strlen(numPrefix);
    char *outRow[fieldCount];
    zeroBytes(outRow, sizeof(outRow));
    int j;
    int itemsInRow = 0;
    for (j=0; j<table->fieldCount; ++j)
        {
	char *numField = table->fields[j];
	if (startsWith(numPrefix, numField))
	    {
	    ++itemsInRow;
	    char *outField = numField + numPrefixSize;
	    int outRowIx = hashIntVal(uniqHash, outField);
	    outRow[outRowIx] = inRow[j];
	    }
	}
    if (itemsInRow == 0)
        break;
    fieldedTableAdd(ft, outRow, fieldCount, i);
    }
fieldedTableToTabFile(ft, path);
fieldedTableFree(&ft);
}

int endFromFileName(char *fileName)
/* Try and figure out end from file name */
{
if (stringIn("_R1_", fileName))
    return 1;
else if (stringIn("_R2_", fileName))
    return 2;
else if (stringIn("_1.", fileName))
    return 1;
else if (stringIn("_2.", fileName))
    return 2;
return 1;
}

int laneFromFileName(char *fileName)
/* Deduce lane from file name.  If there is something of form _L123_ in the midst of
 * file name we'll call it lane.  Otherwise return 0 (they start counting at 1) */
{
char *pat = "_L";
int patLen = strlen(pat);
char *s = fileName;
while ((s = stringIn(pat, s)) != NULL)
    {
    s += patLen;
    char *e = strchr(s, '_');
    if (e == NULL)
        break;
    int midSize = e - s;
    if (midSize == 3)
        {
	char midBuf[midSize+1];
	memcpy(midBuf, s, midSize);
	midBuf[midSize] = 0;
	if (isAllDigits(midBuf))
	    return atoi(midBuf);
	}
    }
return 0;
}

void outputFileTable(struct fieldedTable *table, char *path)
/* Output file table.  Mostly this has to do with unpacking the files[] array */
{
/* Open output file, we'll create it ourselves rather than going through fieldedTable */
FILE *f = mustOpen(path, "w");

/* Most of this routine involves unpacking the files field, which may be an
 * array.  So, let's figure out where the files field is: */
int filesIx = stringArrayIx("files", table->fields, table->fieldCount);
if (filesIx < 0)
    errAbort("Can't find files field");

/* We'll output the fields we derive from files first, and then the rest of them, starting
 * here with the label row. */
fprintf(f, "filename\tfile_format\tseq.lanes.number\tseq.lanes.run");
int i;
for (i=0; i<table->fieldCount; ++i)
    {
    if (i != filesIx)
        fprintf(f, "\t%s", table->fields[i]);
    }
fprintf(f, "\n");
       
/* Now loop through the rows... */
struct fieldedRow *fr;
for (fr = table->rowList; fr != NULL; fr = fr->next)
    {
    char **row = fr->row;
    char *files = row[filesIx];
    struct slName *urlList = csvParse(files);
    struct slName *url;
    for (url = urlList; url != NULL; url = url->next)
	{
	char baseName[FILENAME_LEN], ext[FILEEXT_LEN];
	splitPath(url->name, NULL, baseName, ext);
	fprintf(f, "%s%s\tfastq.gz\t", baseName, ext);
	int lane = laneFromFileName(baseName);
	if (lane > 0)
	    fprintf(f, "%d", lane);
	int end = endFromFileName(baseName);
	fprintf(f, "\tr%d", end);
	for (i=0; i<table->fieldCount; ++i)
	    {
	    if (i != filesIx)
		fprintf(f, "\t%s", row[i]);
	    }
	fprintf(f, "\n");
	}
    slFreeList(urlList);
    }
carefulClose(&f);
}


void addIdForUniqueVals(struct tagStorm *tags, struct tagStanza *stanzaList,
    struct slName *fieldList, char *idTag, char *objType, struct hash *uniqueHash,
    struct dyString *scratch)
/* Go through tagStorm creating and adding IDs on leaf nodes making it so that
 * leaves that have the same value for all fields that begin with conv->fieldPrefix
 * have the same id. */
{
struct tagStanza *stanza;
for (stanza = stanzaList; stanza != NULL; stanza = stanza->next)
    {
    if (stanza->children != NULL)
        addIdForUniqueVals(tags, stanza->children, fieldList, idTag, objType, uniqueHash, scratch);
    else
        {
	/* Create concatenation of all fields in scratch */
	dyStringClear(scratch);
	struct slName *field;
	for (field = fieldList; field != NULL; field = field->next)
	    {
	    dyStringAppendC(scratch, 1);  // use ascii 1 as rare separator
	    dyStringAppend(scratch, emptyForNull(tagFindVal(stanza, field->name)));
	    }

	/* Use it to look up id, creating a new ID if need be. */
	char *idVal = hashFindVal(uniqueHash, scratch->string);
	if (idVal == NULL)
	    {
	    char buf[64];
	    safef(buf, sizeof(buf), "%s_%d", objType, uniqueHash->elCount + 1);
	    idVal = cloneString(buf);
	    hashAdd(uniqueHash, scratch->string, idVal);
	    }
	/* Add id tag to stanza. */
	tagStanzaAdd(tags, stanza, idTag, idVal);
	}
    }
}

struct slName *subsetMatchingPrefix(struct slName *fullList, char *prefix)
/* Return a new list that contains all elements of fullList that start with prefix */
{
struct slName *subList = NULL, *el;
for (el = fullList; el != NULL; el = el->next)
    {
    if (startsWith(prefix, el->name))
       slNameAddHead(&subList, el->name);
    }
slReverse(&subList);
return subList;
}

void addIdFieldForSubtype(struct tagStorm *tags, char *fieldPrefix, char *idValPrefix, char *idTag)
/* Add idTag to leaf nodes where have uniq values of fields starting with fieldPrefix. */
{
struct hash *uniqHash = hashNew(0);
struct dyString *scratch = dyStringNew(0);
struct slName *allFields = tagStormFieldList(tags);
struct slName *ourFields = subsetMatchingPrefix(allFields, fieldPrefix);
addIdForUniqueVals(tags, tags->forest, ourFields, idTag, idValPrefix, uniqHash, scratch);
slNameFreeList(&allFields);
slNameFreeList(&ourFields);
dyStringFree(&scratch);
hashFree(&uniqHash);
}


void renameArrayField(struct tagStorm *tags, char *oldPrefix, int maxIndex, char *oldSuffix,
    char *newPrefix, char *newSuffix)
/* Rename some array fields */
{
int i;
for (i=1; i<=maxIndex; ++i)
    {
    char oldTag[128], newTag[128];
    safef(oldTag, sizeof(oldTag), "%s%d%s", oldPrefix, i,oldSuffix);
    safef(newTag, sizeof(newTag), "%s%d%s", newPrefix, i, newSuffix);
    tagStormSubTags(tags, oldTag, newTag);
    }
}


boolean tagStormInAllFlattenedLeaves(struct tagStorm *tags, struct tagStanza *stanzaList, char *tag)
/* Return TRUE if tag is found in all leaf nodes either directly or via inheritance */
{
struct tagStanza *stanza;
for (stanza = stanzaList; stanza != NULL; stanza = stanza->next)
    {
    if (tagFindLocalVal(stanza, tag) == NULL)
        {
	if (stanza->children)
	    {
	    if (!tagStormInAllFlattenedLeaves(tags, stanza->children, tag))
		return FALSE;
	    }
	else
	    return FALSE;
	}
    }
return TRUE;
}

void hcaStormToTabDir(char *inTags, char *outDir)
/* hcaStormToTabDir - Convert a tag storm file to a directory full of tab separated files that 
 * express pretty much the same thing from the point of view of HCA V4 ingest. */
{
/* Load in tagStorm, list fields and report some stats to help reassure user */
struct tagStorm *tags = tagStormFromFile(inTags);
struct slName *origFieldList = tagStormFieldList(tags);
verbose(1, "Got %d tags and %d fields in %s\n", tagStormCountTags(tags), 
    slCount(origFieldList), inTags);

/* Rename some tags that are arrays or ontologies or arrays of ontologies 
 * but represented as simple or character delimited fields in the spreadsheet */
renameArrayField(tags, "sample.donor.disease.", 3, ".text", "sample.donor.disease.", "");
struct arrayToSepStringContext a2ssc;
a2ssc.arrayName = "sample.donor.disease";
a2ssc.separator = ",";
tagStormTraverse(tags, tags->forest, &a2ssc, stanzaArrayToSeparatedString);
a2ssc.arrayName = "project.experimental_design";
a2ssc.separator = ",";
stanzaArrayToSeparatedString(tags, tags->forest, &a2ssc);

/* Maybe this renaming should go into tagStorm, but it's a little not very pretty  */
tagStormSubTags(tags, "sample.donor.id", "sample.donor.sample_id");

/* Figure out what types of samples we have */
struct entitySubtype *sampleSubtypeList = 
	createEntitySubtypes(sampleSubtypePrefixes, ArraySize(sampleSubtypePrefixes), 
	    origFieldList, "sample.");
struct entitySubtype *lastSubtype = slLastEl(sampleSubtypeList);
verbose(1, "First sample subtype is %s, last is %s\n", sampleSubtypeList->name, lastSubtype->name);

/* Add in any missing IDs or derived from tags */
struct entitySubtype *subtype, *nextSubtype;
for (subtype = sampleSubtypeList; subtype != NULL; subtype = nextSubtype)
    {
    nextSubtype = subtype->next;
    boolean isDonor = sameString("donor", subtype->name);
    char idTag[128];
    safef(idTag, sizeof(idTag), "sample.%s.sample_id", subtype->name); 
    if (nextSubtype == NULL)
        {
	if (isDonor)  // Just use donor id
	    tagStormDeleteTags(tags, "sample.sample_id");
	else
	    tagStormSubTags(tags, "sample.sample_id", idTag);
	}
    else
        {
	char derivedTag[128];
	safef(derivedTag, sizeof(derivedTag), "sample.%s.derived_from", nextSubtype->name);
	if (!isDonor)  // Donor already has donor.sample_id
	    {
	    addIdFieldForSubtype(tags, subtype->fieldPrefix, subtype->name, idTag);
	    }
	tagStormCopyTags(tags, idTag, derivedTag);
	}
    }

/* Add in any species tags to all types of samples and delete original. */
for (subtype = sampleSubtypeList; subtype != NULL; subtype = subtype->next)
    {
    char *name = subtype->name;
    char newTag[128];
    safef(newTag, sizeof(newTag), "sample.%s.ncbi_taxon_id", name);
    tagStormCopyTags(tags, "sample.ncbi_taxon_id", newTag);
    safef(newTag, sizeof(newTag), "sample.%s.genus_species", name);
    tagStormCopyTags(tags, "sample.genus_species", newTag);
    }
tagStormDeleteTags(tags, "sample.ncbi_taxon_id");
tagStormDeleteTags(tags, "sample.genus_species");

/* Move several "naked" sample fields to the last sample part in the chain. */
char *fieldsForLastSample[] = {"supplementary_files", "protocol_ids", "name", "description",
    "sample_accessions.biosd_sample", "well.cell_type.ontology", "well.cell_type.text" };
int i;
for (i=0; i<ArraySize(fieldsForLastSample); ++i)
    {
    char *field = fieldsForLastSample[i];
    char *lastType = lastSubtype->name;
    char oldTag[128], newTag[128];
    safef(oldTag, sizeof(oldTag), "sample.%s", field);
    safef(newTag, sizeof(newTag), "sample.%s.%s", lastType, field);
    tagStormSubTags(tags, oldTag, newTag);
    }

/* Do some misc small changes to donor tab. */
tagStormSubTags(tags, "sample.donor.submitted_id", "sample.donor.name");
tagStormSubTags(tags, "sample.donor.id", "sample.donor.sample_id");

/* Figure out what types of assays we have */
struct entitySubtype *assaySubtypeList = 
	createEntitySubtypes(assaySubtypePrefixes, ArraySize(assaySubtypePrefixes), 
	    origFieldList, "assay.");

/* Make sure that assay.assay_id field is there, copying it from 
 * assay.seq.insdc_run if that is available, otherwise generate it from
 * all assay fields. */
if (!tagStormInAllFlattenedLeaves(tags, tags->forest, "assay.assay_id"))
    {
    if (tagStormInAllFlattenedLeaves(tags, tags->forest, "assay.seq.insdc_run"))
	{
	tagStormCopyTags(tags, "assay.seq.insdc_run", "assay.assay_id");
	}
    else
	{
	addIdFieldForSubtype(tags, "assay.", "assay", "assay.assay_id");
	}
    }

/* Copy assay.assay_id to all assay subtypes. */
for (subtype = assaySubtypeList; subtype != NULL; subtype = subtype->next)
    {
    char idTag[128];
    safef(idTag, sizeof(idTag), "assay.%s.assay_id", subtype->name); 
    tagStormCopyTags(tags, "assay.assay_id", idTag);
    }

/* Add file prefix to a lot of things we want in file tab. */
char sampleId[128];
safef(sampleId, sizeof(sampleId), "sample.%s.sample_id", lastSubtype->name);
tagStormCopyTags(tags, sampleId, "file.sample_id");
tagStormSubTags(tags, "assay.assay_id", "file.assay_id");
tagStormSubTags(tags, "assay.seq.insdc_experiment", "file.seq.insdc_experiment");
tagStormSubTags(tags, "assay.seq.insdc_run", "file.seq.insdc_run");
tagStormSubTags(tags, "assay.seq.files", "file.files");


/* Make initial cut of our conversion tag list */
struct convert *convList = makeConvertList();

/* Make a pass through tagStorm to figure out which fields belong to which tab-sep-file */
struct dyString *scratch = dyStringNew(0);
struct convContext cc = {convList, scratch};
tagStormTraverse(tags, tags->forest, &cc, collectFields);
verbose(2, "Collected fields ok it seems\n");

/* Fill in most of the rest of the convert data structure */
struct convert *conv;
int realConvCount = 0;
for (conv = convList; conv != NULL; conv = conv->next)
    {
    int fieldCount = conv->fieldHash->elCount;
    verbose(2, "%s has %d fields\n", conv->objectName, fieldCount);
    if (fieldCount > 0)
	{
	++realConvCount;

	/* Build up an array of fields from list */
	char *fieldArray[fieldCount];
	int i;
	struct slName *el;
	for (i=0, el=conv->fieldList; i < fieldCount; ++i, el = el->next)
	   fieldArray[i] = el->name;

	/* Create empty table with defined fields, and also other needed fields */
	conv->table = fieldedTableNew(conv->objectName, fieldArray, fieldCount);
	AllocArray(conv->scratchRow, fieldCount);
	conv->uniqRowHash = hashNew(0);
	}
    }
verbose(1, "%d of %d tabs have data\n", realConvCount, slCount(convList));

/* Make a second pass through storm filling in tables */
tagStormTraverse(tags, tags->forest, &cc, fillInTables);
verbose(2, "Survived fillInTables\n");

/* Massage field names in tables a bit */
for (conv = convList; conv != NULL; conv = conv->next)
    {
    struct fieldedTable *table = conv->table;
    if (table != NULL)
	{
	char *objectName = conv->objectName;
	if (startsWith("sample.", objectName))
	    trimSampleFieldNames(table, objectName);
	else if (startsWith("assay.", objectName))
	    trimAssayFieldNames(table, objectName);
	else
	    trimFieldNames(table, objectName);
	}
    }

/* Do output */
makeDirsOnPath(outDir);
for (conv = convList; conv != NULL; conv = conv->next)
    {
    struct fieldedTable *table = conv->table;
    if (table != NULL)
	{
	char path[PATH_LEN];
	safef(path, sizeof(path), "%s/%s.tsv", outDir, conv->tabName);
	verbose(2, "Creating %s with %d columns and %d rows\n", path, 
	    table->fieldCount, table->rowCount);
	table->startsSharp = FALSE;
	char *objectName = conv->objectName;
	if (sameString(objectName, "project"))
	   oneRowTableSideways(table, path);
	else if (sameString(objectName, "project.publications"))
	   oneRowTableUnarray(table, path);
	else if (sameString(objectName, "protocols"))
	   oneRowTableUnarray(table, path);
	else if (sameString(objectName, "project.submitters"))
	   {
	   safef(path, sizeof(path), "%s/%s.tsv", outDir, "contact.submitter");
	   oneRowTableUnarray(table, path);
	   }
	else if (sameString(objectName, "project.contributors"))
	   {
	   safef(path, sizeof(path), "%s/%s.tsv", outDir, "contact.contributors");
	   oneRowTableUnarray(table, path);
	   }
	else if (sameString(objectName, "file"))
	   {
	   outputFileTable(table, path);
	   }
	else
	   fieldedTableToTabFile(table, path);
	}
    }
verbose(1, "Wrote %d files to %s\n", realConvCount, outDir);
}

int main(int argc, char *argv[])
/* Process command line. */
{
optionInit(&argc, argv, options);
if (argc != 3)
    usage();
hcaStormToTabDir(argv[1], argv[2]);
return 0;
}
