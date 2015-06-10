#!/bin/sh

SIZE=`du -sg /var/www/gyazo/data | awk '{print $1}'`
MAIL="Your mail address"

# 100G over ?
if [ $SIZE -lt 100 ]; then
  # not over
  exit
fi

mail -s "[Alert] Gyazo server over 100G" $MAIL << EOF
- command
du -m /var/www/gyazo/data

- result
`du -m /var/www/gyazo/data`
^
MB
EOF

