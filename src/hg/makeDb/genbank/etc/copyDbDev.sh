#!/bin/sh -e

set -e

if [ ! \( "$#" -eq 4 -o "$#" -eq 5 \)  ]; then
    echo "Illegal number of parameters ($#) should be four (4) or five (5)"
    echo "usage: $0 sourceDatabase temporaryDatabase hgwdevHgConfFile fileWithListOfTables [force]"
    exit 1
fi

srcDb=$1
tmpDb=$2
alphaCfg=$3
tableList=$4
force=$5

if [ "$force" != "yes" ]; then
    force="no"
fi

# make locks
if hgsql $srcDb -e  "create table genbankLock (p int)" 
then
    if hgsql $tmpDb -e  "create table genbankLock (p int)" 
    then
        if HGDB_CONF=$alphaCfg hgsql $srcDb -e  "create table genbankLock (p int)" 
        then
            if HGDB_CONF=$alphaCfg hgsql $tmpDb -e  "create table genbankLock (p int)"
            then
                : # all locks successful
            else
                echo "can't create table genbankLock in hgwdev.$tmpDb"
                HGDB_CONF=$alphaCfg hgsql $srcDb -e  "drop table genbankLock"
                hgsql $srcDb -e  "drop table genbankLock"
                hgsql $tmpDb -e  "drop table genbankLock"
                exit 0
            fi
        else
            echo "can't create table genbankLock in hgwdev.$srcDb"
            hgsql $srcDb -e  "drop table genbankLock"
            hgsql $tmpDb -e  "drop table genbankLock"
            exit 0
        fi
    else
        echo "can't create table genbankLock in $tmpDb"
        hgsql $srcDb -e  "drop table genbankLock"
        exit 0
    fi
else
    echo "can't create table genbankLock in $srcDb"
    exit 0
fi

# get the list of genbank tables in the source database
list=`hgsql $srcDb -e "show tables" | egrep -f $tableList`

# drop the tables in the temporary database on dev
(echo -n "drop table if exists ";
for i in $list
do
echo -n "$i,"
done | sed 's/,$/;/') |  hgsql $tmpDb

# mv tables to temporary database
(echo -n "rename table ";
for i in $list
do
echo -n $i" to "$tmpDb.$i","
done | sed 's/,$/;/') | hgsql $srcDb

# rsync contents of temporary database to hgwdev temporary database
for i in $list
do  
#    sudo /cluster/bin/scripts/mypush $tmpDb $i hgwdev-101
    echo $i'.*' 
done > /var/tmp/genbanktables

sudo  mygbpush $tmpDb

# mv the tables back to the source database from the temporary database
(echo -n "rename table ";
for i in $list
do
echo -n $i" to "$srcDb.$i","
done | sed 's/,$/;/') | hgsql $tmpDb


./validateDb.sh $alphaCfg $srcDb $tmpDb $tableList $force

# drop the tables in the source database on hgwdev
(echo -n "drop table if exists ";
for i in $list
do
echo -n "$i,"
done | sed 's/,$/;/') |  HGDB_CONF=$alphaCfg hgsql $srcDb

# mv the tables from the temporary database to the source database on hgwdev
(echo -n "rename table ";
for i in $list
do
echo -n $tmpDb.$i" to "$i","
done | sed 's/,$/;/') | HGDB_CONF=$alphaCfg hgsql $srcDb

# delete locks
HGDB_CONF=$alphaCfg hgsql $tmpDb -e  "drop table genbankLock"
HGDB_CONF=$alphaCfg hgsql $srcDb -e  "drop table genbankLock"
hgsql $srcDb -e  "drop table genbankLock"
hgsql $tmpDb -e  "drop table genbankLock"
