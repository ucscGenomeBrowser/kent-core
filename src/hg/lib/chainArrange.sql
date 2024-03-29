# chainArrange.sql was originally generated by the autoSql program, which also 
# generated chainArrange.c and chainArrange.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#A list of assembly rearrangements
CREATE TABLE chainArrange (
    chrom varchar(255) not null,	# Chromosome (or contig, scaffold, etc.)
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# Name of item
    score int unsigned not null,	# Score from 0-1000
    strand char(1) not null,	# + or -
    thickStart int unsigned not null,	# Start of where display should be thick (start codon)
    thickEnd int unsigned not null,	# End of where display should be thick (stop codon)
    itemRgb int unsigned not null,	# Color
    source longblob not null,	# List of assemblies where break occures
    querySize int unsigned not null,	# size of item on query side
    label varchar(255) not null,	# label for item
              #Indices
    PRIMARY KEY(chrom)
);
