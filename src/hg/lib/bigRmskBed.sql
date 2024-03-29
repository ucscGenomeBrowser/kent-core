# bigRmskBed.sql was originally generated by the autoSql program, which also 
# generated bigRmskBed.c and bigRmskBed.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Repetitive Element Annotation
CREATE TABLE bigRmskBed (
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold
    chromStart int unsigned not null,	# Start position of visualization on chromosome
    chromEnd int unsigned not null,	# End position of visualation on chromosome
    name varchar(255) not null,	# Name repeat, including the type/subtype suffix
    score int unsigned not null,	# Divergence score
    strand char(1) not null,	# + or - for strand
    thickStart int unsigned not null,	# Start position of aligned sequence on chromosome
    thickEnd int unsigned not null,	# End position of aligned sequence on chromosome
    reserved int unsigned not null,	# Reserved
    blockCount int unsigned not null,	# Count of sequence blocks
    blockSizes longblob not null,	# A comma-separated list of the block sizes(+/-)
    blockStarts longblob not null,	# A comma-separated list of the block starts(+/-)
    id int unsigned not null,	# A unique identifier for the joined annotations in this record
    description longblob not null,	# A comma separated list of technical annotation descriptions
              #Indices
    PRIMARY KEY(chrom)
);
