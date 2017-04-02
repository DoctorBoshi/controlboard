#include <assert.h>
#include <math.h>
#include "ite/itu.h"
#include "itu_cfg.h"
#include "itu_private.h"

static const char scaleCoverFlowName[] = "ITUScaleCoverFlow";

extern int CoverFlowGetVisibleChildCount(ITUCoverFlow* coverflow);
extern ITUWidget* CoverFlowGetVisibleChild(ITUCoverFlow* coverflow, int index);

bool ituScaleCoverFlowUpdate(ITUWidget* widget, ITUEvent ev, int arg1, int arg2, int arg3)
{
    bool result = false;
    ITUCoverFlow* coverflow = (ITUCoverFlow*) widget;
    ITUScaleCoverFlow* scalecoverflow = (ITUScaleCoverFlow*) widget;
    assert(scalecoverflow);

    if (ev == ITU_EVENT_TIMER)
    {
        if (coverflow->inc)
        {
            int i, count = CoverFlowGetVisibleChildCount(coverflow);

            for (i = 0; i < count; ++i)
            {
                ITUWidget* child = CoverFlowGetVisibleChild(coverflow, i);

                if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
                    ituWidgetSetX(child, scalecoverflow->itemPos);
                else
                    ituWidgetSetY(child, scalecoverflow->itemPos);

                ituWidgetSetDimension(child, scalecoverflow->itemWidth, scalecoverflow->itemHeight);
            }
        }
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
        if (scalecoverflow->itemWidth > 0 || scalecoverflow->itemHeight > 0 || scalecoverflow->itemPos > 0)
        {
            int i, count = CoverFlowGetVisibleChildCount(coverflow);

            for (i = 0; i < count; ++i)
            {
                ITUWidget* child = CoverFlowGetVisibleChild(coverflow, i);

                if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
                    ituWidgetSetX(child, scalecoverflow->itemPos);
                else
                    ituWidgetSetY(child, scalecoverflow->itemPos);

                ituWidgetSetDimension(child, scalecoverflow->itemWidth, scalecoverflow->itemHeight);
            }            
        }
    }

    result |= ituCoverFlowUpdate(widget, ev, arg1, arg2, arg3);

    if (ev == ITU_EVENT_TIMER)
    {
        if (coverflow->inc)
        {
            int i, count = CoverFlowGetVisibleChildCount(coverflow);

            if (coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
            {
                int index, count2, orgWidth, orgHeight, factor, width, height, x, y;
                int frame = coverflow->frame - 1;

                count2 = count / 2 + 1;
                index = coverflow->focusIndex;

                orgWidth = scalecoverflow->itemWidth;
                orgHeight = scalecoverflow->itemHeight;

                for (i = 0; i < count2; ++i)
                {
                    ITUWidget* child = CoverFlowGetVisibleChild(coverflow, index);

                    if (index == coverflow->focusIndex)
                    {
                        int current_factor = 100;
                        int next_factor = scalecoverflow->factor + (count2 - 1) * (100 - scalecoverflow->factor) / count2;
                        factor = current_factor - (current_factor - next_factor) * frame / coverflow->totalframe;
                    }
                    else
                    {
                        if (coverflow->inc > 0)
                        {
                            int current_factor = scalecoverflow->factor + (count2 - i) * (100 - scalecoverflow->factor) / count2;
                            int next_factor = scalecoverflow->factor + (count2 - (i + 1)) * (100 - scalecoverflow->factor) / count2;
                            factor = current_factor - (current_factor - next_factor) * frame / coverflow->totalframe;
                        }
                        else
                        {
                            int current_factor = scalecoverflow->factor + (count2 - i) * (100 - scalecoverflow->factor) / count2;
                            int next_factor = scalecoverflow->factor + (count2 - (i - 1)) * (100 - scalecoverflow->factor) / count2;
                            factor = current_factor + (next_factor - current_factor) * frame / coverflow->totalframe;
                        }
                    }
                    width = orgWidth * factor / 100;
                    height = orgHeight * factor / 100;
                    x = child->rect.x + (orgWidth - width) / 2;
                    y = child->rect.y + (orgHeight - height) / 2;
                    ituWidgetSetPosition(child, x, y);
                    ituWidgetSetDimension(child, width, height);

                    if (index >= count - 1)
                        index = 0;
                    else
                        index++;
                }

                count2 = count - count2;
                for (i = 0; i < count2; ++i)
                {
                    ITUWidget* child = CoverFlowGetVisibleChild(coverflow, index);
                    int width, height, x, y;

                    if (coverflow->inc > 0)
                    {
                        int current_factor = scalecoverflow->factor + i * (100 - scalecoverflow->factor) / count2;
                        int next_factor = i < count2 ? scalecoverflow->factor + (i + 1) * (100 - scalecoverflow->factor) / count2 : 100;
                        factor = current_factor + (next_factor - current_factor) * frame / coverflow->totalframe;
                    }
                    else
                    {
                        int current_factor = scalecoverflow->factor + i * (100 - scalecoverflow->factor) / count2;
                        int next_factor = i > 0 ? scalecoverflow->factor + (i - 1) * (100 - scalecoverflow->factor) / count2 : scalecoverflow->factor;
                        factor = current_factor - (current_factor - next_factor) * frame / coverflow->totalframe;
                    }

                    width = orgWidth * factor / 100;
                    height = orgHeight * factor / 100;
                    x = child->rect.x + (orgWidth - width) / 2;
                    y = child->rect.y + (orgHeight - height) / 2;
                    ituWidgetSetPosition(child, x, y);
                    ituWidgetSetDimension(child, width, height);

                    if (index >= count - 1)
                        index = 0;
                    else
                        index++;
                }
            }
        }
    }
    else if (ev == ITU_EVENT_LAYOUT)
    {
        int i, orgWidth, orgHeight, count = CoverFlowGetVisibleChildCount(coverflow);

        if (scalecoverflow->itemWidth == 0 && scalecoverflow->itemHeight == 0 && scalecoverflow->itemPos == 0)
        {
            ITUWidget* child = CoverFlowGetVisibleChild(coverflow, coverflow->focusIndex);
            scalecoverflow->itemWidth = child->rect.width;
            scalecoverflow->itemHeight = child->rect.height;

            if (coverflow->coverFlowFlags & ITU_COVERFLOW_VERTICAL)
                scalecoverflow->itemPos = child->rect.x;
            else
                scalecoverflow->itemPos = child->rect.y;
        }
        orgWidth = scalecoverflow->itemWidth;
        orgHeight = scalecoverflow->itemHeight;

        if (coverflow->coverFlowFlags & ITU_COVERFLOW_CYCLE)
        {
            int index, count2, factor, width, height, x, y;
            
            count2 = count / 2 + 1;
            index = coverflow->focusIndex;

            for (i = 0; i < count2; ++i)
            {
                ITUWidget* child = CoverFlowGetVisibleChild(coverflow, index);

                factor = scalecoverflow->factor + (count2 - 1 - i) * (100 - scalecoverflow->factor) / (count2 - 1);
                width = orgWidth * factor / 100;
                height = orgHeight * factor / 100;
                x = child->rect.x + (orgWidth - width) / 2;
                y = child->rect.y + (orgHeight - height) / 2;
                ituWidgetSetPosition(child, x, y);
                ituWidgetSetDimension(child, width, height);

                if (index >= count - 1)
                    index = 0;
                else
                    index++;
            }

            count2 = count - count2;
            for (i = 0; i < count2; ++i)
            {
                ITUWidget* child = CoverFlowGetVisibleChild(coverflow, index);

                factor = scalecoverflow->factor + i * (100 - scalecoverflow->factor) / count2;
                width = orgWidth * factor / 100;
                height = orgHeight * factor / 100;
                x = child->rect.x + (orgWidth - width) / 2;
                y = child->rect.y + (orgHeight - height) / 2;
                ituWidgetSetPosition(child, x, y);
                ituWidgetSetDimension(child, width, height);

                if (index >= count - 1)
                    index = 0;
                else
                    index++;
            }
        }
    }

    return widget->visible ? result : false;
}

void ituScaleCoverFlowInit(ITUScaleCoverFlow* scaleCoverFlow, ITULayout layout)
{
    assert(scaleCoverFlow);

    memset(scaleCoverFlow, 0, sizeof (ITUScaleCoverFlow));

    ituCoverFlowInit(&scaleCoverFlow->coverFlow, layout);

    ituWidgetSetType(scaleCoverFlow, ITU_IMAGECOVERFLOW);
    ituWidgetSetName(scaleCoverFlow, scaleCoverFlowName);
    ituWidgetSetUpdate(scaleCoverFlow, ituScaleCoverFlowUpdate);
}

void ituScaleCoverFlowLoad(ITUScaleCoverFlow* scaleCoverFlow, uint32_t base)
{
    assert(scaleCoverFlow);

    ituCoverFlowLoad(&scaleCoverFlow->coverFlow, base);
    ituWidgetSetUpdate(scaleCoverFlow, ituScaleCoverFlowUpdate);
}
