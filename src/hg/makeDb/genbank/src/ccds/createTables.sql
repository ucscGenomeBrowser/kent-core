DROP TABLE IF EXISTS AccessionRejectionCriteria;
CREATE TABLE AccessionRejectionCriteria (
	acc_rejection_uid int PRIMARY KEY  NOT NULL ,
	name varchar (64) NOT NULL ,
	description text NULL 
) 
;

DROP TABLE IF EXISTS Accessions;
CREATE TABLE Accessions (
	accession_uid int PRIMARY KEY  NOT NULL ,
	nuc_acc varchar (64) NOT NULL ,
	nuc_version int NULL ,
	nuc_gi bigint NULL ,
	prot_acc varchar (64) NOT NULL ,
	prot_version int NULL ,
	prot_gi bigint NULL ,
	organization_uid int NOT NULL ,
	alive tinyint(1) NOT NULL ,
	review_status int NULL 
) 
;

DROP TABLE IF EXISTS Accessions_GroupVersions;
CREATE TABLE Accessions_GroupVersions (
	accession_uid int NOT NULL ,
	group_version_uid int NOT NULL ,
	ccds_status_val_uid int NOT NULL ,
	original_member tinyint(1) NOT NULL ,
	was_public tinyint(1) NOT NULL 
) 
;

DROP TABLE IF EXISTS BuildQualityTests;
CREATE TABLE BuildQualityTests (
        build_uid int NOT NULL ,
        qa_analysis_id int NOT NULL ,
        is_required tinyint(1) NOT NULL ,
        acc_rejection_uid int NOT NULL ,
        val_description varchar (64) NULL
) 
;

DROP TABLE IF EXISTS Builds;
CREATE TABLE Builds (
        build_uid int PRIMARY KEY NOT NULL ,
        tax_id int NOT NULL ,
        ncbi_build_number int NOT NULL ,
        ncbi_build_version int NOT NULL,
        ensembl_build_number int NOT NULL ,
        assembly_acc varchar (16) NOT NULL ,
        assembly_version int NOT NULL ,
        assembly_name varchar (16) NOT NULL ,
        prev_build_uid int NULL ,
        date_created datetime NOT NULL ,
        date_made_public datetime NULL ,
	ncbi_nadb_accession_uid int NULL ,
	ensembl_nadb_accession_uid int NULL ,
	ccds_nadb_accession_uid int NULL 

) 
;

DROP TABLE IF EXISTS CcdsStatistics;
CREATE TABLE CcdsStatistics (
        statistics_uid int PRIMARY KEY NOT NULL ,
        statistics_type_uid int NOT NULL ,
        statistics_html text NULL ,
        build_uid int NOT NULL 
) 
;

DROP TABLE IF EXISTS CcdsStatusVals;
CREATE TABLE CcdsStatusVals (
	ccds_status_val_uid int PRIMARY KEY  NOT NULL ,
	ccds_status varchar (50) NOT NULL 
) 
;

DROP TABLE IF EXISTS CcdsUids;
CREATE TABLE CcdsUids (
	ccds_uid int NOT NULL ,
	group_uid int NOT NULL ,
	latest_version int NOT NULL 
) 
;

DROP TABLE IF EXISTS ChromosomeAccessions;
CREATE TABLE ChromosomeAccessions (
	chromosome_accession_uid int PRIMARY KEY  NOT NULL ,
	organization_uid int NOT NULL ,
	acc varchar (64) NOT NULL ,
	version int NULL ,
	chromosome varchar (8) NOT NULL 
) 
;

DROP TABLE IF EXISTS GeneLocations;
CREATE TABLE GeneLocations (
	gene_id int NOT NULL ,
	build_uid int NOT NULL ,
	chromosome_accession_uid int NOT NULL ,
	location_uid int NOT NULL
) 
;

DROP TABLE IF EXISTS GroupVersions;
CREATE TABLE GroupVersions (
	group_version_uid int PRIMARY KEY  NOT NULL ,
	group_uid int NOT NULL ,
	version int NOT NULL ,
	ncbi_build_number int NOT NULL ,
	first_ncbi_build_version int NOT NULL ,
	last_ncbi_build_version int NOT NULL ,
	gene_id int NOT NULL ,
	location_count int NOT NULL ,
	ccds_status_val_uid int NOT NULL ,
	ccds_version int NULL ,
	was_public tinyint(1) NOT NULL ,
        build_uid int NOT NULL 
) 
;

DROP TABLE IF EXISTS GroupVersions_ChromosomeAccessions;
CREATE TABLE GroupVersions_ChromosomeAccessions (
	group_version_uid int NOT NULL ,
	chromosome_accession_uid int NOT NULL 
) 
;

DROP TABLE IF EXISTS Groups;
CREATE TABLE Groups (
	group_uid int NOT NULL ,
	current_version int NOT NULL ,
	tax_id int NOT NULL ,
	chromosome varchar (8) NOT NULL ,
	orientation char (1) NOT NULL 
) 
;

DROP TABLE IF EXISTS InterpretationSubtypes;
CREATE TABLE InterpretationSubtypes (
	interpretation_subtype_uid int PRIMARY KEY  NOT NULL ,
	interpretation_type_uid int NOT NULL ,
	interpretation_subtype varchar (128) NOT NULL ,
        for_public tinyint(1) NOT NULL ,
        can_edit_comment tinyint(1) NOT NULL 
) 
;

DROP TABLE IF EXISTS InterpretationTypes;
CREATE TABLE InterpretationTypes (
	interpretation_type_uid int PRIMARY KEY  NOT NULL ,
	interpretation_type varchar (128) NOT NULL ,
        for_public tinyint(1) NOT NULL
) 
;

