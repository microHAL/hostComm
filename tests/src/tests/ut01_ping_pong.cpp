/* ========================================================================================================================== */ /**
 @license    BSD 3-Clause
 @copyright  microHAL
 @version    $Id$
 @brief      hostComm unit test, ping pong test

 @authors    Pawel Okas
 created on: 14-09-2015
 last modification: <DD-MM-YYYY>

 @copyright Copyright (c) 2014, microHAL
 All rights reserved.
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following
 conditions are met:
 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
 in the documentation and/or other materials provided with the distribution.
 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived
 from this software without specific prior written permission.
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */ /* ========================================================================================================================== */

#include "doctest.h"

#include <atomic>
#include <thread>
#include "bsp.h"
#include "hostComm.h"
#include "microhal.h"
#include "ut_common.h"

using namespace microhal;
using namespace std::chrono_literals;

TEST_CASE("Ping Pong check") {
    // ports should be open
    REQUIRE(communicationPortA.isOpen());
    REQUIRE(communicationPortB.isOpen());

    // clear ports
    REQUIRE(communicationPortA.clear());
    REQUIRE(communicationPortB.clear());

    // create hostComm device
    HostComm hostCommA(communicationPortA, debugPort, "HostComm A: ");

    INFO("Starting timeProc thread.");

    //	volatile bool run = true;
    //
    //	//create and run hostComm proc task
    //	std::thread hostCommThreadA (procThread, &hostCommA, &run);

    hostCommA.startHostCommThread();

    INFO("Sending ping packets.");
    CHECK(hostCommA.ping(false));
    // these should be failure
    CHECK_FALSE(hostCommA.ping(true));

    REQUIRE(communicationPortB.clear());
    HostComm hostCommB(communicationPortB, debugPort, "HostComm B: ");
    // create and run second hostComm proc task
    hostCommB.startHostCommThread();

    CHECK(hostCommA.ping(true));

    CHECK(hostCommA.ping(false));

    CHECK(hostCommA.ping(false));

    CHECK(hostCommA.ping(true));

    // close hostComm proc thread
    hostCommA.stopHostCommThread();
    hostCommB.stopHostCommThread();
}
