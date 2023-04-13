/* mdbPrint - Prints metadata objects and variables from the mdb metadata table. */

/* Copyright (C) 2013 The Regents of the University of California 
 * See kent/LICENSE or http://genome.ucsc.edu/license/ for licensing information. */
#include "common.h"
#include "linefile.h"
#include "dystring.h"
#include "options.h"
#include "mdb.h"


#define OBJTYPE_DEFAULT "table"

void usage()
/* Explain usage and exit. */
{
errAbort(
"mdbPrint - Prints metadata objects, variables and values from '" MDB_DEFAULT_NAME "' table.\n"
"usage:\n"
"   mdbPrint {db} [-table=] [-byVar] [-line/-count]\n"
"                 [-all]\n"
"                 [-vars=\"var1=val1 var2=val2...\"]\n"
"                 [-obj= [-var= [-val=]]]\n"
"                 [-var= [-val=]]\n"
"                 [-specialHelp]\n"
"Options:\n"
"    {db}     Database to query metadata from.  This argument is required.\n"
"    -table   Table to query metadata from.  Default is the sandbox version of\n"
"             '" MDB_DEFAULT_NAME "'.\n"
"    -byVar   Print each var and val, then all objects that match, as\n"
"             opposed to printing objects and all the var=val pairs that match.\n"
"    -ra      Default. Print each obj with set of indented var val pairs on\n"
"              separate lines and objects as a stanzas (-byVar prints pseudo-RA).\n"
"    -line      Print each obj and all var=val pairs on a single line.\n"
"    -count      Just print count of objects, variables and values selected.\n"
"    -cv        Specify CV file path (e.g. from sandbox)"
"    -validate    Validate mdb objects against cv.ra. (Incompatible with -byVars, -ra, -line.)\n"
"    -validateFull like validate but considers vars not defined in cv as invalid.\n"
"    -experimentify      Groups objs into experiments defined in encodeExp table.\n"
"    -encodeExp={table}  Optionally tell which encodeExp table to use.\n"
"    -specialHelp    Prints help for some special case features.\n"
"  Four alternate ways to select metadata:\n"
"    -all       Will print entire table (this could be huge).\n"
"    -vars={var=val...}  Request a combination of var=val pairs.\n\n"
"        Use: 'var=val'  'var=v%%'  'var='  'var=val1,val2' (val1 or val2).\n"
"             'var!=val' 'var!=v%%' 'var!=' 'var!=val1,val2' are all supported.\n"
"    -obj={objName}  Request a single object.  Can be narrowed by var and val.\n"
"    -var={varName}  Request a single variable.  Can be narrowed by val.\n"
"There are two basic views of the data: by objects and by variables.  The default view "
"is by object.  Each object will print out in an RA style stanza (by default) or as "
"a single line of output containing all var=val pairs. In 'byVar' view, each RA style "
"stanza holds a var val pair and all objects belonging to that pair on separate lines.  "
"Linear 'byVar' view puts the entire var=val pair on one line.  Alternatively, request "
"only counting of objects, variables or values.\n"
"HINT: Use '%%' in any obj, var or val as a wildcard for selection.\n\n"
"Examples:\n"
"  mdbPrint hg19 -vars=\"grant=Snyder cell=GM12878 antibody=CTCF\"\n"
"           Return all objs that satify ALL of the constraints.\n"
"  mdbPrint mm9 -vars=\"grant=Snyder cell=GM12878 antibody=?\" -byVar\n"
"           Return each all vars for all objects with the constraint.\n"
"  mdbPrint hg18 -obj=wgEncodeUncFAIREseqPeaksPanislets -line\n"
"           Return a single formatted metadata line for one object.\n"
"  mdbPrint hg18 -count -var=cell -val=GM%%\n"
"           Return the count of objects which have a cell begining with 'GM'.\n");
}