DROP TABLE IF EXISTS Interpretations;
CREATE TABLE Interpretations (
	interpretation_uid int PRIMARY KEY  NOT NULL ,
	ccds_uid int NULL ,
	group_uid int NULL ,
	group_version_uid int NULL ,
	accession_uid int NULL ,
	parent_interpretation_uid int NULL ,
	date_time datetime NOT NULL ,
	comment text NULL ,
	val_description varchar (50) NULL ,
	char_val text NULL ,
	integer_val int NULL ,
	float_val float NULL ,
	interpretation_type_uid int NOT NULL ,
	interpretation_subtype_uid int NULL ,
	acc_rejection_uid int NULL ,
	interpreter_uid int NOT NULL ,
	program_uid int NULL ,
	reftrack_uid int NULL 
) 
;


DROP TABLE IF EXISTS Interpreters;
CREATE TABLE Interpreters (
	interpreter_uid int PRIMARY KEY  NOT NULL ,
	organization_uid int NOT NULL ,
	name varchar (128) NULL ,
	email text NULL 
) 
;

DROP TABLE IF EXISTS Locations;
CREATE TABLE Locations (
	location_uid int PRIMARY KEY  NOT NULL ,
	chr_start int NOT NULL ,
	chr_stop int NOT NULL 
) 
;

DROP TABLE IF EXISTS Locations_GroupVersions;
CREATE TABLE Locations_GroupVersions (
	location_uid int NOT NULL ,
	group_version_uid int NOT NULL ,
	chromosome varchar (8) NOT NULL 
) 
;

DROP TABLE IF EXISTS NADB_Accessions;
CREATE TABLE NADB_Accessions (
	nadb_accession_uid int PRIMARY KEY  NOT NULL ,
	organization_uid int NULL ,
	acc varchar (64) NOT NULL ,
	version int NOT NULL
) 
;

DROP TABLE IF EXISTS NextIds;
CREATE TABLE NextIds (
	table_name varchar (128) NOT NULL ,
	next_number int NOT NULL 
) 
;

DROP TABLE IF EXISTS Organizations;
CREATE TABLE Organizations (
	organization_uid int PRIMARY KEY  NOT NULL ,
	name varchar (128) NOT NULL ,
	approval_authority tinyint(1) NOT NULL ,
	tax_id int NULL 
) 
;

DROP TABLE IF EXISTS Programs;
CREATE TABLE Programs (
	program_uid int PRIMARY KEY  NOT NULL ,
	organization_uid int NOT NULL ,
	name text NOT NULL ,
	version text NULL 
) 
;

DROP TABLE IF EXISTS ProspectiveAnnotCompare;
CREATE TABLE ProspectiveAnnotCompare (
        group_version_uid int NOT NULL ,
        prot_length_diff int NOT NULL ,
        coverage_pct float NOT NULL ,
        matched_splice_count int NOT NULL ,
        unmatched_splice_count int NOT NULL ,
        total_splice_count int NOT NULL ,
        matched_splice_pct float NOT NULL ,
        identity_pct float NULL
) 
;

DROP TABLE IF EXISTS ProspectiveGroups;
CREATE TABLE ProspectiveGroups (
        group_version_uid int NOT NULL,
        ensembl_gene_id varchar(64) NULL,
        nomenclature_id varchar(32) NULL,
        assembly_problem_id varchar(64) NULL,
        completeness varchar(16) NULL,
        swiss_prot_acc varchar(64) NULL,
        swiss_prot_length int NULL,
        has_annot_overlap tinyint(1) NOT NULL,
        overlap_type varchar(16) NOT NULL,
        ncbi_prot_length int NULL,
        ensembl_prot_length int NULL,
        prev_reject_reason varchar(64) NULL,
        withdrawn_ccds_uid int NULL,
        refseq_status varchar(16) NULL,
        chr_start int NOT NULL ,
        chr_stop int NOT NULL ,
        prospective_status_val_uid int NOT NULL
) 
;

DROP TABLE IF EXISTS ProspectiveStatusVals;
CREATE TABLE ProspectiveStatusVals (
	prospective_status_val_uid int PRIMARY KEY  NOT NULL ,
	prospective_status varchar (50) NOT NULL
) 
;

DROP TABLE IF EXISTS ReportQueries;
CREATE TABLE ReportQueries
(
    query_uid int PRIMARY KEY NOT NULL,
    report_type_uid int NOT NULL,
    report_sort_order int NULL,
    name varchar(64) NOT NULL,
    where_conditions varchar(1024) NULL,
    order_by_columns varchar(256) NULL
) 
;

DROP TABLE IF EXISTS ReportTypes;
CREATE TABLE ReportTypes (
        report_type_uid int PRIMARY KEY  NOT NULL ,
        report_type varchar (128) NOT NULL
) 
;

DROP TABLE IF EXISTS StatisticsTypes;
CREATE TABLE StatisticsTypes (
        statistics_type_uid int PRIMARY KEY  NOT NULL ,
        statistics_type varchar (20) NOT NULL 
) 
;

DROP TABLE IF EXISTS SwissProtMatches;
CREATE TABLE SwissProtMatches (
        prot_acc varchar (64) NOT NULL ,
        prot_version int NOT NULL ,
        swiss_prot_acc varchar (64) NOT NULL ,
        swiss_prot_acc_iso varchar (64) NOT NULL ,
        prot_length int NOT NULL ,
        swiss_prot_length int NOT NULL ,
        identity_pct float NOT NULL ,
        mismatch_count int NOT NULL ,
        gap_count int NOT NULL 
) 
;

