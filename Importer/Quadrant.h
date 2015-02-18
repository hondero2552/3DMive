#pragma once
#include "Debug_ALL.h"
#include <forward_list>
#include "IUIElement.h"

enum QUADRANT { ONE, TWO, THREE, FOUR };

class Quadrant
{
    std::forward_list<IUIElement*> m_UIElementsList;

public:
    Quadrant(void){ }

    std::forward_list<IUIElement*>& GetList(void) { return m_UIElementsList; };

    void AddUIElement(IUIElement* E) 
    {
        bool bExists = false;
        for_each(m_UIElementsList.begin(), m_UIElementsList.end(), [&] (IUIElement*& _ptr) 
        {
            if(_ptr == E)
                bExists = true;
        });
        
        // Only add if it doesn't exist
        if(!bExists)
            m_UIElementsList.push_front(E);
    }

private:
    Quadrant(const Quadrant& Q) { }
};