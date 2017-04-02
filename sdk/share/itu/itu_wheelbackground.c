#include <assert.h>
#include <math.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

static const char wheelBackgroundName[] = "ITUWheelBackground";

bool ituWheelBackgroundUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUWheelBackground* wbg = (ITUWheelBackground*) widget;
    assert(wbg);

    if ((ev == ITU_EVENT_MOUSEDOWN) || (ev == ITU_EVENT_MOUSEMOVE && wbg->pressed))
    {
        if (ituWidgetIsEnabled(widget))
        {
            int x = arg2 - widget->rect.x;
            int y = arg3 - widget->rect.y;

            if (ituWidgetIsInside(widget, x, y))
            {
                int orgX, orgY, vx1, vy1, vx2, vy2, dot, det, value;
                float angle;

                orgX = widget->rect.width / 2;
                orgY = widget->rect.height / 2;

                vx1 = 0;
                vy1 = -orgY;
                vx2 = x - orgX;
                vy2 = y - orgY;

                dot = vx1 * vx2 + vy1 *vy2;
                det = vx1 * vy2 - vy1 * vx2;
                angle = atan2f(det, dot) * (float)(180.0f / M_PI);

                if (angle < 0.0f)
                    angle += 360.0f;
                else if (angle > 360.0f)
                    angle -= 360.0f;

                //printf("(%d, %d) (%d, %d) angle=%f\n", vx1, vy1, vx2, vy2, angle);

                if (ev == ITU_EVENT_MOUSEDOWN)
                {
                    wbg->pressedAngle = (int)roundf(angle);
                    wbg->orgValue = wbg->value;
                }
                else
                {
                    value = wbg->orgValue + (int)roundf(angle) - wbg->pressedAngle;

                    //printf("(%d %d %d %d)\n", value, wbg->value, (int)roundf(angle), wbg->pressedAngle);

                    if (value < 0)
                        value += 360;
                    else if (value > 360)
                        value -= 360;

                    ituWheelBackgroundSetValue(wbg, value);

                    ituExecActions((ITUWidget*)wbg, wbg->actions, ITU_EVENT_CHANGED, value);
                    result = widget->dirty = true;
                }
                wbg->pressed = true;
            }
        }
    }
    else if (ev == ITU_EVENT_MOUSEUP)
    {
        if (ituWidgetIsEnabled(widget))
        {
            if (wbg->pressed)
            {
                wbg->pressedAngle = 0;
                wbg->pressed = false;
                widget->dirty = true;

                result |= widget->dirty;
                return result;
            }
        }
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
    }
    result |= ituIconUpdate(widget, ev, arg1, arg2, arg3);

    return widget->visible ? result : false;
}

void ituWheelBackgroundDraw(ITUWidget* widget, ITUSurface* dest, int x, int y, uint8_t alpha)
{
    ITUWheelBackground* wbg = (ITUWheelBackground*) widget;
    ITUIcon* icon = (ITUIcon*) widget;

    x += widget->rect.x;
    y += widget->rect.y;
    alpha = alpha * widget->alpha / 255;

    if (icon->surf)
    {
        float angle = (float)wbg->value;
        int pointerX = icon->surf->width / 2;
        int pointerY = icon->surf->height / 2;

#if (CFG_CHIP_FAMILY == 9850)
        ituRotate(dest, x, y, icon->surf, pointerX, pointerY, angle, 1.0f, 1.0f);
#else
        ituRotate(dest, x + pointerX, y + pointerY, icon->surf, pointerX, pointerY, angle, 1.0f, 1.0f);        
#endif
    }
    ituWidgetDrawImpl(widget, dest, x, y, alpha);
}

void ituWheelBackgroundInit(ITUWheelBackground* wbg)
{
    assert(wbg);

    memset(wbg, 0, sizeof (ITUWheelBackground));

    ituBackgroundInit(&wbg->bg);

    ituWidgetSetType(wbg, ITU_WHEELBACKGROUND);
    ituWidgetSetName(wbg, wheelBackgroundName);
    ituWidgetSetUpdate(wbg, ituWheelBackgroundUpdate);
    ituWidgetSetDraw(wbg, ituWheelBackgroundDraw);
}

void ituWheelBackgroundLoad(ITUWheelBackground* wbg, uint32_t base)
{
    assert(wbg);

    ituBackgroundLoad(&wbg->bg, base);

    ituWidgetSetUpdate(wbg, ituWheelBackgroundUpdate);
    ituWidgetSetDraw(wbg, ituWheelBackgroundDraw);
}

void ituWheelBackgroundSetValue(ITUWheelBackground* wbg, int value)
{
    assert(wbg);

    if (value < 0 || value > 360)
    {
        LOG_WARN "incorrect value: %d\n", value LOG_END
        return;
    }
    wbg->value = value;

    ituWidgetUpdate(wbg, ITU_EVENT_LAYOUT, 0, 0, 0);
    ituExecActions((ITUWidget*)wbg, wbg->actions, ITU_EVENT_CHANGED, value);
    ituWidgetSetDirty(wbg, true);
}
