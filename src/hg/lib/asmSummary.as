table asmSummary
"NCBI assembly_summary data, see: https://ftp.ncbi.nlm.nih.gov/genomes/ASSEMBLY_REPORTS/README_assembly_summary.txt"
    (
    string assemblyAccession;	"www.ncbi.nlm.nih.gov/assembly/?term=xxx"
    string bioproject;	"www.ncbi.nlm.nih.gov/bioproject/?term=xxx"
    string biosample;	"www.ncbi.nlm.nih.gov/biosample/?term=xxx"
    string wgsMaster;	"www.ncbi.nlm.nih.gov/nuccore/xxx"
    string refseqCategory;	"representative or reference"
    uint taxId;	"www.ncbi.nlm.nih.gov/taxonomy/?term=xxx"
    uint speciesTaxid;	"www.ncbi.nlm.nih.gov/taxonomy/?term=xxx"
    string organismName;	"binomial scientific name"
    string infraspecificName;	"strain/cultivar/ecotype/breed"
    string isolate;	"source of sample"
    string versionStatus;	"latest/suppressed/replaced"
    string assemblyLevel;	"Contig/Scaffold/Complete Genome/Chromosome"
    string releaseType;	"Major/Minor/Patch"
    string genomeRep;	"Full/Partial"
    string seqRelDate;	"date YYYY/MM/DD sequence released to INSDC"
    string asmName;	"submitter supplied name"
    string asmSubmitter;	"institution submitting assembly"
    string gbrsPairedAsm;	"GenBank<->RefSeq GCA/GCF relationship"
    string pairedAsmComp;	"identical/different for GCA<->GCF relationship"
    string ftpPath;	"ftp.ncbi.nlm.nih.gov/genomes/all/GCx/012/345/678/asmId"
    string excludedFromRefseq;	"noted reason for exclusion from RefSeq"
    string relationToTypeMaterial;	"note of assembly relation to sample"
    string assemblyType;	"haploid/diploid/haploid-with-alt-loci/alternate-pseudohaplotype"
    string phyloGroup;	"bacteria/viral/archaea/fungi/metagenomes/invertebrate/other/vertebrate_other/plant/vertebrate_mammalian/protozoa"
    bigint genomeSize;	"total length of all top-level sequences in the primary assembly"
    bigint genomeSizeUngapped;	"genome length not counting gaps (gap == 10 or more Ns)"
    float gcPercent;	"GC percent"
    ushort repliconCount;	"total number of chromosomes, organelle genomes and plasmids"
    uint scaffoldCount;	"number of scaffolds: placed, unlocalzes, unplace, alternate loci and patch"
    uint contigCount;	"number of conts in the primary assembly"
    string annotationProvider;	"the group that provied the annotation on the assembly"
    string annotationName;	"the name of the annotation"
    string annotationDate;	"annotation date YYYY/MM/DD"
    string totalGeneCount;	"total gene count in annotation"
    uint proteinCodingGeneCount;	"protein coding gene count in annotation"
    string nonCodingGeneCount;	"non coding gene count in annotation"
    string pubmedId;	"comma separated list of PubMed ID(s)"
    )