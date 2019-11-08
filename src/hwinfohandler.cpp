/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "commondefine.h"
#include "hd.h"
#include <stdio.h>
#include <stdlib.h>

const char* tempSaveMonitorInfoFile = "monitor_temp_out.txt";

//QString getHwMonitorString()
//{
//    QString monitorStr;

//    hd_data_t *hd_data;
//    hd_t *hd;

//    hd_data = (hd_data_t*)calloc(1, sizeof(hd_data_t) );

//    hd = hd_list(hd_data, hw_monitor, 1, NULL);

//    FILE * monitorFile = fopen( tempSaveMonitorInfoFile, "a+" );
//    if( monitorFile == nullptr )
//    {
//        return monitorStr;
//    }

//    for(; hd; hd = hd->next)
//    {
//        hd_dump_entry(hd_data, hd, monitorFile);
//    }

//    hd_free_hd_list(hd);		/* free it */
//    hd_free_hd_data(hd_data);

//    free(hd_data);

//    long int lSize = ftell (monitorFile);
//    rewind (monitorFile);
//    char* buffer = (char*) malloc (sizeof(char)*lSize);
//    fseek(monitorFile, 0, SEEK_SET);

//    fread(buffer,1,lSize, monitorFile);
//    monitorStr = buffer;
//    free (buffer);
//    fclose (monitorFile);

//    remove( tempSaveMonitorInfoFile );

//    return monitorStr;
//}
