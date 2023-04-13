CREATE TABLE [dbo].[AccessionRejectionCriteria] (
	[acc_rejection_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[name] [varchar] (64) COLLATE Latin1_General_BIN NOT NULL ,
	[description] [varchar] (1024) COLLATE Latin1_General_BIN NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[Accessions] (
	[accession_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[nuc_acc] [varchar] (64) COLLATE Latin1_General_BIN NOT NULL ,
	[nuc_version] [int] NULL ,
	[nuc_gi] [bigint] NULL ,
	[prot_acc] [varchar] (64) COLLATE Latin1_General_BIN NOT NULL ,
	[prot_version] [int] NULL ,
	[prot_gi] [bigint] NULL ,
	[organization_uid] [int] NOT NULL ,
	[alive] [bit] NOT NULL ,
	[review_status] [int] NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[Accessions_GroupVersions] (
	[accession_uid] [int] NOT NULL ,
	[group_version_uid] [int] NOT NULL ,
	[ccds_status_val_uid] [int] NOT NULL ,
	[original_member] [bit] NOT NULL ,
	[was_public] [bit] NOT NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[BuildQualityTests] (
        [build_uid] [int] NOT NULL ,
        [qa_analysis_id] [int] NOT NULL ,
        [is_required] [bit] NOT NULL ,
        [acc_rejection_uid] [int] NOT NULL ,
        [val_description] [varchar] (64) COLLATE Latin1_General_BIN NULL
) ON [PRIMARY]
go

CREATE TABLE [dbo].[Builds] (
        [build_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION NOT NULL ,
        [tax_id] [int] NOT NULL ,
        [ncbi_build_number] [int] NOT NULL ,
        [ncbi_build_version] [int] NOT NULL,
        [ensembl_build_number] [int] NOT NULL ,
        [assembly_acc] [varchar] (16) COLLATE Latin1_General_BIN NOT NULL ,
        [assembly_version] [int] NOT NULL ,
        [assembly_name] [varchar] (16) COLLATE Latin1_General_BIN NOT NULL ,
        [prev_build_uid] [int] NULL ,
        [date_created] [datetime] NOT NULL ,
        [date_made_public] [datetime] NULL ,
	[ncbi_nadb_accession_uid] [int] NULL ,
	[ensembl_nadb_accession_uid] [int] NULL ,
	[ccds_nadb_accession_uid] [int] NULL 

) ON [PRIMARY]
go

CREATE TABLE [dbo].[CcdsStatistics] (
        [statistics_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION NOT NULL ,
        [statistics_type_uid] [int] NOT NULL ,
        [statistics_html] [text] COLLATE Latin1_General_BIN NULL ,
        [build_uid] [int] NOT NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[CcdsStatusVals] (
	[ccds_status_val_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[ccds_status] [varchar] (50) COLLATE Latin1_General_BIN NOT NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[CcdsUids] (
	[ccds_uid] [int] NOT NULL ,
	[group_uid] [int] NOT NULL ,
	[latest_version] [int] NOT NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[ChromosomeAccessions] (
	[chromosome_accession_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[organization_uid] [int] NOT NULL ,
	[acc] [varchar] (64) COLLATE Latin1_General_BIN NOT NULL ,
	[version] [int] NULL ,
	[chromosome] [varchar] (8) COLLATE Latin1_General_BIN NOT NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[GeneLocations] (
	[gene_id] [int] NOT NULL ,
	[build_uid] [int] NOT NULL ,
	[chromosome_accession_uid] [int] NOT NULL ,
	[location_uid] [int] NOT NULL
) ON [PRIMARY]
go

CREATE TABLE [dbo].[GroupVersions] (
	[group_version_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[group_uid] [int] NOT NULL ,
	[version] [int] NOT NULL ,
	[ncbi_build_number] [int] NOT NULL ,
	[first_ncbi_build_version] [int] NOT NULL ,
	[last_ncbi_build_version] [int] NOT NULL ,
	[gene_id] [int] NOT NULL ,
	[location_count] [int] NOT NULL ,
	[ccds_status_val_uid] [int] NOT NULL ,
	[ccds_version] [int] NULL ,
	[was_public] [bit] NOT NULL ,
        [build_uid] [int] NOT NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[GroupVersions_ChromosomeAccessions] (
	[group_version_uid] [int] NOT NULL ,
	[chromosome_accession_uid] [int] NOT NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[Groups] (
	[group_uid] [int] NOT NULL ,
	[current_version] [int] NOT NULL ,
	[tax_id] [int] NOT NULL ,
	[chromosome] [varchar] (8) COLLATE Latin1_General_BIN NOT NULL ,
	[orientation] [char] (1) COLLATE Latin1_General_BIN NOT NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[InterpretationSubtypes] (
	[interpretation_subtype_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[interpretation_type_uid] [int] NOT NULL ,
	[interpretation_subtype] [varchar] (128) COLLATE Latin1_General_BIN NOT NULL ,
        [for_public] [bit] NOT NULL ,
        [can_edit_comment] [bit] NOT NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[InterpretationTypes] (
	[interpretation_type_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[interpretation_type] [varchar] (128) COLLATE Latin1_General_BIN NOT NULL ,
        [for_public] [bit] NOT NULL
) ON [PRIMARY]
go

CREATE TABLE [dbo].[Interpretations] (
	[interpretation_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[ccds_uid] [int] NULL ,
	[group_uid] [int] NULL ,
	[group_version_uid] [int] NULL ,
	[accession_uid] [int] NULL ,
	[parent_interpretation_uid] [int] NULL ,
	[date_time] [datetime] NOT NULL ,
	[comment] [text] COLLATE Latin1_General_BIN NULL ,
	[val_description] [varchar] (50) COLLATE Latin1_General_BIN NULL ,
	[char_val] [varchar] (1024) COLLATE Latin1_General_BIN NULL ,
	[integer_val] [int] NULL ,
	[float_val] [float] NULL ,
	[interpretation_type_uid] [int] NOT NULL ,
	[interpretation_subtype_uid] [int] NULL ,
	[acc_rejection_uid] [int] NULL ,
	[interpreter_uid] [int] NOT NULL ,
	[program_uid] [int] NULL ,
	[reftrack_uid] [int] NULL 
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
go

ALTER TABLE [dbo].[Interpretations] ADD 
	CONSTRAINT [DF_Interpretations_reftrack_uid] DEFAULT (0) FOR [reftrack_uid]
go

CREATE TABLE [dbo].[Interpreters] (
	[interpreter_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[organization_uid] [int] NOT NULL ,
	[name] [varchar] (128) COLLATE Latin1_General_BIN NULL ,
	[email] [varchar] (500) COLLATE Latin1_General_BIN NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[Locations] (
	[location_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[chr_start] [int] NOT NULL ,
	[chr_stop] [int] NOT NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[Locations_GroupVersions] (
	[location_uid] [int] NOT NULL ,
	[group_version_uid] [int] NOT NULL ,
	[chromosome] [varchar] (8) COLLATE Latin1_General_BIN NOT NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[NADB_Accessions] (
	[nadb_accession_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[organization_uid] [int] NULL ,
	[acc] [varchar] (64) COLLATE Latin1_General_BIN NOT NULL ,
	[version] [int] NOT NULL
) ON [PRIMARY]
go

CREATE TABLE [dbo].[NextIds] (
	[table_name] [varchar] (128) COLLATE Latin1_General_BIN NOT NULL ,
	[next_number] [int] NOT NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[Organizations] (
	[organization_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[name] [varchar] (128) COLLATE Latin1_General_BIN NOT NULL ,
	[approval_authority] [bit] NOT NULL ,
	[tax_id] [int] NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[Programs] (
	[program_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[organization_uid] [int] NOT NULL ,
	[name] [varchar] (500) COLLATE Latin1_General_BIN NOT NULL ,
	[version] [varchar] (500) COLLATE Latin1_General_BIN NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[ProspectiveAnnotCompare] (
        [group_version_uid] [int] NOT NULL ,
        [prot_length_diff] [int] NOT NULL ,
        [coverage_pct] [float] NOT NULL ,
        [matched_splice_count] [int] NOT NULL ,
        [unmatched_splice_count] [int] NOT NULL ,
        [total_splice_count] [int] NOT NULL ,
        [matched_splice_pct] [float] NOT NULL ,
        [identity_pct] [float] NULL
) ON [PRIMARY]
go

CREATE TABLE [dbo].[ProspectiveGroups] (
        [group_version_uid] [int] NOT NULL,
        [ensembl_gene_id] [varchar](64) NULL,
        [nomenclature_id] [varchar](32) NULL,
        [assembly_problem_id] [varchar](64) NULL,
        [completeness] [varchar](16) NULL,
        [swiss_prot_acc] [varchar](64) NULL,
        [swiss_prot_length] [int] NULL,
        [has_annot_overlap] [bit] NOT NULL,
        [overlap_type] [varchar](16) NOT NULL,
        [ncbi_prot_length] [int] NULL,
        [ensembl_prot_length] [int] NULL,
        [prev_reject_reason] [varchar](64) NULL,
        [withdrawn_ccds_uid] [int] NULL,
        [refseq_status] [varchar](16) NULL,
        [chr_start] [int] NOT NULL ,
        [chr_stop] [int] NOT NULL ,
        [prospective_status_val_uid] [int] NOT NULL
) ON [PRIMARY]
go

CREATE TABLE [dbo].[ProspectiveStatusVals] (
	[prospective_status_val_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
	[prospective_status] [varchar] (50) COLLATE Latin1_General_BIN NOT NULL
) ON [PRIMARY]
go

CREATE TABLE [dbo].[ReportQueries]
(
    [query_uid] [int] IDENTITY(1,1) NOT FOR REPLICATION NOT NULL,
    [report_type_uid] [int] NOT NULL,
    [report_sort_order] [int] NULL,
    [name] [varchar](64) NOT NULL,
    [where_conditions] [varchar](1024) NULL,
    [order_by_columns] [varchar](256) NULL
) ON [PRIMARY]
go

CREATE TABLE [dbo].[ReportTypes] (
        [report_type_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
        [report_type] [varchar] (128) COLLATE Latin1_General_BIN NOT NULL
) ON [PRIMARY]
go

CREATE TABLE [dbo].[StatisticsTypes] (
        [statistics_type_uid] [int] IDENTITY (1, 1) NOT FOR REPLICATION  NOT NULL ,
        [statistics_type] [varchar] (20) COLLATE Latin1_General_BIN NOT NULL 
) ON [PRIMARY]
go

CREATE TABLE [dbo].[SwissProtMatches] (
        [prot_acc] [varchar] (64) COLLATE Latin1_General_BIN NOT NULL ,
        [prot_version] [int] NOT NULL ,
        [swiss_prot_acc] [varchar] (64) COLLATE Latin1_General_BIN NOT NULL ,
        [swiss_prot_acc_iso] [varchar] (64) COLLATE Latin1_General_BIN NOT NULL ,
        [prot_length] [int] NOT NULL ,
        [swiss_prot_length] [int] NOT NULL ,
        [identity_pct] [float] NOT NULL ,
        [mismatch_count] [int] NOT NULL ,
        [gap_count] [int] NOT NULL ,
) ON [PRIMARY]
go

