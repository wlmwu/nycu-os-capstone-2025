#ifndef MAILBOX_H_
#define MAILBOX_H_

#define LEN_MBOX_RESPONSE_MAX 8

// Mailbox Channels
// https://github.com/raspberrypi/firmware/wiki/Mailboxes
typedef enum {
    kChPowerManagement = 0,
    kChFramebuffer,
    kChVirtualUart,
    kChVchiq,
    kChLeds,
    kChButtons,
    kChTouchScreen,
    kChUnused,
    kChArm2VC,
    kChVC2Arm,
} mbox_channel_t;

// Tag (ARM to VC)
// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
typedef enum {
    /* Videocore */
    kTagGetFirmwareRevision = 0x1,
    /* Hardware */
    kTagGetBoardModel = 0x10001,
    kTagGetBoardRevision,
    kTagGetArmMemory = 0x10005,
} mbox_tag_id_t;

// Request/Response Codes
// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
// Buffer contents section
typedef enum {
    kRCodeRequest = 0x00000000,
    kRCodeSucceed = 0x80000000,
    kRCodeFailed = 0x80000001,
} mbox_request_code_t;

// Tag Request/Response Codes
// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
// Tag format section
typedef enum {
    kTCodeRequest = 0x00000000,
    kTCodeResponse = 0x10000000,
} mbox_tag_request_code_t;

// Tag Request/Response Codes
// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
// Buffer contents section
typedef enum {
    kBufEndTag = 0x00000000,
} MBoxBuffer;

// Mailbox Status Codes
// https://github.com/bieltura/brcm_android_ICS_graphics_stack/blob/master/brcm_usrlib/dag/vmcsx/vcinclude/bcm2708_chip/arm_control.h
// Line 302
typedef enum {
    kStatusFull = 0x80000000,
    kStatusEmpty = 0x40000000,
} MBoxStatus;

/*
    output: output buffer
    tag:    tag identifier
    num_res:    num of results
*/
void mbox_get_info(char* output, mbox_tag_id_t tag, unsigned int num_res);
int mbox_call(unsigned int*);

#endif // MAILBOX_H_