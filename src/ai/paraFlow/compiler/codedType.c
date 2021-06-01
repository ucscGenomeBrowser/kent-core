/* codedType - Types in form that can be used at run time. */

#include "common.h"
#include "hash.h"
#include "dystring.h"
#include "pfType.h"
#include "pfParse.h"
#include "pfCompile.h"
#include "backEnd.h"
#include "codedType.h"

struct codedBaseType 
/* Base type in form that can be saved in static structure. */
    {
    struct codedBaseType *next;
    int id;	  /* Unique identifier. */
    int scope;	  /* Scope this is declared in. */
    char *name;   /* Class or builtin type name. */
    int parentId; /* Id of parent. */
    bool needsCleanup;	/* True if it's an object or string. */
    bool size;		/* Type size. */
    struct pfBaseType *base;	/* Unencoded version */
    int label;		/* Assembly language label. */
    };

struct codedType *codedTypeNew(char *code, struct pfType *type)
/* Make new coded type */
{
static int cotId = 0;
struct codedType *cot;
AllocVar(cot);
cot->id = cotId++;
cot->type = type;
return cot;
}

static void rEncodeType(struct pfType *type, struct dyString *dy)
/* Encode type recursively into dy. */
{
dyStringPrintf(dy, "%s", type->base->name);
if (type->children != NULL)
    {
    dyStringAppendC(dy, '(');
    for (type = type->children; type != NULL; type = type->next)
        {
	rEncodeType(type, dy);
	if (type->next != NULL)
	    dyStringAppendC(dy, ',');
	}
    dyStringAppendC(dy, ')');
    }
}

void encodeType(struct pfType *type, struct dyString *dy)
/* Encode type into dy. */
{
dyStringClear(dy);
rEncodeType(type, dy);
}


static struct codedType *codedTypeLookup(struct hash *hash, 
	struct dyString *dy, struct pfType *type)
/* Find compOutType that corresponds to type in hash */
{
encodeType(type, dy);
return hashMustFindVal(hash, dy->string);
}

int codedTypeId(struct pfCompile *pfc, struct pfType *type)
/* Return run time type ID associated with type */
{
struct dyString *dy = dyStringNew(256);
struct codedType *cot = codedTypeLookup(pfc->runTypeHash, dy, type);
int id = cot->id;
dyStringFree(&dy);
return id;
}

static void printAndSaveCompType(struct pfCompile *pfc, FILE *f, 
	struct hash *hash, struct pfType *type, char *encoding)
/* Save to hash and print out to file run-time encoding of type. */
{
struct codedType *cot = codedTypeNew(encoding, type);
hashAddSaveName(hash, encoding, cot, &cot->code);
fprintf(f, "  {%d, \"%s\"},\n", cot->id, cot->code);
}

static void addTypesAndChildTypes(struct pfCompile *pfc, FILE *f, 
	struct hash *hash, struct dyString *dy, struct pfType *ty)
{
struct pfType *t;
for (t = ty->children; t != NULL; t = t->next)
    addTypesAndChildTypes(pfc, f, hash, dy, t);
encodeType(ty, dy);
if (!hashLookup(hash, dy->string))
    printAndSaveCompType(pfc, f, hash, ty, dy->string);
}

static void rFillCompHash(struct pfCompile *pfc, FILE *f,
	struct hash *hash, struct dyString *dy, struct pfParse *pp)
/* Fill in hash with uniq types.  Print encodings of unique
 * types as we find them to file. */
{
struct pfParse *p;
for (p = pp->children; p != NULL; p = p->next)
    rFillCompHash(pfc, f, hash, dy, p);
if (pp->ty)
    {
    encodeType(pp->ty, dy);
    if (!hashLookup(hash, dy->string))
	addTypesAndChildTypes(pfc, f, hash, dy, pp->ty);
    }
}

static struct codedBaseType *cbtFind(struct codedBaseType *list,
	int scope, char *name)
/* Search through list to find one that matches scope and name. */
{
struct codedBaseType *el;
for (el = list; el != NULL; el = el->next)
    if (el->scope == scope && sameString(el->name, name))
        return el;
errAbort("Can't cbtFind %d:%s", scope, name);
return NULL;
}

static void saveSimpleTypes(struct pfCompile *pfc, FILE *f, struct hash *hash,
	struct dyString *dy, struct codedBaseType *cbt)
/* Make a type out of base type, and print and save it.
 * Also do this for arrays and dirs of that type. */
{
struct pfType *type = pfTypeNew(cbt->base), *dirType, *arrayType;
encodeType(type, dy);
printAndSaveCompType(pfc, f, hash, type, dy->string);
}

static struct hash *hashPrintCompType(struct pfCompile *pfc, 
	struct codedBaseType *cbtList, struct pfParse *program, 
	struct dyString *dy, FILE *f)
/* Create a hash full of codedTypes.  Also print out type 
 * encodings. */
{
struct hash *hash = hashNew(0);
struct codedBaseType *cbt;
struct codedBaseType *intCbt = cbtFind(cbtList, 1, "int");
struct codedBaseType *stringCbt = cbtFind(cbtList, 1, "string");
struct codedBaseType *seriousErrCbt = cbtFind(cbtList, 1, "seriousError");
struct codedBaseType *errCbt = cbtFind(cbtList, 1, "error");

/* Make up int and string types, serious error, and error types. The runtime 
 * depends on these being in this order as the first elements of the 
 * type array. */
saveSimpleTypes(pfc, f, hash, dy, intCbt);
saveSimpleTypes(pfc, f, hash, dy, stringCbt);
saveSimpleTypes(pfc, f, hash, dy, seriousErrCbt);
saveSimpleTypes(pfc, f, hash, dy, errCbt);

rFillCompHash(pfc, f, hash, dy, program);
return hash;
}

