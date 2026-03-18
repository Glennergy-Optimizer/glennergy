#!/bin/bash
set -x

case "$1" in
  add)
    /usr/bin/crontab -l 2>/dev/null | grep -v "# Glennergy" | {
      cat
      echo "* * * * * /usr/local/bin/Glennergy-Spotpris # Glennergy-Spotpris cron"
      echo "* * * * * /usr/local/bin/Glennergy-Meteo # Glennergy-Meteo cron"
    } | /usr/bin/crontab -
    echo "Glennergy cron jobs added."
    ;;

  remove)
    crontab -l 2>/dev/null | grep -v "# Glennergy" | crontab -
    echo "Glennergy cron jobs removed."
    ;;

  *)
    echo "Usage: $0 {add|remove}"
    exit 1
    ;;
esac