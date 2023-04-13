ALTER TABLE AccessionRejectionCriteria ADD INDEX (acc_rejection_uid);
ALTER TABLE Accessions ADD INDEX (accession_uid);
ALTER TABLE Accessions_GroupVersions ADD INDEX (accession_uid), ADD INDEX (group_version_uid);
ALTER TABLE BuildQualityTests ADD INDEX (build_uid), ADD INDEX (qa_analysis_id);
ALTER TABLE Builds ADD INDEX (build_uid);
ALTER TABLE CcdsStatistics ADD INDEX (statistics_uid);
ALTER TABLE CcdsStatusVals ADD INDEX (ccds_status_val_uid);
ALTER TABLE CcdsUids ADD INDEX (ccds_uid);
ALTER TABLE ChromosomeAccessions ADD INDEX (chromosome_accession_uid);
ALTER TABLE GeneLocations ADD INDEX (gene_id), ADD INDEX (build_uid), ADD INDEX (chromosome_accession_uid);
ALTER TABLE GroupVersions ADD INDEX (group_version_uid);
ALTER TABLE GroupVersions_ChromosomeAccessions ADD INDEX (group_version_uid), ADD INDEX (chromosome_accession_uid);
ALTER TABLE Groups ADD INDEX (group_uid);
ALTER TABLE InterpretationSubtypes ADD INDEX (interpretation_subtype_uid);
ALTER TABLE InterpretationTypes ADD INDEX (interpretation_type_uid);
ALTER TABLE Interpretations ADD INDEX (interpretation_uid);
ALTER TABLE Interpreters ADD INDEX (interpreter_uid);
ALTER TABLE Locations ADD INDEX (location_uid);
ALTER TABLE Locations_GroupVersions ADD INDEX (location_uid), ADD INDEX (group_version_uid), ADD INDEX (chromosome);
ALTER TABLE NADB_Accessions ADD INDEX (nadb_accession_uid);
ALTER TABLE NextIds ADD INDEX (table_name);
ALTER TABLE Organizations ADD INDEX (organization_uid);
ALTER TABLE Programs ADD INDEX (program_uid);
ALTER TABLE ProspectiveAnnotCompare ADD INDEX (group_version_uid);
ALTER TABLE ProspectiveGroups ADD INDEX (group_version_uid);
ALTER TABLE ProspectiveStatusVals ADD INDEX (prospective_status_val_uid);
ALTER TABLE ReportQueries ADD INDEX (query_uid);
ALTER TABLE ReportTypes ADD INDEX (report_type_uid);
ALTER TABLE StatisticsTypes ADD INDEX (statistics_type_uid);
ALTER TABLE SwissProtMatches ADD INDEX (prot_acc), ADD INDEX (prot_version), ADD INDEX (swiss_prot_acc), ADD INDEX (swiss_prot_acc_iso);
ALTER TABLE Accessions ADD INDEX (organization_uid), ADD INDEX (review_status);
ALTER TABLE Accessions_GroupVersions ADD INDEX (accession_uid), ADD INDEX (ccds_status_val_uid), ADD INDEX (group_version_uid);
ALTER TABLE BuildQualityTests ADD INDEX (build_uid);
ALTER TABLE BuildQualityTests ADD INDEX (acc_rejection_uid);
ALTER TABLE Builds ADD INDEX (prev_build_uid);
ALTER TABLE Builds ADD INDEX (ncbi_nadb_accession_uid), ADD INDEX (ensembl_nadb_accession_uid), ADD INDEX (ccds_nadb_accession_uid);
ALTER TABLE CcdsStatistics ADD INDEX (statistics_type_uid);
ALTER TABLE CcdsStatistics ADD INDEX (build_uid);
ALTER TABLE CcdsUids ADD INDEX (group_uid);
ALTER TABLE ChromosomeAccessions ADD INDEX (organization_uid);
ALTER TABLE GeneLocations ADD INDEX (build_uid), ADD INDEX (chromosome_accession_uid), ADD INDEX (location_uid);
ALTER TABLE GroupVersions ADD INDEX (ccds_status_val_uid), ADD INDEX (group_uid);
ALTER TABLE GroupVersions ADD INDEX (build_uid);
ALTER TABLE GroupVersions_ChromosomeAccessions ADD INDEX (chromosome_accession_uid), ADD INDEX (group_version_uid);
ALTER TABLE InterpretationSubtypes ADD INDEX (interpretation_type_uid);
ALTER TABLE Interpretations ADD INDEX (acc_rejection_uid), ADD INDEX (accession_uid), ADD INDEX (ccds_uid), ADD INDEX (group_version_uid), ADD INDEX (group_uid), ADD INDEX (interpretation_subtype_uid), ADD INDEX (interpretation_type_uid), ADD INDEX (parent_interpretation_uid), ADD INDEX (interpreter_uid), ADD INDEX (program_uid);
ALTER TABLE Interpreters ADD INDEX (organization_uid);
ALTER TABLE Locations_GroupVersions ADD INDEX (group_version_uid), ADD INDEX (location_uid);
ALTER TABLE NADB_Accessions ADD INDEX (organization_uid);
ALTER TABLE Programs ADD INDEX (organization_uid);
ALTER TABLE ProspectiveAnnotCompare ADD INDEX (group_version_uid);
ALTER TABLE ProspectiveGroups ADD INDEX (group_version_uid);
ALTER TABLE ProspectiveGroups ADD INDEX (prospective_status_val_uid);
ALTER TABLE ReportQueries ADD INDEX (report_type_uid);
