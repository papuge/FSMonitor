//
//  main.c
//  FSMonitor
//
//  Created by Veronika on 4/14/20.
//  Copyright © 2020 Veronika. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <CoreServices/CoreServices.h>

const char *flags[] = {
    "Must Scan Sub Dirs",
    "User Dropped",
    "Kernel Dropped",
    "Event Ids Wrapped",
    "History Done",
    "Root Changed",
    "Mount",
    "Unmount",
    "File Created",
    "Item Removed",
    "",
    "File Renamed",
    "File Modified",
    "",
    "",
    "XattrMod",
    "",
    "",
    "",
    ""};

long long findSize(const char *file_name)
{
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    long int res = ftell(fp);
    fclose(fp);

    return res;
}

char *findCheckSum(const char *path)
{
    char *checkSumCommand = "shasum '";
    char *command = malloc(strlen(checkSumCommand) + strlen(path) + 2);
    strcpy(command, checkSumCommand);
    strcat(command, path);
    strcat(command, "'");

    FILE *fd;
    fd = popen(command, "r");
    if (!fd)
    {
        pclose(fd);
        free(command);
        return "";
    }

    char buffer[256];
    char *checkSum = malloc(256);
    size_t chread;
    if ((chread = fread(buffer, 1, sizeof(buffer), fd)) != 0)
    {
        int pos = 0;
        for (; pos < sizeof(buffer); pos++)
        {
            if (buffer[pos] == ' ')
                break;
        }
        strncpy(checkSum, buffer, pos - 1);
    }

    pclose(fd);
    free(command);

    return checkSum;
}

struct tm getCurrentTime()
{
    time_t t = time(NULL);
    return *localtime(&t);
}

void displayEventFlags(long long eventFlag)
{
    long long bit = 1;
    int flagsCount = sizeof(flags) / sizeof(flags[0]);
    for (int index = 0; index < flagsCount; index++)
    {
        if ((eventFlag & bit) != 0)
        {
            printf("%s ", flags[index]);
        }
        bit <<= 1;
    }
    printf("\n");
}

void myСallback(
    ConstFSEventStreamRef streamRef,
    void *clientCallBackInfo,
    size_t numEvents,
    void *eventPaths,
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[])
{
    char **paths = eventPaths;
    for (size_t i = 0; i < numEvents; i++)
    {
        printf("%llu\t%s\n", eventIds[i], paths[i]);

        long long fileSize = findSize(paths[i]);
        if (fileSize != -1)
        {
            char *checkSum = findCheckSum(paths[i]);
            printf("Check sum: %s Size: %llu bytes\n", checkSum, fileSize);
        }
        else
        {
            printf("%s\n", "File may be deleted");
        }

        struct tm tm = getCurrentTime();
        printf("Datetime: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

        displayEventFlags(eventFlags[i]);
    }
    fflush(stdout);
}

int main(int argc, const char *argv[])
{
    CFStringRef mypath = CFSTR("/Users/grispy/Documents/");
    CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void **)&mypath, 1, NULL);
    void *callbackInfo = NULL;
    FSEventStreamRef stream;
    CFAbsoluteTime latency = 0.5; // in sec

    stream = FSEventStreamCreate(
        NULL,
        &myСallback,
        callbackInfo,
        pathsToWatch,
        kFSEventStreamEventIdSinceNow,
        latency,
        kFSEventStreamCreateFlagFileEvents);

    CFRunLoopRef runLoop = CFRunLoopGetCurrent();

    FSEventStreamScheduleWithRunLoop(stream, runLoop, kCFRunLoopDefaultMode);
    FSEventStreamStart(stream);

    CFRunLoopRun();

    FSEventStreamUnscheduleFromRunLoop(stream, runLoop, kCFRunLoopDefaultMode);
    FSEventStreamStop(stream);
    FSEventStreamInvalidate(stream);
    FSEventStreamRelease(stream);

    return 0;
}
