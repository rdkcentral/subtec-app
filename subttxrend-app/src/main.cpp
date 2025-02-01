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

#include "Application.hpp"
#include "Options.hpp"

using subttxrend::app::Application;
using subttxrend::app::Options;

auto registerSignalListener()
{
    // block signals in this thread and subsequently
    // spawned threads
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

    auto signal_handler = [sigset]() {
        int signum = 0;
        // wait until a signal is delivered:
        sigwait(&sigset, &signum);

        return signum;
    };

    return std::async(std::launch::async, signal_handler);
}

int main(int argc,
         char* argv[])
{
    auto exitListener = registerSignalListener();

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

        auto exitRequested = exitListener.get();
        std::cerr << "subttxrend-app signaled to exit" << std::endl;
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
