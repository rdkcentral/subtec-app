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


#ifndef WAYLANDCPP_FILE_HPP_
#define WAYLANDCPP_FILE_HPP_

#include "Types.hpp"

#include <memory>
#include <string>

namespace waylandcpp
{

/**
 * File wrapper and utilities.
 *
 * This class could be used to create temporary file which then could
 * be mmaped and used as a shared buffers.
 */
class File : public std::enable_shared_from_this<File>
{
    File(const File&) = delete;
    File& operator=(const File&) = delete;

public:
    /** File shared pointer type. */
    typedef std::shared_ptr<File> Ptr;

    /**
     * Creates a new, unique, anonymous file
     *
     * @param size
     *      Size of the file to create.
     *
     * @return
     *      Created file handle or null on error.
     */
    static FilePtr createMappedShmFile(std::size_t size);

    /**
     * Destructor.
     */
    ~File();

    /**
     * Returns native object (file handle).
     *
     * @return
     *      File handle.
     */
    int getNativeObject();

    /**
     * Returns size of the file.
     *
     * @return
     *      Size of the file.
     */
    std::size_t getSize() const;

    /**
     * Checks if file is mapped into memory.
     *
     * @retval true
     *      File is mapped.
     * @retval false
     *      File is not mapped.
     */
    bool isMapped() const;

    /**
     * Returns mapped memory pointer.
     *
     * @return
     *      Mapped memory (null if not mapped).
     */
    uint8_t* getMapping() const;

private:
    /**
     * Constructor.
     *
     * @param fd
     *      Wrapped file descriptor.
     * @param size
     *      Size of the file.
     */
    File(int fd,
         std::size_t size);

    /**
     * Maps into memory.
     *
     * If the file is already mapped returned the previous mapping.
     *
     * The mapping is done for READ+WRITE protection and with SHARED flags.
     *
     * @return
     *      Pointer to mapped memory or null on error.
     */
    void* mmap();

    /**
     * Unmaps the memory.
     *
     * If file was not mapped does nothing.
     */
    void munmap();

    /**
     * Creates anonymous temporary file.
     *
     * @param size
     *      Size of the file to create.
     *
     * @return
     *      Created file handle or -1 on error.
     */
    static int osCreateAnonymousFile(off_t size);

    /**
     * Creates CLOEXEC temporary file.
     *
     * @param tmpname
     *      File name template.
     *
     * @return
     *      Create file handle or -1 on error.
     */
    static int createTmpFileCloexec(char* tmpname);

    /**
     * Sets CLOEXEC on file.
     *
     * @param fd
     *      File descriptor.
     *
     * @return
     *      File handle or -1 on error (the given file is closed on error).
     */
    static int setCloexecOrClose(int fd);

    /** Wrapped file descriptor. */
    const int m_fd;

    /** File size. */
    const std::size_t m_size;

    /** Mapped memory pointer. */
    void* m_memory;
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_FILE_HPP_*/
