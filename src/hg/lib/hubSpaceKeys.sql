# hubSpaceKeys.sql was originally generated by the autoSql program, which also 
# generated hubSpaceKeys.c and hubSpaceKeys.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#Api keys for userNames to upload to hubSpace using hubtools
CREATE TABLE hubSpaceKeys (
    userName varchar(255) not null,	# userName in gbMembers
    apiKey varchar(255) not null,	# random key
              #Indices
    PRIMARY KEY(userName),
    INDEX(apiKey)
);