static struct optionSpec optionSpecs[] = {
    {"table",    OPTION_STRING}, // default "metaDb"
    {"ra",       OPTION_BOOLEAN},// ra format
    {"line",     OPTION_BOOLEAN},// linear format
    {"composite",OPTION_STRING}, // Special case of a common var 
                                 //  (replaces vars="composite=wgEncodeBroadHistone")
    {"count",    OPTION_BOOLEAN},// returns only counts of objects, vars and vals
    {"counts",   OPTION_BOOLEAN},// sames as count
    {"cv",   OPTION_STRING},// specify CV file path
    {"all",      OPTION_BOOLEAN},// query entire table
    {"byVar",    OPTION_BOOLEAN},// With -all prints from var perspective
    {"specialHelp",OPTION_BOOLEAN},// Certain very specialized features are described
    {"obj",      OPTION_STRING}, // objName or objId
    {"var",      OPTION_STRING}, // variable
    {"val",      OPTION_STRING}, // value
    {"validate", OPTION_BOOLEAN},// Validate vars and vals against cv.ra terms
    {"validateFull", OPTION_BOOLEAN},// Like validate but considers vars not in cv as invalid
    {"experimentify",OPTION_BOOLEAN},// Validate Experiments as defined in hgFixed.encodeExp table
    {"encodeExp",OPTION_STRING},     // Optionally tell which encodeExp to use
    {"vars",     OPTION_STRING},// var1=val1 var2=val2...
    {"or",       OPTION_STRING},// or var1=val1 var2=val2...
    {"order",    OPTION_STRING}, // comma delimited list of vars to order result by
    {"separator",OPTION_STRING}, // Optional separator used with order
    {"header",   OPTION_BOOLEAN},// Optional inclusion of header used with order
    {"updDb",    OPTION_STRING},// DB to update
    {"updMdb",   OPTION_STRING},// MDB table to update
    {"updSelect",OPTION_STRING},// Experiment defining vars: "var1,var2"
    {"updVars",  OPTION_STRING},// Vars to update: "var1,var2"
    {NULL,       0}
};

void specialHelp()
/* Explain usage and exit. */
{
errAbort(
"mdbPrint - Extra help for specialty selectors and output for metadata.\n"
"usage:\n"
"   mdbPrint {db} -composite={} -vars=\"var1=val1 || var1=val2...\" [-or=\"var3=val3...\" ]\n"
"            -order={var1,var2,...} [-separator={\"any string\"} [-header]\n"
"            -updDB={db} -updMdb={metaDb} -updSelect=var1,var2,... -updVars=varA,varB,...\n"
"Options:\n"
"    {db}     Database to query metadata from.  This argument is required.\n"
"    -composite={}   Special commonly used var=val pair replaces -vars=\"composite=wgEn...\".\n"
"    -vars={var=val...}  Request a combination of var=val pairs.\n"
"             Use: 'var!=val', 'var=v%%', 'var=v1,v2' (same as 'var=v1 || var=v2').\n"
"    -or={var=val...}  When selecting by -obj or -vars can add orthoganal 'or' condition.\n"
"             Used for more complex selections. Note -composite will be common to both queries.\n"
"Special functions:\n"
"  Print ordered vars:\n"
"    -order       Prints only vars named and in the order named (comma delimited).\n"
"    -separator   Optional, examples: \" \", \"\\t\\t\", \"<td>\" (will make html table).\n"
"    -header      Optionally include header.\n"
"  Print mdbUpdate lines to assist importing metadata from one db.table to another:\n"
"    -updDb      Database to aim mdbUpdate lines at.\n"
"    -updMdb     The metaDb table to aim mdbUpdate lines at.\n"
"    -updSelect  A comma separated list of variables that will be selected with\n"
"                the mdbUpdate (via '-vars').\n"
"    -updVars    A comma separated list of variables that will be set in the\n"
"                mdbUpdate lines (via '-setVars').\n"
"The purpose of this special option is to generate mdbUpdate commands from existing metadata.\n"
"Examples:\n"
"  mdbPrint hg18 -vars=\"composite=wgEncodeYaleChIPseq\" -updDb=hg19 -updMdb=metaDb_braney\n"
"    (cont.)  -updSelect=grant,cell,antibody -updVars=dateSubmitted,dateUnrestricted\n"
"           This command assists importing dateSubmitted from hg18 to hg19 for all\n"
"           objects in hg19 that match the grant, cell and antibody of\n"
"           objects in hg18.  It would result in output that looks something like:\n"
"     mdbUpdate hg19 -table=metaDb_cricket -vars=\"grant=Snyder cell=GM12878 antibody=c-Fos\"\n"
"       (cont.) -setVars=\"dateSubmitted=2009-02-13 dateUnrestricted=2009-11-12\" -test\n"
"     mdbUpdate hg19 -table=metaDb_braney -vars=\"grant=Snyder cell=GM12878 antibody=c-Jun\"\n"
"       (cont.) -setVars=\"dateSubmitted=2009-01-08 dateUnrestricted=2009-08-07\" -test\n"
"     mdbUpdate hg19 ...\n"
"           Note the '-test' in output to allow confirmation of effects before actual update.\n"
"  mdbPrint hg18 -vars=\"composite=wgEncodeYaleChIPseq view=RawSignal\" -updDb=hg18\n"
"    (cont.)     -updMdb=metaDb_vsmalladi -updSelect=obj -updVars=fileName\n"
"           You can select by object too (but not in combination with other vars).\n"
"           This example shows how to assist updating vals to the same mdb, where an editor\n"
"           or awk will also be needed.\n");
}

