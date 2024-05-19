/*
 * Copyright (C) 2019-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2019-2020 Jolla Ltd.
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

#ifndef NCI_TRANSITION_H
#define NCI_TRANSITION_H

#include "nci_types_p.h"

#include <glib-object.h>

/* State transition */

typedef struct nci_transition_priv NciTransitionPriv;

struct nci_transition {
    GObject object;
    NciTransitionPriv* priv;
    NciState* dest;
};

GType nci_transition_get_type(void) NCI_INTERNAL;
#define NCI_TYPE_TRANSITION (nci_transition_get_type())
#define NCI_TRANSITION(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
        NCI_TYPE_TRANSITION, NciTransition))

typedef
void
(*NciTransitionFunc)(
    NciTransition* transition,
    void* user_data);

NciTransition*
nci_transition_ref(
    NciTransition* transition)
    NCI_INTERNAL;

void
nci_transition_unref(
    NciTransition* transition)
    NCI_INTERNAL;

gboolean
nci_transition_start(
    NciTransition* transition)
    NCI_INTERNAL;

void
nci_transition_finished(
    NciTransition* transition)
    NCI_INTERNAL;

void
nci_transition_handle_ntf(
    NciTransition* transition,
    guint8 gid,
    guint8 oid,
    const GUtilData* payload)
    NCI_INTERNAL;

/* Specific transitions */

NciTransition*
nci_transition_reset_new(
    NciSm* sm)
    NCI_INTERNAL;

NciTransition*
nci_transition_idle_to_discovery_new(
    NciSm* sm)
    NCI_INTERNAL;

NciTransition*
nci_transition_deactivate_to_discovery_new(
    NciSm* sm)
    NCI_INTERNAL;

NciTransition*
nci_transition_deactivate_to_idle_new(
    NciSm* sm)
    NCI_INTERNAL;

NciTransition*
nci_transition_listen_active_to_idle_new(
    NciSm* sm)
    NCI_INTERNAL;

NciTransition*
nci_transition_poll_active_to_idle_new(
    NciSm* sm)
    NCI_INTERNAL;

#endif /* NCI_TRANSITION_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
