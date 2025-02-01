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


#include "BackendFactory.hpp"

#if BACKEND_TYPE == BACKEND_TYPE_SHM

#include "WaylandBackendShm.hpp"
#define BACKEND_CLASS_NAME  WaylandBackendShm

#elif BACKEND_TYPE == BACKEND_TYPE_EGL

#include "WaylandBackendEgl.hpp"
#define BACKEND_CLASS_NAME  WaylandBackendEgl

#else

#error Unknown backend type

#endif

namespace subttxrend
{
namespace gfx
{

std::unique_ptr<Backend> BackendFactory::createBackend(BackendListener* listener)
{
    return std::unique_ptr<Backend>(new BACKEND_CLASS_NAME(listener));
}

} // namespace gfx
} // namespace subttxrend
