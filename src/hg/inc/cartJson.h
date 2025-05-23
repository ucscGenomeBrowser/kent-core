/* cartJson - parse and execute JSON commands to update cart and/or return cart data as JSON. */

#ifndef CARTJSON_H
#define CARTJSON_H

#include "cart.h"
#include "hash.h"
#include "jsonWrite.h"
#include "hgFind.h"

#define CARTJSON_COMMAND "cjCmd"

struct cartJson
    // Use the cart to perform commands dispatched from handlerHash; print results with jsonWrite.
    {
    struct cart *cart;
    struct hash *handlerHash;
    struct jsonWrite *jw;
    };

typedef void CartJsonHandler(struct cartJson *cj, struct hash *paramHash);
/* Implementation of some command; paramHash associates parameter names with
 * jsonElement values.  For use with cartJsonRegisterHandler. */

struct cartJson *cartJsonNew(struct cart *cart);
/* Allocate and return a cartJson object with default handlers.
 * cart must have "db" set already. */

char *cartJsonParamDefault(struct hash *paramHash, char *name, char *defaultVal);
/* Convenience function for a CartJsonHandler function: Look up name in paramHash.
 * Return the string contained in its jsonElement value, or defaultVal if not found. */

char *cartJsonOptionalParam(struct hash *paramHash, char *name);
/* Convenience function for a CartJsonHandler function: Look up name in paramHash.
 * Return the string contained in its jsonElement value, or NULL if not found. */

char *cartJsonRequiredParam(struct hash *paramHash, char *name, struct jsonWrite *jw,
                            char *context);
/* Convenience function for a CartJsonHandler function: Look up name in paramHash.
 * If found, return the string contained in its jsonElement value.
 * If not, write an error message (using context) and return NULL. */

void cartJsonRegisterHandler(struct cartJson *cj, char *command, CartJsonHandler *handler);
/* Associate handler with command; when javascript sends command, handler will be executed. */

boolean cartJsonIsNoWarns();
/* Return TRUE if there are no warnings present */

void cartJsonPrintWarnings(struct jsonWrite *jw);
/* If there are warnings, write them out as JSON: */

void cartJsonPushErrHandlers();
/* Push warn and abort handlers for errAbort. */

void cartJsonPopErrHandlers();
/* Pop warn and abort handlers for errAbort. */

void cartJsonExecute(struct cartJson *cj);
/* Get commands from cgi, print Content-type, execute commands, print results as JSON. */

char *cartJsonDumpJson(struct cartJson *cj);
/* Return the string that has been built up so far or an empty string */

void cartJsonFree(struct cartJson **pCj);
/* Close **pCj's contents and nullify *pCj. */

struct hgPositions *genomePosCJ(struct jsonWrite *jw, char *db, char *spec, char **retChromName,
                               int *retWinStart, int *retWinEnd, struct cart *cart, struct searchCategory *categories, boolean categorySearch);
/* Search for positions in genome that match user query.
 * Return an hgp unless there is a problem.  hgp->singlePos will be set if a single
 * position matched. categorySearch determines whether to error out if we can't
 * uniquely determine a position for a multiTerm search
 * Otherwise display list of positions, put # of positions in retWinStart,
 * and return NULL. */

void hgPositionsJson(struct jsonWrite *jw, char *db, struct hgPositions *hgp, struct cart *cart);
/* Write out JSON description of multiple position matches. */

#endif /* CARTJSON_H */
