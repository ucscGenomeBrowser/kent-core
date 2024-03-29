table hg38
"Region Genes with color and attributes"
(
string	chrom;                  		"Reference sequence chromosome or scaffold"
uint	chromStart;             		"Start position of feature on chromosome"
uint	chromEnd;               		"End position of feature on chromosome"
string	name;                   		"Name of gene"
uint	score;                  		"Score"
char[1]	strand;                 		"+ or - for strand"
uint	thickStart;             		"Coding region start"
uint	thickEnd;               		"Coding region end"
uint	itemRgb;                		"Color based on confidence level"
string	blockCount;             		"Number of gene entry"
string	blockSizes;             		"Size of gene"
string	blockStarts;            		"Block begins at first chromStart"
string	confidenceLevel;				"Confidence Level" 
string	panelName;						"Panel Name"
string	panelID;                		"Panel ID"
string	entityName;             		"Entity Name"
string	entityType;						"Entity Type"
string	evidence;               		"Evidence"
string	clinGenHaploinsufficiencyScore;	"ClinGen Haploinsufficiency Score"
string	modeOfInheritance;      		"Mode of Inheritance"
string	diseaseGroup;           		"Disease Group"
string	diseaseSubgroup;        		"Disease Subgroup"
lstring	relevantDisorders;    			"Relevant Disorders"
lstring	status;                 		"Status"
lstring	types;							"Types"
string	versionCreated;         		"Date of Creation"
string	penetrance;             		"Penetrance"
lstring	phenotypes;             		"Phenotypes"
string	publications;           		"PubMed ID"
string	triplosensitivityScore;			"Triplosensitivity Score"
string	typesOfVariants;				"Type of Variants"
string	verboseName;					"Verbose Name"
lstring	mouseOverField;					"Mouse over field displaying Gene Name, Inheritance, Phenotypes, Confidence Level"
)