static void mdbObjPrintUpdateLines(struct mdbObj **mdbObjs,char *dbToUpdate,
                                   char *tableToUpdate, char *varsToSelect,char *varsToUpdate)
// prints mdbUpdate lines to allow taking vars from one db to another (sorts mdbObjs
// so pass pointer). Specialty print for importing vars from one db or table to another
{
if (dbToUpdate == NULL || tableToUpdate == NULL || varsToSelect == NULL || varsToUpdate == NULL)
    errAbort("mdbObjPrintUpdateLines is missing important parameter.\n");

// Parse variables that will be used to select mdb objects
// varsToSelect is comma delimited string of var names.  Vals are discovered in each obj
int selCount = 0;
char **selVars = NULL;
if (differentWord(varsToSelect,"obj"))
    {
    // Sort objs to avoid duplicate mdbUpdate statements
    mdbObjsSortOnVars(mdbObjs, varsToSelect);

    // Parse list of selcting vars (could be simply expId or expId,replicate,view)
    selCount = chopByChar(varsToSelect,',',NULL,0);
    if (selCount <= 0)
        errAbort("mdbObjPrintUpdateLines is missing experiment defining variables.\n");
    selVars = needMem(sizeof(char *) * selCount);
    selCount = chopByChar(varsToSelect,',',selVars,selCount);
    }

// Parse variables that will be updated in selected mdb objects
// varsToUpdate is comma delimited string of var names.  Vals are discovered in each obj
int updCount = chopByChar(varsToUpdate,',',NULL,0);
if (updCount <= 0)
    errAbort("mdbObjPrintUpdateLines is missing variables to set.\n");
char **updVars = needMem(sizeof(char *) * updCount);
updCount = chopByChar(varsToUpdate,',',updVars,updCount);
int ix=0;

// Special case when varsToUpdate contains ONLY expId={startingId}
boolean updExpId = (updCount == 1 && startsWithWordByDelimiter("expId",'=',updVars[0]));
int startingId=0;
if (updExpId)
    {
    startingId = sqlSigned(skipBeyondDelimit(updVars[0],'='));
    updVars[0][strlen("expId")] = '\0';
    }

// For each passed in obj, write an mdbUpdate statement
struct mdbObj *mdbObj = NULL;
struct dyString *thisSelection = dyStringNew(256);
struct dyString *lastSelection = dyStringNew(256);
for (mdbObj=*mdbObjs; mdbObj!=NULL; mdbObj=mdbObj->next)
    {
    if (mdbObj->obj == NULL || mdbObj->deleteThis)
        continue;

    // Build this selection string e.g. -vars="cell=GM23878 antibody=CTCF"
    dyStringClear(thisSelection);
    if (sameWord(varsToSelect,"obj"))
        {
        dyStringPrintf(thisSelection,"-obj=%s",mdbObj->obj);
        }
    else
        {
        dyStringAppend(thisSelection,"-vars=\"");
        for (ix = 0; ix < selCount; ix++)
            {
            char *val = mdbObjFindValue(mdbObj,selVars[ix]);
            if (val != NULL) // TODO what to do for NULLS?
                {
                if (strchr(val, ' ') != NULL) // Has blanks
                    dyStringPrintf(thisSelection,"%s='%s' ",selVars[ix],val);
                    // FIXME: Need to make single quotes work since already within double quotes!
                else
                    dyStringPrintf(thisSelection,"%s=%s ",selVars[ix],val);
                }
            }
        dyStringAppend(thisSelection,"\"");
        }

    // Don't bother making another mdpUpdate line if selection is the same.
    if (dyStringLen(lastSelection) > 0
    && sameString(dyStringContents(lastSelection),dyStringContents(thisSelection)))
        continue;
    dyStringClear(lastSelection);
    dyStringAppend(lastSelection,dyStringContents(thisSelection));

    printf("mdbUpdate %s table=%s %s",dbToUpdate,tableToUpdate,dyStringContents(thisSelection));

    // build the update string e.g. -setVars="dateSubmitted=2009-09-14 dateUnrestricted=2010-06-13"
    printf(" -setVars=\"");
    for (ix = 0; ix < updCount; ix++)
        {
        if (updExpId)
            printf("expId=%u",startingId++); // Special case expId is incrementing
        else
            {
            char *val = mdbObjFindValue(mdbObj,updVars[ix]);
            if (val != NULL) // What to do for NULLS? Ignore
                {
                printf("%s=",updVars[ix]);
                if (strchr(val, ' ') != NULL) // Has blanks
                    printf("'%s' ",val);
                else
                    printf("%s ",val);
                }
            }
        }
    printf("\" -test\n"); // Always test first
    }
dyStringFree(&thisSelection);
dyStringFree(&lastSelection);
}

