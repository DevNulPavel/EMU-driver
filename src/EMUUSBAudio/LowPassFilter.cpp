//
//  LowPassFilter.cpp
//  EMUUSBAudio
//
//  Created by Wouter Pasman on 25/11/14.
//  Copyright (c) 2014 Wouter Pasman. All rights reserved.
//

#include "LowPassFilter.h"
#include "EMUUSBLogging.h"

void LowPassFilter::init(UInt64 inputx) {
    debugIOLog("LowPassFilter::filter init");
    x = inputx;
    dx = EXPECTED_WRAP_TIME;
    u=0;

}


UInt64 LowPassFilter::filter(UInt64 inputx) {
    UInt64 xnext;
    SInt64 unext,du,F;
    
    
    xnext = x + dx ; // the next filtered output
    unext = inputx - xnext; // error u
    
    du = unext - u; // change of the error, for damping
    if (abs(du) < EXPECTED_WRAP_TIME/2000) {
        F = K * unext + DA * du; // force on spring
        dx = dx + F/M ;
    }
    x = xnext;
    u= unext; // update the filter
    
    debugIOLogT("filter %lld -> %lld", inputx, x);
    return x;
}
