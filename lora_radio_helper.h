/**
 * Copyright (c) 2019, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "lorawan/LoRaRadio.h"

#ifndef APP_LORA_RADIO_HELPER_H_
#define APP_LORA_RADIO_HELPER_H_

#include "SX1276_LoRaRadio.h"

SX1276_LoRaRadio radio(PB_15,
                       PB_14,
                       PB_13,
                       PB_12,
                       PE_6,
                       PE_5,
                       PE_4,
                       PE_3,
                       PE_2,
                       PE_1,
                       PE_0,
                       NC,
                       NC,
                       NC,
                       NC,
                       NC,
                       NC,
                       NC);

#endif /* APP_LORA_RADIO_HELPER_H_ */
