#*******************************************************************************
#  Copyright (c) 2017 IBM Corp.
#
#  All rights reserved. This program and the accompanying materials
#  are made available under the terms of the Eclipse Public License v1.0
#  and Eclipse Distribution License v1.0 which accompany this distribution.
#
#  The Eclipse Public License is available at
#     http://www.eclipse.org/legal/epl-v10.html
#  and the Eclipse Distribution License is available at
#    http://www.eclipse.org/org/documents/edl-v10.php.
#
#  Contributors:
#     Ian Craggs - initial version
#*******************************************************************************/

PROJECT(mqtt-sn-tests)

INCLUDE_DIRECTORIES(../src)

ADD_EXECUTABLE(
  test1
  test1.c
)

TARGET_LINK_LIBRARIES(test1 MQTTSNPacket)

ADD_TEST(NAME test1 COMMAND test1)

ADD_EXECUTABLE(
  test2
  test2.c
)

TARGET_LINK_LIBRARIES(test2 MQTTSNPacket)