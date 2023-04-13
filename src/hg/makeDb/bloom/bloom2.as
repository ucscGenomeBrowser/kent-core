table bloom2
"Table with Bloom2 paper mutations"
    (
    string chrom;      "Chromosome (or contig, scaffold, etc.)"
    uint   chromStart; "Start position in chromosome"
    uint   chromEnd;   "End position in chromosome"
    string name;       "Name of item"
    uint   score;      "Score from 0-1000"
    char[1] strand;    "+ or -"
    uint thickStart;   "Start of where display should be thick (start codon)"
    uint thickEnd;     "End of where display should be thick (stop codon)"
    uint itemRgb;     "Color"
    string pos;         "AA-Position in S protein"
    lstring muts;       "Subjects with mutations"
    string _mouseOver;       "MouseOver text"
    )
