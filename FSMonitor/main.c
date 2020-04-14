//
//  main.c
//  FSMonitor
//
//  Created by Veronika on 4/14/20.
//  Copyright © 2020 Veronika. All rights reserved.
//

#include <stdio.h>
#include <CoreServices/CoreServices.h>

void myСallback(
    ConstFSEventStreamRef streamRef,
    void *clientCallBackInfo,
    size_t numEvents,
    void *eventPaths,
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[])
{

    char **paths = eventPaths;
    for (size_t i = 0; i < numEvents; i++) {
      printf("%llu\tflags:%#.8x\t%s\n", eventIds[i], eventFlags[i], paths[i]);
    }
    fflush(stdout);
}

int main(int argc, const char * argv[]) {
    
    CFStringRef mypath = CFSTR("/Users/grispy/Documents/");
    CFArrayRef pathsToWatch = CFArrayCreate(NULL, (const void **)&mypath, 1, NULL);
    void *callbackInfo = NULL;
    FSEventStreamRef stream;
    CFAbsoluteTime latency = 3.0; // in sec

    stream = FSEventStreamCreate(
        NULL,
        &myСallback,
        callbackInfo,
        pathsToWatch,
        kFSEventStreamEventIdSinceNow,
        latency,
        kFSEventStreamCreateFlagNone
    );
    
    CFRunLoopRef runLoop = CFRunLoopGetCurrent();
    
    FSEventStreamScheduleWithRunLoop(stream, runLoop, kCFRunLoopDefaultMode);
    FSEventStreamStart(stream);
    
    CFRunLoopRun();
    
    FSEventStreamUnscheduleFromRunLoop(stream, runLoop, kCFRunLoopDefaultMode);
    FSEventStreamStop(stream);
    FSEventStreamInvalidate(stream);
    FSEventStreamRelease(stream);

    printf("End");
    
    return 0;
}
