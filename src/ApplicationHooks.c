/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

void
vApplicationIdleHook(void)
{
}

void
vApplicationMallocFailedHook(void)
{
    while(1)
    { }
}

void
vApplicationStackOverflowHook(void)
{
    while(1)
    { }
}
