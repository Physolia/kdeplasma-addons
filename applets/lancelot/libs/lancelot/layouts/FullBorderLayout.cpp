/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "FullBorderLayout.h"
#include <lancelot/lancelot.h>
#include <KDebug>

namespace Lancelot {

class FullBorderLayout::Private {
public:
    FullBorderLayout * q;

    explicit Private(FullBorderLayout * parent = 0)
        : q(parent)
    {
        sizes[FullBorderLayout::LeftBorder] = -1;
        sizes[FullBorderLayout::RightBorder] = -1;
        sizes[FullBorderLayout::TopBorder] = -1;
        sizes[FullBorderLayout::BottomBorder] = -1;
    }

    virtual ~Private()
    {
    }

    QMap< FullBorderLayout::Place, QGraphicsLayoutItem * > itemPositions;
    QMap< FullBorderLayout::Border, qreal > sizes;

    void calculateBorderSizes(qreal & top, qreal & bottom, qreal & left, qreal & right) const
    {
        // top
        if ((top = sizes[FullBorderLayout::TopBorder]) < 0) {
            if (itemPositions[FullBorderLayout::Top]) {
                top = itemPositions[FullBorderLayout::Top]->preferredSize().height();
            } else {
                top = 0;
            }
        }

        // bottom
        if ((bottom = sizes[FullBorderLayout::BottomBorder]) < 0) {
            if (itemPositions[FullBorderLayout::Bottom]) {
                bottom = itemPositions[FullBorderLayout::Bottom]->preferredSize().height();
            } else {
                bottom = 0;
            }
        }

        // left
        if ((left = sizes[FullBorderLayout::LeftBorder]) < 0) {
            if (itemPositions[FullBorderLayout::Left]) {
                left = itemPositions[FullBorderLayout::Left]->preferredSize().width();
            } else {
                left = 0;
            }
        }

        // right
        if ((right = sizes[FullBorderLayout::RightBorder]) < 0) {
            if (itemPositions[FullBorderLayout::Right]) {
                right = itemPositions[FullBorderLayout::Right]->preferredSize().width();
            } else {
                right = 0;
            }
        }
    }
};

FullBorderLayout::FullBorderLayout(QGraphicsLayoutItem * parent) :
    QGraphicsLayout(parent), d(new Private(this))
{
}

FullBorderLayout::~FullBorderLayout()
{
    delete d;
}

void FullBorderLayout::setGeometry(const QRectF & rect)
{
    QGraphicsLayout::setGeometry(rect);

    QRectF effectiveRect = geometry();
    qreal left = 0, top = 0, right = 0, bottom = 0;
    // getContentsMargins(&left, &top, &right, &bottom);
    effectiveRect.adjust(+left, +top, -right, -bottom);

    kDebug() << left << " " << top << " ";
    kDebug() << geometry() << " " << rect << " " << effectiveRect;

    qreal topMargin, bottomMargin, leftMargin, rightMargin;
    d->calculateBorderSizes(topMargin, bottomMargin, leftMargin, rightMargin);

    QRectF itemRect;

    itemRect = effectiveRect;
    itemRect.setSize(QSizeF(leftMargin, topMargin));

    if (d->itemPositions[TopLeft]) {
        d->itemPositions[TopLeft]->setGeometry(itemRect);
    }

    itemRect.setLeft(effectiveRect.left() + leftMargin);
    itemRect.setWidth(effectiveRect.width() - leftMargin - rightMargin);

    if (d->itemPositions[Top]) {
        d->itemPositions[Top]->setGeometry(itemRect);
    }

    itemRect.setWidth(rightMargin);
    itemRect.moveRight(effectiveRect.right());

    if (d->itemPositions[TopRight]) {
        d->itemPositions[TopRight]->setGeometry(itemRect);
    }

    itemRect.setTop(effectiveRect.top() + topMargin);
    itemRect.setHeight(effectiveRect.height() - topMargin - bottomMargin);

    if (d->itemPositions[Right]) {
        d->itemPositions[Right]->setGeometry(itemRect);
    }

    itemRect.setHeight(bottomMargin);
    itemRect.moveBottom(effectiveRect.bottom());

    if (d->itemPositions[BottomRight]) {
        d->itemPositions[BottomRight]->setGeometry(itemRect);
    }

    itemRect.setLeft(effectiveRect.left() + leftMargin);
    itemRect.setWidth(effectiveRect.width() - leftMargin - rightMargin);

    if (d->itemPositions[Bottom]) {
        d->itemPositions[Bottom]->setGeometry(itemRect);
    }

    itemRect.setLeft(effectiveRect.left());
    itemRect.setWidth(leftMargin);

    if (d->itemPositions[BottomLeft]) {
        d->itemPositions[BottomLeft]->setGeometry(itemRect);
    }

    itemRect.setTop(effectiveRect.top() + topMargin);
    itemRect.setHeight(effectiveRect.height() - topMargin - bottomMargin);

    if (d->itemPositions[Left]) {
        d->itemPositions[Left]->setGeometry(itemRect);
    }

    itemRect = effectiveRect;
    itemRect.adjust(
            leftMargin, topMargin,
            - rightMargin, - bottomMargin
            );

    if (d->itemPositions[Center]) {
        d->itemPositions[Center]->setGeometry(itemRect);
    }
}

QSizeF FullBorderLayout::sizeHint(Qt::SizeHint which,
        const QSizeF & constraint) const
{
    Q_UNUSED(which);
    Q_UNUSED(constraint);

    if (which == Qt::MaximumSize) {
        return MAX_WIDGET_SIZE;
    }

    qreal topMargin, bottomMargin, leftMargin, rightMargin;
    d->calculateBorderSizes(topMargin, bottomMargin, leftMargin, rightMargin);

    qreal hintHeight = topMargin + bottomMargin;
    qreal hintWidth  = leftMargin + rightMargin;

    if (d->itemPositions[Center]) {
        hintHeight += d->itemPositions[Center]
            ->preferredSize().height();
        hintWidth += d->itemPositions[Center]
            ->preferredSize().width();
    }

    kDebug() << hintWidth << " " << hintHeight;
    return QSizeF(hintWidth, hintHeight);
}

void FullBorderLayout::addItem(QGraphicsLayoutItem * item)
{
    FullBorderLayout::addItem (item, Center);
}

void FullBorderLayout::addItem(QGraphicsLayoutItem * item, Place position)
{
    d->itemPositions[position] = item;
    updateGeometry();
}

int FullBorderLayout::count() const
{
    int count = 0;
    foreach (QGraphicsLayoutItem * i, d->itemPositions) {
        if (i) {
            ++count;
        }
    }
    return count;
}

QGraphicsLayoutItem * FullBorderLayout::itemAt(int index) const
{
    int count = 0;
    foreach (QGraphicsLayoutItem * i, d->itemPositions) {
        if (i) {
            if (index == count) {
                return i;
            }
            count++;
        }
    }
    return 0;
}

void FullBorderLayout::setSize(qreal size, Border border)
{
    d->sizes[border] = size;
    updateGeometry();
}

void FullBorderLayout::setAutoSize(Border border)
{
    d->sizes[border] = -1;
    updateGeometry();
}

qreal FullBorderLayout::size(Border border)
{
    return d->sizes[border];
}

void FullBorderLayout::removeAt(int index)
{
    QMutableMapIterator < Place, QGraphicsLayoutItem * > i(d->itemPositions);
    int count = 0;
    while (i.hasNext()) {
        i.next();
        if (i.value()) {
            if (index == count++) {
                i.remove();
            }
        }
    }
    updateGeometry();
}

} // namespace Lancelot

