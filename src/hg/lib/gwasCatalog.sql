CREATE TABLE gwasCatalog (
    bin           smallint(5) unsigned not null,
    chrom varchar(255) not null,	# Reference sequence chromosome or scaffold
    chromStart int unsigned not null,	# Start position in chromosome
    chromEnd int unsigned not null,	# End position in chromosome
    name varchar(255) not null,	# ID of SNP associated with trait
    pubMedID int unsigned not null,	# PubMed ID of publication of the study
    author varchar(255) not null,	# First author of publication
    pubDate varchar(255) not null,	# Date of publication
    journal varchar(255) not null,	# Journal of publication
    title varchar(1024) not null,	# Title of publication
    trait varchar(255) not null,	# Disease or trait assessed in study
    initSample longblob not null,	# Initial sample size
    replSample longblob not null,	# Replication sample size
    region varchar(255) not null,	# Chromosome band / region of SNP
    genes longblob not null,	        # Reported Gene(s)
    riskAllele longblob not null,	# Strongest SNP-Risk Allele
    riskAlFreq varchar(255) not null,	# Risk Allele Frequency
    pValue varchar(255) not null,	# p-Value
    pValueDesc varchar(255) not null,	# p-Value Description
    orOrBeta varchar(255) not null,	# Odds ratio or beta
    ci95 varchar(255) not null,	# 95 Confidence Interval
    platform varchar(255) not null,	# Platform and [SNPs passing QC]
    cnv enum("Y", "N")  not null,	# Y if Copy Number Variant
              #Indices
    INDEX(chrom,bin),
    INDEX(name)
);
