#/bin/bash

EFFECTIVEUID=$(id -u)

if [ "$EFFECTIVEUID" -ne 0 ]; then
  echo "This uninstaller must be run as root. Exiting..."
  exit
fi

echo "Starting uninstall of DiskAnalyzer..."
echo ""
echo "Removing binaries"

if ($(rm /usr/local/bin/da)); 
then 
    echo " - Removed /usr/local/bin/da"
else 
    " - Failed to rm /usr/local/bin/da. Exiting..." && exit
fi

if ($(rm /opt/diskanalyzer/dad)); 
then 
    echo " - Removed /opt/diskanalyzer/dad"
else 
    " - Failed to rm /opt/diskanalyzer/dad. Exiting..." && exit
fi

if ($(rm /opt/diskanalyzer/diskanalyzer_job)); 
then 
    echo " - Removed /opt/diskanalyzer/diskanalyzer_job"
else 
    " - Failed to rm /opt/diskanalyzer/diskanalyzer_job. Exiting..." && exit
fi

if ($(rmdir /opt/diskanalyzer)); 
then 
    echo " - Removed /opt/diskanalyzer directory"
else 
    " - Failed to remove /opt/diskanalyzer directory. Exiting..." && exit
fi

echo ""
echo "Uninstall ended sucessfully"
