#/bin/bash

EFFECTIVEUID=$(id -u)

if [ "$EFFECTIVEUID" -ne 0 ]; then
  echo "This installer must be run as root. Exiting..."
  exit
fi

echo "Starting installation of DiskAnalyzer..."
echo ""
echo "- Compiling source code"

if ($(gcc daemon.c -o dad -lrt));
then 
    echo " -- Compiled daemon.c"

else 
    echo " -- Failed to compile daemon.c. Exiting..." && exit
fi

if ($(gcc da.c -o da));
then 
    echo " -- Compiled da.c"

else 
    echo " -- Failed to compile da.c. Exiting..." && exit
fi


if ($(gcc da_job.c -o diskanalyzer_job -lrt));
then 
    echo " -- Compiled da_job.c"

else 
    echo " -- Failed to compile da_job.c. Exiting..." && exit
fi

echo "Done compiling"

echo "- Creating /opt/diskanalyzer directory"
if ($(mkdir /opt/diskanalyzer));
then 
    echo "Directory created"
else 
    echo "Failed to create /opt/diskanalyzer. Exiting..." && exit
fi

echo "- Moving executables to appropriate directories"
if ($(mv da /usr/local/bin/da));
then 
    echo " -- Moved da executable"
else 
    echo " -- Failed to mv da to /usr/local/bin/da. Exiting..." && exit
fi

if ($(mv dad /opt/diskanalyzer));
then 
    echo " -- Moved dad binary"
else 
    echo " -- Failed to mv dad to /opt/diskanalyzer/dad. Exiting..." && exit
fi

if ($(mv diskanalyzer_job /opt/diskanalyzer ));
then 
    echo " -- Moved diskanalyzer_job binary"
else 
    echo " -- Failed to mv diskanalyzer_job to /opt/diskanalyzer/diskanalyzer_job. Exiting..." && exit
fi
echo "Executables moved"

echo ""
echo "DiskInstaller sucessfully installed. You can delete the source code files and this installer"
echo "You can find the uninstaller at https://github.com/EdiWEP/disk-analyzer"
echo "Use da --help for usage details"
