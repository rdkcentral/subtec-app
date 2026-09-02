/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 Liberty Global Service B.V.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************************/


#include <future>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <system_error>
#include <pthread.h>

#include "Application.hpp"
#include <subttxrend/ctrl/Options.hpp>

using subttxrend::app::Application;
using subttxrend::ctrl::Options;

static bool blockTerminationSignals(sigset_t& sigset)
{
    if (sigemptyset(&sigset) != 0) {
        return false;
    }

    if (sigaddset(&sigset, SIGINT) != 0) {
        return false;
    }

    if (sigaddset(&sigset, SIGTERM) != 0) {
        return false;
    }

    if (pthread_sigmask(SIG_BLOCK, &sigset, nullptr) != 0) {
        return false;
    }

    return true;
}

static auto createSignalListener(const sigset_t& sigset)
{
    return std::async(std::launch::async, [sigset]() {
        int signum = 0;
        const int rc = sigwait(&sigset, &signum);
        if (rc != 0) {
            throw std::system_error(rc, std::generic_category(), "sigwait failed");
        }
        return signum;
    });
}

int main(int argc,
         char* argv[])
{
#ifndef __APPLE__
    sigset_t exitSignalSet;
    if (!blockTerminationSignals(exitSignalSet)) {
        std::cerr << "Failed to block termination signals" << std::endl;
        return EXIT_FAILURE;
    }
#endif
    int rv = EXIT_FAILURE;

    try {
        Options options(argc, argv);

        if (!options.isValid()) {
            options.printUsage();
            return EXIT_FAILURE;
        }

        if (options.hasSeparate()) {
            if (options.getSeparate() == Options::Key::HELP) {
                options.printUsage();
                return EXIT_SUCCESS;
            } else {
                std::cerr << "Internal error. Separate option not handled correctly." << std::endl;
                return EXIT_FAILURE;
            }
        }

        Application app(options);
        app.runAsync();
        std::cerr << "subttxrend-app started" << std::endl;

#ifndef __APPLE__
        auto exitListener = createSignalListener(exitSignalSet);
        auto signalNum = exitListener.get();
        std::cerr << "subttxrend-app signaled (" << signalNum << ") to exit" << std::endl;
#else // __APPLE__
        app.startBlockingApplicationWindow();
#endif
        app.quit();
        rv = EXIT_SUCCESS;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught" << std::endl;
    }

    return rv;
}
