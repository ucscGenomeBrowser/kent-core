table civicdbBed
"Browser extensible data (12 fields) plus information about a CIViC DB locus."
(
    string chrom;                "Chromosome (or contig, scaffold, etc.)"
    uint   chromStart;           "Start position in chromosome"
    uint   chromEnd;             "End position in chromosome"
    string name;                 "Name of item"
    uint   score;                "Score from 0-1000"
    char[1] strand;              "+ or -"
    uint thickStart;             "Start of where display should be thick (start codon)"
    uint thickEnd;               "End of where display should be thick (stop codon)"
    uint reserved;               ""
    int blockCount;              "Number of blocks"
    int[blockCount] blockSizes;  "Comma separated list of block sizes"
    int[blockCount] chromStarts; "Start positions relative to chromStart"
    string origVariant;          "Link to CIViC DB original variant"
    string origReference;        "Genome reference of original annotation"
    lstring alleleRegistryId;    "ClinVar Variation ID"
    lstring clinvarId;           "ClinGen Allele Registry Accession"
    string lastReviewDate;       "Date of last CIViC review"
    lstring diseaseLink;         "Associated disease types"
    lstring therapies;           "Associated therapies"
    lstring mouseOverHTML;       "Disease and therapy summary"
)