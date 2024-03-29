# quickLiftChain.sql was originally generated by the autoSql program, which also 
# generated quickLiftChain.c and quickLiftChain.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#bigChain files used by quickLift. */
CREATE TABLE quickLiftChain (
    id int unsigned not null auto_increment,	# Auto-incrementing ID
    fromDb varchar(255) not null,	# Short name of 'from' database.  'hg15' or the like
    toDb varchar(255) not null,	# Short name of 'to' database.  'hg16' or the like
    path longblob not null,	# Path to bigChain file
              #Indices
    PRIMARY KEY(id),
    INDEX(fromDb),
    INDEX(toDb)
);
