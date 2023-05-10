/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROGRESSLINE_H
#define PROGRESSLINE_H

#include <QLabel>

class ProgressLine : public QLabel
{
    Q_OBJECT
public:
    explicit ProgressLine(QWidget *parent = 0);

    qreal value() const;
    void setValue(const qreal &value);

    qreal max() const;
    void setMax(const qreal &max);

    qreal min() const;
    void setMin(const qreal &min);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    qreal m_value;
    qreal m_max;
    qreal m_min;
};

#endif // PROGRESSLINE_H