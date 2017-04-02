#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "bootloader.h"
#include "config.h"

static char pkgFilePath[PATH_MAX];
static ITCFileStream fileStream;

ITCStream* OpenUpgradePackage(void)
{
    ITPDriveStatus* driveStatusTable;
    ITPDriveStatus* driveStatus = NULL;
    int i;

    // try to find the package drive
    ioctl(ITP_DEVICE_DRIVE, ITP_IOCTL_GET_TABLE, &driveStatusTable);

    for (i = ITP_MAX_DRIVE - 1; i >= 0; i--)
    {
        driveStatus = &driveStatusTable[i];
        if (driveStatus->avail && driveStatus->removable)
        {
            char buf[PATH_MAX], *ptr;

            LOG_DBG "drive[%d]:disk=%d\n", i, driveStatus->disk LOG_END

            // get file path from list
            strcpy(buf, CFG_UPGRADE_FILENAME_LIST);
            ptr = strtok(buf, " ");
            do
            {
                strcpy(pkgFilePath, driveStatus->name);
                strcat(pkgFilePath, ptr);

                if (itcFileStreamOpen(&fileStream, pkgFilePath, false) == 0)
                {
                    LOG_INFO "Found package file %s\n", pkgFilePath LOG_END
                    return &fileStream.stream;
                }
                else
                {
                    LOG_DBG "try to fopen(%s) fail:0x%X\n", pkgFilePath, errno LOG_END
                }
            }
            while ((ptr = strtok(NULL, " ")) != NULL);
        }
    }
    LOG_DBG "cannot find package file.\n" LOG_END
    return NULL;
}

void DeleteUpgradePackage(void)
{
    if (remove(pkgFilePath))
        LOG_ERR "Delete %s fail: %d\n", pkgFilePath, errno LOG_END
}
