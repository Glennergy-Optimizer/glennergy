#!/bin/bash
set -x

case "$1" in
  add)
    /usr/bin/crontab -l 2>/dev/null | grep -v "# Glennergy" | {
      cat
      echo "* * * * * flock -n /tmp/glennergy-meteo.lock sh -c 'umask 0002; timeout 45s /usr/local/bin/Glennergy-Meteo >> /var/log/glennergy/meteo-cron.log 2>&1' # Glennergy-Meteo cron"
      echo "* * * * * flock -n /tmp/glennergy-spotpris.lock sh -c 'umask 0002; timeout 45s /usr/local/bin/Glennergy-Spotpris >> /var/log/glennergy/spotpris-cron.log 2>&1' # Glennergy-Spotpris cron"
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
