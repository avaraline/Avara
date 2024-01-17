#pragma once

class Renderer {
public:
    virtual ~Renderer() {};

    /**
     * Update the game window with the latest rendered frame.
     */
    virtual void RefreshWindow() = 0;
    
    /**
     * Check to see if this renderer supports static world objects.
     *
     * @return whether the renderer supports a static world
     */
    virtual bool UsesStaticWorld() = 0;
};
