#include "mailbox.h"
#include "reg_mailbox.h"
#include "utils.h"

void mbox_get_info(char* output, mbox_tag_id_t tag, unsigned int num_res) {
    unsigned int  __attribute__((aligned(16))) buf[LEN_MBOX_RESPONSE_MAX];  // Mailbox gets only upper 28 bits as the address (lower 4 bits is channel). `aligned(16)` is to ensure the start address is `0x...10`.
    buf[0] = LEN_MBOX_RESPONSE_MAX * 4;     // buffer size in bytes
    buf[1] = kRCodeRequest;                 // buffer request/response code
                                            // sequence of concatenated tags
    buf[2] = tag;                                // tag identifier
    buf[3] = num_res * 4;                        // value buffer size in bytes
    buf[4] = kTCodeRequest;                      // request/response code
    buf[5] = 0;                                  // value buffer
    buf[6] = 0;                                  // value buffer (if buf[3] == 4, this one can be eliminated or ignore)
    buf[7] = kBufEndTag;                    // 0x0 (end tag)

    if (mbox_call(buf) ) {
        for (int i = 0; i < num_res; ++i) {
            int offset = i * LEN_U32_HEX_STR;
            output[0 + offset] = '0';
            output[1 + offset] = 'x';
            uint2hexstr(&output[2 + offset], buf[5 + i]);
            output[10 + offset] = '\0';
        }
    }
}

int mbox_call(unsigned int *buf) {
    unsigned int msg = (unsigned int)(unsigned long)buf;        // Create mailbox message. First, get buf address.
    msg &= ~(0xF);                                              // Clear lower 4 bits for channel
    msg |= kChArm2VC;

    while ( *MAILBOX_STATUS & kStatusFull );
    *MAILBOX_WRITE = msg;
    while ( *MAILBOX_STATUS & kStatusEmpty );
    if (msg == *MAILBOX_READ) {
        return buf[1] == kRCodeSucceed;
    } else {
        return 0;
    }

}