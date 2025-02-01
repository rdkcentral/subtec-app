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


#ifndef DVBSUBDECODER_REGION_HPP_
#define DVBSUBDECODER_REGION_HPP_

#include <cstdint>
#include <array>

#include "Consts.hpp"
#include "Clut.hpp"
#include "Pixmap.hpp"
#include "ObjectInstance.hpp"

namespace dvbsubdecoder
{

class Clut;

/**
 * Region.
 *
 * Information about single subtitles region.
 */
class Region
{
public:
    /**
     * Constructor.
     */
    Region();

    /**
     * Destructor.
     */
    ~Region() = default;

    /**
     * Resets region to default state.
     */
    void reset();

    /**
     * Initializes region.
     *
     * @param width
     *      Region width.
     * @param height
     *      Region height.
     * @param pixmapMemory
     *      Region pixmap memory.
     * @param compatibilityLevel
     *      Region compatibility level.
     * @param depth
     *      Region depth.
     * @param clut
     *      Region clut.
     */
    void init(std::int32_t width,
              std::int32_t height,
              std::uint8_t* pixmapMemory,
              std::uint8_t compatibilityLevel,
              std::uint8_t depth,
              Clut* clut);

    /**
     * Returns region ID.
     *
     * @return
     *      Region id.
     */
    std::uint8_t getId() const;

    /**
     * Sets region id.
     *
     * @param id
     *      Region id.
     */
    void setId(std::uint8_t id);

    /**
     * Returns region version.
     *
     * @return
     *      Region version.
     */
    std::uint8_t getVersion() const;

    /**
     * Sets region version.
     *
     * @param version
     *      Version.
     */
    void setVersion(std::uint8_t version);

    /**
     * Returns region background color index.
     *
     * @return
     *      Background color index.
     */
    std::uint8_t getBackgroundIndex() const;

    /**
     * Sets region background color index.
     *
     * @param index
     *      Background color index.
     */
    void setBackgroundIndex(std::uint8_t index);

    /**
     * Removes first region object.
     *
     * @return
     *      First object reference or null if there are none in the region.
     */
    ObjectInstance* removeFirstObject()
    {
        return m_objectRefs.removeFirst();
    }

    /**
     * Adds region object.
     *
     * @param objectRef
     *      Object to be added.
     */
    void addObject(ObjectInstance* objectRef)
    {
        m_objectRefs.add(objectRef);
    }

    /**
     * Returns first region object.
     *
     * @return
     *      First object reference or null if there are none in the region.
     */
    const ObjectInstance* getFirstObject() const
    {
        return m_objectRefs.getFirst();
    }

    /**
     * Returns next region object.
     *
     * @param prev
     *      Previous object.
     *
     * @return
     *      First object reference or null if there are none in the region.
     */
    const ObjectInstance* getNextObject(const ObjectInstance* prev) const
    {
        return m_objectRefs.getNext(prev);
    }

    /**
     * Returns region width.
     *
     * @return
     *      Width in pixels.
     */
    std::int32_t getWidth() const;

    /**
     * Returns region height.
     *
     * @return
     *      Height in pixels.
     */
    std::int32_t getHeight() const;

    /**
     * Returns region compatibility level.
     *
     * Note this is coded according to spec, not as number of bits.
     *
     * @return
     *      Compatibility level.
     */
    std::uint8_t getCompatibilityLevel() const;

    /**
     * Returns region depth.
     *
     * Note this is coded according to spec, not as number of bits.
     *
     * @return
     *      Depth of the region.
     */
    std::uint8_t getDepth() const;

    /**
     * Returns region CLUT id.
     *
     * @return
     *      CLUT identifier.
     */
    std::uint8_t getClutId() const;

    /**
     * Returns region pixmap.
     *
     * @return
     *      Region pixmap.
     */
    Pixmap& getPixmap();

    /**
     * Returns region CLUT.
     *
     * @return
     *      CLUT object. May be null if not set.
     */
    Clut* getClut();

    /**
     * Returns CLUT array.
     *
     * @return
     *      CLUT array matching display's depth.
     */
    const std::uint32_t* getClutArray() const;

private:
    /** ID. */
    std::uint8_t m_id;

    /** Version. */
    std::uint8_t m_version;

    /** Region pixmap. */
    Pixmap m_pixmap;

    /** Region CLUT (null if not set). */
    Clut* m_clut;

    /** Compatibility level. */
    std::uint8_t m_compatibilityLevel;

    /** Depth. */
    std::uint8_t m_depth;

    /** Background color index. */
    std::uint8_t m_backgroundIndex;

    /** Object references. */
    ObjectInstanceList m_objectRefs;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_REGION_HPP_*/
