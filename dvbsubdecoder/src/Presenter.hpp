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


#ifndef DVBSUBDECODER_PRESENTER_HPP_
#define DVBSUBDECODER_PRESENTER_HPP_

namespace dvbsubdecoder
{

class DecoderClient;
class Database;
class Rectangle;
class RenderingState;

/**
 * Subtitles presenter.
 *
 * The Presenter class is responsible for performing graphical operations
 * required to render the subtitles on screen. Using current state and
 * decoded data it calls the client interface to execute steps needed to
 * draw subtitles.
 */
class Presenter
{
public:
    /**
     * Constructor.
     *
     * @param client
     *      Client interface.
     * @param database
     *      Database to use.
     */
    Presenter(DecoderClient& client,
              Database& database);

    /**
     * Destructor.
     */
    ~Presenter() = default;

    /**
     * Resets presentation state.
     */
    void reset();

    /**
     * Draws the subtitles.
     *
     * Requests the decoder to invoke the client callbacks to render the
     * subtitles on-screen.
     */
    void draw();

    /**
     * Invalidates the specified subtitle decoder.
     */
    void invalidate();

private:
    /**
     * Builds current state.
     *
     * @param state
     *      State to fill with current bounds and regions to render.
     */
    void buildCurrentState(RenderingState& state);

    /**
     * Notifies new bounds if needed.
     *
     * @param currentState
     *      Current state.
     * @param previousState
     *      Previous state.
     * @param modifiedRectangle
     *      Modified area rectangle to be calculated.
     */
    void notifyScreenBounds(RenderingState& currentState,
                            RenderingState& previousState,
                            Rectangle& modifiedRectangle);

    /**
     * Detects regions that were not modified.
     *
     * Updates the dirty flag in regions.
     *
     * @param currentState
     *      Current state.
     * @param previousState
     *      Previous state.
     */
    void detectUnmodifiedRegions(RenderingState& currentState,
                                 RenderingState& previousState);

    /**
     * Clears regions.
     *
     * Issues rectangle clear operations if there are regions in
     * previous state which are no longer present.
     *
     * @param previousState
     *      Previous state.
     * @param modifiedRectangle
     *      Modified area rectangle to be calculated.
     */
    void clearRegions(const RenderingState& previousState,
                      Rectangle& modifiedRectangle);

    /**
     * Draws regions.
     *
     * Issues rectangle draw operations if there are regions in
     * current state which were not present in previous state.
     *
     * @param currentState
     *      Current state.
     * @param modifiedRectangle
     *      Modified area rectangle to be calculated.
     */
    void drawRegions(const RenderingState& currentState,
                     Rectangle& modifiedRectangle);

    /**
     * Checks if one rectangle is inside another.
     *
     * @param bounds
     *      Rectangle defining bounds.
     * @param rect
     *      Rectangle to check if it is inside.
     *
     * @return
     *      True if given rectangle is within bounds, false otherwise.
     */
    bool isRectangleInside(const Rectangle& bounds,
                           const Rectangle& rect);

    /** Client interface. */
    DecoderClient& m_client;

    /** Database to use. */
    Database& m_database;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_PRESENTER_HPP_*/
