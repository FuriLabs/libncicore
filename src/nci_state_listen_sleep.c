/*
 * Copyright (C) 2020-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2020 Jolla Ltd.
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *     distribution.
 *
 *  3. Neither the names of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#include "nci_sm.h"
#include "nci_state_impl.h"
#include "nci_util_p.h"
#include "nci_log.h"

typedef NciState NciStateListenSleep;
typedef NciStateClass NciStateListenSleepClass;

GType nci_state_listen_sleep_get_type(void) NCI_INTERNAL;
#define THIS_TYPE (nci_state_listen_sleep_get_type())
#define PARENT_CLASS (nci_state_listen_sleep_parent_class)

G_DEFINE_TYPE(NciStateListenSleep, nci_state_listen_sleep, NCI_TYPE_STATE)

/*==========================================================================*
 * Implementation
 *==========================================================================*/

static
void
nci_state_listen_sleep_intf_activated_ntf(
    NciState* self,
    const GUtilData* payload)
{
    NciIntfActivationNtf ntf;
    NciModeParam mode_param;
    NciActivationParam activation_param;
    NciSm* sm = nci_state_sm(self);

    /*
     * [NFCForum-TS-NCI-1.0]
     * 5.2.7 State RFST_LISTEN_SLEEP
     *
     * ...
     * If the NFCC receives a valid RF wake up command(s) followed
     * by successful activation procedure, the NFCC SHALL send
     * RF_INTF_ACTIVATED_NTF (Listen mode) to the DH. At that point,
     * the state is changed back to RFST_LISTEN_ACTIVE.
     */
    if (nci_parse_intf_activated_ntf(&ntf, &mode_param, &activation_param,
        payload->bytes, payload->size)) {
        if (nci_listen_mode(ntf.mode)) {
            /*
             * Switch the state first because RF_INTF_ACTIVATED_NTF handler
             * may want to change the state again (e.g. if configuration is
             * unsupported).
             */
            nci_sm_enter_state(sm, NCI_RFST_LISTEN_ACTIVE, NULL);
            nci_sm_intf_activated(sm, &ntf);
            return;
        } else {
            GDEBUG("Unexpected activation mode 0x%02x", ntf.mode);
        }
    }
    /* Oops */
    nci_sm_stall(sm, NCI_STALL_ERROR);
}

static
void
nci_state_listen_sleep_rf_deactivate_ntf(
    NciState* self,
    const GUtilData* payload)
{
    NciSm* sm = nci_state_sm(self);
    NciRfDeactivateNtf ntf;

    if (nci_parse_rf_deactivate_ntf(&ntf, payload)) {
        switch (ntf.type) {
        case NCI_DEACTIVATE_TYPE_DISCOVERY:
            /*
             * [NFCForum-TS-NCI-1.0]
             * 5.2.7 State RFST_LISTEN_SLEEP
             *
             * ...
             * On detection of remote RF field off, the NFCC SHALL send
             * RF_DEACTIVATE_NTF (Discovery, RF Link Loss) to the DH. The
             * RF Communication state will then change to RFST_DISCOVERY.
             *
             * For NFC-B Technology, when the NFCC detects a command during
             * the RF communication, which forces returning to the IDLE state,
             * as defined in the [ACTIVITY] Listen Mode state machine, the
             * NFCC SHALL send an RF_DEACTIVATE_NTF (Discovery, NFC-B_Bad_AFI)
             * to the DH. The state will then change to RFST_DISCOVERY.
             */
            nci_sm_enter_state(sm, NCI_RFST_DISCOVERY, NULL);
            return;
        case NCI_DEACTIVATE_TYPE_SLEEP_AF:
        case NCI_DEACTIVATE_TYPE_SLEEP:
            /* We are already there */
        case NCI_DEACTIVATE_TYPE_IDLE:
            /* This one is not supposed to happen spontaneously */
            break;
        }
        GDEBUG("Unexpected RF_DEACTIVATE_NTF");
    }
    /* Oops */
    nci_sm_stall(sm, NCI_STALL_ERROR);
}

/*==========================================================================*
 * Interface
 *==========================================================================*/

NciState*
nci_state_listen_sleep_new(
    NciSm* sm)
{
    NciState* self = g_object_new(THIS_TYPE, NULL);

    nci_state_init_base(self, sm, NCI_RFST_LISTEN_SLEEP, "RFST_LISTEN_SLEEP");
    return self;
}

/*==========================================================================*
 * Methods
 *==========================================================================*/

static
void
nci_state_listen_sleep_handle_ntf(
    NciState* self,
    guint8 gid,
    guint8 oid,
    const GUtilData* payload)
{
    switch (gid) {
    case NCI_GID_RF:
        switch (oid) {
        case NCI_OID_RF_INTF_ACTIVATED:
            nci_state_listen_sleep_intf_activated_ntf(self, payload);
            return;
        case NCI_OID_RF_DEACTIVATE:
            nci_state_listen_sleep_rf_deactivate_ntf(self, payload);
            return;
        }
        break;
    }
    NCI_STATE_CLASS(PARENT_CLASS)->handle_ntf(self, gid, oid, payload);
}

/*==========================================================================*
 * Internals
 *==========================================================================*/

static
void
nci_state_listen_sleep_init(
    NciStateListenSleep* self)
{
}

static
void
nci_state_listen_sleep_class_init(
    NciStateListenSleepClass* klass)
{
    NCI_STATE_CLASS(klass)->handle_ntf = nci_state_listen_sleep_handle_ntf;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
