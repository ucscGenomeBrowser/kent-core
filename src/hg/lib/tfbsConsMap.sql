# tfbsConsMap.sql was originally generated by the autoSql program, which also 
# generated tfbsConsMap.c and tfbsConsMap.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#tfbsConsMap Data
CREATE TABLE tfbsConsMap (
    id varchar(255) not null,	# TRANSFAC id
    ac varchar(255) not null,	# gene-regulation.com AC
              #Indices
    PRIMARY KEY(id)
);
