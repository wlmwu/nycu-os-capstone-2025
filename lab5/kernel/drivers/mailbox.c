#include "mailbox.h"
#include "reg_mailbox.h"
#include "utils.h"
#include <stdint.h>

void mbox_get_info(uint32_t* output, mbox_tag_id_t tag, unsigned int n_ret) {
    unsigned int  __attribute__((aligned(16))) buf[LEN_MBOX_RESPONSE_MAX];  // Mailbox gets only upper 28 bits as the address (lower 4 bits is channel). `aligned(16)` is to ensure the start address is `0x...10`.
    buf[0] = LEN_MBOX_RESPONSE_MAX * 4;     // buffer size in bytes
    buf[1] = kRCodeRequest;                 // buffer request/response code
                                            // sequence of concatenated tags
    buf[2] = tag;                                // tag identifier
    buf[3] = n_ret * 4;                        // value buffer size in bytes
    buf[4] = kTCodeRequest;                      // request/response code
    buf[5] = 0;                                  // value buffer
    buf[6] = 0;                                  // value buffer (if buf[3] == 4, this one can be eliminated or ignore)
    buf[7] = kBufEndTag;                    // 0x0 (end tag)

    if (mbox_call(buf, kChArm2VC)) {
        for (int i = 0; i < n_ret; ++i) {
            output[i] = buf[5 + i];
        }
    }
}

int mbox_call(unsigned int *buf, mbox_channel_t channel) {
    unsigned int msg = (unsigned int)(unsigned long)buf;        // Create mailbox message. First, get buf address.
    msg &= ~(0xF);                                              // Clear lower 4 bits for channel
    msg |= channel;

    while ( *MAILBOX_STATUS & kStatusFull );
    *MAILBOX_WRITE = msg;
    while ( *MAILBOX_STATUS & kStatusEmpty );
    if (msg == *MAILBOX_READ) {
        return buf[1] == kRCodeSucceed;
    } else {
        return 0;
    }
}