static int sortCompositeFirst(const void *va, const void *vb)
/* Promote composite object type to head of mdb object list */
{
const struct mdbObj *a = *((struct mdbObj **)va);
const struct mdbObj *b = *((struct mdbObj **)vb);
if (mdbObjIsComposite((struct mdbObj *)a))
    return -1;
else if (mdbObjIsComposite((struct mdbObj *)b))
    return 1;
return 0;
}

int main(int argc, char *argv[])
// Process command line.
{
struct mdbObj   * mdbObjs   = NULL;
struct mdbByVar * mdbByVars = NULL;
int objsCnt=0, varsCnt=0,valsCnt=0;
int retCode = 0;

if (argc == 1)
    usage();

optionInit(&argc, argv, optionSpecs);

if (optionExists("specialHelp"))
    specialHelp();

if (argc < 2)
    {
    verbose(1, "REQUIRED 'DB' argument not found:\n");
    usage();
    }

char *db = argv[1];
char *table = optionVal("table",NULL);
char *order = optionVal("order",NULL);
char *orVars = NULL;

char *cv  = optionVal("cv",NULL);
if (cv != NULL)
    cvFileDeclare(cv);

boolean raStyle = TRUE;
if (optionExists("line") && !optionExists("ra"))
    raStyle = FALSE;

boolean justCounts = (optionExists("count") || optionExists("counts"));
boolean byVar = optionExists("byVar");
boolean validate = (optionExists("validate") || optionExists("validateFull"));

char *encodeExp = NULL;
if (optionExists("experimentify"))
    {
    encodeExp = optionVal("encodeExp","encodeExp");
    if (strlen(encodeExp) == 0 || sameWord("std",encodeExp))
        encodeExp = "encodeExp";
    }
else if (optionExists("encodeExp"))
    errAbort("-encodeExp option requires -experimentify option.\n");

if ((validate || encodeExp != NULL) 
&&  (byVar || optionExists("line") || optionExists("ra") || optionExists("order")))
    {
    verbose(1, "Incompatible to combine validate or experimentify option with "
               "'byVar', 'line', 'ra' or 'order':\n");
    usage();
    }

boolean all = optionExists("all");
if (all)
    {
    if (optionExists("obj")
    || optionExists("var") || optionExists("val") || optionExists("vars"))
        usage();
    }
else if (optionExists("obj"))
    {
    mdbObjs = mdbObjCreate(optionVal("obj",  NULL),optionVal("var", NULL), optionVal("val", NULL));
    }
else if (optionExists("vars"))
    {
    char *vars = optionVal("vars", NULL);
    orVars = strstr(vars," || ");
    if (orVars != NULL)
        {
        *orVars = '\0';
        orVars += 4;
        }
    mdbByVars = mdbByVarsLineParse(vars);
    if (optionExists("composite"))
        mdbByVarAppend(mdbByVars,"composite", optionVal("composite", NULL),FALSE);
    if (optionExists("var"))
        mdbByVarAppend(mdbByVars,optionVal("var", NULL), optionVal("val", NULL),FALSE);
    }
else if (optionExists("composite"))
    {
    mdbByVars = mdbByVarCreate("composite", optionVal("composite", NULL));
    if (optionExists("var"))
        mdbByVarAppend(mdbByVars,optionVal("var", NULL), optionVal("val", NULL),FALSE);
    }
else if (optionExists("var"))
    {
    mdbByVars = mdbByVarCreate(optionVal("var", NULL),optionVal("val", NULL));
    if (optionExists("composite"))
        mdbByVarAppend(mdbByVars,"composite", optionVal("composite", NULL),FALSE);
    }
else
    usage();

if (optionExists("or") || orVars != NULL)
    {
    if (byVar)
        errAbort("Unsupported to use -or with -byVar.\n");
    if (optionExists("or"))
        {
        if (!optionExists("vars") && !optionExists("obj"))
            errAbort("Incompatible to use -or without -vars or -obj'.\n");
        if (orVars != NULL)
            errAbort("Incompatible to use -or and ' || ' in -vars.\n");
        else
            orVars = optionVal("or", NULL);
        }
    }

struct sqlConnection *conn = sqlConnect(db);

// Find the table if necessary
if (table == NULL)
    {
    table = mdbTableName(conn,TRUE); // Look for sandBox name first
    if (table == NULL)
        errAbort("TABLE NOT FOUND: '%s.%s'.\n",db,MDB_DEFAULT_NAME);
    }
if (encodeExp != NULL)
    verbose(1, "Using tables named '%s.%s' and 'hgFixed.%s'.\n",db,table,encodeExp);
else
    verbose(1, "Using table named '%s.%s'.\n",db,table);

if (byVar)
    {
    if (!all && !validate && mdbByVars == NULL) // assertable
        usage();

    // Requested a single var
    struct mdbByVar * queryResults = mdbByVarsQuery(conn,table,mdbByVars);
    if (queryResults == NULL)
        verbose(1, "No metadata met your selection criteria\n");
    else
        {
        objsCnt=mdbByVarCount(queryResults,FALSE,FALSE);
        varsCnt=mdbByVarCount(queryResults,TRUE ,FALSE);
        valsCnt=mdbByVarCount(queryResults,FALSE,TRUE );
        if (!justCounts)
            mdbByVarPrint(queryResults,raStyle);
        mdbByVarsFree(&queryResults);
        }
    }
else
    {
    struct mdbObj * queryResults = NULL;
    if (mdbByVars != NULL)
        {
        // Requested a set of var=val pairs and looking for the
        // unique list of objects that have all of them!
        queryResults = mdbObjsQueryByVars(conn,table,mdbByVars);
        }
    else
        {
        // Requested a single obj
        queryResults = mdbObjQuery(conn,table,mdbObjs);
        }

    boolean resort = FALSE;
    while (orVars != NULL)
        {
        char *vars = orVars;
        orVars = strstr(vars," || ");
        if (orVars != NULL)
            {
            *orVars = '\0';
            orVars += 4;
            }
        struct mdbByVar * orByVars = orByVars = mdbByVarsLineParse(vars);
        if (optionExists("composite"))
            mdbByVarAppend(orByVars,"composite", optionVal("composite", NULL),FALSE);
        if (optionExists("var"))
            mdbByVarAppend(orByVars,optionVal("var", NULL), optionVal("val", NULL),FALSE);
        struct mdbObj * orResults = mdbObjsQueryByVars(conn,table,orByVars);
        if (orResults != NULL)
            {
            // Merge be removing dups from orResults and cating together.
            orResults = mdbObjIntersection(&orResults,queryResults);
            queryResults = slCat(queryResults,orResults);
            resort = TRUE;
            }
        }
    if (resort)
        slSort(&queryResults,&mdbObjCmp); // Need to be returned to obj order
    //if(optionExists("or"))
    //    {
    //    struct mdbByVar * orByVars = orByVars = mdbByVarsLineParse(optionVal("or", NULL));
    //    if (optionExists("composite"))
    //        mdbByVarAppend(orByVars,"composite", optionVal("composite", NULL),FALSE);
    //    if (optionExists("var"))
    //        mdbByVarAppend(orByVars,optionVal("var", NULL), optionVal("val", NULL),FALSE);
    //    struct mdbObj * orResults = mdbObjsQueryByVars(conn,table,orByVars);
    //    // Merge be removing dups from orResults and cating together.
    //    orResults = mdbObjIntersection(&orResults,queryResults);
    //    queryResults = slCat(queryResults,orResults);
    //    slSort(&queryResults,&mdbObjCmp); // Need to be returned to obj order
    //    }

    if (queryResults == NULL)
        {
        verbose(1, "No metadata met your selection criteria\n");
        retCode = 1;
        }
    else
        {
        objsCnt=mdbObjCount(queryResults,TRUE);
        varsCnt=mdbObjCount(queryResults,FALSE);
        valsCnt=varsCnt;
        if (!justCounts)
            {
            if (optionExists("updSelect")) // Special print of mdbUpdate lines
                {
                if (!optionExists("updDb") || !optionExists("updMdb") || !optionExists("updVars"))
                    errAbort("To print mdbUpdate lines, all the following values are needed: "
                             "'-updDb=' '-updMdb=' '-updSelect=' '-updVars='.\n");

                mdbObjPrintUpdateLines(&queryResults,optionVal("updDb",NULL),
                                       optionVal("updMdb",NULL),optionVal("updSelect",NULL),
                                       optionVal("updVars",NULL));
                }
            else if (encodeExp != NULL) // Organizes vars as experiments and validates expId values
                {
                struct mdbObj *updatable = mdbObjsEncodeExperimentify(conn,db,table,encodeExp,
                                                                     &queryResults,2,FALSE,FALSE);
                                                                    // 2=full experiments described
                printf("%d of %d obj%s can have their experiment IDs updated now.\n",
                       slCount(updatable),objsCnt,(objsCnt==1?"":"s"));
                if (slCount(updatable) < objsCnt)
                    retCode = 2;
                mdbObjsFree(&updatable);
                }
            else if (validate) // Validate vars and vals against cv.ra
                {
                int invalids = mdbObjsValidate(queryResults,optionExists("validateFull"));
                verbose(1,"%d invalid%s of %d variable%s\n",invalids,(invalids==1?"":"s"),
                        varsCnt,(varsCnt==1?"":"s"));
                if (invalids > 0)
                    retCode = 3;
                }
            else if (order != NULL)
                {
                char *sep = optionVal("separator",NULL);
                boolean header = optionExists("header");
                mdbObjPrintOrderedToStream(stdout,&queryResults,order, sep, header);
                }
            else
                {
                // Default operation here
                if (optionExists("composite") && !mdbObjIsComposite(queryResults))
                    /* Pull composite metaObject to head of list for readability */
                    slSort(&queryResults, &sortCompositeFirst);
                mdbObjPrint(queryResults,raStyle);
                }
            }
        mdbObjsFree(&queryResults);
        }
    }
sqlDisconnect(&conn);

if (justCounts)
    {
    printf("%d object%s\n",objsCnt,(objsCnt==1?"":"s"));
    printf("%d variable%s\n",varsCnt,(varsCnt==1?"":"s"));
    printf("%d value%s\n",valsCnt,(valsCnt==1?"":"s"));
    }
else if ( varsCnt>0 || valsCnt>0 || objsCnt>0 )
    {
    if (byVar)
        verbose(1,"vars:%d  vals:%d  objects:%d\n",varsCnt,valsCnt,objsCnt);
    else
        verbose(1,"objects:%d  vars:%d\n",objsCnt,varsCnt);
    }

if (mdbObjs)
    mdbObjsFree(&mdbObjs);
if (mdbByVars)
    mdbByVarsFree(&mdbByVars);

return retCode;
}
