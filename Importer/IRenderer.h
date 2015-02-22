#if defined(WINDOWS) || defined(_WINDOWS)
#pragma once
#endif
#ifndef IRENDERER_H
#define IRENDERER_H

class IRenderer
{
public:
    IRenderer() {}
private:
    IRenderer(const IRenderer& _renderer) {}// private copy constructor
};

#endif