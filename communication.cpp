/* Copyright 2020 Purdea Andrei
 * Copyright 2022 Matthew Wolf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "communication.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
//#include "../../util_comm.h"
#include "./util_comm.h"
#include <wchar.h>

Communication::Communication()
{
    if (hid_init() != 0)
    {
        std::cerr << "Error: Unable to run hid_init()" << std::endl;
        exit(-1);
    };
}

Communication::~Communication()
{
    hid_exit();
}

std::vector<std::string> Communication::scan()
{
    QMutexLocker locker(&mutex);
    std::vector<std::string> ret;
    hid_device_info *usb_devs, *cur_usb_dev, *devinfo;

    int i = -1;
    int pid_codes_kbd_pid[2] = {0x4707, 0x6262};
    int qmk_kbd_pid[5] = {0X0F62, 0x1F62, 0x0F77, 0x0B53, 0x1B53};

    //printf("Scanning\n");
    //hid_device_info *enu = hid_enumerate(0x0481, 0x0002);
    //hid_device_info *devinfo = enu;

    usb_devs = hid_enumerate(0x0, 0x0);
    devinfo = NULL;

    cur_usb_dev = usb_devs;
    while (cur_usb_dev) {
	    if (cur_usb_dev->vendor_id == PID_CODES_USB_PID) {
                    for (i=0; i < (int)(sizeof(pid_codes_kbd_pid) / sizeof(pid_codes_kbd_pid[0])); i++) {
                            if (cur_usb_dev->product_id == pid_codes_kbd_pid[i]) {
                                    devinfo = hid_enumerate(PID_CODES_USB_PID, cur_usb_dev->product_id);
                            }
                    }
	    } else if (cur_usb_dev->vendor_id == QMK_USB_PID) {
                    for (i=0; i < (int)(sizeof(qmk_kbd_pid) / sizeof(qmk_kbd_pid[0])); i++) {
                            if (cur_usb_dev->product_id == qmk_kbd_pid[i]) {
                                    devinfo = hid_enumerate(QMK_USB_PID, cur_usb_dev->product_id);
                            }
                    }
            } else if (cur_usb_dev->vendor_id == PANDREW_USB_ID ) {
                    if (cur_usb_dev->product_id == 0x0002) {
                            devinfo = hid_enumerate(PANDREW_USB_ID, 0x0002);
                    }
            }
            cur_usb_dev = cur_usb_dev->next;
    }
    hid_free_enumeration(usb_devs);

    while (devinfo != NULL)
    {
        if ((NULL != devinfo->manufacturer_string) &&
            (NULL != devinfo->product_string) &&
            (0==wcscmp(devinfo->manufacturer_string, L"Tom Wong-Cornall")) &&
            (0==wcscmp(devinfo->product_string, L"ibm-capsense-usb")))
        {
            if (devinfo->interface_number == 1)
            {
                ret.push_back(std::string(devinfo->path) + XWHATSIT_ENDING_STRING);
            }
        } else {
            if (devinfo->interface_number == 1)
            {
                ret.push_back(devinfo->path);
            }
        }
        devinfo = devinfo->next;
    }
    hid_free_enumeration(devinfo);
    return ret;
}

Device *Communication::open(std::string path)
{
    return new Device(path, mutex);
}
