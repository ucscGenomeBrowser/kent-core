# transMapInfo.sql was originally generated by the autoSql program, which also 
# generated transMapInfo.c and transMapInfo.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#genomeDB information about transMap alignments
CREATE TABLE transMapInfo (
    mappedId varchar(255) not null,	# id of mapped alignment
    srcDb char(16) not null,	# source db
    srcId varchar(255) not null,	# id of source alignment
    mappingId varchar(255) not null,	# id of chain used for mapping
    chainSubset enum("unknown", "all", "syn", "rbest") not null,	# chain subset used
              #Indices
    PRIMARY KEY(mappedId)
);
