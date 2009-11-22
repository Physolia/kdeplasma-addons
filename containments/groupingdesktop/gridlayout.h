/*
 *   Copyright 2009 by Giulio Camuffo <giuliocamuffo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef GRIDLAYOUT_H
#define GRIDLAYOUT_H

#include "abstractgroup.h"

class QGraphicsGridLayout;

class Position;
class Spacer;

namespace Plasma
{
    class Applet;
}

class GridLayout : public AbstractGroup
{
    Q_OBJECT
    public:
        GridLayout(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
        ~GridLayout();

        void saveAppletLayoutInfo(Plasma::Applet* applet, KConfigGroup group) const;
        void restoreAppletLayoutInfo(Plasma::Applet* applet, const KConfigGroup& group);
        void layoutApplet(Plasma::Applet* applet);
        QString pluginName() const;

    protected:
        void dragMoveEvent(QGraphicsSceneDragDropEvent *event);

    private slots:
        void onAppletRemoved(Plasma::Applet *applet);

    private:
        enum Orientation {
            Horizontal = 0,
            Vertical = 1
        };

        QGraphicsLayoutItem *removeItemAt(int row, int column, bool fillLayout = true);
        QGraphicsLayoutItem *removeItemAt(Position position, bool fillLayout = true);
        void removeItem(QGraphicsWidget *item, bool fillLayout = true);
        void showItemTo(QGraphicsWidget *movingWidget, const QPointF &pos);
        void insertItemAt(QGraphicsWidget *item, int row, int column, Orientation orientation);
        Position itemPosition(QGraphicsItem *item) const;
        int nearestBoundair(qreal pos, qreal size) const;

        QGraphicsGridLayout *m_layout;
        Spacer *m_spacer;

        friend class Spacer;
};

#endif // GRIDLAYOUT_H
