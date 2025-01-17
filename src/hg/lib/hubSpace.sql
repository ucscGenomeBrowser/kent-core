# hubSpace.sql was originally generated by the autoSql program, which also 
# generated hubSpace.c and hubSpace.h.  This creates the database representation of
# an object which can be loaded and saved from RAM in a fairly 
# automatic way.

#file storage table for users to store uploaded tracks
CREATE TABLE hubSpace (
    userName varchar(255) not null,	# userName of user uploading file
    fileName varchar(10000) not null,	# name of uploaded files or directory. The actual path to this file is different
    fileSize bigint not null,	# size of the uploaded file
    fileType varchar(255) not null,	# track type of file, or subdir if not a track file
    creationTime timestamp default CURRENT_TIMESTAMP(),	# first upload time
    lastModified timestamp,	# last change time
    db varchar(255) not null,	# genome assembly associated with this file
    location varchar(10000) not null,	# file system path or URL to file
    md5sum varchar(255) not null,	# md5sum of file
    parentDir varchar(1024) not null, # parent directory of file
              #Indices
    PRIMARY KEY(userName, fileName(500), parentDir(245)),
    INDEX(userName),
    INDEX(fileName(25)),
    INDEX(fileType),
    INDEX(db),
    INDEX(location(25)),
    INDEX(creationTime),
    INDEX(lastModified),
    INDEX(md5sum),
    INDEX(parentDir)
);