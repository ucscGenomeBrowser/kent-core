table hgncBig62
"A cross-reference table between HUGO Gene Nomenclature Committee (HGNC) IDs and other database IDs."
    (
    string chrom;      "Chromosome (or contig, scaffold, etc.)"
    uint   chromStart; "Start position in chromosome"
    uint   chromEnd;   "End position in chromosome"
    string name;       "HGNC Identifier"
    uint   score;      "Score from 0-1000"
    char[1] strand;    "Placeholder value"
    uint thickStart;   "Start of where display should be thick (start codon)"
    uint thickEnd;     "End of where display should be thick (stop codon)"
    uint reserved;     "Used as itemRgb as of 2004-11-22"

    string symbol; "Gene Symbol"
    string geneName; "Gene Name"
    string locus_group; "Locus Group"
    string locus_type; "Locus Type"
    string status; "Status"
    string location; "Cytogenetic Location"
    string location_sortable; "Cytogenetic Location (sortable)"
    string alias_symbol; "Other Symbols Used"
    lstring alias_name; "Other Names Used"
    string prev_symbol; "Previous Approved HGNC Symbols"
    lstring prev_name; "Previous Approved HGNC Names"
    string gene_group; "Gene Family"
    string gene_group_id; "Gene Family ID"
    string date_approved_reserved; "Date Symbol First Approved"
    string date_symbol_changed; "Date Symbol Last Changed"
    string date_name_changed; "Date Name Last Changed"
    string date_modified; "Data Entry Last Changed"
    string entrez_id; "Entrez gene ID"
    string ensembl_gene_id; "Ensembl gene ID"
    string vega_id; "Vega gene ID"
    string ucsc_id; "UCSC gene ID"
    string ena; "International Nucleotide Sequence Database Collaboration  accession"
    string refseq_accession; "RefSeq nucleotide accesions(s)"
    string ccds_id; "Consensus CDS ID"
    string uniprot_ids; "UniProt protein accession."
    string pubmed_id; "Pubmed PMID"
    string mgd_id; "Mouse genome informatics ID"
    string rgd_id; "Rat genome database ID"
    lstring lsdb; "Locus Specific Mutations Database ID"
    string cosmic; "Catalogue of somatic mutations ID"
    string omim_id; "Online Mendelian Inheritance in Man ID"
    string mirbase; "mirBase ID"
    string homeodb; "Homeibox Database ID"
    string snornabase; "snoRNABase ID"
    string bioparadigms_slc; "Bioparadigms SLC ID"
    string orphanet; "Orphanet ID"
    string pseudogeneOrg; "PseudogeneOrg ID"
    string horde_id; "HORDE ID"
    string merops; "MEROPS peptide database"
    string imgt; "ImMunoGeneTics ID"
    string iuphar; "IUPHAR ID"
    string kznf_gene_catalog; "KZNF Gene Catalog ID"
    string mamitTrnadb; "Mamit-tRNA ID"
    string cd; "Cell Differentiation Database ID"
    string lncrnadb; "lncRNA Database ID"
    string enzyme_id; "ENZYME EC accession"
    string intermediate_filament_db; "Intermediate Filament Database"
    string rna_central_ids; "RNAcentral ID"
    string lncipedia; "LNCipedia ID"
    string gtrnadb; "Genomic tRNA Database ID"
    string agr; "Alliance of Genome Resources ID"
    string mane_select; "MANE Select ID(s)"
    string gencc; "GenCC ID"
    )