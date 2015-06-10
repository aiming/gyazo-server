#!/bin/sh

logfile=/var/www/gyazo/record.log

df -m | grep 183768 | awk -vD=`date '+%Y/%m/%d'` '{print D" total "$1"m use "$2"m free "$3"m use(%) "$4 }' >> $logfile
du -m /var/www/gyazo/data | awk -vD=`date '+%Y/%m/%d'` '{print D" gyazo_use "$1"m" }' >> $logfile
