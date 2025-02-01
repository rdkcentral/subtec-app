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


#ifndef DVBSUBDECODER_DATABASE_HPP_
#define DVBSUBDECODER_DATABASE_HPP_

#include "Config.hpp"
#include "Status.hpp"
#include "Display.hpp"
#include "Page.hpp"
#include "Region.hpp"
#include "Clut.hpp"
#include "RenderingState.hpp"
#include "ObjectPool.hpp"
#include "ObjectTablePool.hpp"

namespace dvbsubdecoder
{

class PixmapAllocator;

/**
 * Subtitles database.
 *
 * The database class is responsible for holding parsed subtitles data and
 * managing the memory needed to store it.
 */
class Database
{
public:
    /**
     * Constructor.
     *
     * @param specVersion
     *      Selected specification version.
     * @param pixmapAllocator
     *      Pixmap allocator to use.
     */
    Database(Specification specVersion,
             PixmapAllocator& pixmapAllocator);

    /**
     * Destructor.
     */
    ~Database();

    /**
     * Resets the database on epoch start.
     *
     * Releases regions, cluts and pixmaps.
     */
    void epochReset();

    /**
     * Commits the page after it was sucesfully decoded.
     */
    void commitPage();

    /**
     * Returns status.
     *
     * @return
     *      Status data.
     */
    Status& getStatus()
    {
        return m_status;
    }

    /**
     * Returns current display definition.
     *
     * @return
     *      Display definition object.
     */
    Display& getCurrentDisplay()
    {
        return m_currentDisplay;
    }

    /**
     * Returns parsed display definition.
     *
     * @return
     *      Display definition object.
     */
    Display& getParsedDisplay()
    {
        return m_parsedDisplay;
    }

    /**
     * Returns page.
     *
     * @return
     *      Page object.
     */
    Page& getPage()
    {
        return m_page;
    }

    /**
     * Checks if region can be added.
     *
     * @return
     *      True if region can be added, false otherwise.
     */
    bool canAddRegion() const;

    /**
     * Returns region.
     *
     * @param id
     *      ID of the region to get.
     *
     * @return
     *      Region object or nullptr if there is no such region.
     */
    Region* getRegionById(std::uint8_t id)
    {
        return m_regions.getById(id);
    }

    /**
     * Returns number of defined regions.
     *
     * @return
     *      Number of defined regions.
     */
    std::size_t getRegionCount() const
    {
        return m_regions.getCount();
    }

    /**
     * Returns region.
     *
     * @param index
     *      Index of region.
     *
     * @return
     *      Region object or nullptr if there is no such region.
     */
    Region* getRegionByIndex(std::size_t index)
    {
        return m_regions.getByIndex(index);
    }

    /**
     * Returns CLUT.
     *
     * @param id
     *      ID of the CLUT to get.
     *
     * @return
     *      Region clut or nullptr if there is no such clut in database.
     */
    Clut* getClutById(std::uint8_t id)
    {
        return m_cluts.getById(id);
    }

    /**
     * Adds new region.
     *
     * @param regionId
     *      Region identifier.
     * @param width
     *      Region width.
     * @param height
     *      Region height.
     * @param compatibilityLevel
     *      Region compatibility level.
     * @param depth
     *      Region depth (code).
     * @param clutId
     *      Region clut ID.
     *
     * @return
     *      Added region or null on error.
     */
    Region* addRegionAndClut(std::uint8_t regionId,
                             std::int32_t width,
                             std::int32_t height,
                             std::uint8_t compatibilityLevel,
                             std::uint8_t depth,
                             std::uint8_t clutId);

    /**
     * Swaps current and previous rendering states.
     */
    void swapRenderingStates();

    /**
     * Returns current rendering state.
     *
     * @return
     *      Current rendering state.
     */
    RenderingState& getCurrentRenderingState();

    /**
     * Returns previous rendering state.
     *
     * @return
     *      Previous rendering state.
     */
    RenderingState& getPreviousRenderingState();

    /**
     * Removes all object references.
     *
     * @param region
     *      Region object.
     */
    void removeRegionObjects(Region* region);

    /**
     * Adds object reference to region.
     *
     * @param region
     *      Region to which object is added.
     * @param objectId
     *      Object identifier.
     * @param positionX
     *      Object position X.
     * @param positionY
     *      Object position Y.
     *
     * @return
     *      True if object was aded, false on error (no free references).
     */
    bool addRegionObject(Region* region,
                         std::uint16_t objectId,
                         std::int32_t positionX,
                         std::int32_t positionY);

private:
    /** Pixmap allocator. */
    PixmapAllocator& m_pixmapAllocator;

    /** Flag indicating that it is start of epoch. */
    bool m_isEpochStart;

    /** Status data. */
    Status m_status;

    /** Display definition (current). */
    Display m_currentDisplay;

    /** Display definition (parsed). */
    Display m_parsedDisplay;

    /** Page information. */
    Page m_page;

    /** Regions. */
    ObjectTablePool<Region, std::uint8_t, MAX_SUPPORTED_REGIONS> m_regions;

    /** CLUTs */
    ObjectTablePool<Clut, std::uint8_t, MAX_SUPPORTED_CLUTS> m_cluts;

    /** Pool of object references. */
    ObjectPool<ObjectInstance, MAX_SUPPORTED_OBJECTS> m_objectRefsPool;

    /** Rendering states. */
    RenderingState m_renderingStates[2];

    /** Rendering state - current. */
    RenderingState* m_renderingStatePrevious;

    /** Rendering state - previous. */
    RenderingState* m_renderingStateCurrent;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_DATABASE_HPP_*/
