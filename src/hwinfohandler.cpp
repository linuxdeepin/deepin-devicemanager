#include "hwinfohandler.h"
#include "hd.h"
#include <stdio.h>
#include <stdlib.h>

const char* tempSaveMonitorInfoFile = "monitor_temp_out.txt";

QString getHwMonitorString()
{
    QString monitorStr;

    hd_data_t *hd_data;
    hd_t *hd;

    hd_data = (hd_data_t*)calloc(1, sizeof(hd_data_t) );

    hd = hd_list(hd_data, hw_monitor, 1, NULL);

    FILE * monitorFile = fopen( tempSaveMonitorInfoFile, "a+" );
    if( monitorFile == nullptr )
    {
        return monitorStr;
    }

    for(; hd; hd = hd->next)
    {
        hd_dump_entry(hd_data, hd, monitorFile);
    }

    hd_free_hd_list(hd);		/* free it */
    hd_free_hd_data(hd_data);

    free(hd_data);

    long int lSize = ftell (monitorFile);
    rewind (monitorFile);
    char* buffer = (char*) malloc (sizeof(char)*lSize);
    fseek(monitorFile, 0, SEEK_SET);

    fread(buffer,1,lSize, monitorFile);
    monitorStr = buffer;
    free (buffer);
    fclose (monitorFile);

    remove( tempSaveMonitorInfoFile );

    return monitorStr;
}
