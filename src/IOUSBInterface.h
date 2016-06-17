//
//  IOUSBInterface.h
//  EMUUSBAudio
//
//  Created by Wouter Pasman on 05/06/16.
//  Copyright (c) 2016 com.emu. All rights reserved.
//

#ifndef __EMUUSBAudio__IOUSBInterface__
#define __EMUUSBAudio__IOUSBInterface__

#include "osxversion.h"


#ifdef HAVE_OLD_USB_INTERFACE
/******************** 10.9 *********************/

#include <IOKit/usb/IOUSBInterface.h>

class IOUSBInterface1: public IOUSBInterface {
public:
    inline UInt8 getInterfaceNumber() {
        return GetInterfaceNumber();
    }

    /*! @result the index of the string descriptor describing the interface
     */
    inline UInt8  getInterfaceStringIndex() {
        return GetInterfaceStringIndex();
    };
    
    /*!
     @function GetDevice
     returns the device the interface is part of.
     @result Pointer to the IOUSBDevice object which is the parent of this IOUSBInterface object.
     */
    inline IOUSBDevice *getDevice1() {
        return OSDynamicCast(IOUSBDevice, GetDevice());
    }

    /*!
     @function findPipe
     Find a pipe of the interface that matches the requirements,
     starting from the beginning of the interface's pipe list
     @param request Requirements for pipe to match, updated with the found pipe's
     properties.
     @result Pointer to the pipe, or NULL if no pipe matches the request.
     */
    IOUSBPipe* findPipe(uint8_t direction, uint8_t type) {
        IOUSBFindEndpointRequest request;
        request.direction = direction;
        request.type = type;
        request.interval = 0xFF;
        request.maxPacketSize = 0;
        return FindNextPipe(NULL, &request);
    }

};


#else

/******************** 10.11 and higher *********************/
#include <IOKit/usb/IOUSBHostInterface.h>
#include <IOUSBDevice.h>
#include "EMUUSBLogging.h"
#include "IOUSBPipe.h"
//#include <IOKit/usb/USBSpec.h> deprecated. But where is USBAnyrDir?

class IOUSBInterface1: public IOUSBHostInterface {
public:
    /*!
     @function GetDevice
     returns the device the interface is part of.
     @result Pointer to the IOUSBDevice object which is the parent of this IOUSBInterface object.
     */
    inline IOUSBDevice *getDevice1() {
        return OSDynamicCast(IOUSBDevice ,getDevice());
    }
    
    /*!
     * @brief Return the current frame number of the USB bus
     *
     * @description This method will return the current frame number of the USB bus.  This is most useful for
     * scheduling future isochronous requests.
     *
     * @param theTime If not NULL, this will be updated with the current system time
     *
     * @result The current frame number
     */
    inline uint64_t getFrameNumber(AbsoluteTime* theTime = NULL) const {
        return getFrameNumber();
    }
    
    inline UInt8 getInterfaceNumber() {
        return getInterfaceDescriptor()->bInterfaceNumber;
    }
    
    /*! @result the index of the string descriptor describing the interface
     */
    inline UInt8  getInterfaceStringIndex() {
        return getInterfaceDescriptor()->iInterface;
    };
    
    /*!
     @function findPipe
     Find a pipe of the interface that matches the requirements,
     starting from the beginning of the interface's pipe list
     @param direction the direction for the required pipe. eg kUSBInterrupt or kUSBIsoc or kUSBAnyType
     @param type the type of the required pipe: kUSBIn or kUSBOut
     @result Pointer to the pipe, or NULL if no pipe matches the request.
     */
    IOUSBPipe* findPipe(uint8_t direction, uint8_t type) {
        debugIOLog("+findPipe: dir=%d, type = %d", direction, type);

        const StandardUSB::ConfigurationDescriptor* configDesc = getConfigurationDescriptor();
        if (configDesc==NULL)
        {
            debugIOLog("-findpipe: fail, no config descriptor available!");
            return NULL;
        }
        const StandardUSB::InterfaceDescriptor* ifaceDesc = getInterfaceDescriptor();
        if (ifaceDesc==NULL)
        {
            debugIOLog("-findpipe: fail, no interface descriptor available!");
            return NULL;
        }
        
        const EndpointDescriptor* ep = NULL;
        while ((ep = StandardUSB::getNextEndpointDescriptor(configDesc, ifaceDesc, ep)))
        {
            // check if endpoint matches type and direction
            uint8_t epDirection = StandardUSB::getEndpointDirection(ep);
            uint8_t epType = StandardUSB::getEndpointType(ep);
            
            debugIOLog("endpoint found: epDirection = %d, epType = %d", epDirection, epType);
            
            if ( direction == epDirection && type == epType )
            {
                IOUSBHostPipe* pipe = copyPipe(StandardUSB::getEndpointAddress(ep));
                if (pipe == NULL)
                {
                    debugIOLog("-findpipe: found matching pipe but copyPipe failed");
                    return NULL;
                }
                debugIOLog("-findpipe: success");
                pipe->release();
                return (IOUSBPipe *)pipe;
            }
        }
        debugIOLog("findPipe: no matching endpoint found");
        return NULL;
    }
    
};
#endif

#endif /* defined(__EMUUSBAudio__IOUSBInterface__) */
