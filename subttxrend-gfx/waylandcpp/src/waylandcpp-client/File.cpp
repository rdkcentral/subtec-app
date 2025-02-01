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


#include "File.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>
#include <cassert>
#include <cerrno>

namespace waylandcpp
{

FilePtr File::createMappedShmFile(std::size_t size)
{
    int fd = osCreateAnonymousFile(size);
    if (fd != -1)
    {
        auto file = FilePtr(new File(fd, size));
        if (file->mmap())
        {
            return file;
        }
    }

    return nullptr;
}

File::File(int fd,
           std::size_t size) :
        m_fd(fd),
        m_size(size),
        m_memory(MAP_FAILED)
{
    // noop
}

File::~File()
{
    munmap();
    (void) ::close(m_fd);
}

std::uint8_t* File::getMapping() const
{
    if (isMapped())
    {
        return reinterpret_cast<std::uint8_t*>(m_memory);
    }
    else
    {
        return nullptr;
    }
}

int File::getNativeObject()
{
    return m_fd;
}

std::size_t File::getSize() const
{
    return m_size;
}

bool File::isMapped() const
{
    return (m_memory != MAP_FAILED);
}

void* File::mmap()
{
    if (!isMapped())
    {
        m_memory = ::mmap(NULL, m_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                m_fd, 0);
    }

    return getMapping();
}

void File::munmap()
{
    if (isMapped())
    {
        (void) ::munmap(m_memory, m_size);
        m_memory = MAP_FAILED;
    }
}

int File::osCreateAnonymousFile(off_t size)
{
    static const std::string fileNameTemplate("/weston-shared-XXXXXX");

    const char* xdgRuntimeDirEnv = ::getenv("XDG_RUNTIME_DIR");
    if (!xdgRuntimeDirEnv)
    {
        errno = ENOENT;
        return -1;
    }
    const std::string path(xdgRuntimeDirEnv);

    std::string fileNameBuffer = path + fileNameTemplate;

    std::unique_ptr<char[]> nameTemplateBuffer(
            new char[fileNameBuffer.length() + 1]);
    (void) std::strcpy(nameTemplateBuffer.get(), fileNameBuffer.c_str());

    int fd = createTmpFileCloexec(nameTemplateBuffer.get());
    if (fd < 0)
    {
        return -1;
    }

    if (::ftruncate(fd, size) < 0)
    {
        (void) ::close(fd);
        return -1;
    }

    /* ARRISEOS-23286 - Added allocation of the storage space for the temporary file
     *  /run/weston/weston-shared-XXXXXX
     *  which is mmapped as shared memory and used as Wayland buffer as ftruncate does not
     *  perform actual write to the file.
     *
     * If allocation fails process is terminated due to failed assertion.
     *
     * Without allocation if the file was created on the filesystem which has not enough free space,
     * an attempt to write to the Wayland buffer results in the process using subttxrend-gfx being
     * terminated by SIGBUS.
     */
    int status =  fallocate(fd, 0, 0, size);
    if (status < 0)
    {
        (void) ::close(fd);
        assert(status >= 0);
        return -1;
    }

    return fd;
}

int File::createTmpFileCloexec(char* tmpname)
{
    int fd = -1;

#ifdef HAVE_MKOSTEMP
    fd = ::mkostemp(tmpname, O_CLOEXEC);
    if (fd >= 0)
    {
        ::unlink(tmpname);
    }
#else
    fd = ::mkstemp(tmpname);
    if (fd >= 0)
    {
        fd = setCloexecOrClose(fd);
        ::unlink(tmpname);
    }
#endif

    return fd;
}

int File::setCloexecOrClose(int fd)
{
    do
    {
        if (fd == -1)
        {
            break;
        }

        long flags = ::fcntl(fd, F_GETFD);
        if (flags == -1)
        {
            break;
        }

        if (::fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
        {
            break;
        }

        return fd;
    } while (false);

    if (fd != -1)
    {
        (void) ::close(fd);
    }

    return -1;
}

} // namespace waylandcpp
