#if defined(WINDOWS) || defined(_WINDOWS)
#pragma once
#endif
#ifndef UIELEMENT_TYPES_H
#define UIELEMENT_TYPES_H

enum UIBUTTON 
{   
    AXESONOFF,

    BACKARROW,
    BACKFACECULLING,

    CAMERAMENU,
    CAMERAMOVEMENT,
    CAMERAZOOMSPEED,

    DELETEMESH,
    
    AMBIENT_TERM,
    DIFFUSE_TERM,
    SPECULAR_TERM,

    RED_CHANNEL,
    GREEN_CHANNEL,
    BLUE_CHANNEL,
    ALPHA_CHANNEL,
    SPECULAR_FACTOR,

    FLIPFACES,
    FOCUS_ON_MESH,

    ENVIRONMENTMENU,
    EXPORTMESH,
    
    GRIDONOFF,
    
    IMPORTMESH,
    IMPORTEXPORT,

    LIGHT_TO_EYE,
    LIGHT_FOLLOW_CAMERA,
    
    MATERIAL_EDIT_BUTTON,
    MATERIAL_SCROLL_BAR,
    MATERIALMENU,
    MESHMENU,
    MOUSECAPTURED,

    NO_BUTTON,    
    NORMALSONOFF,

    OUTSIDE_SCREEN,

    SHOWHIDECONTEXTMENU,

    TEXTURESONOFF,

    WIREFRAMEMODE
};

enum BUTTON_STATUS
{
    DEFAULT,
    PERMANENTLYACTIVE
};

enum HOVERING_EFFECT
{
    NOHOVERINGEFFECT,
    DRAWMASK_H,
    CHANGEBITMAP_H,
    DRAWOUTLINE_H
};

enum CLICKED_EFFECT
{
    NOCLICKEDEFFECT,
    CHANGEBITMAP_C,
    DRAWMASK_C
};

enum ACTIVE_EFFECT              // When the button is permanently active
{
    NOPERMANENTSTATUS,
    CHANGEBITMAP_A,
    DRAWMASK_A
};

enum DEFAULT_EFFECT
{
    NODEFAULTEFFECT,
    DRAWOUTLINE_D,
    DRAWMASK_D
};
#endif