static void rPrintTypedFields(struct hash *compTypeHash, 
	struct dyString *dy, struct pfBaseType *base, struct dyString *fieldDy)
/* Recursively print fields. */
{
struct pfType *field;
if (base->parent != NULL)
    rPrintTypedFields(compTypeHash, dy, base->parent, fieldDy);
for (field = base->fields; field != NULL; field = field->next)
    {
    struct codedType *cot = codedTypeLookup(compTypeHash, dy, field);
    if (fieldDy->stringSize)
	dyStringAppendC(fieldDy, ',');
    dyStringPrintf(fieldDy, "%d:%s", cot->id, field->fieldName);
    }
}

int codedBaseTypeCmpId(const void *va, const void *vb)
/* Compare to sort based on id. */
{
const struct codedBaseType *a = *((struct codedBaseType **)va);
const struct codedBaseType *b = *((struct codedBaseType **)vb);
return a->id - b->id;
}

static struct codedBaseType *getBaseTypes(struct pfCompile *pfc)
/* Get list of all abse types in program sorted by ID */
{
struct codedBaseType *cbtList = NULL, *cbt;
struct slRef *ref;
/* Get list of base types sorted by id. */
for (ref = pfc->scopeRefList; ref != NULL; ref = ref->next)
    {
    struct pfScope *scope = ref->val;
    struct hashEl *hel, *helList = hashElListHash(scope->types);
    int scopeId = scope->id;
    for (hel = helList; hel != NULL; hel = hel->next)
        {
	struct pfBaseType *base = hel->val;
	if (base->scope == scope)  /* Avoid included ones. */
	    {
	    AllocVar(cbt);
	    slAddHead(&cbtList, cbt);
	    cbt->id = base->id;
	    cbt->scope = scopeId;
	    cbt->name = base->name;
	    if (base->parent != NULL)
		cbt->parentId = base->parent->id;
	    cbt->needsCleanup = base->needsCleanup;
	    cbt->size = base->size;
	    cbt->base = base;
	    }
	}
    hashElFreeList(&helList);
    }
slSort(&cbtList, codedBaseTypeCmpId);
return cbtList;
}

#define pfBaseInfoName "_pf_base_info"
#define pfBaseInfoCountName "_pf_base_info_count"
#define pfTypeInfoName "_pf_type_info"
#define pfTypeInfoCountName "_pf_type_info_count"
#define pfFieldInfoName "_pf_field_info"
#define pfFieldInfoCountName "_pf_field_info_count"


struct hash *codedTypesToC(struct pfCompile *pfc, 
	struct pfParse *program, FILE *f)
/* Traverse parse tree and encode all types referenced in it.
 * Also print out the types in C structures that the runtime
 * system can interpret. */
{
struct pfScope *scope;
struct hash *compTypeHash;
struct dyString *dy = dyStringNew(0);
struct dyString *fieldDy = dyStringNew(0);
struct slRef *ref;
struct codedBaseType *cbt, *cbtList = getBaseTypes(pfc);

/* Write out base types in a C table. */
fprintf(f, "/* All base types */\n");
fprintf(f, "struct _pf_base_info %s[] = {\n", pfBaseInfoName);
for (cbt = cbtList; cbt != NULL; cbt = cbt->next)
    {
    fprintf(f, "  {%d, ", cbt->id);
    fprintf(f, "\"%d:", cbt->scope);
    fprintf(f, "%s\", ", cbt->name);
    fprintf(f, "%d, ", cbt->parentId);
    fprintf(f, "%d, ", cbt->needsCleanup);
    fprintf(f, "%d, ", cbt->size);
    fprintf(f, "},\n");
    }
fprintf(f, "};\n");
fprintf(f, "int %s = %d;\n\n", pfBaseInfoCountName, slCount(cbtList));

fprintf(f, "/* All composed types */\n");
fprintf(f, "struct _pf_type_info %s[] = {\n", pfTypeInfoName);
compTypeHash = hashPrintCompType(pfc, cbtList, program, dy, f);
fprintf(f, "};\n");
fprintf(f, "int %s = sizeof(%s)/sizeof(%s[0]);\n\n",
	pfTypeInfoCountName, pfTypeInfoName, pfTypeInfoName);

fprintf(f, "/* All field lists. */\n");
fprintf(f, "struct _pf_field_info %s[] = {\n", pfFieldInfoName);
for (ref = pfc->scopeRefList; ref != NULL; ref = ref->next)
    {
    struct pfScope *scope = ref->val;
    struct hashEl *hel, *helList = hashElListHash(scope->types);
    int scopeId = scope->id;
    slSort(&helList, hashElCmp);
    for (hel = helList; hel != NULL; hel = hel->next)
        {
	struct pfBaseType *base = hel->val;
	if (base->isClass)
	    {
	    if (base->scope == scope)
		{
		fprintf(f, "  {%d, ", base->id);
		fprintf(f, "\"");
		dyStringClear(fieldDy);
		rPrintTypedFields(compTypeHash, dy, base, fieldDy);
		fprintf(f, "%s", fieldDy->string);
		fprintf(f, "\"");
		fprintf(f, "},\n");
		}
	    }
	}
    hashElFreeList(&helList);
    }
fprintf(f, "};\n");
fprintf(f, "int %s = sizeof(%s)/sizeof(%s[0]);\n\n",
	pfFieldInfoCountName, pfFieldInfoName, pfFieldInfoName);
fprintf(f, "\n");

dyStringFree(&fieldDy);
dyStringFree(&dy);
return compTypeHash;
}

