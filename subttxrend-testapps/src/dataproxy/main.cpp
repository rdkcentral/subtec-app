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


#include <cstdlib>
#include <iostream>

#include "DataProxyApp.hpp"

int main(int argc,
         char* argv[])
{
    std::string appName = argv[0];
    std::vector<std::string> arguments;

    for (int i = 1; i < argc; ++i)
    {
        arguments.push_back(argv[i]);
    }

    subttxrend::testapps::DataProxyApp app(appName, arguments);

    return app.run();
}
